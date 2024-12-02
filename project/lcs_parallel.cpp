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

class Barrier
{
public:
  explicit Barrier(int num_threads)
      : num_threads_(num_threads), count_(0), generation_(0) {}

  void wait()
  {
    std::unique_lock<std::mutex> lock(mutex_);

    // Get the generation number when entering the barrier
    int gen = generation_;

    // Increment the count of threads that have reached the barrier
    if (++count_ == num_threads_)
    {
      // If the last thread reached, reset for the next generation
      generation_++;
      count_ = 0;
      cv_.notify_all(); // Wake all threads to continue
    }
    else
    {
      // Wait for other threads to reach the barrier
      cv_.wait(lock, [this, gen]
               { return gen != generation_; });
    }
  }

private:
  int num_threads_;
  int count_;
  int generation_;
  std::mutex mutex_;
  std::condition_variable cv_;
};

struct Coords
{
  int row;
  int col;
};

// Derived class for parallel computation of Longest Common Subsequence (LCS)
class LongestCommonSubsequenceParallel : public LongestCommonSubsequence
{
protected:
  int numThreads;     // Number of threads for parallel computation
  Barrier sync_point; // Barrier for thread synchronization
  std::vector<double> thread_times_taken;
  // std::vector<unsigned int> columns_processed;
  std::atomic<unsigned int> current_diagonal;

  double solve_time_taken;
  std::vector<Timer> thread_timers;
  Timer solve_timer;

  std::vector<Coords> thread_coords;

  /**
   * Function to compute a portion of a diagonal.
   *
   * @param diagonal_index Index of the diagonal to compute.
   * @param start_index Starting index within the diagonal for this thread.
   * @param n_elements Number of cells to process in this diagonal.
   */
  void computeDiagonal(const int diagonal_index, const int start_index,
                       const int n_elements)
  {
    // Calculate the starting cell of the diagonal
    int start_row = std::max(0, diagonal_index - length_b + 1);
    int start_col = std::min(diagonal_index, length_b - 1);

    // Adjust starting cell for this thread's portion
    start_row += start_index;
    start_col -= start_index;

    // Iterate through assigned cells in this diagonal
    for (int i = 0; i < n_elements; ++i)
    {
      if (start_row >= 0 && start_row < length_a && start_col >= 0 &&
          start_col < length_b)
      {
        computeCell(start_row + 1,
                    start_col + 1); // Call computeCell (1-based)
      }
      start_row++;
      start_col--;
    }
  }

  void computeColumn(int thread_id, int col)
  {
    // Iterate over column, check left neighbor's coords when necessary.
    int row = 1; // Start at 1 because row 0 is all zeros.

    // Update coords.
    thread_coords[thread_id] = {row, col};

    while (row < matrix_height)
    {
      int left_neighbor_id = thread_id == 0 ? numThreads - 1 : thread_id - 1;

      /* If left neighbor is on a column greater than ours, then that means
      the column to our left is complete and we can safely compute the entirety
      of the column that we're on. */
      /* Busy wait until left neighbor's row is greater than our row.
      Then we know that it is safe to read entry to our left. */
      while (thread_coords[left_neighbor_id].col < col && thread_coords[left_neighbor_id].row <= row)
        ;

      // Compute cell once it is safe to do so.
      computeCell(row, col);
    }
  }

  void solveParallel(int thread_id)
  {
    thread_timers[thread_id].start(); // Start timing for this thread

    // Start computing columns at index 1, because column 0 is all zeros.
    int col = thread_id + 1;
    while (col < matrix_width)
    {
      computeColumn(thread_id, col);
      // Jump to column after the rightmost neighbor.
      col += numThreads;
    }

    thread_times_taken[thread_id] = thread_timers[thread_id].stop(); // Stop timing
  }

  /**
   * Solve LCS using parallel threads.
   */
  void solveParallelDiagonal(int thread_id)
  {
    const int max_diagonals = length_a + length_b + 1;
    thread_timers[thread_id].start(); // Start timing for this thread
    // Process each diagonal one at a time
    while (current_diagonal < max_diagonals)
    {
      int diagonal_index =
          current_diagonal; // Get the next diagonal to process

      // Determine the size of the diagonal
      int diagonal_length =
          std::min({diagonal_index + 1, length_a, length_b,
                    length_a + length_b - diagonal_index - 1});

      // Divide work among threads
      int min_cells_per_thread = diagonal_length / numThreads;
      int excess_cells = diagonal_length % numThreads;

      // Calculate the work range for this thread
      int start_index =
          thread_id * min_cells_per_thread + std::min(thread_id, excess_cells);
      int n_elements =
          min_cells_per_thread + (thread_id < excess_cells ? 1 : 0);

      // Compute this thread's portion of the diagonal
      if (n_elements > 0)
      {
        computeDiagonal(diagonal_index, start_index, n_elements);
        // columns_processed[thread_id] += n_elements; // Update stats
      }

      sync_point.wait(); // Synchronize threads after this diagonal
      if (thread_id == 0)
      {
        // Increment shared diagonal index.
        current_diagonal++;
      }
      sync_point.wait();
    }
    thread_times_taken[thread_id] = thread_timers[thread_id].stop(); // Stop timing
  }

public:
  // Constructor
  LongestCommonSubsequenceParallel(const std::string &sequence_a,
                                   const std::string &sequence_b, int threads)
      : LongestCommonSubsequence(sequence_a, sequence_b),
        numThreads(std::max(1, threads)),
        sync_point(numThreads),
        thread_times_taken(numThreads, 0.0),
        thread_timers(numThreads),
        thread_coords(numThreads),
        // columns_processed(numThreads, 0),
        current_diagonal(0)
  {
  }

  // Main solve method
  virtual void solve() override
  {
    solve_timer.start();                             // Start overall timing
    const int n_diagonals = length_a + length_b - 1; // Total diagonals

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
    printf("-_-_-_-_-_-_-_ LCS Parallel Statistics _-_-_-_-_-_-_-\n\n");
    // printf("Thread ID || Columns Computed || Time Taken\n");
    // for (int id = 0; id < numThreads; id++)
    // {
    //   printf("%i || %u || %lf\n", id, columns_processed[id],
    //          thread_times_taken[id]);
    // }
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
  // lcs.print();
  // printf("\n");
  lcs.printThreadStats();
  printf("Total time taken: %lf\n", total_time_taken);

  return 0;
}
