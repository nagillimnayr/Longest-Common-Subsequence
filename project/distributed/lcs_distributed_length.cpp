

#include <algorithm> // std::max, std::min
#include <iostream>
#include <mpi.h>

#include "../cxxopts.hpp"
#include "lcs_distributed.h"

/**
 * This version of the distributed LCS implementation only determines the length
 * of the LCS, not the subsequence itself. This allows us to avoid the costly
 * overhead of gathering the disparate sub-matrices together at the end.
 *
 * */
class LCSDistributedLength : public LCSDistributedBase
{
protected:
  virtual void solve() override
  {
    LCSDistributedBase::solve();
    determineLongestSubsequenceLength();
  }

public:
  LCSDistributedLength(
      const std::string &sequence_a,
      const std::string &sequence_b,
      const int world_size,
      const int world_rank,
      int *start_cols,
      int *sub_str_widths,
      const std::string &global_sequence_b)
      : LCSDistributedBase(sequence_a, sequence_b, world_size, world_rank, start_cols, sub_str_widths)
  {
    this->solve();
  }

  virtual ~LCSDistributedLength()
  {
  }

  virtual void printLCSLength() override
  {
    std::cout << "Length of the longest common subsequence: " << lcs_length << "\n";
  }

  virtual void printInfo() override
  {
    printLCSLength();
  }
};

int main(int argc, char *argv[])
{
  cxxopts::Options options("lcs_distributed_length",
                           "Distributed LCS implementation using MPI.");

  options.add_options(
      "inputs",
      {{"sequence_a", "First input sequence.", cxxopts::value<std::string>()},
       {"sequence_b", "Second input sequence.",
        cxxopts::value<std::string>()}});

  auto command_options = options.parse(argc, argv);

  std::string sequence_a = command_options["sequence_a"].as<std::string>();
  std::string sequence_b = command_options["sequence_b"].as<std::string>();

  MPI_Init(NULL, NULL);

  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if (world_rank == 0)
  {
    printf("-------------------- LCS Distributed (Length) --------------------\n");
    printf("n_processes: %d\n\n", world_size);
  }
  MPI_Barrier(MPI_COMM_WORLD);

  int length_a = sequence_a.length();
  int length_b = sequence_b.length();

  const int min_n_cols_per_process = length_b / world_size;
  const int excess = length_b % world_size;

  /* We need to keep track of which columns are mapped to which processes so
  we can gather them together again at the end with MPI_Gatherv.*/
  int *sub_str_widths = new int[world_size];
  int *start_cols = new int[world_size];
  for (int rank = 0; rank < world_size; rank++)
  {
    int start_col, n_cols;
    n_cols = min_n_cols_per_process;
    if (rank < excess)
    {
      start_col = rank * (min_n_cols_per_process + 1);
      n_cols++;
    }
    else
    {
      start_col = (rank * min_n_cols_per_process) + excess;
    }

    start_cols[rank] = start_col;
    sub_str_widths[rank] = n_cols;
  }

  int start_col = start_cols[world_rank];
  int n_cols = sub_str_widths[world_rank];

  // Divide up sequence B.
  std::string local_sequence_b = sequence_b.substr(start_col, n_cols);

  LCSDistributedLength lcs(
      sequence_a,
      local_sequence_b,
      world_size,
      world_rank,
      start_cols,
      sub_str_widths,
      sequence_b);

  // Print solution.
  lcs.print();

  delete[] sub_str_widths;
  delete[] start_cols;

  MPI_Finalize();

  return 0;
}
