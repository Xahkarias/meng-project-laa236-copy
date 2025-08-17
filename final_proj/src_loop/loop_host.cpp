#include <iostream>
#include <vector>
#include <algorithm>  // for std::fill
#include <iomanip>    // for std::setw
#include <cstring>    // For memcpy
#include <string>     // For std::string
#include <fstream>    // For file operations
#include "xrt/xrt_device.h"
#include "xrt/xrt_kernel.h"
#include "xrt/xrt_bo.h"
#include "../defines.hpp"
#include <chrono>
#include <cmath>

static int numTiles(int value, int x) {
    return std::ceil((value + x - 1) / x);
}
static int ceilToMultiple(int value, int x) {
    return ((value + x - 1) / x) * x;
}

// Structure to hold test data
struct TestCase {
    std::string seq1;
    std::string seq2;
    std::string expectedAligned1;
    std::string expectedAligned2;
};

// Helper function to truncate and display strings
std::string truncateString(const std::string& str, int maxLength = 30) {
    if (str.length() <= maxLength) {
        return str;
    }
    return str.substr(0, maxLength) + "... [" + std::to_string(str.length()) + " characters long - not displayed]";
}

// Function to load test cases from file
std::vector<TestCase> loadTestCases(const std::string& filename = "../datasets/sequence_test_cases.txt") {
    std::vector<TestCase> testCases;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return testCases;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines or comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        TestCase tc;
        size_t pos = 0;
        
        // Parse test case from line (format: seq1,seq2,expectedAligned1,expectedAligned2)
        // Get seq1
        pos = line.find(',');
        if (pos == std::string::npos) continue;
        tc.seq1 = line.substr(0, pos);
        line = line.substr(pos + 1);
        
        // Get seq2
        pos = line.find(',');
        if (pos == std::string::npos) continue;
        tc.seq2 = line.substr(0, pos);
        line = line.substr(pos + 1);
        
        // Get expectedAligned1
        pos = line.find(',');
        if (pos == std::string::npos) continue;
        tc.expectedAligned1 = line.substr(0, pos);
        
        // Get expectedAligned2
        tc.expectedAligned2 = line.substr(pos + 1);
        
        testCases.push_back(tc);
    }
    
    file.close();
    return testCases;
}

