#!/bin/bash
source "$HOME/oss-cad-suite/environment"

if [ -f ./synth.blif ]; then
  echo "skipping synthesis, BLIF file already exists."
fi

if [ "$#" -ne 1 ]; then
  echo "Usage: $0 verilog_file" >&2
  exit 1
fi

# Create new script with substitution
cp synth.ys temp.ys
sed -i "s|SPEC_FILE|$1|" temp.ys

# Run yosys
yosys temp.ys > ./yosys.log 2>&1
