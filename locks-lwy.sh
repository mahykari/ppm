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
# The sets represent ndoors and wordlen respectively.
for comb in {100,300,500,1000,3000,10000}; do
  IFS='_' read -r nlocks <<< "$comb"
  printf "I: parameters nlocks=%5d\n" $nlocks

  for s in {768,1024,1536,2048,3072}; do
    printf "I:   security=%4d\n" $s
    # In case we want to skip this iteration,
    # both log files should exist.
    if [ -f "./logs/monitor_$nlocks-$s.log" ] && \
      [ -f "./logs/system_$nlocks-$s.log" ]
    then
      printf "I:  skipping iteration; logs already exist...\n"
      continue
    fi
    # Preparing the synthesis script with the new parameters.
    sed "s/SPEC_FILE/-D NLOCKS=$nlocks SPEC_FILE/g" \
    synth.ys.bak > synth.ys

    mslen=$(( nlocks ))
    sslen=$(( 2 * nlocks ))
    # Calling Monitor and System with the current combination
    ./Monitor -proto lwy -security $s -mslen $mslen -sslen $sslen \
      -spec locks.v -nlocks $nlocks \
      > ./logs/monitor_$nlocks-$s.log 2>&1 &
    mpid=$!
    ./System -proto lwy -security $s -mslen $mslen -sslen $sslen \
      -sys locks -nlocks $nlocks \
      > ./logs/system_$nlocks-$s.log 2>&1 &
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
  rm -f synth.blif
done

# Restoring the original synthesis script
mv synth.ys.bak synth.ys
