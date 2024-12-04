#include <iostream>

#include "cxxopts.hpp" // Header file for option parsing library (cxxopts)
#include "lcs.h"

// Class implementing the Serial version of the Longest Common Subsequence
// algorithm
class LongestCommonSubsequenceSerial : public LongestCommonSubsequence
{
private:
  // Override the solve method from LongestCommonSubsequence class
  virtual void solve() override
  {
    timer.start();        // Start the overall timer to measure the execution time
    matrix_timer.start(); // Start the matrix computation timer

    // Nested loops to iterate through each cell in the matrix and compute the
    // LCS values
    for (int i = 1; i < matrix_height; i++)
    {
      for (int j = 1; j < matrix_width; j++)
      {
        computeCell(i, j); // Calculate the LCS value for cell (i, j)
      }
    }

    // Stop the matrix timer and record the time taken for matrix computations
    matrix_time_taken = matrix_timer.stop();

    // After the matrix is filled, determine the longest common subsequence from
    // the matrix
    determineLongestCommonSubsequence();

    // Stop the overall timer and record the total time taken
    time_taken = timer.stop();
  }

public:
  // Constructor that initializes the sequences and calls the solve method
  LongestCommonSubsequenceSerial(const std::string &sequence_a,
                                 const std::string &sequence_b)
      : LongestCommonSubsequence(sequence_a, sequence_b)
  {
    this->solve(); // Solve the LCS for the given sequences
  }

  // Destructor
  virtual ~LongestCommonSubsequenceSerial() {}

  // Override the print method to display the results
  virtual void print() override
  {
    printInfo();      // Print information about the LCS problem
    printTimeTaken(); // Print the time taken to compute the LCS
  }
};

// Main function for running the serial LCS algorithm
int main(int argc, char *argv[])
{
  // Define and parse command-line options using cxxopts
  cxxopts::Options options("lcs_serial", "Serial LCS implementation.");

  options.add_options(
      "inputs", {
                    {"sequence_a", "First input sequence.",
                     cxxopts::value<std::string>()->default_value("")}, // First input sequence
                    {"sequence_b", "Second input sequence.",
                     cxxopts::value<std::string>()->default_value("")}, // Second input sequence
                    {"input_file", "Path to input .csv file.",
                     cxxopts::value<std::string>()->default_value("")} // Input file.
                });

  // Parse the command-line options
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

  // Print a separator line for clarity in the output
  printf("-------------------- LCS Serial --------------------\n");

  // Create an instance of LongestCommonSubsequenceSerial and solve the LCS
  LongestCommonSubsequenceSerial lcs(sequence_a, sequence_b);

  // Print the length of the LCS and the time taken to compute it
  lcs.printInfo();
  lcs.printTimeTaken();

  return 0; // Exit the program successfully
}
