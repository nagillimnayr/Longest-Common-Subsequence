#include <algorithm> // std::max, std::min
#include <iostream>
#include <mpi.h>

#include "lcs_distributed.h"

/* Row-wise version of distributed LCS. */

class LCSDistributedColumn : public LongestCommonSubsequenceDistributed
{
protected:
  int *comm_buffer; // For sending / receiving to other processes.

  virtual void solve() override
  {
    determineLongestCommonSubsequence();
  }

public:
  LCSDistributedColumn(
      const std::string &sequence_a,
      const std::string &sequence_b,
      const int world_size,
      const int world_rank)
      : LongestCommonSubsequenceDistributed(sequence_a, sequence_b, world_size, world_rank)
  {
    this->solve();
  }

  virtual ~LCSDistributedColumn()
  {
  }
};

int main(int argc, char *argv[])
{
  std::string sequence_a = "dlpkgcqiuyhnjkahftgdrfdsfaaaaagsza";
  std::string sequence_b = "drfghjkfdszfwsfgftradesaardgtfyawa";

  MPI_Init(NULL, NULL);

  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  LCSDistributedColumn lcs(sequence_a, sequence_b, world_size, world_rank);
  // Print solution.
  if (world_rank == 0)
  {
    lcs.print();
  }

  MPI_Finalize();

  return 0;
}
