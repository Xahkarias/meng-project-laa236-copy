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
#include <numeric>    // For std::accumulate

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
};

// Helper function to truncate and display strings
std::string truncateString(const std::string& str, int maxLength = 30) {
    if (str.length() <= maxLength) {
        return str;
    }
    return str.substr(0, maxLength) + "... [" + std::to_string(str.length()) + " characters long - not displayed]";
}

// Function to load test cases from file
std::vector<TestCase> loadTestCases(const std::string& filename = "../datasets/eval_dataset.txt") {
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
        
        // Parse test case from line (format: seq1,seq2)
        // Get seq1
        pos = line.find(',');
        if (pos == std::string::npos) continue;
        tc.seq1 = line.substr(0, pos);
        line = line.substr(pos + 1);
        
        // Get seq2
        tc.seq2 = line;  // Take the rest of the line as seq2
        testCases.push_back(tc);
    }
    
    file.close();
    return testCases;
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options] [test_case_index]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -f <filename>       Specify input file (default: ../datasets/eval_dataset.txt)" << std::endl;
    std::cout << "  -x <directory>      Specify directory where build_dir.hw.xilinx_u250_gen3x16_xdma_4_1_202210_1/SW_syst.link.xclbin is located" << std::endl;
    std::cout << "  -b <num_runs>       Run benchmark with specified number of iterations (default: 1)" << std::endl;
    std::cout << "  -s <tile_size>      Specify the tile dimension size (mandatory)" << std::endl;
    std::cout << "  -h                  Display this help message" << std::endl;
}

// Function to run a single execution and return the time
double runSingleExecution(TestCase& testCase, xrt::device& myDevice, xrt::xclbin& xclbin, int tileDimension, bool printOutput = true) {
    // Define input sequences from the selected test case
    int seqsize[2] = {static_cast<int>(testCase.seq1.length()), 
                      static_cast<int>(testCase.seq2.length())}; // Original sequence lengths
    
    // Calculate padded sizes and number of tiles
    int inputsize1 = ceilToMultiple(seqsize[0], tileDimension) + 1;
    int inputsize2 = ceilToMultiple(seqsize[1], tileDimension) + 1;
    int numTilesVar[2] = {
        numTiles(seqsize[0], tileDimension), // Number of horizontal tiles
        numTiles(seqsize[1], tileDimension)  // Number of vertical tiles
    };
    
    if (printOutput) {
        std::cout << "Input sizes: " << inputsize1 << " || " << inputsize2 << std::endl;
        std::cout << "Tile counts: " << numTilesVar[0] << " || " << numTilesVar[1] << std::endl;
    }
    
    // Allocate memory for sequences
    char* seq1 = (char*)malloc(inputsize1 * sizeof(char));
    char* seq2 = (char*)malloc(inputsize2 * sizeof(char));
    
    // Copy sequences from test case
    strcpy(seq1, testCase.seq1.c_str());
    strcpy(seq2, testCase.seq2.c_str());
    
    // Output buffers for aligned sequences
    char* alignedSeq1 = (char*)malloc((seqsize[0]+seqsize[1]) * sizeof(char));
    char* alignedSeq2 = (char*)malloc((seqsize[0]+seqsize[1]) * sizeof(char));
    
    // Pointer for score buffer - size needs to include the boundary cells
    int buffer_horz_size = numTilesVar[0] * (tileDimension + 1)  * 2;
    int buffer_vert_size = numTilesVar[1];

    // Allocate buffer
    int* buffer = (int*)malloc(sizeof(int) * buffer_horz_size * buffer_vert_size);

    // Print input with truncation
    if (printOutput) {
        std::cout << "Sequence 1: " << truncateString(seq1) << std::endl;
        std::cout << "Sequence 2: " << truncateString(seq2) << std::endl;
    }

    #ifdef DEBUG
        if (printOutput) std::cout << "Input Load Complete" << std::endl;
    #endif

    auto uuid = myDevice.load_xclbin(xclbin);

    xrt::kernel SW_basic_linear(myDevice, uuid, "SW_basic_linear");

    xrt::run RunObj = xrt::run(SW_basic_linear);

    // Make the buffers
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
        if (printOutput) std::cout << "Buffers Created " << std::endl;
    #endif

    // Copy host data
    auto start = std::chrono::high_resolution_clock::now();
    seq1_bo.write(seq1, sizeof(char) * inputsize1, 0);
    seq2_bo.write(seq2, sizeof(char) * inputsize2, 0);
    seqsz_bo.write(seqsize, sizeof(int) * 2, 0);
    tilenum_bo.write(numTilesVar, sizeof(int) * 2, 0);
    // DO NOT CALL WRITE OR SYNC FOR BUFFER
    seq1_bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    seq2_bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    seqsz_bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    tilenum_bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    #ifdef DEBUG
        if (printOutput) std::cout << "Buffers Written " << std::endl;
    #endif

    // Run the kernel
    RunObj.set_arg(0, seq1_bo);
    RunObj.set_arg(1, seq2_bo);
    RunObj.set_arg(2, buffer_bo);
    RunObj.set_arg(3, seqsz_bo);
    RunObj.set_arg(4, tilenum_bo);
    RunObj.set_arg(5, align1_bo);
    RunObj.set_arg(6, align2_bo);
    RunObj.start();
    RunObj.wait();

    #ifdef DEBUG
        if (printOutput) std::cout << "Execution Complete " << std::endl;
    #endif    

    // Read back the output
    align1_bo.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    align2_bo.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    
    #ifdef DEBUG
        if (printOutput) std::cout << "Execution Complete " << std::endl;
    #endif  
    
    align1_bo.read(alignedSeq1);
    align2_bo.read(alignedSeq2);

    // Reverse the aligned sequences
    std::reverse(alignedSeq1, alignedSeq1 + strlen(alignedSeq1));
    std::reverse(alignedSeq2, alignedSeq2 + strlen(alignedSeq2));

    if (printOutput) {
        // Print results with truncation
        std::cout << "Aligned 1 : " << truncateString(alignedSeq1) << std::endl;
        std::cout << "Aligned 2 : " << truncateString(alignedSeq2) << std::endl;
    }

    // Free memory
    std::free(seq1);
    std::free(seq2);
    std::free(alignedSeq1);
    std::free(alignedSeq2);
    std::free(buffer);

    return duration.count(); // Return execution time
}

