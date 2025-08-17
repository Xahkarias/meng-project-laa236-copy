# Smith Waterman Linear FPGA Implementation
For workflow purposes

# csim / csynth / cosim:
1. `mkdir build`
2. `cd build` 
2. `vitis_hls -f ../test_smith_waterman.tcl`

# Software Emulation:
Note: steps 1-4 can be ignored if already done for HW_EMU
1. `mkdir build`
2. `cd build` 
3. Build the hostfile with 
```
g++ -o smith_waterman_basic_linear.exe ../smith_waterman_basic_linear_host.cpp -I/opt/xilinx/xrt/include -L/opt/xilinx/xrt/lib -lxrt_coreutil -std=c++17
```
4. Setup the device config with
```
emconfigutil --platform xilinx_u250_gen3x16_xdma_4_1_202210_1
```
5. Build the sw emulation object (xo) file with
```
v++ -c \
    --target sw_emu \
    -k smith_waterman_basic_linear \
    -f /opt/xilinx/platforms/xilinx_u250_gen3x16_xdma_4_1_202210_1/xilinx_u250_gen3x16_xdma_4_1_202210_1.xpfm \
    -o smith_waterman_basic_linear.xo \
    -I .. \
    ../smith_waterman_basic_linear.cpp
```
6. Link the .xo into a .xclbin
```
v++ -l \
    --target sw_emu \
    --platform /opt/xilinx/platforms/xilinx_u250_gen3x16_xdma_4_1_202210_1/xilinx_u250_gen3x16_xdma_4_1_202210_1.xpfm \
    -o smith_waterman_basic_linear.xclbin \
    smith_waterman_basic_linear.xo
```
7. Set the simulation mode argument
```
export XCL_EMULATION_MODE=sw_emu
```
8. Run the executable with no arguments.

# Hardware Emulation:
Note: steps 1-4 can be ignored if already done for SW_EMU
1. `mkdir build`
2. `cd build` 
3. Build the hostfile with 
```
g++ -o smith_waterman_basic_linear.exe ../smith_waterman_basic_linear_host.cpp -I/opt/xilinx/xrt/include -L/opt/xilinx/xrt/lib -lxrt_coreutil -std=c++17
```
4. Setup the device config with
```
emconfigutil --platform xilinx_u250_gen3x16_xdma_4_1_202210_1
```
5. Build the hw emulation object (xo) file with
```
v++ -c \
    --target hw_emu \
    -k smith_waterman_basic_linear \
    -f /opt/xilinx/platforms/xilinx_u250_gen3x16_xdma_4_1_202210_1/xilinx_u250_gen3x16_xdma_4_1_202210_1.xpfm \
    -o smith_waterman_basic_linear.xo \
    -I /home/laa236/zz_meng_proj/POA_HLS/basic_linear \
    ../smith_waterman_basic_linear.cpp
```
6. Link the .xo into a .xclbin
```
v++ -l \
    --target hw_emu \
    --platform /opt/xilinx/platforms/xilinx_u250_gen3x16_xdma_4_1_202210_1/xilinx_u250_gen3x16_xdma_4_1_202210_1.xpfm \
    -o smith_waterman_basic_linear.xclbin \
    smith_waterman_basic_linear.xo
```
7. Set the simulation mode argument 
```
export XCL_EMULATION_MODE=hw_emu
```
8. Run the executable with no arguments.

# Pushing to the FPGA:
Note: steps 1-2 can be ignored if already done for the emulation steps
1. `mkdir build`
2. `cd build` 
### I ***HIGHLY*** recommend nuking the build folder before the FPGA
3. NUKE THE BUILD: BE CAREFUL WITH THIS COMMAND `rm -rf ./* ./.*`
4. Build the hostfile with 
```
g++ -o smith_waterman_basic_linear.exe ../smith_waterman_basic_linear_host.cpp -I/opt/xilinx/xrt/include -L/opt/xilinx/xrt/lib -lxrt_coreutil -std=c++17
```
5. Setup the device config with
```
emconfigutil --platform /opt/xilinx/platforms/xilinx_u250_gen3x16_xdma_4_1_202210_1/xilinx_u250_gen3x16_xdma_4_1_202210_1.xpfm --od .
```
6. Triple check that the emulation mode is unset
```
unset XCL_EMULATION_MODE
```
7. Build the new .xo file
```
v++ -c \
    --target hw \
    -k smith_waterman_basic_linear \
    -f /opt/xilinx/platforms/xilinx_u250_gen3x16_xdma_4_1_202210_1/xilinx_u250_gen3x16_xdma_4_1_202210_1.xpfm \
    -o smith_waterman_basic_linear.xo \
    -I /home/laa236/zz_meng_proj/POA_HLS/basic_linear \
    ../smith_waterman_basic_linear.cpp
```
8. Set the FPGA settings
```
source /opt/xilinx/Vitis/2023.2/settings64.sh
```
### This will take 2 hours, you ***MUST*** use *tmux* or `nohup` to avoid the compile stopping due to the terminal closing
9. Generate the final bitstream
```
/opt/xilinx/Vitis/2023.2/bin/v++ \
    --vivado.prop "run.__KERNEL__.{STEPS.SYNTH_DESIGN.ARGS.MORE OPTIONS}={-directive sdx_optimization_effort_high}" \
    --vivado.prop "run.impl_1.{STEPS.PLACE_DESIGN.ARGS.MORE OPTIONS}={-retiming}" \
    --vivado.prop run.impl_1.STEPS.PHYS_OPT_DESIGN.IS_ENABLED=true \
    --vivado.prop run.impl_1.STEPS.POST_ROUTE_PHYS_OPT_DESIGN.IS_ENABLED=true \
    --advanced.misc "report=type report_timing_summary name impl_report_timing_summary_route_design_summary \
                    steps {route_design} runs {impl_1} options {-max_paths 10}" \
    --advanced.misc "report=type report_timing_summary name impl_report_timing_summary_post_route_phys_opt_design_summary \
                    steps {post_route_phys_opt_design} runs {impl_1} options {-max_paths 10}" \
    -l \
    --save-temps \
    -t hw \
    --platform /opt/xilinx/platforms/xilinx_u250_gen3x16_xdma_4_1_202210_1/xilinx_u250_gen3x16_xdma_4_1_202210_1.xpfm \
    --temp_dir ./_x.hw.xilinx_u250_gen3x16_xdma_4_1_202210_1 \
    --optimize 3 \
    --kernel_frequency 200 \
    -o ./build_dir.hw.xilinx_u250_gen3x16_xdma_4_1_202210_1/smith_waterman_basic_linear.link.xclbin \
    smith_waterman_basic_linear.xo
```
10. Run the executable with an argument of 1