#include <iostream>
#include <vector>
#include <algorithm>  // for std::fill
#include <iomanip>    // for std::setw
#include <cstring>  // For memcpy
#include "xrt/xrt_device.h"
#include "xrt/xrt_kernel.h"
#include "xrt/xrt_bo.h"

// Match the same dimension macros that appear in your HLS code
#define MAX_SIZE 32

int main(int argc, char* argv[]) {

    xrt::device myDevice; 
    xrt::xclbin xclbin;
    if (argc > 1) {
        std::cout << "Running in FPGA mode: " << std::endl;
        myDevice = xrt::device("0000:88:00.1");
        xclbin = xrt::xclbin("build_dir.hw.xilinx_u250_gen3x16_xdma_4_1_202210_1/smith_waterman_basic_linear.link.xclbin");
    } else {
        std::cout << "Running in emu mode" << std::endl;
        myDevice = xrt::device(0);
        xclbin = xrt::xclbin("smith_waterman_basic_linear.xclbin");
    }

    auto uuid = myDevice.load_xclbin(xclbin);

    xrt::kernel smith_waterman_basic_linear(myDevice, uuid, "smith_waterman_basic_linear");

    std::cout << "Step 1 Complete " << std::endl;

    //making vector vars
    std::vector<char> seq1_data(MAX_SIZE, 0);
    std::vector<char> seq2_data(MAX_SIZE, 0);
    std::vector<char> align1_data(MAX_SIZE);
    std::vector<char> align2_data(MAX_SIZE);
    std::vector<int> seqsize = {7, 7};

    std::cout << "Step 2 Complete " << std::endl;

    //putting the data in
    std::string seq1 = "AGCTGAC";
    std::string seq2 = "GCTAGAC";

    std::memcpy(seq1_data.data(), seq1.c_str(), seq1.size());
    std::memcpy(seq2_data.data(), seq2.c_str(), seq2.size());

    std::cout << "Step 3 Complete " << std::endl;
    //buffers
    auto seq1_bo = xrt::bo(myDevice, sizeof(char) * MAX_SIZE, smith_waterman_basic_linear.group_id(0));
    auto seq2_bo = xrt::bo(myDevice, sizeof(char) * MAX_SIZE, smith_waterman_basic_linear.group_id(1));
    auto seqsz_bo = xrt::bo(myDevice, sizeof(int) * 2, smith_waterman_basic_linear.group_id(2));
    auto align1_bo = xrt::bo(myDevice, sizeof(char) * MAX_SIZE, smith_waterman_basic_linear.group_id(3));
    auto align2_bo = xrt::bo(myDevice, sizeof(char) * MAX_SIZE, smith_waterman_basic_linear.group_id(4));

    std::cout << "Step 4 Complete " << std::endl;
    // Copy host data into device buffers
    seq1_bo.write(seq1_data.data());
    seq2_bo.write(seq2_data.data());
    seqsz_bo.write(seqsize.data());

    seq1_bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    seq2_bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    seqsz_bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    std::cout << "Step 5 Complete " << std::endl;
    // Run the kernel
    auto run = smith_waterman_basic_linear(seq1_bo, seq2_bo, seqsz_bo, align1_bo, align2_bo);
    run.wait(); // block until kernel is finished

    std::cout << "Step 6 Complete " << std::endl;
    // Read back the output
    align1_bo.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    align2_bo.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    align1_bo.read(align1_data.data());
    align2_bo.read(align2_data.data());

    // Print results
    std::cout << "Sequence 1: " << seq1 << std::endl;
    std::cout << "Sequence 2: " << seq2 << std::endl;
    std::cout << "Aligned 1 : " << std::string(align1_data.begin(), align1_data.end()) << std::endl;
    std::cout << "Aligned 2 : " << std::string(align2_data.begin(), align2_data.end()) << std::endl;

    return 0;
}