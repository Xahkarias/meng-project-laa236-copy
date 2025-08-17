#!/bin/bash

# Default number of lines to execute
NUM_LINES=${1:-1}  # Default to 1 if no argument is provided

# Define the list of TCL scripts
TCL_SCRIPTS=(
    "../tcl_scripts/SW_loop/test_SW_7.tcl"
    "../tcl_scripts/SW_loop/test_SW_10.tcl"
    "../tcl_scripts/SW_loop/test_SW_20.tcl"
    "../tcl_scripts/SW_loop/test_SW_30.tcl"
    "../tcl_scripts/SW_loop/test_SW_40.tcl"
    "../tcl_scripts/SW_loop/test_SW_50.tcl"
    "../tcl_scripts/SW_loop/test_SW_60.tcl"
    "../tcl_scripts/SW_loop/test_SW_80.tcl"
    "../tcl_scripts/SW_loop/test_SW_120.tcl"
)

# Execute the first X lines
for (( i=0; i<$NUM_LINES; i++ )); do
    vitis_hls -f "${TCL_SCRIPTS[$i]}"
done