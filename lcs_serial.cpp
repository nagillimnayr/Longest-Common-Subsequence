#include <iostream>

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
        processCell(i, j);
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

int main(int argc, char *argv[])
{
  std::string sequence_a = "dlpkgcqiuyhnjka";
  std::string sequence_b = "drfghjkf";

  LongestCommonSubsequenceSerial lcs(sequence_a, sequence_b);
  lcs.print();
  return 0;
}
