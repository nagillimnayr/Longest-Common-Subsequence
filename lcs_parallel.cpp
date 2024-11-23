#include <iostream>

#include "lcs_parallel.h"

int main(int argc, char *argv[])
{
  std::string sequence_a = "dlpkgcqiuyhnjka";
  std::string sequence_b = "drfghjkf";

  LongestCommonSubsequenceParallel lcs(sequence_a, sequence_b);
  lcs.print();
  return 0;
}
