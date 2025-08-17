# Set the project name and top-level function
set project_name "SW_syst_4"
set top_function "SW_basic_linear"

# Create a new project
open_project $project_name

#set_property -name "CONFIG.CFLAGS" -value "-std=c++11" -objects [get_files smith_waterman_basic_linear.cpp]

# Set the solution name
set solution_name "SW_syst_4"
open_solution $solution_name

# Set the target FPGA device (modify as per your board)
set_part xcu250-figd2104-2L-e

# Define clock period (modify as needed)
create_clock -period 10

# Add source files
add_files ../src_syst/syst_kernel.cpp
add_files ../defines.hpp
add_files ../sw_algo.hpp
add_files ../testbench/csim_tb.cpp -tb

# Set the top function
set_top $top_function


# Run C simulation
# UPDATE THIS WITH THE DATASET AMOUNT
csim_design -argv "-f ../../../../../datasets/mimic_vitis_gui.txt -t 1"


# Close the project
close_project
exit
