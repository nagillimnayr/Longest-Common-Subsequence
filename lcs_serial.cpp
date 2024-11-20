#include <algorithm> // std::max
#include <iomanip>
#include <iostream>
#include <string.h>

class LongestCommonSubsequenceSerial
{
private:
  const std::string sequence_a;
  const std::string sequence_b;
  const uint length_a; // Length of sequence_a.
  const uint length_b; // Length of sequence_b.

  uint **matrix; /* Solution matrix - matrix[i][j] stores the length of
    the longest common subsequence of the first i-1 elements of sequence_a
    and the first j-1 elements of sequence_b. */

  void solve()
  {
    for (uint i = 0; i < length_a; i++)
    {
      for (uint j = 0; j < length_b; j++)
      {
        /* If characters are the same, set the entry to
        1 + the entry diagonally to the top left. */
        if (sequence_a[i] == sequence_b[j])
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
          continue;
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
    }
  }

public:
  LongestCommonSubsequenceSerial(const std::string sequence_a, const std::string sequence_b)
      : sequence_a(sequence_a), sequence_b(sequence_b), length_a(sequence_a.length()), length_b(sequence_b.length())
  {
    matrix = new uint *[length_a];
    for (uint i = 0; i < length_a; i++)
    {
      matrix[i] = new uint[length_b];
      for (uint j = 0; j < length_b; j++)
      {
        matrix[i][j] = 0;
      }
    }

    this->solve();
  }

  ~LongestCommonSubsequenceSerial()
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
    return matrix[length_a - 1][length_b - 1];
  }

  // Print the matrix to the console.
  void print() const
  {
    /* Prints the matrix in the format:
     *      b1 b2 b3
     * a0 [ x  x  x ]
     * a1 [ x  x  x ]
     * a2 [ x  x  x ]
     */
    // Print the elements of sequence_b along the top row.
    std::cout << std::setw(3) << " " << std::right; /* Extra padding before first element
      to account for sequence_a being printed down the left side. */
    for (uint j = 0; j < length_b; j++)
    {
      std::cout << std::setw(2) << sequence_b[j];
    }
    std::cout << "\n";

    for (uint i = 0; i < length_a; i++)
    {
      // Print sequence_a down the left side of the matrix.
      std::cout << std::left << std::setw(2) << sequence_a[i] << "["
                << std::right;
      for (uint j = 0; j < length_b; j++)
      {
        std::cout << std::setw(2) << matrix[i][j];
      }
      std::cout << " ]\n";
    }
    std::cout << std::endl;

    std::cout << "Sequence A: " << sequence_a << "\n";
    std::cout << "Sequence B: " << sequence_b << "\n";
    std::cout << "Length of the longest common subsequence: " << getLongestSubsequenceLength() << "\n";
    // std::cout << "Longest common subsequence: " << getLongestSubsequence() << "\n";
    std::cout << std::endl;
  }
};

int main(int argc, char *argv[])
{

  std::string sequence_a = "drfghjk";
  std::string sequence_b = "dlpkgcqiuyhnjk";

  LongestCommonSubsequenceSerial lcs(sequence_a, sequence_b);
  lcs.print();

  return 0;
}
