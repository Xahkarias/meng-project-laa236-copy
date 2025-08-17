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
add_files ../testbench/csim_tb_boundary.cpp -tb

# Set the top function
set_top $top_function


# Run C simulation
# UPDATE THIS WITH THE DATASET AMOUNT
for {set i 0} {$i < 16} {incr i} {
    if {$i == 0} {
        csim_design -argv "$i" -clean
    } else {
        csim_design -argv "$i"
    }
}
for {set i 0} {$i < 16} {incr i} {
    if {$i == 0} {
        csim_design -argv "-f ../../../../../datasets/internally_align_test_cases.txt $i"
    } else {
        csim_design -argv "-f ../../../../../datasets/internally_align_test_cases.txt $i"
    }
}
for {set i 0} {$i < 8} {incr i} {
    if {$i == 0} {
        csim_design -argv "-f ../../../../../datasets/long_test_cases.txt $i"
    } else {
        csim_design -argv "-f ../../../../../datasets/long_test_cases.txt $i"
    }
}
for {set i 0} {$i < 9} {incr i} {
    if {$i == 0} {
        csim_design -argv "-f ../../../../../datasets/no_align_test_cases.txt $i"
    } else {
        csim_design -argv "-f ../../../../../datasets/no_align_test_cases.txt $i"
    }
}
for {set i 0} {$i < 11} {incr i} {
    if {$i == 0} {
        csim_design -argv "-f ../../../../../datasets/perfect_align_test_cases.txt $i"
    } else {
        csim_design -argv "-f ../../../../../datasets/perfect_align_test_cases.txt $i"
    }
}

# Close the project
close_project
exit
