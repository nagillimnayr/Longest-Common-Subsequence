#include <iostream>

#include "lcs_distributed.h"

int main(int argc, char *argv[])
{
  std::string sequence_a = "dlpkgcqiuyhnjka";
  std::string sequence_b = "drfghjkf";

  LongestCommonSubsequenceDistributed lcs(sequence_a, sequence_b);
  lcs.print();
  return 0;
}
