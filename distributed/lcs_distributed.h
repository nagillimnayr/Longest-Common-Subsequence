#ifndef _LCS_DISTRIBUTED_H_
#define _LCS_DISTRIBUTED_H_
#include <algorithm> // std::max, std::min
#include <iostream>
#include <mpi.h>

#include "../lcs.h"

struct Pair
{
  int first;
  int second;
};

class LongestCommonSubsequenceDistributed : public LongestCommonSubsequence
{
protected:
  const int world_size;
  const int world_rank;

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
    * A diagonal is traversed from the top-right element to the bottom-left element.
    *  ex: Order of traversal of diagonal at index 2:
    ```
        [ x, x, 0, x ]
        [ x, 1, x, x ]
        [ 2, x, x, x ]
    ```
    *  ex: Order of traversal of entire matrix:
    ```
        [ 1, 2, 4 ]
        [ 3, 5, 7 ]
        [ 6, 8, 9 ]
    ```
    *
    * If the index of the diagonal is less than the width of the matrix, then
    * the starting index within that diagonal will be in the leftmost column.
    * If the index of the diagonal is greater than or equal to the width of the
    * matrix, then the starting index within that diagonal will be in the last row.
    * If the index of the diagonal is equal to the width of the matrix - 1, then
    * the starting index within that diagonal will be in the leftmost column and
    * the bottom row (in other words, the bottom-left cell).
    *
    ```
        [ 0, 1, 2, 3 ]
        [ x, x, x, 4 ]
        [ x, x, x, 5 ]
    ```
    * The width of the matrix is equal to the length of sequence B.
    *
    * Returns a pair of ints representing the starting indices of the diagonal.
    */
  Pair getDiagonalStart(int diagonal_index)
  {
    // Determine where the starting element of the diagonal is.
    int i, j;
    /* If the diagonal index is less than the width of the matrix, then the
    starting cell will be in the topmost row (i = 0) and the column will be
    equal to the index of the diagonal. */
    if (diagonal_index < length_b)
    {
      i = 0;
      j = diagonal_index;
    }
    /* If the diagonal index is greater than or equal to the width of the
    matrix, then the starting cell will be in the rightmost column. */
    else
    {
      i = diagonal_index - length_b + 1;
      j = length_b - 1;
    }

    // Adjust indices to account for the extra row and column of 0s.
    i++;
    j++;
    return {i, j};
  }

  virtual void solve() = 0;

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
  }

  virtual ~LongestCommonSubsequenceDistributed()
  {
  }
};

#endif
