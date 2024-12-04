

#include <algorithm> // std::max, std::min
#include <iostream>
#include <mpi.h>

#include "cxxopts.hpp"
#include "lcs.h"

/**
 * If the specific longest common subsequence is required, then the sub-matrices
 * can be gathered together once all of the entries have been computed.
 *
 * This version of the distributed LCS implementation tries to improve the
 * performance of the acquisition of the LCS subsequence by performing the
 * backtrace within the local processes and then using MPI send/receive to
 * let the next process know which index to pick up the task from.
 *
 * */
class LCSDistributed : public LongestCommonSubsequence
{
protected:
  const int world_size;
  const int world_rank;

  int lcs_length = -1; /* The length of the longest common subsequence. */

  /* Need to keep track of this info globally for MPI_Gatherv(). */
  int *start_cols;
  int *sub_str_widths;
  std::string global_sequence_b;

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
      matrix_time_taken = matrix_timer.stop();
    }
  }

  /* Broadcast the length of the LCS from the rightmost process to every other
  process.*/
  void broadcastLCSLength()
  {
    if (world_rank == world_size - 1)
    {
      lcs_length = LongestCommonSubsequence::getLongestSubsequenceLength();
    }

    MPI_Bcast(&lcs_length, 1, MPI_INT, world_size - 1, MPI_COMM_WORLD);
  }

  virtual void determineLongestCommonSubsequence() override
  {
    /* Each process will need to know the length of the LCS so that it can
    allocate the necessary buffer space. */
    broadcastLCSLength();

    char *lcs_buffer;
    lcs_buffer = new char[lcs_length + 1];
    lcs_buffer[lcs_length] = '\0';
    int index = lcs_length - 1;

    int comm_buffer[2]; // Used to transmit row index and LCS string index.
    int row = matrix_height - 1;
    /* Each process except the rightmost will have to wait for its neighbor to
    the right to finish. */

    if (world_rank != world_size - 1)
    {
      /* Rightside neighbor must tell us which row to start at. */
      MPI_Recv(
          &comm_buffer,
          2,
          MPI_INT,
          world_rank + 1,
          0,
          MPI_COMM_WORLD,
          MPI_STATUS_IGNORE);
      row = comm_buffer[0];
      index = comm_buffer[1];

      /* Next, the process needs to receive the partially completed LCS from
      the neighbor to the right. */
      MPI_Recv(
          lcs_buffer,
          lcs_length,
          MPI_CHAR,
          world_rank + 1,
          0,
          MPI_COMM_WORLD,
          MPI_STATUS_IGNORE);
    }

    /* Once we have acquired the necessary data from our neighbor, we can
    continue the trace. Always starting from the leftmost column. */
    int col = matrix_width - 1;

    int current, top, left, top_left;
    top = left = top_left = 0;

    while (row > 0 && col > 0 && index >= 0)
    {
      current = matrix[row][col];
      top_left = matrix[row - 1][col - 1];
      top = matrix[row - 1][col];
      left = matrix[row][col - 1];

      if (top_left == current)
      {
        // Go to entry to the top-left.
        row--;
        col--;
        continue;
      }

      /* If the elements above, to the left, and diagonally to the top left
      are all the same,  */
      if (top_left == top && top_left == left)
      {
        lcs_buffer[index] = sequence_a[row - 1];
        index--;
        // Go to entry to the top-left.
        row--;
        col--;
        continue;
      }

      /* If the entry to the top left is lower than the current entry but is not
      equal to the entries above and to the left, then either the one above or
      the one to the left must be the same as the current. */
      if (top == current)
      {
        // Go to the entry above.
        row--;
        continue;
      }
      else
      {
        // If it wasn't the one above, it must be the one to the left.
        col--;
      }
    }

    /* Once this process has finished tracing its sub-matrix, pass the work on
    to the next process. */
    if (world_rank > 0)
    {
      comm_buffer[0] = row;
      comm_buffer[1] = index;
      MPI_Send(
          &comm_buffer,
          2,
          MPI_INT,
          world_rank - 1,
          0,
          MPI_COMM_WORLD);

      MPI_Send(
          lcs_buffer,
          lcs_length,
          MPI_CHAR,
          world_rank - 1,
          0,
          MPI_COMM_WORLD);
    }

    if (world_rank == 0)
    {
      longest_common_subsequence = lcs_buffer;
    }
    delete[] lcs_buffer;
  }

  void solveDistributed()
  {
    matrix_timer.start();
    for (int row = 1; row < matrix_height; row++)
    {
      for (int col = 1; col < matrix_width; col++)
      {
        computeCell(row, col);
      }
    }
    // MPI_Barrier(MPI_COMM_WORLD);
    matrix_time_taken = timer.stop();
  }

  virtual void solve() override
  {
    timer.start();
    solveDistributed();
    determineLongestCommonSubsequence();
    time_taken = timer.stop();
  }

public:
  LCSDistributed(
      const std::string &sequence_a,
      const std::string &sequence_b,
      const int world_size,
      const int world_rank,
      int *start_cols,
      int *sub_str_widths,
      const std::string &global_sequence_b)
      : LongestCommonSubsequence(sequence_a, sequence_b),
        world_size(world_size),
        world_rank(world_rank),
        start_cols(start_cols),
        sub_str_widths(sub_str_widths),
        global_sequence_b(global_sequence_b)
  {

    this->solve();
  }

  virtual ~LCSDistributed()
  {
  }

  virtual int getLongestSubsequenceLength() override
  {
    return lcs_length;
  }

  virtual void printInfo() override
  {
    std::cout << "Longest common subsequence: " << longest_common_subsequence << "\n";
    printLCSLength();
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

  virtual void printTotalTimeTaken()
  {
    if (world_rank == 0)
    {
      printf("Total time taken: %lf\n", time_taken);
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
      printf("rank | n_cols | time_taken\n");
      printStats(0, matrix_time_taken);
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
      printInfo();
      printf("\n");
      printTimeTaken();
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }
};

int main(int argc, char *argv[])
{
  cxxopts::Options options("lcs_distributed",
                           "Distributed LCS implementation using MPI.");

  options.add_options(
      "inputs",
      {
          {"sequence_a", "First input sequence.",
           cxxopts::value<std::string>()->default_value("")}, // First input sequence
          {"sequence_b", "Second input sequence.",
           cxxopts::value<std::string>()->default_value("")}, // Second input sequence
          {"input_file", "Path to input .csv file.",
           cxxopts::value<std::string>()->default_value("")} // Input file.
      });

  auto command_options = options.parse(argc, argv);
  // Retrieve the input sequences from command-line arguments.
  std::string sequence_a = command_options["sequence_a"].as<std::string>();
  std::string sequence_b = command_options["sequence_b"].as<std::string>();
  std::string input_file = command_options["input_file"].as<std::string>();

  if (input_file != "")
  {
    // Read sequences from .csv file if file path was provided.
    read_input_csv(input_file, sequence_a, sequence_b);
  }

  if (sequence_a.length() < 1 || sequence_b.length() < 1)
  {
    std::cerr << "Error: sequences cannot be empty." << std::endl;
    exit(1);
  }

  MPI_Init(NULL, NULL);

  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if (world_rank == 0)
  {
    printf("-------------------- LCS Distributed --------------------\n");
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

  LCSDistributed lcs(
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
