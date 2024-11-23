#include "lcs.h"
#include <iostream>

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

int main(int argc, char *argv[])
{
  std::string sequence_a = "dlpkgcqiuyhnjka";
  std::string sequence_b = "drfghjkf";

  LongestCommonSubsequenceParallel lcs(sequence_a, sequence_b);
  lcs.print();
  return 0;
}
