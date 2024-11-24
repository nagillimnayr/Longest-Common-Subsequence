#include <iostream>

#include "lcs.h"

class LongestCommonSubsequenceSerial : public LongestCommonSubsequence
{
private:
  virtual void solve() override
  {
    for (uint i = 1; i < matrix_height; i++)
    {
      for (uint j = 1; j < matrix_width; j++)
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
  std::string sequence_a = "dpgcr";
  std::string sequence_b = "dghrf";

  LongestCommonSubsequenceSerial lcs(sequence_a, sequence_b);
  lcs.print();
  return 0;
}
