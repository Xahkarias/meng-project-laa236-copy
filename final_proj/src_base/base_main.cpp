#include <iostream>
#include <chrono>
#include <fstream>
#include <string>
#include <vector>
#include <numeric>
#include <algorithm>
#include "base_main.hpp"
#include <sstream>
#include "../defines.hpp"
#include <cstring>

// Structure to hold test data
struct TestCase {
    std::string seq1;
    std::string seq2;
    std::string expectedAligned1;
    std::string expectedAligned2;
};

// Function to load test cases from file
std::vector<TestCase> loadTestCases(const std::string& filename, bool skipVerification = false) {
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
        if (skipVerification) {
            // If skipping verification, we only need the second sequence
            tc.seq2 = line;  // Take the rest of the line as seq2
            testCases.push_back(tc);
            continue;  // Skip parsing the expected alignments
        } else {
            // Original behavior - expect full format
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
        }
        
        testCases.push_back(tc);
    }
    
    file.close();
    return testCases;
}

// Function to run a single test case and return execution time
double runTestCase(const TestCase& testCase, bool printOutput = true, bool skipVerification = false) {
    // Get sequence sizes
    size_t size1 = testCase.seq1.length();
    size_t size2 = testCase.seq2.length();
    
    // Allocate memory for sequences
    char* seq1Ptr = (char*)malloc((size1+1) * sizeof(char));
    char* seq2Ptr = (char*)malloc((size2+1) * sizeof(char));
    
    if (seq1Ptr == nullptr || seq2Ptr == nullptr) {
        std::cerr << "Memory allocation failed for sequences" << std::endl;
        return -1.0;
    }
    
    // Copy sequences from test case
    strcpy(seq1Ptr, testCase.seq1.c_str());
    strcpy(seq2Ptr, testCase.seq2.c_str());
    
    // Start timing
    auto start = std::chrono::high_resolution_clock::now();
    
    // Run Smith-Waterman algorithm
    std::pair<std::string, std::string> out = smithWaterman(seq1Ptr, size1, seq2Ptr, size2);
    
    // End timing
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    
    if (printOutput) {
        // Print input, output, and timing with length checks
        if (size1 <= 30) {
            std::cout << "Sequence 1: " << seq1Ptr << std::endl;
        } else {
            std::cout << "Sequence 1: [" << size1 << " characters long - not displayed]" << std::endl;
        }
        
        if (size2 <= 30) {
            std::cout << "Sequence 2: " << seq2Ptr << std::endl;
        } else {
            std::cout << "Sequence 2: [" << size2 << " characters long - not displayed]" << std::endl;
        }
        
        if (out.first.length() <= 30) {
            std::cout << "Aligned 1 : " << out.first << std::endl;
        } else {
            std::cout << "Aligned 1 : [" << out.first.length() << " characters long - not displayed]" << std::endl;
        }
        
        if (out.second.length() <= 30) {
            std::cout << "Aligned 2 : " << out.second << std::endl;
        } else {
            std::cout << "Aligned 2 : [" << out.second.length() << " characters long - not displayed]" << std::endl;
        }
        
        // Check if expected values were provided and match actual output
        if (!skipVerification && !testCase.expectedAligned1.empty() && !testCase.expectedAligned2.empty()) {
            if (testCase.expectedAligned1.length() <= 30) {
                std::cout << "Expected 1: " << testCase.expectedAligned1 << std::endl;
            } else {
                std::cout << "Expected 1: [" << testCase.expectedAligned1.length() << " characters long - not displayed]" << std::endl;
            }
            
            if (testCase.expectedAligned2.length() <= 30) {
                std::cout << "Expected 2: " << testCase.expectedAligned2 << std::endl;
            } else {
                std::cout << "Expected 2: [" << testCase.expectedAligned2.length() << " characters long - not displayed]" << std::endl;
            }
            
            bool matchesExpected = (out.first == testCase.expectedAligned1) && 
                                  (out.second == testCase.expectedAligned2);
            
            if (!matchesExpected) {
                std::cout << "Output does not match expected result!" << std::endl;
            } else {
                std::cout << "Output matches expected result." << std::endl;
            }
        }
    }
    
    // Cleanup
    free(seq1Ptr);
    free(seq2Ptr);
    
    return duration.count();
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options] [test_case_index]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -f <filename>       Specify input file (default: ../datasets/sequence_test_cases.txt)" << std::endl;
    std::cout << "  -b <num_runs>       Run benchmark with specified number of iterations (default: 10)" << std::endl;
    std::cout << "  -e                  Skip correctness verification (for datasets without expected alignments)" << std::endl;
    std::cout << "  -h                  Display this help message" << std::endl;
}

