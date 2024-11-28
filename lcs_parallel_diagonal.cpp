// CMPT 431 D100 - Parallel LCS Program
// Name: Adam Pywell

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
class LongestCommonSubsequenceParallel : public LongestCommonSubsequence {
 private:
  int numThreads;             // Number of threads for parallel computation
  std::barrier<> sync_point;  // Barrier for thread synchronization

  /**
   * Function to compute a portion of a diagonal.
   *
   * @param diagonal_index Index of the diagonal to compute.
   * @param start_index Starting index within the diagonal for this thread.
   * @param n_elements Number of cells to process in this diagonal.
   */
  void computeSubDiagonal(const int diagonal_index, const int start_index,
                          const int n_elements) {
    // Calculate starting cell of the diagonal
    int start_col =
        std::max(0, diagonal_index - static_cast<int>(length_a) + 1);
    int start_row = diagonal_index - start_col;

    // Adjust starting cell for this thread's portion
    start_row += start_index;
    start_col -= start_index;

    // Iterate through assigned cells in this diagonal
    for (int i = 0; i < n_elements; ++i) {
      if (start_row >= 0 && start_row < length_a && start_col >= 0 &&
          start_col < length_b) {
        computeCell(start_row + 1,
                    start_col + 1);  // Call computeCell (1-based)
      }
      start_row++;
      start_col--;
    }
  }

  /**
   * Solve LCS using parallel threads.
   */
  void solveParallel() {
    const int n_diagonals = length_a + length_b - 1;  // Total diagonals

    // Process each diagonal one at a time
    for (int diagonal_index = 0; diagonal_index < n_diagonals;
         ++diagonal_index) {
      // Determine the size of the diagonal
      int diagonal_length =
          std::min({diagonal_index + 1, length_a, length_b,
                    length_a + length_b - diagonal_index - 1});

      // Divide work among threads
      int min_cells_per_thread = diagonal_length / numThreads;
      int excess_cells = diagonal_length % numThreads;

      // Launch threads
      std::vector<std::thread> threads;
      for (int thread_id = 0; thread_id < numThreads; ++thread_id) {
        threads.emplace_back([&, thread_id]() {
          // Calculate work range for this thread
          int start_index = thread_id * min_cells_per_thread +
                            std::min(thread_id, excess_cells);
          int n_elements =
              min_cells_per_thread + (thread_id < excess_cells ? 1 : 0);

          computeSubDiagonal(diagonal_index, start_index, n_elements);
          sync_point
              .arrive_and_wait();  // Synchronize threads after this diagonal
        });
      }

      // Join threads
      for (auto &thread : threads) {
        thread.join();
      }
    }

    // After processing all diagonals, determine the LCS
    determineLongestCommonSubsequence();  // Base class function to get LCS
  }

 public:
  LongestCommonSubsequenceParallel(const std::string &sequence_a,
                                   const std::string &sequence_b, int threads)
      : LongestCommonSubsequence(sequence_a, sequence_b),
        numThreads(std::max(1, threads)),
        sync_point(numThreads) {}

  void solve() override { solveParallel(); }
  ~LongestCommonSubsequenceParallel() = default;
};

int main(int argc, char *argv[]) {
  // Create command line options objects
  cxxopts::Options options("lcs_parallel",
                           "LCS program for CMPT 431 project using threads");

  options.add_options(
      "inputs",
      {{"nThreads", "Number of threads for the program",
        cxxopts::value<int>()->default_value("1")},
       {"sequenceFile1", "First sequence file", cxxopts::value<std::string>()},
       {"sequenceFile2", "Second sequence file",
        cxxopts::value<std::string>()}});

  // Parse command-line options
  auto command_options = options.parse(argc, argv);
  int n = command_options["nThreads"].as<int>();
  std::string seqOne = command_options["sequenceFile1"].as<std::string>();
  std::string seqTwo = command_options["sequenceFile2"].as<std::string>();

  // Validate input
  if (seqOne.empty() || seqTwo.empty()) {
    std::cerr << "Error: Sequences cannot be empty.\n";
    return 1;
  }

  if (n <= 0) {
    std::cerr << "Error: Number of threads must be greater than zero.\n";
    return 1;
  }

  // Create and solve the LCS problem
  LongestCommonSubsequenceParallel lcs(seqOne, seqTwo, n);

  // Solve LCS Problem
  lcs.solve();

  // Print the result
  lcs.print();

  return 0;
}
