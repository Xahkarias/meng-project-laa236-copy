# alias_setup.sh
alias mode_sw_emu='export XCL_EMULATION_MODE=sw_emu'
alias mode_hw_emu='export XCL_EMULATION_MODE=hw_emu'
alias mode_no_emu='unset XCL_EMULATION_MODE'

# export PATH=/usr/local/cuda/bin:$PATH
export PATH=/usr/local/cuda/bin:$PATH
source /opt/xilinx/Vitis/2023.2/settings64.sh
source source /work/shared/common/allo/vitis_2023.2_u250.sh