int main(int argc, char* argv[])
{
    // Default test case index
    int testCaseIndex = 0;
    
    // Default input file
    std::string inputFile = "../datasets/sequence_test_cases.txt";
    
    // Default number of benchmark runs (0 means no benchmark)
    int benchmarkRuns = 0;
    
    // New flag for skipping correctness verification
    bool skipVerification = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (i < argc - 1 && strcmp(argv[i], "-f") == 0) {
            // -f flag for specifying input file
            inputFile = argv[i + 1];
            i++; // Skip the next argument since we've used it
        } else if (i < argc - 1 && strcmp(argv[i], "-b") == 0) {
            // -b flag for benchmark mode with specified number of runs
            benchmarkRuns = std::atoi(argv[i + 1]);
            if (benchmarkRuns <= 0) {
                std::cerr << "Error: Number of benchmark runs must be positive." << std::endl;
                return 1;
            }
            i++; // Skip the next argument since we've used it
        } else if (strcmp(argv[i], "-e") == 0) {
            // -e flag for skipping correctness verification
            skipVerification = true;
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
    if (skipVerification) {
        std::cout << "Correctness verification disabled" << std::endl;
    }
    
    // Load test cases from the specified file
    std::vector<TestCase> testCases = loadTestCases(inputFile, skipVerification);
    
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
    
    if (benchmarkRuns > 0) {
        // Benchmark mode - run the test multiple times
        std::cout << "Running benchmark with " << benchmarkRuns << " iterations for test case " << testCaseIndex << std::endl;
        
        std::vector<double> times;
        times.reserve(benchmarkRuns);
        
        // Run tests
        for (int i = 0; i < benchmarkRuns; i++) {
            std::cout << "Iteration " << (i + 1) << "/" << benchmarkRuns << ":" << std::endl;
            // Only print output for the first run
            double time = runTestCase(selectedTest, i == 0, skipVerification);
            times.push_back(time);
            std::cout << "Time: " << time << " seconds" << std::endl;
            std::cout << "--------------------------------" << std::endl;
        }
        
        // Calculate statistics (discarding the first run)
        if (benchmarkRuns > 1) {
            double sum = std::accumulate(times.begin() + 1, times.end(), 0.0);
            double avg = sum / (benchmarkRuns - 1);
            
            // Find min and max (excluding first run)
            double min = *std::min_element(times.begin() + 1, times.end());
            double max = *std::max_element(times.begin() + 1, times.end());
            
            std::cout << "Benchmark results (excluding first run):" << std::endl;
            std::cout << "  Average time: " << avg << " seconds" << std::endl;
            std::cout << "  Min time: " << min << " seconds" << std::endl;
            std::cout << "  Max time: " << max << " seconds" << std::endl;
        } else {
            std::cout << "Note: At least 2 iterations are needed to calculate statistics excluding first run." << std::endl;
        }
    } else {
        // Normal mode - run single test
        std::cout << "Running test case " << testCaseIndex << std::endl;
        double time = runTestCase(selectedTest, true, skipVerification);
        std::cout << "Total time taken: " << time << " seconds" << std::endl;
    }
    
    return 0;
}