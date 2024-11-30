#include <algorithm>
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
//  strings and finds the longest common subsequence between the the two. This
//  version of the algorithm works with threads to complete the calculations in
//  parallel
// ***

class Barrier
{
public:
  explicit Barrier(int n_threads)
      : n_threads(n_threads), count(0), generation(0) {}

  void wait()
  {
    std::unique_lock<std::mutex> lock(mutex);

    // Get the generation number when entering the barrier
    int gen = generation;

    // Increment the count of threads that have reached the barrier
    if (++count == n_threads)
    {
      // If the last thread reached, reset for the next generation
      generation++;
      count = 0;
      cv.notify_all(); // Wake all threads to continue
    }
    else
    {
      // Wait for other threads to reach the barrier
      cv.wait(lock, [this, gen]
              { return gen != generation; });
    }
  }

private:
  int n_threads;
  int count;
  int generation;
  std::mutex mutex;
  std::condition_variable cv;
};

// Converted class from Base LongestCommonSubsequence (LCS) for parallel verion
class LongestCommonSubsequenceParallel : public LongestCommonSubsequence
{
protected:
  int numThreads;     // Number of threads for parallel computation
  Barrier sync_point; // Barrier for thread synchronization
  std::vector<double> thread_times_taken;
  std::vector<unsigned int> columns_processed;

  double solve_time_taken;
  Timer thread_timer;
  Timer solve_timer;

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
    // Calculate starting cell of the diagonal
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

  /**
   * Solve LCS using parallel threads.
   */
  void solveParallel()
  {
    solve_timer.start();
    const int n_diagonals = length_a + length_b - 1; // Total diagonals

    // Process each diagonal one at a time
    for (int diagonal_index = 0; diagonal_index < n_diagonals;
         ++diagonal_index)
    {
      // Determine the size of the diagonal
      int diagonal_length =
          std::min({diagonal_index + 1, length_a, length_b,
                    length_a + length_b - diagonal_index - 1});

      // Divide work among threads
      int min_cells_per_thread = diagonal_length / numThreads;
      int excess_cells = diagonal_length % numThreads;

      // Launch threads
      std::vector<std::thread> threads;
      for (int thread_id = 0; thread_id < numThreads; ++thread_id)
      {
        threads.emplace_back([&, thread_id]()
                             {
          thread_timer.start();
          // Calculate work range for this thread
          int start_index = thread_id * min_cells_per_thread +
                            std::min(thread_id, excess_cells);
          int n_elements =
              min_cells_per_thread + (thread_id < excess_cells ? 1 : 0);
          computeDiagonal(diagonal_index, start_index, n_elements);

          // Update columns processed and time taken for this thread
          columns_processed[thread_id] += n_elements;
          thread_times_taken[thread_id] += thread_timer.stop();
          // Synchronize threads after this diagonal
          sync_point.wait(); });
      }

      // Join threads
      for (auto &thread : threads)
      {
        thread.join();
      }
    }

    solve_time_taken = solve_timer.stop();
    // After processing all diagonals, determine the LCS
    determineLongestCommonSubsequence(); // Base class function to get LCS
  }

public:
  LongestCommonSubsequenceParallel(const std::string &sequence_a,
                                   const std::string &sequence_b, int threads)
      : LongestCommonSubsequence(sequence_a, sequence_b),
        numThreads(std::max(1, threads)),
        sync_point(numThreads),
        thread_times_taken(numThreads, 0.0),
        columns_processed(numThreads, 0) {}

  void printThreadStats()
  {
    printf("-_-_-_-_-_-_-_ LCS Parallel Statistics _-_-_-_-_-_-_-\n\n");
    printf("Thread ID || Columns Computed || Time Taken\n");
    for (int id = 0; id < numThreads; id++)
    {
      printf("%i || %u || %lf\n", id, columns_processed[id],
             thread_times_taken[id]);
    }
    printf("Solve Time Taken: %f\n", solve_time_taken);
  }

  virtual void solve() override { solveParallel(); }
};

int main(int argc, char *argv[])
{
  Timer program_timer;
  double total_time_taken = 0.0;
  program_timer.start();

  // Create command line options objects
  cxxopts::Options options("lcs_parallel",
                           "LCS program for CMPT 431 project using threads");

  options.add_options(
      "inputs",
      {{"n_threads", "Number of threads for the program",
        cxxopts::value<int>()->default_value("1")},
       {"sequence_a", "First sequence file", cxxopts::value<std::string>()},
       {"sequence_b", "Second sequence file", cxxopts::value<std::string>()}});

  // Parse command-line options
  auto command_options = options.parse(argc, argv);
  int n_threads = command_options["n_threads"].as<int>();

  std::string sequence_a = command_options["sequence_a"].as<std::string>();
  std::string sequence_b = command_options["sequence_b"].as<std::string>();

  // Validate input
  if (sequence_a.empty() || sequence_a.empty())
  {
    std::cerr << "Error: Sequences cannot be empty.\n";
    return 1;
  }

  if (n_threads <= 0)
  {
    std::cerr << "Error: Number of threads must be greater than zero.\n";
    return 1;
  }

  printf("_-_-_-_-_-_-_-_-_ LCS Parallel _-_-_-_-_-_-_-_-_\n");
  printf("Number of Threads: %d\n", n_threads);
  printf("Initializing Parallel Solver\n");

  // Create and solve the LCS problem
  LongestCommonSubsequenceParallel lcs(sequence_a, sequence_b, n_threads);

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
