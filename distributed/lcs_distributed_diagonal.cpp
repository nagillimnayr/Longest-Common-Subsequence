#include <algorithm> // std::max, std::min
#include <iostream>
#include <mpi.h>

#include "lcs_distributed.h"

class LongestCommonSubsequenceDistributedDiagonal : public LongestCommonSubsequenceDistributed
{
private:
  int *comm_buffer; // For sending / receiving to other processes.

  int count = 0;

  /**
   * Returns the Indices of the starting cell of the diagonal.
   *
   * Diagonal indices are counted from top-left to bottom-right.
   *
   ```
        [ 0, 1, 2, 3 ]
        [ 1, 2, 3, 4 ]
        [ 2, 3, 4, 5 ]
   ```
   * A diagonal is traversed from bottom-left element to top-right element.
   *  ex: Order of traversal of diagonal at index 2.
   *
   ```
        [ x, x, 2, x ]
        [ x, 1, x, x ]
        [ 0, x, x, x ]
   ```
   *
   * If the index of the diagonal is less than the height of the matrix, then
   * the starting index within that diagonal will in the leftmost column.
   * If the index of the diagonal is greater than or equal to the height of the
   * matrix, then the starting index within that diagonal will be in the last row.
   * If the index of the diagonal is equal to the height of the matrix - 1, then
   * the starting index within that diagonal will be in the leftmost column and
   * the bottom row (in other words, the bottom-left cell).
   *
   ```
        [ 0, x, x, x ]
        [ 1, x, x, x ]
        [ 2, 3, 4, 5 ]
   ```
   * The height of the matrix is equal to the length of sequence A.
   *
   */
  Pair getIndicesOfDiagonalStart(int diagonal_index)
  {
    // Determine where the starting element of the diagonal is.
    int i, j;
    /* If the diagonal index is less than the height of the matrix, then the
    starting cell will be in the leftmost column (j = 0) and the row will be
    equal to the index of the diagonal. */
    if (diagonal_index < length_a)
    {
      j = diagonal_index;
      i = 0;
    }
    /* If the diagonal index is greater than or equal to the height of the
    matrix, then the starting cell will be in the last row. */
    else
    {
      i = diagonal_index - length_b + 1;
      j = length_b - 1;
    }

    // Adjust to account for extra row and column of 0s.
    i++;
    j++;
    return {i, j};
  }

  /**
   * Iterate over a slice of a diagonal.
   */
  void computeSubDiagonal(
      const int diagonal_index,
      const int start_index, // Starting index within the diagonal.
      const int n_elements   // Number of cells to process.
  )
  {
    // Determine where the starting element of the diagonal is.
    Pair indices = getIndicesOfDiagonalStart(diagonal_index);
    int i = indices.first;
    int j = indices.second;

    /* Determine the starting position within the diagonal. */
    i += start_index;
    j -= start_index;

    // Bounds for i index;
    const int max_i = std::min(i + n_elements - 1, matrix_height - 1);
    // Bounds for j index;
    const int min_j = std::max(j - n_elements + 1, 1);

    // std::cout << "diagonal_index: " << diagonal_index
    //           << " | start_i: " << i
    //           << " | max_i: " << max_i
    //           << " | start_j: " << j
    //           << " | min_j: " << min_j
    //           << " | n_elems: " << n_elements
    //           << std::endl;

    // Iterate diagonally.
    while (i <= max_i && j >= min_j)
    {
      // matrix[i][j] = ++count;
      // matrix[i][j] = world_rank + 1;
      computeCell(i, j);
      i++; // Go down one row.
      j--; // Go left one column.
    }
  }

