

#include <algorithm> // std::max, std::min
#include <iostream>
#include <mpi.h>

#include "../cxxopts.hpp"
#include "lcs_distributed.h"

/**
 * If the specific longest common subsequence is required, then the sub-matrices
 * can be gathered together once all of the entries have been computed.
 *
 * This version of the distributed LCS implementation use MPI_Gatherv() to
 * gather together all of the sub-matrices at the end so that the combined
 * matrix can be back-traced to determine the specific LCS subsequence. This
 * approach incurs a significant amount of overhead, as MPI_Gatherv() is called
 * for each row of the matrix.
 *
 * */
class LCSDistributedGather : public LCSDistributedBase
{
protected:
  int **local_matrix;
  int **global_matrix;

  std::string global_sequence_b;

  /** Gathers the sub-matrices from each process together in the root process. */
  void gather()
  {
    // printPerProcessMatrix();

    int global_matrix_width = global_sequence_b.length() + 1;

    // Gather all of the data into the root process:
    if (world_rank == 0)
    {

      // Allocate space for the combined matrix.
      global_matrix = new int *[matrix_height];
      for (int row = 0; row < matrix_height; row++)
      {
        global_matrix[row] = new int[global_matrix_width];
        // Fill first column with zeros.
        global_matrix[row][0] = 0;
      }
      for (int col = 0; col < global_matrix_width; col++)
      {
        // Fill first row with zeros.
        global_matrix[0][col] = 0;
      }

      /* Sub-matrices may be of different widths, and because the matrix is
      divided column-wise, passing the entire sub-matrix with MPI_Gather would
      not properly order the combined matrix.

      However, if we call MPI_Gatherv for each row, the resultant ordering
      should be correct. */

      // We can skip the first row since it is all zeros.
      for (int row = 1; row < matrix_height; row++)
      {
        MPI_Gatherv(
            matrix[row] + 1,
            sub_str_widths[0],
            MPI_INT,
            global_matrix[row] + 1,
            sub_str_widths,
            start_cols,
            MPI_INT,
            0, // Root process.
            MPI_COMM_WORLD);
      }

      sequence_b = global_sequence_b;
      length_b = sequence_b.length();
      /* Use local matrix to keep track of the processes' computed sub matrix. */
      local_matrix = matrix;
      matrix = global_matrix;
      global_matrix = nullptr;
      matrix_width = global_matrix_width;
      max_length = std::min(length_a, length_b);
    }
    else
    {
      // We can skip the first row since it is all zeros.
      for (int row = 1; row < matrix_height; row++)
      {
        MPI_Gatherv(
            matrix[row] + 1,
            sub_str_widths[world_rank],
            MPI_INT,
            nullptr,
            sub_str_widths,
            start_cols,
            MPI_INT,
            0, // Root process.
            MPI_COMM_WORLD);
      }
    }
  }

  virtual void determineLongestCommonSubsequence() override
  {
    /* Must gather the sub-matrices together before we can trace back through
    the combined matrix. */
    gather();

    if (world_rank == 0)
    {
      total_time_taken = total_timer.stop();
      LongestCommonSubsequence::determineLongestCommonSubsequence();
    }
  }

  virtual void solve() override
  {
    LCSDistributedBase::solve();
    determineLongestCommonSubsequence();
  }

public:
  LCSDistributedGather(
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
    global_matrix = nullptr;
    local_matrix = nullptr;
    this->solve();
  }

  virtual ~LCSDistributedGather()
  {
    if (global_matrix)
    {
      for (int row = 0; row < matrix_height; row++)
      {
        delete[] global_matrix[row];
      }
      delete[] global_matrix;
    }
    if (local_matrix)
    {
      for (int row = 0; row < matrix_height; row++)
      {
        delete[] local_matrix[row];
      }
      delete[] local_matrix;
    }
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
    printf("-------------------- LCS Distributed (Gather) --------------------\n");
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

  LCSDistributedGather lcs(
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
