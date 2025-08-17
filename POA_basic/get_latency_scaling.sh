#!/bin/bash

# List of static file paths
files=(
  "/home/laa236/meng-project-laa236/POA_basic/build/project_POA_sw_7/solution_sw_7/sim/report/POA_basic_linear_cosim.rpt"
  "/home/laa236/meng-project-laa236/POA_basic/build/project_POA_sw_10/solution_sw_10/sim/report/POA_basic_linear_cosim.rpt"
  "/home/laa236/meng-project-laa236/POA_basic/build/project_POA_sw_20/solution_sw_20/sim/report/POA_basic_linear_cosim.rpt"
  "/home/laa236/meng-project-laa236/POA_basic/build/project_POA_sw_30/solution_sw_30/sim/report/POA_basic_linear_cosim.rpt"
  "/home/laa236/meng-project-laa236/POA_basic/build/project_POA_sw_40/solution_sw_40/sim/report/POA_basic_linear_cosim.rpt"
  "/home/laa236/meng-project-laa236/POA_basic/build/project_POA_sw_50/solution_sw_50/sim/report/POA_basic_linear_cosim.rpt"
  "/home/laa236/meng-project-laa236/POA_basic/build/project_POA_sw_60/solution_sw_60/sim/report/POA_basic_linear_cosim.rpt"
  "/home/laa236/meng-project-laa236/POA_basic/build/project_POA_sw_80/solution_sw_80/sim/report/POA_basic_linear_cosim.rpt"
  "/home/laa236/meng-project-laa236/POA_basic/build/project_POA_sw_120/solution_sw_120/sim/report/POA_basic_linear_cosim.rpt"
)

# Header
echo "File                                | Total Execution Time"
echo "------------------------------------|----------------------"

# Loop through each file
for file in "${files[@]}"; do
  if [ -f "$file" ]; then
    # Extract the solution name (e.g., solution_sw_10) from the file path using 'awk' or 'sed'
    #solution=$(echo "$file" | sed 's/\(solution_[^/]*\).*/\1/')
    solution=$(echo "$file" | sed -E 's|.*/(solution_[^/]+).*|\1|')
    
    # Extract the execution time
    exec_time=$(grep 'Verilog' "$file" | awk -F'|' '{gsub(/^[ \t]+|[ \t]+$/, "", $10); print $10}')
    
    # Print the solution name and execution time
    printf "%-35s | %s\n" "$solution" "$exec_time"
  else
    # If the file doesn't exist, print the solution name as 'not found'
    solution=$(basename "$file" | sed 's/\(solution_[^/]*\).*/\1/')
    printf "%-35s | %s\n" "$solution" "File not found"
  fi
done