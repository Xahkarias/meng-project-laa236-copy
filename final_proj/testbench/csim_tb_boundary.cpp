#include <cstdlib>
#include <iostream>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include "../defines.hpp"
#include "../sw_algo.hpp"
#include <algorithm>
#include <cmath>

static int ceilToMultiple(int value, int x) {
    return ((value + x - 1) / x) * x;
}

static int numTiles(int value, int x) {
    return std::ceil((value + x - 1) / x);
}

// Structure to hold test data
struct TestCase {
    std::string seq1;
    std::string seq2;
    std::string expectedAligned1;
    std::string expectedAligned2;
};

// Function to load test cases from file
//ignores comments
std::vector<TestCase> loadTestCases(const std::string& filename) {
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

// Helper function to display sequences with length limit
void displaySequence(const char* label, const char* sequence, bool truncate = true) {
    if (!truncate || strlen(sequence) <= 30) {
        std::cout << label << sequence << std::endl;
    } else {
        std::cout << label << "[" << strlen(sequence) << " characters long - not displayed]" << std::endl;
    }
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options] [test_case_index]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -f <filename>       Specify input file (default: ../../../../../datasets/sequence_test_cases.txt)" << std::endl;
    std::cout << "  -t                  Disable truncation of sequence display" << std::endl;
    std::cout << "  -h                  Display this help message" << std::endl;
}

int main(int argc, char* argv[]) {
    // Default test case index
    int testCaseIndex = 0;
    
    // Default input file
    std::string inputFile = "../../../../../datasets/sequence_test_cases.txt";
    
    // Default truncation setting
    bool truncateOutput = true;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (i < argc - 1 && strcmp(argv[i], "-f") == 0) {
            // -f flag for specifying input file
            inputFile = argv[i + 1];
            i++; // Skip the next argument since we've used it
        } else if (strcmp(argv[i], "-t") == 0) {
            // -t flag to disable truncation
            truncateOutput = false;
        } else if (strcmp(argv[i], "-h") == 0) {
            // -h flag for help
            printUsage(argv[0]);
            return 0;
        } else {
            // Assume it's the test case index
            testCaseIndex = std::atoi(argv[i]);
        }
    }
    
    std::cout << "Using input file: " << inputFile << std::endl;
    
    // Load test cases from the specified file
    std::vector<TestCase> testCases = loadTestCases(inputFile);
    
    if (testCases.empty()) {
        std::cerr << "No test cases found in file '" << inputFile << "'." << std::endl;
        return 1;
    }
    
    if (testCaseIndex < 0 || testCaseIndex >= static_cast<int>(testCases.size())) {
        std::cerr << "Invalid test case index. Valid range: 0-" << (testCases.size() - 1) << std::endl;
        return 1;
    }
    
    // Get the selected test case
    TestCase selectedTest = testCases[testCaseIndex];
    
    std::cout << "Running test case " << testCaseIndex << std::endl;
    
    // Define input, includes null character
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
    int buffer_horz_size = numTilesVar[0] * (TILE_DIMENSION + 1) * 2;
    int buffer_vert_size = numTilesVar[1];
    int* buffer = (int*)malloc(sizeof(int) * buffer_horz_size * buffer_vert_size);
   
    // Print input with truncation setting from command line
    displaySequence("Sequence 1: ", seq1, truncateOutput);
    displaySequence("Sequence 2: ", seq2, truncateOutput);
   
    // Call the HLS function
    SW_basic_linear(seq1, seq2, buffer, seqsize, numTilesVar, alignedSeq1, alignedSeq2);
    
    // Reverse the aligned sequences
    std::reverse(alignedSeq1, alignedSeq1 + strlen(alignedSeq1));
    std::reverse(alignedSeq2, alignedSeq2 + strlen(alignedSeq2));
   
    // Print results with truncation setting from command line
    displaySequence("Aligned 1 : ", alignedSeq1, truncateOutput);
    displaySequence("Aligned 2 : ", alignedSeq2, truncateOutput);
   
    // Verification
    bool matchesExpected = true;
    
    // Display expected results with truncation setting from command line
    if (!selectedTest.expectedAligned1.empty() && !selectedTest.expectedAligned2.empty()) {
        displaySequence("Expected 1: ", selectedTest.expectedAligned1.c_str(), truncateOutput);
        displaySequence("Expected 2: ", selectedTest.expectedAligned2.c_str(), truncateOutput);
        
        matchesExpected = (strcmp(alignedSeq1, selectedTest.expectedAligned1.c_str()) == 0) && 
                         (strcmp(alignedSeq2, selectedTest.expectedAligned2.c_str()) == 0);
        
        if (!matchesExpected) {
            std::cout << "Output does not match expected result!" << std::endl;
        } else {
            std::cout << "Output matches expected result." << std::endl;
        }
    }
    
    // Cleanup
    free(seq1);
    free(seq2);
    free(alignedSeq1);
    free(alignedSeq2);
    free(buffer);
    
    // Final result
    if (matchesExpected) {
        std::cout << "Test Passed!" << std::endl;
        return 0; // Success
    } else {
        std::cout << "Test Failed: Incorrect alignment" << std::endl;
        return 1; // Failure
    }
}