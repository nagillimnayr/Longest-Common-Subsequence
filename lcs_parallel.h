#ifndef _LCS_PARALLEL_H_
#define _LCS_PARALLEL_H_

#include <algorithm> // std::max
#include <iomanip>
#include <iostream>
#include <string.h>

#include "lcs.h"

class LongestCommonSubsequenceParallel : public LongestCommonSubsequence
{
private:
  virtual void solve() override
  {
    // For sequences of length m and n, the number of diagonals will be
    // (m + n - 1)

    determineLongestCommonSubsequence();
  }

public:
  LongestCommonSubsequenceParallel(const std::string sequence_a, const std::string sequence_b)
      : LongestCommonSubsequence(sequence_a, sequence_b)
  {
    this->solve();
  }

  virtual ~LongestCommonSubsequenceParallel()
  {
  }
};

#endif
