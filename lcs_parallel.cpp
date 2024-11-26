// CMPT 431 D100 - Parallel LCS Program
// Name: Adam Pywell
// Name: Ryan Milligan

// Imported Libraries
#include <algorithm>
#include <barrier>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "cxxopts.hpp"
#include "lcs.h"

// ***
//  This is the parallel version of the LCS program that takes two separate
//  strings and finds the longs common subsequence between the the two. This
//  version of the algorithm works with threads to complete the calculations in
//  parallel
// ***

// Converted class from Base LongestCommonSubsequence (LCS) for parallel verion
class LongestCommonSubsequenceParallel : public LongestCommonSubsequence
{
private:
  int numThreads;            // Number of threads used for multihreaded execution
  std::barrier<> sync_point; // Barrier for thread synchronization after each
                             // diagonal computation chunk

  // Function to compute all cells in a given diagonal
  void computeDiagonal(int numThreads, const int diagonal)
  {
    // Calculate the range of columns and rows for the current diagonal
    int start_col = std::max(0, diagonal - static_cast<int>(length_a) + 1);
    int start_row = diagonal - start_col;
    int end_col = std::min(diagonal + 1, static_cast<int>(length_b));

    // Iterate through cell in the current diagonal
    for (int col = start_col; col < end_col; ++col)
    {
      int row = diagonal - col;
      computeCell(row, col); // Compute the LCS value for the current cell
    }
  }

  // Calculate the LCS by using highest order diagonal in parallel
  // Threads synchronize at the end of each diagonal computation
  void solveParallel()
  {
    const int max_diagonals =
        length_a + length_b - 1; // Total number of diagonals for process

    // Loop through diagonals in sets, with one set of diagonals processed by
    // all threads
    for (int diagonal = 0; diagonal < max_diagonals; diagonal += numThreads)
    {
      // Create vector to hold list of thread objects
      std::vector<std::thread> threads;
      // Start threads to compute diagonals in parallel
      for (int t = 0; t < numThreads; t++)
      {
        threads.emplace_back([&]()
                             {
          // Each thread processes a subset of diagonals
          for (int d = diagonal + t; d < max_diagonals; d += numThreads) {
            computeDiagonal(t, d);  // Compute cells for the assigned diagonal
            sync_point.arrive_and_wait();  // Synchronize all threads
          } });
      }

      // Join all threads together, finish parallel portion for this computation
      for (std::thread &thread : threads)
      {
        thread.join();
      }
    }

    // Trace back through matrix to determine the LCS
    determineLongestCommonSubsequence();
  }

public:
  LongestCommonSubsequenceParallel(const std::string &seqOne,
                                   const std::string &seqTwo, int n)
      : LongestCommonSubsequence(seqOne, seqTwo), // Call base class constructor
        numThreads(n),                            // Set number of threads for parallel class
        sync_point(n)                             // Set amount of workers for barrier object
  {
  }

  // Override solve function to invoke parallel version
  void solve() override { solveParallel(); }
};

// Main function to create parallel class instance, send sequences to solve
// and call class print function for results and matrix to terminal
int main(int argc, char *argv[])
{
  // Create command line options objects
  cxxopts::Options options("lcs_parallel",
                           "lcs program for cmpt 431 project using threads");

  options.add_options("inputs",
                      {{"nThreads", "Number of threads for the program",
                        cxxopts::value<int>()->default_value("1")},
                       {"sequenceFile1", "Number of threads for the program",
                        cxxopts::value<std::string>()},
                       {"sequenceFile2", "Number of threads for the program",
                        cxxopts::value<std::string>()}});

  // Variables to hold command line values parsed from the command_options
  auto command_options = options.parse(argc, argv);
  int n = command_options["nThreads"].as<int>();
  std::string seqOne = command_options["sequenceFile1"].as<std::string>();
  std::string seqTwo = command_options["sequenceFile2"].as<std::string>();

  // Create instance of the Parallel LCS solver class
  LongestCommonSubsequenceParallel lcs(seqOne, seqTwo, n);

  // Solve the LCS problem and print results (ie. Values and Matrix)
  lcs.solve(); // Perform parallel LCS computation
  lcs.print(); // Print resulting LCS and DP matrix

  return 0;
}
