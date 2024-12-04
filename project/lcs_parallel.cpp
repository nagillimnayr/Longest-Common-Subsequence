#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "cxxopts.hpp"
#include "lcs.h"

// ***
//  This is the parallel version of the LCS program that takes two separate
//  strings and finds the longest common subsequence between them. This version
//  uses threads to perform calculations in parallel.
// ***

// Derived class for parallel computation of Longest Common Subsequence (LCS)
class LongestCommonSubsequenceParallel : public LongestCommonSubsequence
{
protected:
  int numThreads; // Number of threads for parallel computation
  std::vector<double> thread_times_taken;

  double solve_time_taken;
  std::vector<Timer> thread_timers;
  Timer solve_timer;

  std::vector<std::atomic<int>> thread_row_indices; /* To let other threads know which row a
  particular thread is on. */

  std::condition_variable cv; /* Used to signal next thread to the right
  that they may begin their row. */
  std::mutex mutex;           /* For the condition variable. */

  void solveParallel(int thread_id)
  {
    thread_timers[thread_id].start(); // Start timing for this thread

    int min_cols_per_thread = length_b / numThreads;
    int excess_cols = length_b % numThreads;

    int start_col;
    int n_cols = min_cols_per_thread;
    if (thread_id < excess_cols)
    {
      start_col = thread_id * (min_cols_per_thread + 1);
      n_cols++;
    }
    else
    {
      start_col = (thread_id * min_cols_per_thread) + excess_cols;
    }
    start_col += 1; // Offset by 1 because first column is all zeros.
    int end_col = std::min(start_col + n_cols - 1, matrix_width - 1);
    int row, col;
    thread_row_indices[thread_id] = 1;
    for (row = 1; row < matrix_height; row++)
    {
      /* If not the leftmost thread, check if thread to the left is on a row
      greater than ours. If not, busy wait until they are. */
      if (thread_id > 0)
      {
        if (thread_row_indices[thread_id - 1] <= row)
        {
          std::unique_lock<std::mutex> ulock(mutex);
          /* Wait until thread to the left is on the row after us.*/
          cv.wait(ulock, [this, &thread_id, &row]()
                  { return thread_row_indices[thread_id - 1] > row; });
          ulock.unlock();
        }
      }

      /* Once the neighbor to the left is on the row after us, we can freely
      compute our segment of the row. */
      for (col = start_col; col <= end_col; col++)
      {
        computeCell(row, col);
      }

      /* This needs to be updated at the end of the loop to ensure that the last
      update is recorded. */
      thread_row_indices[thread_id] += 1;

      /* Signal other threads to wake up. */
      cv.notify_all();
    }

    thread_times_taken[thread_id] = thread_timers[thread_id].stop(); // Stop timing
  }

public:
  // Constructor
  LongestCommonSubsequenceParallel(const std::string &sequence_a,
                                   const std::string &sequence_b, int threads)
      : LongestCommonSubsequence(sequence_a, sequence_b),
        numThreads(std::max(1, threads)),
        thread_times_taken(numThreads, 0.0),
        thread_timers(numThreads),
        thread_row_indices(numThreads)
  {
  }

  // Main solve method
  virtual void solve() override
  {
    solve_timer.start(); // Start overall timing
    // Launch threads
    std::vector<std::thread> threads(numThreads);

    for (int i = 0; i < numThreads; i++)
    {
      threads[i] = std::thread(&LongestCommonSubsequenceParallel::solveParallel,
                               this, i);
    }

    for (int i = 0; i < numThreads; i++)
    {
      threads[i].join();
    }

    solve_time_taken = solve_timer.stop(); // Stop overall timing

    // After processing all diagonals, determine the LCS
    determineLongestCommonSubsequence();
  }

  // Print thread-level statistics
  void printThreadStats()
  {
    printf("\n-_-_-_-_-_-_-_ LCS Parallel Statistics _-_-_-_-_-_-_-\n\n");
    printf("Thread ID || Time Taken\n");
    for (int id = 0; id < numThreads; id++)
    {
      printf("%9d || %lf\n", id, thread_times_taken[id]);
    }
    printf("Solve Time Taken: %f\n", solve_time_taken);
  }
};

int main(int argc, char *argv[])
{
  Timer program_timer;
  double total_time_taken = 0.0;
  program_timer.start();

  // Create command-line options object
  cxxopts::Options options("lcs_parallel",
                           "LCS program for CMPT 431 project using threads");

  options.add_options(
      "inputs",
      {{"n_threads", "Number of threads for the program",
        cxxopts::value<int>()->default_value("1")},
       {"sequence_a", "First sequence.", cxxopts::value<std::string>()},
       {"sequence_b", "Second sequence.", cxxopts::value<std::string>()}});

  // Parse command-line options
  auto command_options = options.parse(argc, argv);
  int n = command_options["n_threads"].as<int>();

  std::string seqA = command_options["sequence_a"].as<std::string>();
  std::string seqB = command_options["sequence_b"].as<std::string>();

  // Validate input
  if (seqA.empty() || seqB.empty())
  {
    std::cerr << "Error: Sequences cannot be empty.\n";
    return 1;
  }

  if (n <= 0)
  {
    std::cerr << "Error: Number of threads must be greater than zero.\n";
    return 1;
  }

  printf("_-_-_-_-_-_-_-_-_ LCS Parallel _-_-_-_-_-_-_-_-_\n");
  printf("Number of Threads: %d\n", n);
  printf("Initializing Parallel Solver\n");

  // Create and solve the LCS problem
  LongestCommonSubsequenceParallel lcs(seqA, seqB, n);

  // Solve LCS Problem
  printf("Starting LCS Parallel Solver\n");
  lcs.solve();
  total_time_taken = program_timer.stop();
  printf("LCS Parallel Solver Finished\n\n");

  // Print the result
  printf("-_-_-_-_-_-_-_ LCS Parallel Results _-_-_-_-_-_-_-\n");
  lcs.print();
  lcs.printThreadStats();
  printf("Total time taken: %lf\n", total_time_taken);

  return 0;
}
