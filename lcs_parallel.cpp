//CMPT 431 D100 - Parallel LCS Program
//Name: Adam Pywell - 301 335 414
//Name: Ryan 

#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
#include <barrier>
#include <string>
#include "cxxopts.hpp"
#include "lcs.h"


class LongestCommonSubSequenceParallel : public LongestCommonSubsequence { 
private:
        int numThreads;
        std::barrier<> sync_point;

    void computeDiagonal(int numThreads, const int diagonal) {
        int start_col = std::max(0, diagonal - static_cast<int>(length_a) + 1);
        int start_row = diagonal - start_col;
        int end_col = std::min(diagonal + 1, static_cast<int>(length_b));

        for (int col = start_col; col < end_col; ++col) {
            int row = diagonal - col;
            computeCell(row, col);
        }
    }

    void solveParallel() {
        const int max_diagonals = length_a + length_b - 1;

        for (int diagonal = 0; diagonal < max_diagonals; diagonal += numThreads) {
            std::vector<std::thread> threads;

            for (int t = 0; t < numThreads; t++) {
                threads.emplace_back([&]() {
                    for(int d = diagonal + t; d < max_diagonals; d += numThreads) {
                        computeDiagonal(t,d);
                        sync_point.arrive_and_wait();
                    }
                }); 
            }

            for (std::thread &thread : threads) {
                thread.join();
            }
        } 

        determineLongestCommonSubsequence(); 
    }

public:
    LongestCommonSubSequenceParallel(const std::string &seqOne, const std::string &seqTwo, int n)
        : LongestCommonSubsequence(seqOne, seqTwo),
        numThreads(n),
        sync_point(n) {} 

     void solve() override {
        solveParallel();
    }

};

// Main function to create parallel class instance, send sequences to solve
// and print results to terminal
int main(int argc, char* argv[]) {
    cxxopts::Options options("lcs_parallel", "lcs program for cmpt 431 project using threads");

    options.add_options(
        "inputs", {
            {"nThreads", "Number of threads for the program", cxxopts::value<int>()->default_value("1")},
            {"sequenceFile1", "Number of threads for the program", cxxopts::value<std::string>()},
            {"sequenceFile2", "Number of threads for the program", cxxopts::value<std::string>()}
    });
    auto command_options = options.parse(argc, argv); 
    int n = command_options["nThreads"].as<int>();
    std::string seqOne = command_options["sequenceFile1"].as<std::string>();
    std::string seqTwo = command_options["sequenceFile2"].as<std::string>();

    LongestCommonSubSequenceParallel lcs(seqOne,seqTwo, n);

    lcs.solve();
    lcs.print();

    return 0;
}
