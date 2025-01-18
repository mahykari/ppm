#!/bin/bash
# This script will execute the protocol
# with the timekeeper spec and system
# for multiple combinations of parameters.

cleanup() {
  # Killing the processes if they are still running
  kill $mpid $spid
  # Restoring the original synthesis script
  mv synth.ys.bak synth.ys
  # Removing the BLIF file to force synthesis on next iteration
  rm synth.blif
}

handle_sigint() {
  echo "E: script interrupted; stopping..."
  cleanup
  exit 1
}

trap handle_sigint SIGINT

# Ensure everything is built.
make -j4

# Backing up the synthesis script
# as it will be modified for each combination.
cp synth.ys synth.ys.bak
# New directory for the logs.
mkdir -p logs

# Run the protocol with the timekeeper spec and system.
# The sets represent n_in (number of internal doors) and wordlen respectively.
N_EX=30
for comb in {10,30,100}_{8,16,32}; do
  IFS='_' read -r n_in w <<< "$comb"
  printf "I: parameters n_ex=%2d, n_in=%3d, wordlen=%2d\n" $N_EX $n_in $w

  for s in {128,256,768,1024,1536,2048,3072}; do
    printf "I:   security=%4d\n" $s
    # In case we want to skip this iteration,
    # both log files should exist.
    if [ -f "./logs/monitor_$N_EX-$n_in-$w-$s.log" ] && \
      [ -f "./logs/system_$N_EX-$n_in-$w-$s.log" ]
    then
      printf "I:  skipping iteration; logs already exist...\n"
      continue
    fi
    # Preparing the synthesis script with the new parameters.
    sed "s/SPEC_FILE/-D N_EX=$N_EX -D N_IN=$n_in -D WORDLEN=$w SPEC_FILE/g" \
    synth.ys.bak > synth.ys

    mslen=$((w * 2))
    sslen=$(( 4 * w * (N_EX + n_in) ))
    # Calling Monitor and System with the current combination
    ./Monitor -proto lwy -security $s -mslen $mslen -sslen $sslen \
      -spec timekeeper+.v -wordlen $w -nex $N_EX -nin $n_in \
      > ./logs/monitor_$N_EX-$n_in-$w-$s.log 2>&1 &
    mpid=$!
    ./System -proto lwy -security $s -mslen $mslen -sslen $sslen \
      -sys timekeeper+ -wordlen $w -nex $N_EX -nin $n_in \
      > ./logs/system_$N_EX-$n_in-$w-$s.log 2>&1 &
    spid=$!

    # Waiting for the processes to finish
    wait $mpid $spid
    if [ $? -ne 0 ]; then
      printf "E: at least one process exited with failure; "
      printf "stopping the script...\n"
      cleanup
      exit 1
    fi
  done

  # Removing the BLIF file to force synthesis on next iteration
  rm synth.blif
done

# Restoring the original synthesis script
mv synth.ys.bak synth.ys
