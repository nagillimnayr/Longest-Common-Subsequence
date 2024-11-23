#include <iostream>

#include "lcs_distributed.h"
#include "lcs_parallel.h"
#include "lcs_serial.h"

int main(int argc, char *argv[])
{
  std::string sequence_a = "dlpkgcqiuyhnjka";
  std::string sequence_b = "drfghjkf";

  LongestCommonSubsequenceSerial lcs(sequence_a, sequence_b);
  lcs.print();
  return 0;
}