//#define DEBUG
int main(int argc, char* argv[]) {
    // Default test case index
    int testCaseIndex = 0;
    char mode = 'S'; // Default to SW emulation
    std::string testFilePath = "../datasets/sequence_test_cases.txt"; // Default path
    
    //INPUTS
    //H = hw emu
    //S = sw emu
    //F = fpga
    xrt::device myDevice; 
    xrt::xclbin xclbin;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-F" || std::string(argv[i]) == "-H" || std::string(argv[i]) == "-S") {
            mode = argv[i][1]; // Get the mode (F, H, or S)
        } else if (std::string(argv[i]) == "-f" && i + 1 < argc) {
            testFilePath = argv[i + 1]; // Get the file path
            i++; // Skip the next argument since we've processed it
        } else {
            // Assume this is the test case index
            testCaseIndex = std::atoi(argv[i]);
        }
    }
    
    // Select device and xclbin based on mode
    if (mode == 'F') {
        xclbin = xrt::xclbin("build_dir.hw.xilinx_u250_gen3x16_xdma_4_1_202210_1/SW_loop.link.xclbin");
        std::cout << "Running in FPGA mode: " << std::endl;
        myDevice = xrt::device("0000:88:00.1");
        
    } else if (mode == 'S') {
        xclbin = xrt::xclbin("SW_loop_sw.xclbin");
        std::cout << "Running in sw emu mode" << std::endl;
        myDevice = xrt::device(0);
        
    } else if (mode == 'H') {
        xclbin = xrt::xclbin("SW_loop_hw.xclbin");
        std::cout << "Running in hw emu mode" << std::endl;
        myDevice = xrt::device(0);
    } else {
        std::cout << "BAD MODE, use -F -S or -H" << std::endl;
        return 1;
    }

    // Load test cases from file
    std::vector<TestCase> testCases = loadTestCases(testFilePath);
    
    if (testCases.empty()) {
        std::cerr << "No test cases found in file '" << testFilePath << "'." << std::endl;
        return 1;
    }
    
    if (testCaseIndex < 0 || testCaseIndex >= static_cast<int>(testCases.size())) {
        std::cerr << "Invalid test case index. Valid range: 0-" << (testCases.size() - 1) << std::endl;
        return 1;
    }
    
    // Get the selected test case
    TestCase selectedTest = testCases[testCaseIndex];
    std::cout << "Running test case " << testCaseIndex << std::endl;

    // Define input sequences from the selected test case
    int seqsize[2] = {static_cast<int>(selectedTest.seq1.length()), 
                      static_cast<int>(selectedTest.seq2.length())}; // Original sequence lengths
    
    // Calculate padded sizes and number of tiles
    int inputsize1 = ceilToMultiple(seqsize[0], TILE_DIMENSION) + 1;
    int inputsize2 = ceilToMultiple(seqsize[1], TILE_DIMENSION) + 1;
    int numTilesVar[2] = {
        numTiles(seqsize[0], TILE_DIMENSION), // Number of horizontal tiles
        numTiles(seqsize[1], TILE_DIMENSION)  // Number of vertical tiles
    };
    
    std::cout << "Input sizes: " << inputsize1 << " || " << inputsize2 << std::endl;
    std::cout << "Tile counts: " << numTilesVar[0] << " || " << numTilesVar[1] << std::endl;
    
    // Allocate memory for sequences
    char* seq1 = (char*)malloc(inputsize1 * sizeof(char));
    char* seq2 = (char*)malloc(inputsize2 * sizeof(char));
    
    // Copy sequences from test case
    strcpy(seq1, selectedTest.seq1.c_str());
    strcpy(seq2, selectedTest.seq2.c_str());
    
    // Output buffers for aligned sequences
    char* alignedSeq1 = (char*)malloc((seqsize[0]+seqsize[1]) * sizeof(char));
    char* alignedSeq2 = (char*)malloc((seqsize[0]+seqsize[1]) * sizeof(char));
    
    // Pointer for score buffer - size needs to include the boundary cells
    int buffer_horz_size = numTilesVar[0] * TILE_DIMENSION + 1;
    int buffer_vert_size = numTilesVar[1] * TILE_DIMENSION + 1;

    //WE WANT TO REMOVE THESE MALLOCS LATER
    int* buffer = (int*)malloc(sizeof(int) * buffer_horz_size * buffer_vert_size);

    #ifdef DEBUG
        std::cout << "Input Load Complete" << std::endl;
    #endif

    // Print input with truncation
    std::cout << "Sequence 1: " << truncateString(seq1) << std::endl;
    std::cout << "Sequence 2: " << truncateString(seq2) << std::endl;

    auto uuid = myDevice.load_xclbin(xclbin);

    xrt::kernel SW_basic_linear(myDevice, uuid, "SW_basic_linear");

    xrt::run RunObj = xrt::run(SW_basic_linear);

    //make the funny buffers
    auto seq1_bo = xrt::bo(myDevice, sizeof(char) * inputsize1, 
                           SW_basic_linear.group_id(0));
    auto seq2_bo = xrt::bo(myDevice, sizeof(char) * inputsize2, 
                           SW_basic_linear.group_id(1));
    auto buffer_bo = xrt::bo(myDevice, sizeof(int) * buffer_horz_size * buffer_vert_size, 
                            xrt::bo::flags::device_only, SW_basic_linear.group_id(2));
    auto seqsz_bo = xrt::bo(myDevice, sizeof(int) * 2, 
                            SW_basic_linear.group_id(5));
    auto tilenum_bo = xrt::bo(myDevice, sizeof(int) * 2, 
                            SW_basic_linear.group_id(6));
    auto align1_bo = xrt::bo(myDevice, sizeof(char) * (seqsize[0]+seqsize[1]), 
                            SW_basic_linear.group_id(3));
    auto align2_bo = xrt::bo(myDevice, sizeof(char) * (seqsize[0]+seqsize[1]), 
                            SW_basic_linear.group_id(4));

    #ifdef DEBUG
        std::cout << "Buffers Created " << std::endl;
    #endif

    auto start = std::chrono::high_resolution_clock::now();
    //copy host data
    seq1_bo.write(seq1, sizeof(char) * inputsize1, 0);
    seq2_bo.write(seq2, sizeof(char) * inputsize2, 0);
    seqsz_bo.write(seqsize, sizeof(int) * 2, 0);
    tilenum_bo.write(numTilesVar, sizeof(int) * 2, 0);
    //DO NOT CALL WRITE OR SYNC FOR BUFFER
    seq1_bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    seq2_bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    seqsz_bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    tilenum_bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    #ifdef DEBUG
        std::cout << "Buffers Written " << std::endl;
    #endif

    //run the kernel
    RunObj.set_arg(0, seq1_bo);
    RunObj.set_arg(1, seq2_bo);
    RunObj.set_arg(2, buffer_bo);
    RunObj.set_arg(3, seqsz_bo);
    RunObj.set_arg(4, tilenum_bo);
    RunObj.set_arg(5, align1_bo);
    RunObj.set_arg(6, align2_bo);
    RunObj.start();
    RunObj.wait();



    // Read back the output
    align1_bo.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    align2_bo.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    #ifdef DEBUG
        std::cout << "Execution Complete " << std::endl;
    #endif    
    align1_bo.read(alignedSeq1);
    align2_bo.read(alignedSeq2);

    // Reverse the aligned sequences
    std::reverse(alignedSeq1, alignedSeq1 + strlen(alignedSeq1));
    std::reverse(alignedSeq2, alignedSeq2 + strlen(alignedSeq2));

    // Print results with truncation
    std::cout << "Aligned 1 : " << truncateString(alignedSeq1) << std::endl;
    std::cout << "Aligned 2 : " << truncateString(alignedSeq2) << std::endl;

    // Verification
    bool matchesExpected = true;
    
    std::cout << "Expected 1: " << truncateString(selectedTest.expectedAligned1) << std::endl;
    std::cout << "Expected 2: " << truncateString(selectedTest.expectedAligned2) << std::endl;
    
    matchesExpected = (strcmp(alignedSeq1, selectedTest.expectedAligned1.c_str()) == 0) && 
                     (strcmp(alignedSeq2, selectedTest.expectedAligned2.c_str()) == 0);
    
    if (!matchesExpected) {
        std::cout << "Output does not match expected result!" << std::endl;
    }

    std::free(seq1);
    std::free(seq2);
    std::free(alignedSeq1);
    std::free(alignedSeq2);
    std::free(buffer);

    std::cout << "Total time taken: " << duration.count() << " seconds" << std::endl;
    // Final result
    if (matchesExpected) {
        std::cout << "Test Passed!" << std::endl;
        return 0; // Success
    } else {
        std::cout << "Test Failed: Incorrect alignment" << std::endl;
        return 1; // Failure
    }
}