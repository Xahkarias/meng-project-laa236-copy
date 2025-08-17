# Set the project name and top-level function
set project_name "project_POA_sw_40"
set top_function "POA_basic_linear"

# Create a new project
open_project $project_name

#set_property -name "CONFIG.CFLAGS" -value "-std=c++11" -objects [get_files smith_waterman_basic_linear.cpp]

# Set the solution name
set solution_name "solution_sw_40"
open_solution $solution_name

# Set the target FPGA device (modify as per your board)
set_part xcu250-figd2104-2L-e

# Define clock period (modify as needed)
create_clock -period 10

# Add source files
add_files ../POA_basic_linear.cpp
add_files ../POA_basic_linear.hpp
add_files ../POA_basic_linear_tb_sw_40.cpp -tb

# Set the top function
set_top $top_function

# Set the interface pragmas
# Memory-mapped AXI interfaces for input/output sequences
# set_directive_interface -mode m_axi -depth 256 smithWaterman seq1
# set_directive_interface -mode m_axi -depth 256 smithWaterman seq2
# set_directive_interface -mode m_axi -depth 256 smithWaterman alignedSeq1
# set_directive_interface -mode m_axi -depth 256 smithWaterman alignedSeq2

# AXI-Lite interface for scalar arguments (size1, size2, and return control)
# set_directive_interface -mode s_axilite smithWaterman size1
# set_directive_interface -mode s_axilite smithWaterman size2
# set_directive_interface -mode s_axilite smithWaterman return

# Run C simulation (optional, for verification)
csim_design

# Run HLS synthesis
csynth_design

# Run co-simulation to verify synthesized RTL
cosim_design -rtl verilog

# Export the RTL as an IP core
#export_design -flow syn -format xo -rtl verilog -output ./smith_waterman_basic_linear.xo

# Close the project
close_project
exit
# run using `vitis_hls -f ../test_POA_basic.tcl`