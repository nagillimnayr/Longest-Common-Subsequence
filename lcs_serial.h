#ifndef _LCS_SERIAL_H_
#define _LCS_SERIAL_H_

#include <algorithm> // std::max
#include <iomanip>
#include <iostream>
#include <string.h>

#include "lcs.h"

class LongestCommonSubsequenceSerial : public LongestCommonSubsequence
{
private:
  virtual void solve() override
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

    determineLongestCommonSubsequence();
  }

public:
  LongestCommonSubsequenceSerial(const std::string sequence_a, const std::string sequence_b)
      : LongestCommonSubsequence(sequence_a, sequence_b)
  {
    this->solve();
  }

  virtual ~LongestCommonSubsequenceSerial()
  {
  }
};

#endif
