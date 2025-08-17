#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include "extern_src/ssw_cpp.h"
#include <chrono>
#include <numeric>  // for std::accumulate
#include <algorithm> // for std::min_element, std::max_element
#include <cmath>     // for std::sqrt
#include <functional> // for std::inner_product

using std::string;
using std::cout;
using std::endl;

// Structure to hold test data
struct TestCase {
    std::string ref;
    std::string query;
};

// Function to load test cases from file
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
        
        // Parse test case from line (format: ref,query)
        pos = line.find(',');
        if (pos == std::string::npos) continue;
        tc.ref = line.substr(0, pos);
        tc.query = line.substr(pos + 1);
        
        testCases.push_back(tc);
    }
    
    file.close();
    return testCases;
}

static void PrintAlignment(const StripedSmithWaterman::Alignment& alignment) {
    cout << "===== SSW result =====" << endl;
    cout << "Best Smith-Waterman score:\t" << alignment.sw_score << endl
         << "Next-best Smith-Waterman score:\t" << alignment.sw_score_next_best << endl
         << "Reference start:\t" << alignment.ref_begin << endl
         << "Reference end:\t" << alignment.ref_end << endl
         << "Query start:\t" << alignment.query_begin << endl
         << "Query end:\t" << alignment.query_end << endl
         << "Next-best reference end:\t" << alignment.ref_end_next_best << endl
         << "Number of mismatches:\t" << alignment.mismatches << endl
         << "Cigar: " << alignment.cigar_string << endl;
    cout << "======================" << endl;
}

static void VisualizeAlignment(const StripedSmithWaterman::Alignment& alignment,
                               const string& ref, const string& query) {
    if (alignment.cigar_string.empty()) {
        cout << "No alignment to visualize (empty CIGAR)." << endl;
        return;
    }
    
    string ref_aligned;
    string query_aligned;
    string match_line;
    
    int ref_pos = alignment.ref_begin;
    int query_pos = alignment.query_begin;
    
    // Parse the CIGAR string directly
    string cigar = alignment.cigar_string;
    size_t pos = 0;
    
    while (pos < cigar.length()) {
        // Extract the length
        size_t next_pos = cigar.find_first_not_of("0123456789", pos);
        if (next_pos == string::npos) break;
        
        int length = std::stoi(cigar.substr(pos, next_pos - pos));
        char op = cigar[next_pos];
        pos = next_pos + 1;
        
        // Process based on operation
        switch (op) {
            case '=': // Match
                for (int j = 0; j < length; ++j) {
                    if (ref_pos < ref.length() && query_pos < query.length()) {
                        ref_aligned += ref[ref_pos];
                        query_aligned += query[query_pos];
                        match_line += '|'; // Match
                        ref_pos++;
                        query_pos++;
                    }
                }
                break;
                
            case 'X': // Mismatch
                for (int j = 0; j < length; ++j) {
                    if (ref_pos < ref.length() && query_pos < query.length()) {
                        ref_aligned += ref[ref_pos];
                        query_aligned += query[query_pos];
                        match_line += ' '; // Mismatch
                        ref_pos++;
                        query_pos++;
                    }
                }
                break;
                
            case 'I': // Insertion to reference
                for (int j = 0; j < length; ++j) {
                    if (query_pos < query.length()) {
                        ref_aligned += '-';
                        query_aligned += query[query_pos];
                        match_line += ' ';
                        query_pos++;
                    }
                }
                break;
                
            case 'D': // Deletion from reference
                for (int j = 0; j < length; ++j) {
                    if (ref_pos < ref.length()) {
                        ref_aligned += ref[ref_pos];
                        query_aligned += '-';
                        match_line += ' ';
                        ref_pos++;
                    }
                }
                break;
                
            case 'S': // Soft clipping
                for (int j = 0; j < length; ++j) {
                    if (query_pos < query.length()) {
                        ref_aligned += '-';
                        query_aligned += query[query_pos];
                        match_line += ' ';
                        query_pos++;
                    }
                }
                break;
        }
    }
    
    cout << "===== Alignment Visualization =====" << endl;
    cout << "Reference: " << ref_aligned << endl;
    cout << "           " << match_line << endl;
    cout << "Query:     " << query_aligned << endl;
    cout << "==================================" << endl;
}

// Function to run an alignment and return the execution time
double runAlignment(const TestCase& testCase, StripedSmithWaterman::Alignment* alignment, int customMaskLen = -1) {
    // Declares a default Aligner
    StripedSmithWaterman::Aligner aligner(3, 3, 2, 2);
    // Declares a default filter
    StripedSmithWaterman::Filter filter;
    
    int32_t maskLen;
    if (customMaskLen > 0) {
        // Use the custom mask length if provided
        maskLen = customMaskLen;
    } else {
        // Use the default calculation
        maskLen = testCase.query.length() / 2;
        maskLen = maskLen < 15 ? 15 : maskLen;
    }
    
    // Start timing
    auto start = std::chrono::high_resolution_clock::now();
    
    // Aligns the query to the ref
    aligner.Align(testCase.query.c_str(), testCase.ref.c_str(), 
                 testCase.ref.size(), filter, alignment, maskLen);
    
    // End timing
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    
    return duration.count();
}

