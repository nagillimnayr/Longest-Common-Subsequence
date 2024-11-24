#ifndef _LCS_H_
#define _LCS_H_
#include <iostream>

#include <algorithm> // std::max
#include <iomanip>
#include <string.h>

/** Abstract Base class for LCS implementations */
class LongestCommonSubsequence
{
protected:
  const std::string sequence_a;
  const std::string sequence_b;
  const uint length_a;   // Length of sequence_a.
  const uint length_b;   // Length of sequence_b.
  const uint max_length; /* The longest common subsequence cannot be longer
  const uint max_length; /* The longest common subsequence cannot be longer
  than the shorter of the two input sequences. */
  std::string longest_common_subsequence;

  const uint matrix_width;  // Width of the matrix.
  const uint matrix_height; // Height of the matrix.

  uint **matrix; /* Solution matrix - matrix[i][j] stores the length of
    the longest common subsequence of the first i-1 elements of sequence_a
    and the first j-1 elements of sequence_b. */

  /* The logic for computing individual entries of the matrix is the same
  regardless of which algorithm is being used. */
  virtual void processCell(const uint i, const uint j)
  {
    /* Adjust the indices when accessing sequences, to account for extra row and
    column of 0s. in the matrix. */
    /* If characters are the same, set the entry to
        1 + the entry diagonally to the top left. */
    if (sequence_a[i - 1] == sequence_b[j - 1])
    {
      /* If i is 0 then we are in the top row,
      if j is 0 then we are in the leftmost column,
      either way, there is no entry diagonally to the top left, so treat it as 0. */
      uint top_left = 0;
      if (i > 0 && j > 0)
      {
        top_left = matrix[i - 1][j - 1];
      }
      matrix[i][j] = top_left + 1;
      return;
    }
    /* If characters are not the same, set entry to the higher of either the entry directly above or the entry directly to the left. */
    uint top, left;
    top = left = 0;
    if (i > 0)
    {
      top = matrix[i - 1][j];
    }
    if (j > 0)
    {
      left = matrix[i][j - 1];
    }
    matrix[i][j] = std::max(top, left);
  }

  // Traces through the matrix to reconstruct the longest common subsequence.
  void determineLongestCommonSubsequence()
  {
    // Start at the maximal entry, the bottom-right.
    uint i = matrix_height - 1;
    uint j = matrix_width - 1;
    uint current = matrix[i][j];
    longest_common_subsequence.resize(current, ' ');
    uint index = current - 1;
    while (index >= 0)
    {
      // Don't go out of bounds.
      i = std::max(i, 0u);
      j = std::max(j, 0u);

      if (i == 0 && j == 0)
      {
        // longest_common_subsequence[0] = sequence_a[0];
        break;
      }

      uint current = matrix[i][j];
      uint top, left, top_left;
      top = left = top_left = 0;
      if (i > 0 && j > 0)
      {
        top_left = matrix[i - 1][j - 1];
      }
      if (i > 0)
      {
        top = matrix[i - 1][j];
      }
      if (j > 0)
      {
        left = matrix[i][j - 1];
      }

      if (top_left == current)
      {
        // Go to entry to the top-left.
        i--;
        j--;
        continue;
      }

      /* If the elements above, to the left, and diagonally to the top left
      are all the same,  */
      if (top_left == top && top_left == left)
      {
        longest_common_subsequence[index] = sequence_a[i - 1];
        index--;
        // Go to entry to the top-left.
        i--;
        j--;

        continue;
      }

      /* If the entry to the top left is lower than the current entry but is not
      equal to the entries above and to the left, then either the one above or
      the one to the left must be the same as the current. */
      if (top == current)
      {
        // Go to the entry above.
        i--;
        continue;
      }
      else
      {
        // If it wasn't the one above, it must be the one to the left.
        j--;
      }
    }
  }

  virtual void solve() = 0;

public:
  LongestCommonSubsequence(const std::string sequence_a, const std::string sequence_b)
      : sequence_a(sequence_a), sequence_b(sequence_b),
        length_a(sequence_a.length()), length_b(sequence_b.length()),
        max_length(std::min(length_a, length_b)),
        matrix_width(length_b + 1), matrix_height(length_a + 1)
  {
    matrix = new uint *[matrix_height];
    for (uint i = 0; i < matrix_height; i++)
    {
      matrix[i] = new uint[matrix_width];
      // Fill leftmost column with 0s.
      matrix[i][0] = 0;

      // for (uint j = 0; j < matrix_width; j++)
      // {
      //   // Fill with 0s.
      //   matrix[i][j] = 0;
      // }
    }
    for (uint j = 0; j < matrix_width; j++)
    {
      // Fill top row with 0s.
      matrix[0][j] = 0;
    }
  }

  virtual ~LongestCommonSubsequence()
  {
    for (uint i = 0; i < length_a; i++)
    {
      delete matrix[i];
    }
    delete[] matrix;
  }

  // Returns the length of the longest common subsequence.
  uint getLongestSubsequenceLength() const
  {
    return matrix[matrix_height - 1][matrix_width - 1];
  }

  // Print the matrix to the console.
  void printMatrix() const
  {
    /* Prints the matrix in the format:
     *      b1 b2 b3
     * a0 [ x  x  x ]
     * a1 [ x  x  x ]
     * a2 [ x  x  x ]
     */

    // Determine the number of digits in the largest number.
    uint max_num = getLongestSubsequenceLength();
    uint n_digits = 1;
    uint n = max_num;
    while (n >= 10)
    {
      n /= 10;
      n_digits++;
    }
    uint min_field_width = n_digits + 1;

    // Print the elements of sequence_b along the top row.
    std::cout
        << std::setw(3) << " " << std::right; /* Extra padding before first element
        to account for sequence_a being printed down the left side. */
    std::cout << std::setw(min_field_width) << " ";
    for (uint j = 1; j < matrix_width; j++)
    {
      std::cout << std::setw(min_field_width) << sequence_b[j - 1];
    }
    std::cout << "\n";

    for (uint i = 0; i < matrix_height; i++)
    {
      // Print sequence_a down the left side of the matrix.
      if (i > 0)
      {
        std::cout << sequence_a[i - 1];
      }
      else
      {
        std::cout << " ";
      }
      std::cout << " [" << std::right;

      for (uint j = 0; j < matrix_width; j++)
      {
        std::cout << std::setw(min_field_width) << matrix[i][j];
      }
      std::cout << " ]\n";
    }
    std::cout << std::endl;
  }

  void printInfo()
  {
    std::cout << "Sequence A: " << sequence_a << "\n";
    std::cout << "Sequence B: " << sequence_b << "\n";
    std::cout << "Length of the longest common subsequence: " << getLongestSubsequenceLength() << "\n";
    std::cout << "Longest common subsequence: " << longest_common_subsequence << "\n";
  }

  void print()
  {
    printMatrix();
    printInfo();
  }
};

#endif