//#define DEBUG
int main(int argc, char* argv[]) {
    // Default test case index
    int testCaseIndex = 0;
    std::string testFilePath = "../datasets/eval_dataset.txt"; // Default path
    std::string xclbinDir = "."; // Default to current directory
    int benchmarkRuns = 1; // Default to running once
    int tileDimension = -1; // Must be set by -s flag
    
    // FPGA-only implementation
    xrt::device myDevice; 
    xrt::xclbin xclbin;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-f" && i + 1 < argc) {
            testFilePath = argv[i + 1]; // Get the file path
            i++; // Skip the next argument since we've processed it
        } else if (std::string(argv[i]) == "-x" && i + 1 < argc) {
            xclbinDir = argv[i + 1]; // Get the directory path
            i++; // Skip the next argument since we've processed it
        } else if (std::string(argv[i]) == "-b" && i + 1 < argc) {
            benchmarkRuns = std::atoi(argv[i + 1]); // Get the number of benchmark runs
            if (benchmarkRuns <= 0) {
                std::cerr << "Error: Number of benchmark runs must be positive." << std::endl;
                return 1;
            }
            i++; // Skip the next argument since we've processed it
        } else if (std::string(argv[i]) == "-s" && i + 1 < argc) {
            tileDimension = std::atoi(argv[i + 1]); // Get the tile dimension
            if (tileDimension <= 0) {
                std::cerr << "Error: Tile dimension must be positive." << std::endl;
                return 1;
            }
            i++; // Skip the next argument since we've processed it
        } else if (std::string(argv[i]) == "-h") {
            printUsage(argv[0]);
            return 0;
        } else {
            // Assume this is the test case index
            testCaseIndex = std::atoi(argv[i]);
        }
    }
    
    // Check if tile dimension was specified
    if (tileDimension == -1) {
        std::cerr << "Error: Tile dimension (-s) is mandatory." << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    std::cout << "Using tile dimension: " << tileDimension << std::endl;
    
    // Construct the full xclbin path by appending the required path to the specified directory
    std::string fullXclbinPath;
    // Add trailing slash to directory if not present
    if (!xclbinDir.empty() && xclbinDir.back() != '/') {
        xclbinDir += '/';
    }
    fullXclbinPath = xclbinDir + "build_dir.hw.xilinx_u250_gen3x16_xdma_4_1_202210_1/SW_syst.link.xclbin";
    
    // Always use FPGA mode
    std::cout << "Running in FPGA mode" << std::endl;
    std::cout << "Using xclbin file: " << fullXclbinPath << std::endl;
    xclbin = xrt::xclbin(fullXclbinPath);
    myDevice = xrt::device("0000:88:00.1");

    // Load test cases from file
    std::cout << "Using input file: " << testFilePath << std::endl;
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
    
    // If benchmarking mode is enabled
    if (benchmarkRuns > 1) {
        std::cout << "Running benchmark with " << benchmarkRuns << " iterations" << std::endl;
        
        std::vector<double> executionTimes;
        executionTimes.reserve(benchmarkRuns);
        
        // Run the test multiple times
        for (int i = 0; i < benchmarkRuns; i++) {
            std::cout << "Run " << (i + 1) << "/" << benchmarkRuns << ": ";
            double time = runSingleExecution(selectedTest, myDevice, xclbin, tileDimension, i == 0); // Only print output for the first run
            executionTimes.push_back(time);
            std::cout << "Time: " << time << " seconds" << std::endl;
            std::cout << "--------------------------------" << std::endl;
        }
        
        // Calculate statistics
        double totalTime = std::accumulate(executionTimes.begin(), executionTimes.end(), 0.0);
        double avgTime = totalTime / benchmarkRuns;
        
        // Find min and max
        double minTime = *std::min_element(executionTimes.begin(), executionTimes.end());
        double maxTime = *std::max_element(executionTimes.begin(), executionTimes.end());
        
        // Calculate standard deviation
        double sumSquaredDiff = 0.0;
        for (double time : executionTimes) {
            double diff = time - avgTime;
            sumSquaredDiff += diff * diff;
        }
        double stdDev = std::sqrt(sumSquaredDiff / benchmarkRuns);
        
        // Print benchmark results
        std::cout << "Benchmark results:" << std::endl;
        std::cout << "  Average time: " << avgTime << " seconds" << std::endl;
        std::cout << "  Min time: " << minTime << " seconds" << std::endl;
        std::cout << "  Max time: " << maxTime << " seconds" << std::endl;
        std::cout << "  Standard deviation: " << stdDev << " seconds" << std::endl;
    } else {
        // Run a single execution
        double time = runSingleExecution(selectedTest, myDevice, xclbin, tileDimension, true);
        std::cout << "Total time taken: " << time << " seconds" << std::endl;
    }
    
    std::cout << "Test completed." << std::endl;
    return 0; // Success
}