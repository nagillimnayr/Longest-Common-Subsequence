#ifndef _LCS_DISTRIBUTED_H_
#define _LCS_DISTRIBUTED_H_
#include <algorithm> // std::max
#include <iomanip>
#include <iostream>
#include <mpi.h>
#include <string.h>

#include "lcs.h"

class LongestCommonSubsequenceDistributed : public LongestCommonSubsequence
{
private:
  virtual void solve() override
  {
    // For sequences of length m and n, the number of diagonals will be
    // (m + n - 1)

    determineLongestCommonSubsequence();
  }

public:
  LongestCommonSubsequenceDistributed(const std::string sequence_a, const std::string sequence_b)
      : LongestCommonSubsequence(sequence_a, sequence_b)
  {
    this->solve();
  }

  virtual ~LongestCommonSubsequenceDistributed()
  {
  }
};

#endif
