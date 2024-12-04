#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// Include necessary headers
#include "cxxopts.hpp"  // Command-line option parser library
#include "lcs.h"  // Header file containing the LongestCommonSubsequence class

// ***
//  This is the parallel version of the LCS program that calculates the longest
//  common subsequence between two sequences. It utilizes multiple threads to
//  perform computations in parallel.
// ***

// Derived class for parallel computation of Longest Common Subsequence (LCS)
class LongestCommonSubsequenceParallel : public LongestCommonSubsequence {
 protected:
  int numThreads;  // Number of threads to be used for parallel computation
  std::vector<double>
      thread_times_taken;  // Vector to store the time taken by each thread

  double solve_time_taken;  // Time taken to compute the overall LCS
  std::vector<Timer>
      thread_timers;  // Timer objects to measure each thread's execution time
  Timer solve_timer;  // Timer for the overall solve process

  std::vector<std::atomic<int>>
      thread_row_indices;  // Atomic indices to ensure safe row updates by each
                           // thread

  std::condition_variable
      cv;            // Condition variable used for thread synchronization
  std::mutex mutex;  // Mutex to protect the condition variable and ensure safe
                     // synchronization

  // Function executed by each thread to compute the LCS for a portion of the
  // matrix
  void solveParallel(int thread_id) {
    thread_timers[thread_id].start();  // Start the timer for the current thread

    int min_cols_per_thread =
        length_b / numThreads;  // Minimum columns per thread
    int excess_cols =
        length_b %
        numThreads;  // Extra columns that can't be evenly distributed

    int start_col;
    int n_cols = min_cols_per_thread;
    if (thread_id < excess_cols) {
      start_col =
          thread_id * (min_cols_per_thread +
                       1);  // Assign extra column to threads with a smaller ID
      n_cols++;
    } else {
      start_col = (thread_id * min_cols_per_thread) +
                  excess_cols;  // Distribute the remaining columns evenly
    }
    start_col +=
        1;  // Offset by 1 because the first column is initialized to zero
    int end_col = std::min(
        start_col + n_cols - 1,
        matrix_width - 1);  // Calculate the ending column for the thread

    int row, col;
    thread_row_indices[thread_id] = 1;  // Set initial row index for the thread
    for (row = 1; row < matrix_height; row++) {
      // If this is not the leftmost thread, wait until the thread to the left
      // finishes processing the row
      if (thread_id > 0) {
        if (thread_row_indices[thread_id - 1] <= row) {
          std::unique_lock<std::mutex> ulock(
              mutex);  // Lock the mutex to protect shared data
          // Wait until the thread on the left is done with the current row
          cv.wait(ulock, [this, &thread_id, &row] {
            return thread_row_indices[thread_id - 1] > row;
          });
          ulock.unlock();  // Unlock after waiting
        }
      }

      // Once the left neighbor is done, process the current row for the
      // assigned columns
      for (col = start_col; col <= end_col; col++) {
        computeCell(row, col);  // Compute the LCS value for the current cell
      }

      thread_row_indices[thread_id] +=
          1;  // Update the row index for this thread

      // Notify other threads that they can wake up and continue processing
      cv.notify_all();
    }

    thread_times_taken[thread_id] =
        thread_timers[thread_id]
            .stop();  // Stop the timer for the current thread
  }

 public:
  // Constructor that initializes the LCS solver with the sequences and number
  // of threads
  LongestCommonSubsequenceParallel(const std::string &sequence_a,
                                   const std::string &sequence_b, int threads)
      : LongestCommonSubsequence(sequence_a, sequence_b),
        numThreads(std::max(1, threads)),  // Ensure at least one thread
        thread_times_taken(numThreads, 0.0),
        thread_timers(numThreads),
        thread_row_indices(numThreads) {}

  // Override the solve method to compute the LCS in parallel using threads
  virtual void solve() override {
    solve_timer.start();  // Start the overall timer for LCS computation

    // Launch a vector of threads to perform parallel LCS computation
    std::vector<std::thread> threads(numThreads);
    for (int i = 0; i < numThreads; i++) {
      threads[i] = std::thread(&LongestCommonSubsequenceParallel::solveParallel,
                               this, i);  // Start each thread
    }

    // Wait for all threads to finish their work
    for (int i = 0; i < numThreads; i++) {
      threads[i].join();  // Join each thread to ensure they all complete before
                          // proceeding
    }

    solve_time_taken = solve_timer.stop();  // Stop the overall timer

    // After all threads have finished, determine the LCS based on the matrix
    determineLongestCommonSubsequence();
  }

  // Print statistics related to each thread's execution time
  void printThreadStats() {
    printf("-_-_-_-_-_-_-_ LCS Parallel Statistics _-_-_-_-_-_-_-\n\n");
    printf("Thread ID || Time Taken\n");
    for (int id = 0; id < numThreads; id++) {
      printf("%9d || %lf\n", id,
             thread_times_taken[id]);  // Print each thread's execution time
    }
    printf(
        "Solve Time Taken: %f\n",
        solve_time_taken);  // Print the total time for solving the LCS problem
  }
};

int main(int argc, char *argv[]) {
  Timer program_timer;  // Timer for measuring total program execution time
  double total_time_taken = 0.0;
  program_timer.start();  // Start the program timer

  // Create command-line options for input parsing
  cxxopts::Options options("lcs_parallel",
                           "LCS program for CMPT 431 project using threads");

  // Define the command-line arguments (number of threads and two input
  // sequences)
  options.add_options(
      "inputs",
      {{"n_threads", "Number of threads for the program",
        cxxopts::value<int>()->default_value("1")},  // Default to 1 thread
       {"sequence_a", "First sequence.",
        cxxopts::value<std::string>()},  // First input sequence
       {"sequence_b", "Second sequence.",
        cxxopts::value<std::string>()}});  // Second input sequence

  // Parse the command-line options
  auto command_options = options.parse(argc, argv);
  int n = command_options["n_threads"]
              .as<int>();  // Get the number of threads from user input

  std::string seqA = command_options["sequence_a"]
                         .as<std::string>();  // Get the first sequence
  std::string seqB = command_options["sequence_b"]
                         .as<std::string>();  // Get the second sequence

  // Validate that the sequences are non-empty
  if (seqA.empty() || seqB.empty()) {
    std::cerr << "Error: Sequences cannot be empty.\n";
    return 1;
  }

  // Validate that the number of threads is positive
  if (n <= 0) {
    std::cerr << "Error: Number of threads must be greater than zero.\n";
    return 1;
  }

  // Print basic information about the parallel LCS run
  printf("_-_-_-_-_-_-_-_-_ LCS Parallel _-_-_-_-_-_-_-_-_\n");
  printf("Number of Threads: %d\n", n);
  printf("Initializing Parallel Solver\n");

  // Create and solve the LCS problem with the specified number of threads
  LongestCommonSubsequenceParallel lcs(seqA, seqB, n);

  printf("Starting LCS Parallel Solver\n");
  lcs.solve();  // Compute the LCS using parallel threads
  total_time_taken =
      program_timer.stop();  // Stop the program timer after solving
  printf("LCS Parallel Solver Finished\n\n");

  // Print the results and performance statistics
  printf("-_-_-_-_-_-_-_ LCS Parallel Results _-_-_-_-_-_-_-\n");
  lcs.printThreadStats();
  printf("Total time taken: %lf\n",
         total_time_taken);  // Print the total time taken by the program

  return 0;  // Return successful exit code
}
