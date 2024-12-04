# Longest Common Subsequence (LCS) Project

This project implements the Longest Common Subsequence (LCS) algorithm in three different versions:
- **Serial**: Single-threaded version.
- **Parallel**: Multi-threaded version.
- **Distributed**: Distributed version using MPI.

## Prerequisites

Ensure that the following are installed on your system:
- **C++14 Compiler** (e.g., g++ or clang++)
- **MPI Compiler** (e.g., mpic++)
- **CMake** (optional for building)
- **thread** library (for the parallel version)

## File Structure

- `lcs_serial.cpp`: Serial implementation of LCS.
- `lcs_parallel.cpp`: Parallel implementation of LCS using threads.
- `lcs_distributed.cpp`: Distributed implementation of LCS using MPI.
- `lcs.h`: Header file containing LCS function declarations.
- `timer.h`: Header file to measure execution time.
- `cxxopts.hpp`: Header file for handling command-line arguments.
- `Makefile`: Makefile for building all three versions of the program.

## Generating Input Files

To test the LCS implementations, you need input files containing two sequences for which the LCS is calculated. Each sequence is stored in a separate file, and these files should contain strings of characters.

Here's an updated version of the README section that includes information about CSV files, which can be relevant for storing and handling sequence data in a structured format:

```markdown
### Step 1: Create Input Files

You can generate input files manually or use the following formats:

- **Input File 1 (`input1.txt`)**:
  ```
  AGGTAB
  ```
- **Input File 2 (`input2.txt`)**:
  ```
  GXTXAYB
  ```

You can use any text editor to create these files, or you can use a script to generate different sequences for testing.

In addition to text files, you can also use **CSV files** to store sequences in a more structured format. For example:

- **CSV Input File (`input.csv`)**:
  ```
  AGGTAB, GXTXAYB
  ```

In this case, the sequences are stored as comma-separated values, which can be parsed easily by the program.

### Example of Python script to generate input files (Text and CSV format):
```python
import csv

def generate_input_files():
    # Create text files
    with open('input1.txt', 'w') as f1, open('input2.txt', 'w') as f2:
        f1.write("AGGTAB\n")
        f2.write("GXTXAYB\n")

    # Create CSV file
    with open('input.csv', 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["AGGTAB", "GXTXAYB"])

generate_input_files()
```

This Python script will generate:
1. Two text files (`input1.txt` and `input2.txt`) containing the sequences.
2. A CSV file (`input.csv`) with the sequences stored as comma-separated values.

You can choose the format that best suits your needs, but both text and CSV formats are supported for testing.

```

This Python script will generate the two input files needed for the LCS algorithms. Save this script and run it in the same directory as your LCS project.

## Building the Project

To build the project, you can use the `Makefile`. Run the following command in the root directory of the project:

```bash
make
```

This will compile all the necessary executables:
- `lcs_serial`: Serial version of LCS.
- `lcs_parallel`: Parallel version of LCS.
- `lcs_distributed`: Distributed version of LCS using MPI.

If you need to clean the project directory (e.g., remove compiled files), run:

```bash
make clean
```

## Running the Programs

### 1. Run the Serial Version

To run the serial version of the LCS algorithm, use the following command:
```bash
./lcs_serial --seqeunce_a input1.txt --sequence_b input2.txt
```

This will run the serial LCS algorithm on the input sequences stored in `input1.txt` and `input2.txt`.

### 2. Run the Parallel Version

To run the parallel version of the LCS algorithm (using multiple threads), use the following command:
```bash
./lcs_parallel --n_threads X --sequence_a input1.txt --sequence_b input2.txt
```

This will run the parallel LCS algorithm on the input sequences, utilizing multiple threads for faster computation.

In the above command:
- `--n_threads X`: Specifies that X processes will be used for the distributed execution. You can adjust the number based on your system's available resources.
- `--sequence_a`: Input parameter for the first seqeunce (A).
- `--sequence_b`: Input parameter for the second seqeunce (B).


### 3. Run the Distributed Version (MPI)

To run the distributed version of the LCS algorithm (using MPI), use the following command:
```bash
mpirun --n_threads X --sequence_a input1.txt --sequence_b input2.txt
```
This will run the distributed LCS algorithm on the input sequences, utilizing multiple processes for faster computation.

In the above command:
- `mpirun`: Starts the MPI processes.
- `--n_threads X`: Specifies that X processes will be used for the distributed execution. You can adjust the number based on your system's available resources.
- `--sequence_a`: Input parameter for the first seqeunce (A).
- `--sequence_b`: Input parameter for the second seqeunce (B).

### Output

Each version of the LCS program will output the time taken for the execution of the algorithm and the computed LCS length.

## Performance Metrics

The program uses a timer to measure the execution time of the LCS algorithm for each version. The time taken for execution will be displayed in the output once the program finishes running.

## Clean Up

To remove the compiled binaries and object files:

```bash
make clean
```

This will remove the following files:
- `lcs_serial`
- `lcs_parallel`
- `lcs_distributed`
- Object files (`*.o`, `*.obj`)

## Summary of Updates:
1. **Generating Input Files**: 
   - Detailed steps are included to create the input files manually or with a Python script.
   - Clear instructions on how to generate `input1.txt` and `input2.txt`.
   
2. **Running the Programs**: 
   - Clearly explains how to run each of the three versions (serial, parallel, distributed).
   - Added specific example commands for each version of the program, making it easy to follow.
