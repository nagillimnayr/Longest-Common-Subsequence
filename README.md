# Longest Common Subsequence (LCS) Project

This project implements the Longest Common Subsequence (LCS) algorithm in three different versions:

- **Serial**: Single-threaded version.
- **Parallel**: Multi-threaded version.
- **Distributed**: Distributed version using MPI.

## Prerequisites

Ensure that the following are installed on your system:

- **C++14 Compiler** (e.g., g++ or clang++)
- **MPI Compiler** (e.g., mpic++)
- **Make**
- **Python3** (For generating input files)

## File Structure

- `lcs_serial.cpp`: Serial implementation of LCS.
- `lcs_parallel.cpp`: Parallel implementation of LCS using threads.
- `lcs_distributed.cpp`: Distributed implementation of LCS using MPI.
- `lcs.h`: Header file containing Abstract base class that LCS implementations inherit from.
- `timer.h`: Header file containing custom timer class for measuring execution time.
- `cxxopts.hpp`: Header file of third-party library for handling command-line arguments.
- `Makefile`: Makefile for building all three versions of the program.
- `generate_sequences.py`

## Generating Input Files

To test the LCS implementations as easily as possible, a python script, `generate_sequences.py` has been provided for generating input files.

To run this script:

```bash

python generate_sequences.py <length>
```

Where `<length>` is the desired length of the input sequences.

Example:

```bash

python generate_sequences.py 10
```

Will generate a file named `sequences_L10.csv` in a directory named `data/`.

The file will contain two sequences of the specified length, separated by a comma:

```csv
ATGTGCACTG,GATGTGAACG
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

For all of the programs, there are two ways to pass in input sequences.
The first is to pass a path to an input .csv file containing two sequences to the `--input_file` command-line argument:

```bash
./lcs_serial --input_file=data/sequences_L10.csv
```

The second way is to pass the sequences directly to the command-line arguments `--sequence_a` and `--sequence_b`:

```bash
./lcs_serial --sequence_a=ATGTGCACTG --sequence_b=GATGTGAACG
```

If both of these methods are used simultaneously, the program will defer to the .csv file, and will overwrite the sequences with the contents read from the file.

### 1. Run the Serial Version

To run the serial version of the LCS algorithm, use the following command:

```bash
./lcs_serial --sequence_a=<first-sequence> --sequence_b=<second-sequence>
```

or

```bash
./lcs_serial --input_file=<path-to-csv-file>
```

### 2. Run the Parallel Version

To run the parallel version of the LCS algorithm (using multiple threads), use the following command:

```bash
./lcs_parallel --n_threads=<number-of-threads> --sequence_a=<first-sequence> --sequence_b=<second-sequence>
```

or

```bash
./lcs_parallel --n_threads=<number-of-threads> --input_file=<path-to-csv-file>
```

### 3. Run the Distributed Version (MPI)

To run the distributed version of the LCS algorithm (using MPI), use the following command:

```bash
mpirun -n <number-of-processes> lcs_distributed --sequence_a=<first-sequence> --sequence_b=<second-sequence>
```

or

```bash
mpirun -n <number-of-processes> lcs_distributed --input_file=<path-to-csv-file>
```

### Output

Each version of the LCS program will output the time taken for the execution of the algorithm and the computed LCS length.

## Performance Metrics

The program uses a timer to measure the execution time of the LCS algorithm for each version. The time taken for execution will be displayed in the output once the program finishes running.
