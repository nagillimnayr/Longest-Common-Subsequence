#include <algorithm> // std::max, std::min
#include <iostream>
#include <mpi.h>

#include "lcs_distributed.h"

/* Column-wise version of distributed LCS. */

class LCSDistributedColumn : public LongestCommonSubsequenceDistributed
{
protected:
  uint *comm_buffer; // For sending / receiving to other processes.

  virtual void solve() override
  {

    for (int rank = 0; rank < world_size; rank++)
    {
      if (rank == world_rank)
      {
        std::cout << "Rank: " << world_rank << "\n";
        printMatrix();
        std::cout << std::endl;
      }
      MPI_Barrier(MPI_COMM_WORLD);
    }

    /**
     * Split the matrix into columns and assign the columns to different
     * processes.
     *
     * Each block is then traversed in anti-diagonal order.
     **/

    const uint min_n_cols_per_process = length_b / world_size;
    const uint excess = length_b % world_size;

    uint start_col, end_col, n_cols;
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

    /* Determine number of diagonals in sub-matrix. */
    uint n_diagonals = n_cols + length_a - 1;
    int count = 0;
    for (uint diagonal = 0; diagonal < n_diagonals; diagonal++)
    {
      Pair diagonal_start = getDiagonalStart(diagonal, n_cols);
      // Offset column index by the starting column.
      int i = diagonal_start.first;
      int j = diagonal_start.second + start_col;
      int max_i = length_a - 1;
      int min_j = start_col;
      uint comm_value;
      while (i <= max_i && j >= min_j)
      {
        /* If we are computing a cell in the leftmost column of our local block,
        then we need to get data from the cells in the rightmost column of our
        neighboring process to the left. Unless we are the leftmost process. */
        if (j == start_col && world_rank != 0)
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

        processCell(i, j);
        // matrix[i][j] = count++;

        /* If we are computing a cell in the rightmost column of our local
        block, we must send the results to our neighbor to the right once we
        are done. Unless we are the rightmost process. */
        if (j == end_col && world_rank != world_size - 1)
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

    // Gather all of the data into the main process:

    // determineLongestCommonSubsequence();
  }

public:
  LCSDistributedColumn(
      const std::string sequence_a,
      const std::string sequence_b,
      const int world_size,
      const int world_rank)
      : LongestCommonSubsequenceDistributed(sequence_a, sequence_b, world_size, world_rank)
  {
    comm_buffer = new uint[max_length];
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

  const uint min_n_cols_per_process = length_b / world_size;
  const uint excess = length_b % world_size;

  uint start_col, end_col, n_cols;
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

  LCSDistributedColumn lcs(sequence_a, sequence_b, world_size, world_rank);
  LCSDistributedColumn lcs(sequence_a, local_sequence_b, world_size, world_rank);
  // Print solution.
  // if (world_rank == 0)
  // {
  //   lcs.print();
  // }

  for (int rank = 0; rank < world_size; rank++)
  {
    if (rank == world_rank)
    {
      std::cout << "Rank: " << world_rank << "\n";
      lcs.printMatrix();
      std::cout << std::endl;
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }
  MPI_Finalize();

  return 0;
}
