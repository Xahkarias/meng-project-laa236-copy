# Set the project name and top-level function
set project_name "SW_systold_4"
set top_function "SW_basic_linear"

# Create a new project
open_project $project_name

#set_property -name "CONFIG.CFLAGS" -value "-std=c++11" -objects [get_files smith_waterman_basic_linear.cpp]

# Set the solution name
set solution_name "SW_systold_4"
open_solution $solution_name

# Set the target FPGA device (modify as per your board)
set_part xcu250-figd2104-2L-e

# Define clock period (modify as needed)
create_clock -period 10

# Add source files
add_files ../src_systold/systold_kernel.cpp
add_files ../defines.hpp
add_files ../sw_algo.hpp
add_files ../testbench/cosim_tb_hardcode.cpp -tb

# Set the top function
set_top $top_function

# Run C simulation (optional, for verification)
#csim_design

# Run HLS synthesis
#csynth_design

# Run co-simulation to verify synthesized RTL
cosim_design -rtl verilog -O

# Export the RTL as an IP core
#export_design -flow syn -format xo -rtl verilog -output ./smith_waterman_basic_linear.xo

# Close the project
close_project
exit

# run using `vitis_hls -f ../test_POA_basic.tcl`