  virtual void solve() override
  {
    /* For sequences of length m and n, the number of diagonals will be
    (m + n - 1). Each cell of a diagonal can be computed in parallel, as
    they have no dependence on one another. Each diagonal, however, depends
    on the previous two diagonals. */

    const int n_diagonals = length_a + length_b - 1;

    // Traverse matrix in diagonal-major order.
    for (int diagonal_index = 0; diagonal_index < n_diagonals; diagonal_index++)
    {
      // Split up the diagonal amongst the processes.

      // Determine the size of the diagonal.
      /**
       * length_a == length_b
       * [ 1, x, x, x ]
       * [ 2, x, x, x ]
       * [ 3, x, x, x ]
       * [ 4, 3, 2, 1 ]
       *
       * length_a < length_b
       * [ 1, x, x, x ]
       * [ 2, x, x, x ]
       * [ 3, 3, 2, 1 ]
       *
       *
       * length_a > length_b
       * [ 1, x, x, x ]
       * [ 2, x, x, x ]
       * [ 3, x, x, x ]
       * [ 4, x, x, x ]
       * [ 4, 3, 2, 1 ]
       *
       *     0, 1, 2, 3, 4, 5, 6, 7
       * 0 [ 1, x, x, x, x, x, x, x ]
       * 1 [ 2, x, x, x, x, x, x, x ]
       * 2 [ 3, x, x, x, x, x, x, x ]
       * 3 [ 4, x, x, x, x, x, x, x ]
       * 4 [ 5, x, x, x, x, x, x, x ]
       * 5 [ 6, x, x, x, x, x, x, x ]
       * 6 [ 7, x, x, x, x, x, x, x ]
       * 7 [ 8, 7, 6, 5, 4, 3, 2, 1 ]
       *
       * NOTE: The length of any diagonal cannot be greater than the length of
       * the smaller dimension (shorter sequence).
       */
      int diagonal_length;
      if (diagonal_index < max_length)
      {
        diagonal_length = diagonal_index + 1;
      }
      else
      {
        diagonal_length = n_diagonals - diagonal_index;
      }
      /* NOTE: The length of any diagonal cannot be greater than the length of
      the smaller dimension (the shorter sequence). */
      diagonal_length = std::min(diagonal_length, max_length);

      /* If there are fewer cells in the diagonal than there are processes,
      then some processes will have no work to do. */
      int n_processes = std::min(world_size, diagonal_length);

      if (world_rank < n_processes)
      {
        // Minimum number of cells for a process to compute in this diagonal.
        int min_cells_per_process = diagonal_length / n_processes;
        int excess = diagonal_length % n_processes;

        // Determine where each process should start within the diagonal.
        int start_index;
        int n_cells = min_cells_per_process;
        // If there are n cells remaining, then n processes should get 1 extra cell.
        if (world_rank < excess)
        {
          // Give one extra cell to the first n processes.
          start_index = world_rank * (min_cells_per_process + 1);
          n_cells++;
        }
        else
        {
          /* Offset the rest of the processes' starting positions by the number
          of excess cells */
          start_index = (world_rank * min_cells_per_process) + excess;
        }

        computeSubDiagonal(diagonal_index, start_index, n_cells);
      }
      // continue;

      /** Synchronize processes:
       * Before we can continue to the next diagonal, each process will need to
       * receive the data for the entries which it will depend on to compute the
       * next diagonal.
       *
       * We don't want every process to broadcast its entire matrix, as only a
       * subset of the entries along one diagonal will have been updated since
       * the last syncronization step.
       *
       * We can broadcast just the diagonal instead of the entire matrix.
       *
       * A further optimization we could do would be to record the indices that
       * were updated for each rank, and broadcast just those indices and the
       * new scores at those indices.
       */
      /**/
      Pair diagonal_start = getIndicesOfDiagonalStart(diagonal_index);
      for (int rank = 0; rank < world_size; rank++)
      {
        if (rank == world_rank)
        {
          int i = diagonal_start.first;
          int j = diagonal_start.second;
          for (int k = 0; k < diagonal_length; k++)
          {
            // Copy diagonal into comm buffer.
            comm_buffer[k] = matrix[i][j];
            i++;
            j--;
          }
        }
        // Broadcast diagonal entries.
        MPI_Bcast(
            comm_buffer,
            diagonal_length,
            MPI_UNSIGNED,
            rank,
            MPI_COMM_WORLD);
        if (rank != world_rank)
        {
          int i = diagonal_start.first;
          int j = diagonal_start.second;
          // Copy received diagonal entries into local matrix.
          for (int k = 0; k < diagonal_length; k++)
          {
            // Copy diagonal from comm buffer into local matrix.
            int score = comm_buffer[k];
            // Ignore 0s.
            if (score > 0)
            {
              matrix[i][j] = score;
            }
            i++;
            j--;
          }
        }
      }
    }

    determineLongestCommonSubsequence();
  }

public:
  LongestCommonSubsequenceDistributedDiagonal(
      const std::string &sequence_a,
      const std::string &sequence_b,
      const int world_size,
      const int world_rank)
      : LongestCommonSubsequenceDistributed(sequence_a, sequence_b, world_size, world_rank)
  {
    comm_buffer = new int[max_length];
    this->solve();
  }

  virtual ~LongestCommonSubsequenceDistributedDiagonal()
  {
    delete[] comm_buffer;
  }
};

int main(int argc, char *argv[])
{
  std::string sequence_a = "dpgcr";
  std::string sequence_b = "dghrf";

  MPI_Init(NULL, NULL);

  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  LongestCommonSubsequenceDistributedDiagonal lcs(sequence_a, sequence_b, world_size, world_rank);

  // for (int rank = 0; rank < world_size; rank++)
  // {
  //   if (rank == world_rank)
  //   {
  //     std::cout << std::endl;
  //     lcs.printMatrix();
  //     // std::cout << std::endl;
  //   }
  //   MPI_Barrier(MPI_COMM_WORLD);
  // }

  // Print solution.
  if (world_rank == 0)
  {
    lcs.print();
  }

  MPI_Finalize();

  return 0;
}
