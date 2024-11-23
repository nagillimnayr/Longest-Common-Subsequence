#include <iostream>

#include "lcs.h"

class LongestCommonSubsequenceDistributed : public LongestCommonSubsequence
{
private:
  const int world_size;
  const int world_rank;

  // Iterate over diagonal and process each cell.
  void processDiagonal(const int start_i, const int start_j)
  {
    int i = start_i;
    int j = start_j;
    while (i >= 0 && j < length_b)
    {
      processCell(i, j);
      i--; // Go up one row.
      j++; // Go right one column.
    }
  }

  virtual void solve() override
  {
    /* For sequences of length m and n, the number of diagonals will be
    (m + n - 1). Each cell of a diagonal can be computed in parallel, as
    they have no dependence on one another. Each diagonal, however, depends
    on the previous two diagonals. */

    const uint n_diagonals = length_a + length_b - 1;

    int a;
    for (a = 0; a < length_a; a++)
    {
      // Start at leftmost cell of the diagonal.
      processDiagonal(a, 0);
    }
    a = length_a - 1;
    print();
    for (int b = 1; b < length_b; b++)
    {
      // Start at leftmost cell of the diagonal.
      processDiagonal(a, b);
    }

    determineLongestCommonSubsequence();
  }

public:
  LongestCommonSubsequenceDistributed(
      const std::string sequence_a,
      const std::string sequence_b,
      const int world_size,
      const int world_rank)
      : LongestCommonSubsequence(sequence_a, sequence_b),
        world_size(world_size),
        world_rank(world_rank)
  {
    this->solve();
  }

  virtual ~LongestCommonSubsequenceDistributed()
  {
  }
};

int main(int argc, char *argv[])
{
  std::string sequence_a = "dlpkgcqiuyhnjka";
  std::string sequence_b = "drfghjkf";

  // MPI_Init(NULL, NULL);

  int world_size;
  // MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int world_rank;
  // MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  world_size = world_rank = 0;
  LongestCommonSubsequenceDistributed lcs(sequence_a, sequence_b, world_size, world_rank);
  lcs.print();

  return 0;
}
