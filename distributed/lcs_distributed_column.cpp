#include <algorithm> // std::max, std::min
#include <iostream>
#include <mpi.h>

#include "lcs_distributed.h"

/* Column-wise version of distributed LCS. */

class LCSDistributedColumn : public LongestCommonSubsequenceDistributed
{
protected:
  int *comm_buffer; // For sending / receiving to other processes.

  int **local_matrix;
  int **global_matrix;

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
        matrix[i][j - 1] = comm_value;
      }

      computeCell(i, j);
      // matrix[i][j] = ++count;

      /* If we are computing a cell in the rightmost column of our local
      matrix, we must send the results to our neighbor to the right once we
      are done. Unless we are the rightmost process. */
      if (j == matrix_width - 1 && world_rank != world_size - 1)
      {
        comm_value = matrix[i][j];
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

  virtual void solve() override
  {

    /* Determine number of diagonals in sub-matrix. */
    int n_diagonals = length_b + length_a - 1;
    for (int diagonal = 0; diagonal < n_diagonals; diagonal++)
    {
      computeDiagonal(diagonal);
    }

    int global_matrix_width = global_sequence_b.length();

    // Gather all of the data into the main process:
    if (world_rank == 0)
    {
      /* Use local matrix to keep track of the processes' computed sub matrix. */
      local_matrix = matrix;
      // Allocate space for the combined matrix.
      global_matrix = new int *[matrix_height];
      for (int row = 0; row < matrix_height; row++)
      {
        global_matrix[row] = new int[global_matrix_width];
      }
    }

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
    this->solve();
  }

  virtual ~LCSDistributedColumn()
  {
    delete[] comm_buffer;
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