// Function to run benchmark with multiple iterations
void runBenchmark(const TestCase& testCase, int iterations, int customMaskLen = -1) {
    std::vector<double> times;
    StripedSmithWaterman::Alignment alignment;
    
    std::cout << "\n===== Benchmarking (" << iterations << " iterations) =====" << std::endl;
    
    // Print mask length info
    int actualMaskLen = (customMaskLen > 0) ? customMaskLen : 
                        (testCase.query.length() / 2 < 15 ? 15 : testCase.query.length() / 2);
    std::cout << "Using maskLen parameter: " << actualMaskLen << std::endl;
    
    // Run the first time (warm-up)
    double firstTime = runAlignment(testCase, &alignment, customMaskLen);
    std::cout << "Warm-up run: " << firstTime << " seconds (not included in average)" << std::endl;
    
    // Run remaining iterations
    for (int i = 0; i < iterations; i++) {
        double time = runAlignment(testCase, &alignment, customMaskLen);
        times.push_back(time);
        std::cout << "Run " << (i + 1) << ": " << time << " seconds" << std::endl;
    }
    
    // Calculate average time
    double sum = std::accumulate(times.begin(), times.end(), 0.0);
    double avg = sum / times.size();
    
    // Calculate min and max times
    double min = *std::min_element(times.begin(), times.end());
    double max = *std::max_element(times.begin(), times.end());
    
    // Calculate standard deviation
    double sq_sum = std::inner_product(times.begin(), times.end(), times.begin(), 0.0);
    double stdev = std::sqrt(sq_sum / times.size() - avg * avg);
    
    // Remove the timing results from here as we'll print them after the alignment
    
    // Print alignment results just once
    PrintAlignment(alignment);
    VisualizeAlignment(alignment, testCase.ref, testCase.query);
    
    // Print the timing information after the alignment details
    std::cout << "\nBenchmark Results (excluding warm-up run):" << std::endl;
    std::cout << "  Average time: " << avg << " seconds" << std::endl;
    std::cout << "  Min time: " << min << " seconds" << std::endl;
    std::cout << "  Max time: " << max << " seconds" << std::endl;
    std::cout << "  Standard deviation: " << stdev << " seconds" << std::endl;
    std::cout << "=================================" << std::endl;
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options] <test_case_index>" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -f <filename>       Specify input file (default: sequences.txt)" << std::endl;
    std::cout << "  -b [iterations]     Run benchmark (default: 10 iterations)" << std::endl;
    std::cout << "  -m <maskLen>        Set mask length for SSW algorithm (default: max(15, query_length/2))" << std::endl;
    std::cout << "  -h                  Display this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << " -f my_sequences.txt 2" << std::endl;
    std::cout << "  " << programName << " -b 5 0" << std::endl;
    std::cout << "  " << programName << " -m 8 3" << std::endl;
    std::cout << "  " << programName << " -f my_sequences.txt -b 20 -m 32 3" << std::endl;
}

int main(int argc, char* argv[]) {
    // Default input file
    std::string inputFile = "sequences.txt";
    
    // Test case index (required)
    int testCaseIndex = -1;
    
    // Benchmarking flag and iterations
    bool runBenchmarkFlag = false;
    int benchmarkIterations = 10; // default iterations
    
    // Custom mask length
    int customMaskLen = -1; // -1 means use default calculation
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            // -h flag for help
            printUsage(argv[0]);
            return 0;
        } else if (i < argc - 1 && strcmp(argv[i], "-f") == 0) {
            // -f flag for specifying input file
            inputFile = argv[i + 1];
            i++; // Skip the next argument since we've used it
        } else if (strcmp(argv[i], "-b") == 0) {
            // -b flag for benchmarking
            runBenchmarkFlag = true;
            
            // Check if next argument is a number (iterations)
            if (i + 1 < argc && isdigit(argv[i + 1][0])) {
                benchmarkIterations = std::atoi(argv[i + 1]);
                i++; // Skip the next argument since we've used it
            }
        } else if (i < argc - 1 && strcmp(argv[i], "-m") == 0) {
            // -m flag for custom mask length
            customMaskLen = std::atoi(argv[i + 1]);
            i++; // Skip the next argument since we've used it
        } else {
            // Assume it's the test case index
            testCaseIndex = std::atoi(argv[i]);
        }
    }
    
    // Check if a test case index was provided
    if (testCaseIndex < 0) {
        std::cerr << "Error: Test case index is required." << std::endl;
        printUsage(argv[0]);
        return 1;
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
    
    // Display the selected test case
    std::cout << "Running test case " << testCaseIndex << std::endl;
    if (selectedTest.ref.length() <= 50) {
        std::cout << "Reference: " << selectedTest.ref << std::endl;
    } else {
        std::cout << "Reference: [" << selectedTest.ref.length() << " characters long - first 50: "
                  << selectedTest.ref.substr(0, 50) << "...]" << std::endl;
    }
    
    if (selectedTest.query.length() <= 50) {
        std::cout << "Query: " << selectedTest.query << std::endl;
    } else {
        std::cout << "Query: [" << selectedTest.query.length() << " characters long - first 50: "
                  << selectedTest.query.substr(0, 50) << "...]" << std::endl;
    }
    
    if (runBenchmarkFlag) {
        // Run benchmark mode
        runBenchmark(selectedTest, benchmarkIterations, customMaskLen);
    } else {
        // Run single alignment
        StripedSmithWaterman::Alignment alignment;
        double time = runAlignment(selectedTest, &alignment, customMaskLen);
        
        // Print alignment stats
        PrintAlignment(alignment);
        
        // Visualize the actual alignment
        VisualizeAlignment(alignment, selectedTest.ref, selectedTest.query);
        

        
        std::cout << "Total time taken: " << time << " seconds" << std::endl;
    }
    
    return 0;
}