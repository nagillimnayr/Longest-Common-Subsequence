

#include <algorithm> // std::max, std::min
#include <iostream>
#include <mpi.h>

#include "../cxxopts.hpp"
#include "lcs_distributed.h"

/**
 * If the specific longest common subsequence is required, then the sub-matrices
 * can be gathered together once all of the entries have been computed.
 *
 * This version of the distributed LCS implementation tries to improve the
 * performance of the acquisition of the LCS subsequence by performing the
 * backtrace within the local processes and then using MPI send/receive to
 * let the next process know which index to pick up the task from.
 *
 * */
class LCSDistributed : public LCSDistributedBase
{
protected:
  std::string global_sequence_b;

  /* Broadcast the length of the LCS from the rightmost process to every other
  process.*/
  void broadcastLCSLength()
  {
    if (world_rank == world_size - 1)
    {
      lcs_length = LongestCommonSubsequence::getLongestSubsequenceLength();
    }

    MPI_Bcast(&lcs_length, 1, MPI_INT, world_size - 1, MPI_COMM_WORLD);
  }

  virtual void determineLongestCommonSubsequence() override
  {
    /* Each process will need to know the length of the LCS so that it can
    allocate the necessary buffer space. */
    broadcastLCSLength();

    char *lcs_buffer;
    lcs_buffer = new char[lcs_length + 1];
    lcs_buffer[lcs_length] = '\0';
    int index = lcs_length - 1;

    int comm_buffer[2]; // Used to transmit row index and LCS string index.
    int row = matrix_height - 1;
    /* Each process except the rightmost will have to wait for its neighbor to
    the right to finish. */

    if (world_rank != world_size - 1)
    {
      /* Rightside neighbor must tell us which row to start at. */
      MPI_Recv(
          &comm_buffer,
          2,
          MPI_INT,
          world_rank + 1,
          0,
          MPI_COMM_WORLD,
          MPI_STATUS_IGNORE);
      row = comm_buffer[0];
      index = comm_buffer[1];

      /* Next, the process needs to receive the partially completed LCS from
      the neighbor to the right. */
      MPI_Recv(
          lcs_buffer,
          lcs_length,
          MPI_CHAR,
          world_rank + 1,
          0,
          MPI_COMM_WORLD,
          MPI_STATUS_IGNORE);
    }

    /* Once we have acquired the necessary data from our neighbor, we can
    continue the trace. Always starting from the leftmost column. */
    int col = matrix_width - 1;

    int current, top, left, top_left;
    top = left = top_left = 0;

    while (row > 0 && col > 0 && index >= 0)
    {
      current = matrix[row][col];
      top_left = matrix[row - 1][col - 1];
      top = matrix[row - 1][col];
      left = matrix[row][col - 1];

      if (top_left == current)
      {
        // Go to entry to the top-left.
        row--;
        col--;
        continue;
      }

      /* If the elements above, to the left, and diagonally to the top left
      are all the same,  */
      if (top_left == top && top_left == left)
      {
        lcs_buffer[index] = sequence_a[row - 1];
        index--;
        // Go to entry to the top-left.
        row--;
        col--;
        continue;
      }

      /* If the entry to the top left is lower than the current entry but is not
      equal to the entries above and to the left, then either the one above or
      the one to the left must be the same as the current. */
      if (top == current)
      {
        // Go to the entry above.
        row--;
        continue;
      }
      else
      {
        // If it wasn't the one above, it must be the one to the left.
        col--;
      }
    }

    /* Once this process has finished tracing its sub-matrix, pass the work on
    to the next process. */
    if (world_rank > 0)
    {
      comm_buffer[0] = row;
      comm_buffer[1] = index;
      MPI_Send(
          &comm_buffer,
          2,
          MPI_INT,
          world_rank - 1,
          0,
          MPI_COMM_WORLD);

      MPI_Send(
          lcs_buffer,
          lcs_length,
          MPI_CHAR,
          world_rank - 1,
          0,
          MPI_COMM_WORLD);
    }

    if (world_rank == 0)
    {
      total_time_taken = total_timer.stop();
      longest_common_subsequence = lcs_buffer;
    }
    delete[] lcs_buffer;
  }

  virtual void solve() override
  {
    LCSDistributedBase::solve();
    determineLongestCommonSubsequence();
  }

public:
  LCSDistributed(
      const std::string &sequence_a,
      const std::string &sequence_b,
      const int world_size,
      const int world_rank,
      int *start_cols,
      int *sub_str_widths,
      const std::string &global_sequence_b)
      : LCSDistributedBase(sequence_a, sequence_b, world_size, world_rank, start_cols, sub_str_widths),
        global_sequence_b(global_sequence_b)
  {

    this->solve();
  }

  virtual ~LCSDistributed()
  {
  }

  virtual int getLongestSubsequenceLength() override
  {
    return lcs_length;
  }

  virtual void printInfo() override
  {
    printLCSLength();
    // std::cout << "Longest common subsequence: " << longest_common_subsequence << "\n";
  }
};

int main(int argc, char *argv[])
{
  cxxopts::Options options("lcs_distributed",
                           "Distributed LCS implementation using MPI.");

  options.add_options(
      "inputs",
      {{"sequence_a", "First input sequence.", cxxopts::value<std::string>()},
       {"sequence_b", "Second input sequence.",
        cxxopts::value<std::string>()}});

  auto command_options = options.parse(argc, argv);

  std::string sequence_a = command_options["sequence_a"].as<std::string>();
  std::string sequence_b = command_options["sequence_b"].as<std::string>();

  MPI_Init(NULL, NULL);

  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if (world_rank == 0)
  {
    printf("-------------------- LCS Distributed --------------------\n");
    printf("n_processes: %d\n\n", world_size);
  }
  MPI_Barrier(MPI_COMM_WORLD);

  int length_a = sequence_a.length();
  int length_b = sequence_b.length();

  const int min_n_cols_per_process = length_b / world_size;
  const int excess = length_b % world_size;

  /* We need to keep track of which columns are mapped to which processes so
  we can gather them together again at the end with MPI_Gatherv.*/
  int *sub_str_widths = new int[world_size];
  int *start_cols = new int[world_size];
  for (int rank = 0; rank < world_size; rank++)
  {
    int start_col, n_cols;
    n_cols = min_n_cols_per_process;
    if (rank < excess)
    {
      start_col = rank * (min_n_cols_per_process + 1);
      n_cols++;
    }
    else
    {
      start_col = (rank * min_n_cols_per_process) + excess;
    }

    start_cols[rank] = start_col;
    sub_str_widths[rank] = n_cols;
  }

  int start_col = start_cols[world_rank];
  int n_cols = sub_str_widths[world_rank];

  // Divide up sequence B.
  std::string local_sequence_b = sequence_b.substr(start_col, n_cols);

  LCSDistributed lcs(
      sequence_a,
      local_sequence_b,
      world_size,
      world_rank,
      start_cols,
      sub_str_widths,
      sequence_b);

  // Print solution.
  lcs.print();

  delete[] sub_str_widths;
  delete[] start_cols;

  MPI_Finalize();

  return 0;
}
