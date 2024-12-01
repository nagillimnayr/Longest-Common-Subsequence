# README

## Project: Longest Common Subsequence (LCS) (TBD)

This project contains three implementations of the Longest Common Subsequence algorithm:  
1. **Serial Version (`lcs_serial`)**  
2. **Parallel Version (`lcs_parallel`)**  
3. **Distributed Version (`lcs_distributed`)**

The following instructions will guide you through generating input files, compiling the code, and running each version.

## 1. Generating Input Files (TBD)

The input files should represent two sequences for which the LCS will be computed. Below are the steps to generate them using C++:

1. **Input File Generator (`lcs_input_generator.cpp`)**:
   - Compile the provided input generator (if not already compiled):
     ```bash
     g++ -std=c++14 -o lcs_input_generator lcs_input_generator.cpp
     ```
   - Run the executable to generate input files:
     ```bash
     ./lcs_input_generator <size> <output_file>
     ```
     - `<size>`: Length of each sequence.
     - `<output_file>`: Name of the file where the sequences will be stored.

   **Example**:
   ```bash
   ./lcs_input_generator 1000 input.txt
   ```

2. **Input File Format**:
   - Each input file will contain two lines, with one sequence per line.

## 2. Compilation Instructions (TBD)

### Prerequisites
- **Compiler**: `g++` and `mpic++` are required for compilation.
- **Build Tool**: Ensure `make` is installed.

### Compiling the Code
Run the following command to compile all three versions of the program:
```bash
make
```

This will generate three executables:
- `lcs_serial`
- `lcs_parallel`
- `lcs_distributed`

If you need to recompile or clean up, use:
```bash
make clean
```

## 3. Running the Code (TBD)

### 3.1 Serial Version
Run the serial version using:
```bash
./lcs_serial <input_file>
```
**Example**:
```bash
./lcs_serial input.txt
```

### 3.2 Parallel Version
Run the parallel version using:
```bash
./lcs_parallel <input_file> <num_threads>
```
- `<num_threads>`: Number of threads for parallel execution.

**Example**:
```bash
./lcs_parallel input.txt 4
```

### 3.3 Distributed Version
Run the distributed version using MPI:
```bash
mpirun -np <num_processes> ./lcs_distributed <input_file>
```
- `<num_processes>`: Number of processes for distributed execution.

**Example**:
```bash
mpirun -np 4 ./lcs_distributed input.txt
```

## 4. Example Workflow

1. Generate an input file with sequences of size 1000:
   ```bash
   ./lcs_input_generator 1000 input.txt
   ```

2. Compile the code:
   ```bash
   make
   ```

3. Run all versions:
   - Serial:
     ```bash
     ./lcs_serial input.txt
     ```
   - Parallel (with 4 threads):
     ```bash
     ./lcs_parallel input.txt 4
     ```
   - Distributed (with 4 processes):
     ```bash
     mpirun -np 4 ./lcs_distributed input.txt
     ```

## 5. Output Format (TBD)

The output will include:
- The length of the Longest Common Subsequence.
- Execution time for the computation.

Example:
```text
LCS Length: 42
Execution Time: 0.012 seconds
```

## 6. Cleaning Up (TBD)
To remove generated executables and temporary files:
```bash
make clean
```