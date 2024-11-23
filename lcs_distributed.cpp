#include <algorithm> // std::max, std::min
#include <iostream>

#include "lcs.h"

class LongestCommonSubsequenceDistributed : public LongestCommonSubsequence
{
private:
  const int world_size;
  const int world_rank;

  // Iterate over diagonal and process each cell.
  void processDiagonal(const int start_i, const int start_j)
  {
    int i = start_i;
    int j = start_j;
    while (i >= 0 && j < length_b)
    {
      processCell(i, j);
      i--; // Go up one row.
      j++; // Go right one column.
    }
  }

  /**
   * Iterate over a slice of a diagonal.
   *
   * Diagonal indices are counted from top-left to bottom-right.
   * [ 0, 1, 2, 3 ]
   * [ 1, 2, 3, 4 ]
   * [ 2, 3, 4, 5 ]
   *
   * A diagonal is traversed from bottom-left element to top-right element.
   * ex: Order of traversal of diagonal at index 2.
   * [ x, x, 2, x ]
   * [ x, 1, x, x ]
   * [ 0, x, x, x ]
   *
   * If the index of the diagonal is less than the height of the matrix, then
   * the starting index within that diagonal will in the leftmost column.
   * If the index of the diagonal is greater than or equal to the height of the
   * matrix, then the starting index within that diagonal will be in the last row.
   * If the index of the diagonal is equal to the height of the matrix - 1, then
   * the starting index within that diagonal will be in the leftmost column and
   * the bottom row (in other words, the bottom-left cell).
   * [ 0, x, x, x ]
   * [ 1, x, x, x ]
   * [ 2, 3, 4, 5 ]
   * The height of the matrix is equal to the length of sequence A.
   */
  void processSubDiagonal(
      const int diagonal_index,
      const int start_index, // Starting index within the diagonal.
      const int n_elements   // Number of cells to process.
  )
  {
    // Determine where the leftmost element of the diagonal is.
    int i, j;
    /* If the diagonal index is less than the height of the matrix, then the
    starting cell will be in the leftmost column (j = 0) and the row will be
    equal to the index of the diagonal. */
    if (diagonal_index < length_a)
    {
      i = diagonal_index;
      j = 0;
    }
    /* If the diagonal index is greater than or equal to the height of the
    matrix, then the starting cell will be in the last row. */
    else
    {
      i = length_a - 1;
      j = diagonal_index - length_a + 1;
    }

    // Bounds for i index;
    const int min_i = std::max(i - n_elements, 0);
    // Bounds for j index;
    const int max_j = std::min(j + n_elements, (int)length_b - 1);

    // Iterate diagonally.
    while (i >= min_i && j <= max_j)
    {
      processCell(i, j);
      i--; // Go up one row.
      j++; // Go right one column.
    }
  }

  virtual void solve() override
  {
    /* For sequences of length m and n, the number of diagonals will be
    (m + n - 1). Each cell of a diagonal can be computed in parallel, as
    they have no dependence on one another. Each diagonal, however, depends
    on the previous two diagonals. */

    const uint n_diagonals = length_a + length_b - 1;

    int a;
    for (a = 0; a < length_a; a++)
    {
      // Start at leftmost cell of the diagonal.
      processDiagonal(a, 0);
    }
    a = length_a - 1;
    print();
    for (int b = 1; b < length_b; b++)
    {
      // Start at leftmost cell of the diagonal.
      processDiagonal(a, b);
    }

    determineLongestCommonSubsequence();
  }

public:
  LongestCommonSubsequenceDistributed(
      const std::string sequence_a,
      const std::string sequence_b,
      const int world_size,
      const int world_rank)
      : LongestCommonSubsequence(sequence_a, sequence_b),
        world_size(world_size),
        world_rank(world_rank)
  {
    this->solve();
  }

  virtual ~LongestCommonSubsequenceDistributed()
  {
  }
};

int main(int argc, char *argv[])
{
  std::string sequence_a = "dlpkgcqiuyhnjka";
  std::string sequence_b = "drfghjkf";

  // MPI_Init(NULL, NULL);

  int world_size;
  // MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int world_rank;
  // MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  world_size = world_rank = 0;
  LongestCommonSubsequenceDistributed lcs(sequence_a, sequence_b, world_size, world_rank);
  lcs.print();

  return 0;
}
