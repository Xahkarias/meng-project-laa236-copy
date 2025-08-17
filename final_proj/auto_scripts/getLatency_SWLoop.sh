#!/bin/bash

SCRIPT_DIR="$(dirname "$0")"

# Define relative paths based on the script's directory
files=(
  "$SCRIPT_DIR/../build/SW_loop_7/SW_loop/sim/report/SW_basic_linear_cosim.rpt"
  "$SCRIPT_DIR/../build/SW_loop_10/SW_loop/sim/report/SW_basic_linear_cosim.rpt"
  "$SCRIPT_DIR/../build/SW_loop_20/SW_loop/sim/report/SW_basic_linear_cosim.rpt"
  "$SCRIPT_DIR/../build/SW_loop_30/SW_loop/sim/report/SW_basic_linear_cosim.rpt"
  "$SCRIPT_DIR/../build/SW_loop_40/SW_loop/sim/report/SW_basic_linear_cosim.rpt"
  "$SCRIPT_DIR/../build/SW_loop_50/SW_loop/sim/report/SW_basic_linear_cosim.rpt"
  "$SCRIPT_DIR/../build/SW_loop_60/SW_loop/sim/report/SW_basic_linear_cosim.rpt"
  "$SCRIPT_DIR/../build/SW_loop_80/SW_loop/sim/report/SW_basic_linear_cosim.rpt"
  "$SCRIPT_DIR/../build/SW_loop_120/SW_loop/sim/report/SW_basic_linear_cosim.rpt"
)

# Header
echo "POA Loop LinearD                    | Total Execution Time"
echo "------------------------------------|----------------------"

# Loop through each file
for file in "${files[@]}"; do
  if [ -f "$file" ]; then
    # Extract the SW_loop_X section
    poa_section=$(echo "$file" | sed -E 's|.*/(SW_loop_[0-9]+).*|\1|')
    
    # Extract the execution time
    exec_time=$(grep 'Verilog' "$file" | awk -F'|' '{gsub(/^[ \t]+|[ \t]+$/, "", $10); print $10}')
    
    # Print the extracted section and execution time
    printf "%-35s | %s\n" "$poa_section" "$exec_time"
  else
    # If the file doesn't exist, print 'File not found'
    poa_section=$(echo "$file" | sed -E 's|.*/(SW_loop_[0-9]+).*|\1|')
    printf "%-35s | %s\n" "$poa_section" "File not found"
  fi
done
