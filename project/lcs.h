#ifndef _LCS_H_
#define _LCS_H_
#include <iostream>

#include "timer.h"
#include <algorithm> // std::max
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string.h>

/** Abstract Base class for LCS implementations */
class LongestCommonSubsequence
{
protected:
  const std::string sequence_a;
  std::string sequence_b;
  const int length_a; // Length of sequence_a.
  int length_b;       // Length of sequence_b.
  int max_length;     /* The longest common subsequence cannot be longer
      const int max_length; /* The longest common subsequence cannot be longer
      than the shorter of the two input sequences. */
  std::string longest_common_subsequence;

  int matrix_width;        // Width of the matrix.
  const int matrix_height; // Height of the matrix.

  int **matrix; /* Solution matrix - matrix[i][j] stores the length of
    the longest common subsequence of the first i-1 elements of sequence_a
    and the first j-1 elements of sequence_b. */

  /* Simple custom timer for recording time intervals. */
  Timer timer;
  /* Total time taken (in seconds) to complete computation of the LCS. */
  double time_taken = 0.0;

  /* Timer to record how long it takes to compute all entries of the matrix. */
  Timer matrix_timer;
  /* Time taken to compute all entries of the matrix. */
  double matrix_time_taken = 0.0;

  /* The logic for computing individual entries of the matrix is the same
  regardless of which algorithm is being used. */
  virtual void
  computeCell(const int row, const int col)
  {
    /* Adjust the indices when accessing sequences, to account for extra row and
    column of 0s. in the matrix. */
    /* If characters are the same, set the entry to
        1 + the entry diagonally to the top left. */
    if (sequence_a[row - 1] == sequence_b[col - 1])
    {
      /* If i is 0 then we are in the top row,
      if j is 0 then we are in the leftmost column,
      either way, there is no entry diagonally to the top left, so treat it as 0. */
      int top_left = 0;
      if (row > 0 && col > 0)
      {
        top_left = matrix[row - 1][col - 1];
      }
      matrix[row][col] = top_left + 1;
      return;
    }
    /* If characters are not the same, set entry to the higher of either the entry directly above or the entry directly to the left. */
    int top, left;
    top = left = 0;
    if (row > 0)
    {
      top = matrix[row - 1][col];
    }
    if (col > 0)
    {
      left = matrix[row][col - 1];
    }
    matrix[row][col] = std::max(top, left);
  }

  // Traces through the matrix to reconstruct the longest common subsequence.
  virtual void determineLongestCommonSubsequence()
  {
    // Start at the maximal entry, the bottom-right.
    int i = matrix_height - 1;
    int j = matrix_width - 1;
    int current = matrix[i][j];
    longest_common_subsequence.resize(current, ' ');
    int index = current - 1;
    while (index >= 0 && i > 0 && j > 0)
    {
      current = matrix[i][j];
      int top, left, top_left;
      top = left = top_left = 0;

      top_left = matrix[i - 1][j - 1];
      top = matrix[i - 1][j];
      left = matrix[i][j - 1];

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

  virtual void
  solve() = 0;

public:
  LongestCommonSubsequence(const std::string &sequence_a, const std::string &sequence_b)
      : sequence_a(sequence_a), sequence_b(sequence_b),
        length_a(sequence_a.length()), length_b(sequence_b.length()),
        max_length(std::min(length_a, length_b)),
        matrix_width(length_b + 1), matrix_height(length_a + 1)
  {
    matrix = new int *[matrix_height];
    for (int i = 0; i < matrix_height; i++)
    {
      matrix[i] = new int[matrix_width];
      // Fill leftmost column with 0s.
      matrix[i][0] = 0;
    }
    for (int j = 0; j < matrix_width; j++)
    {
      // Fill top row with 0s.
      matrix[0][j] = 0;
    }
  }

  virtual ~LongestCommonSubsequence()
  {
    for (int row = 0; row < matrix_height; row++)
    {
      delete[] matrix[row];
    }
    delete[] matrix;
  }

  // Returns the length of the longest common subsequence.
  virtual int getLongestSubsequenceLength()
  {
    return matrix[matrix_height - 1][matrix_width - 1];
  }

  // Print the matrix to the console.
  void printMatrix()
  {
    std::cout << "\n";

    /* Prints the matrix in the format:
     *      b1 b2 b3
     * a0 [ x  x  x ]
     * a1 [ x  x  x ]
     * a2 [ x  x  x ]
     */

    // Determine the number of digits in the largest number.
    int max_num = getLongestSubsequenceLength();
    int n_digits = 1;
    int n = max_num;
    while (n >= 10)
    {
      n /= 10;
      n_digits++;
    }
    int min_field_width = n_digits + 1;

    // Print the elements of sequence_b along the top row.
    std::cout
        << std::setw(3) << " " << std::right; /* Extra padding before first element
        to account for sequence_a being printed down the left side. */
    std::cout << std::setw(min_field_width) << " ";
    for (int j = 1; j < matrix_width; j++)
    {
      std::cout << std::setw(min_field_width) << sequence_b[j - 1];
    }
    std::cout << "\n";

    for (int i = 0; i < matrix_height; i++)
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

      for (int j = 0; j < matrix_width; j++)
      {
        std::cout << std::setw(min_field_width) << matrix[i][j];
      }
      std::cout << " ]\n";
    }
    std::cout << std::endl;
  }

  virtual void printLCS()
  {
    std::cout << "Sequence A: " << sequence_a << "\n";
    std::cout << "Sequence B: " << sequence_b << "\n";
    std::cout << "Longest common subsequence: " << longest_common_subsequence << "\n";
  }

  virtual void printLCSLength()
  {
    std::cout << "Length of the longest common subsequence: " << getLongestSubsequenceLength() << "\n";
  }

  virtual void printInfo()
  {
    printLCS();
    printLCSLength();
  }

  virtual void print()
  {
    printMatrix();
    printInfo();
  }

  virtual void printMatrixTimeTaken()
  {
    printf("Time taken to compute matrix: %lf\n", matrix_time_taken);
  }

  virtual void printTotalTimeTaken()
  {
    printf("Total time taken: %lf\n", time_taken);
  }

  virtual void printTimeTaken()
  {
    printMatrixTimeTaken();
    printTotalTimeTaken();
  }
};

void read_input_csv(const std::string &input_file_path, std::string &sequence_a, std::string &sequence_b)
{
  std::ifstream in_file(input_file_path);
  if (!in_file.is_open())
  {
    std::cerr << "Error reading file: " << input_file_path << std::endl;
    exit(1);
  }
  std::getline(in_file, sequence_a, ',');
  std::getline(in_file, sequence_b, ',');
}

#endif
