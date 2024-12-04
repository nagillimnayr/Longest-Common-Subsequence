#include <iostream>

#include "cxxopts.hpp"
#include "lcs.h"

class LongestCommonSubsequenceSerial : public LongestCommonSubsequence
{
private:
  virtual void solve() override
  {
    timer.start();
    matrix_timer.start();
    for (int i = 1; i < matrix_height; i++)
    {
      for (int j = 1; j < matrix_width; j++)
      {
        computeCell(i, j);
      }
    }
    matrix_time_taken = matrix_timer.stop();
    determineLongestCommonSubsequence();
    time_taken = timer.stop();
  }

public:
  LongestCommonSubsequenceSerial(const std::string &sequence_a, const std::string &sequence_b)
      : LongestCommonSubsequence(sequence_a, sequence_b)
  {
    this->solve();
  }

  virtual ~LongestCommonSubsequenceSerial()
  {
  }

  virtual void print() override
  {
    printInfo();
    printTimeTaken();
  }
};

int main(int argc, char *argv[])
{

  cxxopts::Options options("lcs_serial",
                           "Serial LCS implementation.");

  options.add_options(
      "inputs",
      {{"sequence_a", "First input sequence.", cxxopts::value<std::string>()},
       {"sequence_b", "Second input sequence.",
        cxxopts::value<std::string>()}});

  auto command_options = options.parse(argc, argv);

  std::string sequence_a = command_options["sequence_a"].as<std::string>();
  std::string sequence_b = command_options["sequence_b"].as<std::string>();

  printf("-------------------- LCS Serial --------------------\n");

  LongestCommonSubsequenceSerial lcs(sequence_a, sequence_b);
  lcs.printMatrix();
  lcs.printInfo();
  // lcs.printLCSLength();
  lcs.printTimeTaken();
  return 0;
}
