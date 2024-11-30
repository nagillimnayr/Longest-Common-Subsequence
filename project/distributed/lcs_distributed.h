#ifndef _LCS_DISTRIBUTED_H_
#define _LCS_DISTRIBUTED_H_

#include <algorithm> // std::max, std::min
#include <iostream>
#include <mpi.h>

#include "../lcs.h"

/**
 * Distributed LCS. This version divides the score matrix into blocks of
 * columns and assigns each block to a process.
 *
 * Example (Numbers represent process ranks):
  ```
  [ 0 0 1 1 2 2 ]
  [ 0 0 1 1 2 2 ]
  [ 0 0 1 1 2 2 ]
  [ 0 0 1 1 2 2 ]
  [ 0 0 1 1 2 2 ]
  [ 0 0 1 1 2 2 ]
  ```
 *
 * With this task mapping, each process depends only on data from the process
 * to its left, which makes communication quite simple. When computing an entry
 * in a process' leftmost column, it must receive the data from the entry in
 * the same row but rightmost column of the left neighboring process. After
 * computing an entry in the rightmost column, the data from that entry must
 * be sent to the neighboring process to the right.
 *
 * */
class LCSDistributedBase : public LongestCommonSubsequence
{
protected:
  const int world_size;
  const int world_rank;

  Timer total_timer;
  double total_time_taken = 0.0;

  int lcs_length = -1; /* The length of the longest common subsequence. */

  /* Need to keep track of this info globally for MPI_Gatherv(). */
  int *start_cols;
  int *sub_str_widths;

  virtual void computeCell(const int row, const int col)
  {
    int comm_value;
    /* If we are computing a cell in the leftmost column of our local matrix,
      then we need to get data from the cells in the rightmost column of our
      neighboring process to the left. Unless we are the leftmost process. */
    if (col == 1 && world_rank != 0)
    {
      MPI_Recv(
          &comm_value,
          1, // Only need a single value.
          MPI_UNSIGNED,
          world_rank - 1, // Source: Get from neighbor to the left.
          row,            // Tag: Row index.
          MPI_COMM_WORLD,
          MPI_STATUS_IGNORE);
      // Store the value in the local matrix.
      matrix[row][col - 1] = comm_value;
    }

    LongestCommonSubsequence::computeCell(row, col);

    /* If we are computing a cell in the rightmost column of our local
    matrix, we must send the results to our neighbor to the right once we
    are done. Unless we are the rightmost process. */
    if (col == matrix_width - 1 && world_rank != world_size - 1)
    {
      comm_value = matrix[row][col];
      MPI_Send(
          &comm_value,
          1,
          MPI_UNSIGNED,
          world_rank + 1, // Destination: Send to neighbor to the right.
          row,
          MPI_COMM_WORLD);
    }
  }

  virtual void determineLongestSubsequenceLength()
  {
    /* Once the sub-matrices have been computed, we will need to send the
    bottom right entry of the rightmost process to the root process. */
    if (world_rank == world_size - 1)
    {
      lcs_length = LongestCommonSubsequence::getLongestSubsequenceLength();
      MPI_Send(
          &lcs_length,
          1,
          MPI_INT,
          0,
          0,
          MPI_COMM_WORLD);
    }
    else if (world_rank == 0)
    {
      MPI_Recv(
          &lcs_length,
          1,
          MPI_INT,
          world_size - 1,
          0, MPI_COMM_WORLD,
          MPI_STATUS_IGNORE);
      total_time_taken = total_timer.stop();
    }
  }

  virtual void solve() override
  {
    total_timer.start();
    timer.start();

    for (int row = 1; row < matrix_height; row++)
    {
      for (int col = 1; col < matrix_width; col++)
      {
        computeCell(row, col);
      }
    }

    time_taken = timer.stop();

    total_time_taken = total_timer.stop();
  }

public:
  LCSDistributedBase(
      const std::string &sequence_a,
      const std::string &sequence_b,
      const int world_size,
      const int world_rank,
      int *start_cols,
      int *sub_str_widths)
      : LongestCommonSubsequence(sequence_a, sequence_b),
        world_size(world_size),
        world_rank(world_rank),
        start_cols(start_cols),
        sub_str_widths(sub_str_widths)
  {
  }

  virtual ~LCSDistributedBase()
  {
  }

  void printPerProcessMatrices()
  {
    for (int rank = 0; rank < world_size; rank++)
    {
      if (rank == world_rank)
      {
        std::cout << "\nRank: " << world_rank << "\n";
        printMatrix();
      }
      MPI_Barrier(MPI_COMM_WORLD);
    }
  }

  void printTotalTime()
  {
    if (world_rank == 0)
    {
      printf("Total time taken: %lf\n", total_time_taken);
    }
  }

  void printStats(int rank, double time)
  {
    if (world_rank != 0)
      return;

    printf("%4d | %6d | %lf\n",
           rank,
           sub_str_widths[rank],
           time);
  }

  void printPerProcessStats()
  {
    if (world_rank == 0)
    {
      printf("\nrank | n_cols | time_taken\n");
      printStats(0, time_taken);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    /* Send stats to root process to ensure proper ordering of print statements. */
    for (int rank = 1; rank < world_size; rank++)
    {

      if (rank == world_rank)
      {
        MPI_Send(
            &time_taken,
            1,
            MPI_DOUBLE,
            0,
            0,
            MPI_COMM_WORLD);
      }
      else if (world_rank == 0)
      {
        double time;
        MPI_Recv(
            &time,
            1,
            MPI_DOUBLE,
            rank,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);
        printStats(rank, time);
      }
    }
  }

  virtual void print() override
  {
    printPerProcessStats();
    MPI_Barrier(MPI_COMM_WORLD);
    if (world_rank == 0)
    {
      printTotalTime();
      printf("\n");
      printInfo();
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }
};

#endif
