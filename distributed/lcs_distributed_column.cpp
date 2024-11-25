#include "../cxxopts.hpp"
#include <algorithm> // std::max, std::min
#include <iostream>
#include <mpi.h>

#include "lcs_distributed.h"

/**
 * Distributed LCS. This version divides the score matrix into blocks of
 * columns and assigns each block to a process.
 *
 * Example (Numbers represent process ranks):
  ```
  [ 0 0 1 1 2 2 ]
  [ 0 0 1 1 2 2 ]
  [ 0 0 1 1 2 2 ]
  [ 0 0 1 1 2 2 ]
  [ 0 0 1 1 2 2 ]
  [ 0 0 1 1 2 2 ]
  ```
 * Each process then traverses its sub-matrix in diagonal-major order.
 *
 * With this task mapping, each process only depends on data from the process
 * to its left, which makes communication quite simple. When computing an entry
 * in a process' leftmost column, it must receive the data from the entry in
 * the same row but rightmost column of the left neighboring process. After
 * computing an entry in the rightmost column, the data from that entry must
 * be sent to the neighboring process to the right.
 *
 * If the specific longest common subsequence is required, then the sub-matrices
 * can be gathered together once all of the entries have been computed. (This
 * could probably be optimized by performing the trace-back locally and then
 * sending the indices + subsequence string to the left neighboring process
 * instead of sending the entire sub-matrices over the network.).
 *
 * If only the length of the longest common subsequence is required, then this
 * gathering step can be skipped.
 *
 * */
class LCSDistributedColumn : public LongestCommonSubsequenceDistributed
{
protected:
  int *comm_buffer; // For sending / receiving to other processes.

  int *local_matrix;
  int *global_matrix;

  std::string global_sequence_b;

  void computeDiagonal(int diagonal_index)
  {
    Pair diagonal_start = getDiagonalStart(diagonal_index);
    int i = diagonal_start.first;
    int j = diagonal_start.second;
    int max_i = matrix_height - 1;
    int min_j = 1;
    int comm_value;
    while (i <= max_i && j >= min_j)
    {
      /* If we are computing a cell in the leftmost column of our local matrix,
      then we need to get data from the cells in the rightmost column of our
      neighboring process to the left. Unless we are the leftmost process. */
      if (j == min_j && world_rank != 0)
      {
        MPI_Recv(
            &comm_value,
            1, // Only need a single value.
            MPI_UNSIGNED,
            world_rank - 1, // Source: Get from neighbor to the left.
            i,              // Tag: Row index.
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);
        // Store the value in the local matrix.
        set(i, j - 1, comm_value);
      }

      computeCell(i, j);
      // matrix[i][j] = ++count;

      /* If we are computing a cell in the rightmost column of our local
      matrix, we must send the results to our neighbor to the right once we
      are done. Unless we are the rightmost process. */
      if (j == matrix_width - 1 && world_rank != world_size - 1)
      {
        comm_value = get(i, j);
        MPI_Send(
            &comm_value,
            1,
            MPI_UNSIGNED,
            world_rank + 1, // Destination: Send to neighbor to the right.
            i,
            MPI_COMM_WORLD);
      }

      i++; // Go down by one.
      j--; // Go left by one.
    }
  }

  /** Gathers the sub-matrices from each process together in the root process. */
  void gather()
  {
    int global_matrix_width = global_sequence_b.length();

    // Gather all of the data into the root process:
    if (world_rank == 0)
    {
      /* Use local matrix to keep track of the processes' computed sub matrix. */
      local_matrix = matrix;
      // Allocate space for the combined matrix.
      global_matrix = new int[matrix_height * global_matrix_width];

      MPI_Gather(
          nullptr,

      );
    }
    else
    {
      MPI_Gather(

      );
    }
  }

  virtual void solve() override
  {

    /* Determine number of diagonals in sub-matrix. */
    int n_diagonals = length_b + length_a - 1;
    for (int diagonal = 0; diagonal < n_diagonals; diagonal++)
    {
      computeDiagonal(diagonal);
    }

    gather();

    // determineLongestCommonSubsequence();
  }

public:
  LCSDistributedColumn(
      const std::string &sequence_a,
      const std::string &sequence_b,
      const int world_size,
      const int world_rank,
      const std::string global_sequence_b)
      : LongestCommonSubsequenceDistributed(sequence_a, sequence_b, world_size, world_rank),
        global_sequence_b(global_sequence_b)
  {
    comm_buffer = new int[max_length];
    global_matrix = nullptr;
    this->solve();
  }

  virtual ~LCSDistributedColumn()
  {
    delete[] comm_buffer;
    if (global_matrix)
    {
      delete[] global_matrix;
    }
  }
};

int main(int argc, char *argv[])
{
  std::string sequence_a = "dlrkgcqiuyh";
  std::string sequence_b = "drfghjkfdsz";

  MPI_Init(NULL, NULL);

  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if (world_rank == 0)
  {
    std::cout << "Sequence A: " << sequence_a << std::endl;
    std::cout << "Sequence B: " << sequence_b << std::endl;
  }
  MPI_Barrier(MPI_COMM_WORLD);

  int length_a = sequence_a.length();
  int length_b = sequence_b.length();

  const int min_n_cols_per_process = length_b / world_size;
  const int excess = length_b % world_size;

  int start_col, end_col, n_cols;
  n_cols = min_n_cols_per_process;
  if (world_rank < excess)
  {
    start_col = world_rank * (min_n_cols_per_process + 1);
    n_cols++;
  }
  else
  {
    start_col = (world_rank * min_n_cols_per_process) + excess;
  }
  end_col = start_col + n_cols - 1;

  // Divide up sequence B.
  std::string local_sequence_b = sequence_b.substr(start_col, n_cols);

  for (int rank = 0; rank < world_size; rank++)
  {
    if (rank == world_rank)
    {
      std::cout << "\nRank: " << world_rank << " | start_col: " << start_col
                << " | end_col: " << end_col << "\n | local sequence B: "
                << local_sequence_b << std::endl;
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }

  /*--------------------------------------------------------------------------*/

  // LCSDistributedColumn lcs(sequence_a, sequence_b, world_size, world_rank);
  LCSDistributedColumn lcs(sequence_a, local_sequence_b, world_size, world_rank, sequence_b);
  // Print solution.
  // if (world_rank == 0)
  // {
  //   lcs.print();
  // }

  for (int rank = 0; rank < world_size; rank++)
  {
    if (rank == world_rank)
    {
      std::cout << "\nRank: " << world_rank << "\n";
      lcs.printMatrix();
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }
  MPI_Finalize();

  return 0;
}
