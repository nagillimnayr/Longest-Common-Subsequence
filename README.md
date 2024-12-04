# **Longest Common Subsequence (LCS) Project**

## **Overview**

This project implements three versions of the Longest Common Subsequence (LCS) algorithm:

- **Serial**: A basic single-threaded LCS implementation.
- **Parallel**: A parallelized version using multiple threads.
- **Distributed**: A distributed version using MPI to calculate the LCS across multiple processes.

The goal is to compare the performance of these implementations using different sequence lengths, and to generate output files detailing the results of each run. The project utilizes Slurm for job scheduling and batch processing on a cluster environment.

## **Directory Structure**

```
LCS_Project/
├── data/
│   ├── avg_time_L100.csv
│   ├── avg_time_L1000.csv
│   ├── avg_time_L10000.csv
│   ├── L100.csv
│   ├── L1000.csv
│   ├── L10000.csv
│   ├── speedup_L100.csv
│   ├── speedup_L1000.csv
│   └── speedup_L10000.csv
├── graphs/
│   ├── avg_time_L100.png
│   ├── avg_time_L1000.png
│   ├── avg_time_L10000.png
│   ├── speedup_L100.png
│   ├── speedup_L1000.png
│   └── speedup_L10000.png
├── project/
|   └── Data/random/
|         ├──sequences_L10.csv
|         ├── sequences_L100.csv
|         ├── sequences_L1000.csv
|         └── sequences_L10000.csv
│   ├── lcs_distributed.cpp
│   ├── lcs_parallel.cpp
│   ├── lcs_serial.cpp
│   ├── lcs.h
│   ├── Makefile
│   ├── run-distributed.py
│   ├── run-parallel.py
│   ├── run-serial.py
│   ├── submit-distributed.sh
│   ├── submit-parallel.sh
│   ├── submit-serial.sh
│   ├── timer.h
│   ├── .gitignore
│   ├── .gitattributes
│   ├── generate_sequences.py
│   ├── make_graphs.py
│   └── record_data.py
├── README.md
└── report.sh
```

## **Setup**

Before running the scripts, ensure you have the following:

- **Sequences**: Ensure that the sequence files are available in `data/` directory. These files should follow the naming convention `L<length>.csv` where `<length>` is the length of the sequence (e.g., `L100.csv`, `L1000.csv`, `L10000.csv`).
  
- **Slurm Batch Scripts**: Both `submit-serial.sh`, `submit-parallel.sh`, and `submit-distributed.sh` are used to submit Slurm jobs for serial, parallel, and distributed LCS runs, respectively. Ensure these scripts are properly configured for your system.

---

## **Run Parallel Version**

The `run-parallel.py` script automates the submission of parallel LCS jobs for execution on a cluster using Slurm. It runs the LCS algorithm for different sequence lengths and multiple runs, with varying numbers of threads.

### **Script Details**  
1. **User ID**: Set to `"rcm9"` (you can modify this to your own user ID).
2. **Algorithm**: This script is designed for the parallel version (`ALGO = "parallel"`).
3. **Thread Counts**: The script runs jobs for thread counts of 1, 2, 4, and 8 (`thread_counts = [1, 2, 4, 8]`).
4. **Sequence Lengths**: The script runs jobs for sequence lengths of 10000, 1000, and 100 (`sequence_lengths = [10000, 1000, 100]`).
5. **Runs**: Each job will be executed 8 times (`n_runs = 8`).

### **Setup and Execution**:
1. **Ensure Sequences**: Ensure that the sequences are generated in the `data/` directory in the format `L<length>.csv`. The script reads these sequences to run the LCS algorithm.
2. **Job Submission**: The script submits each parallel job using Slurm's `sbatch` command. It waits for jobs to finish before submitting new ones by checking the current job queue with `squeue`.
3. **Output Directory**: For each sequence length, the script creates a subdirectory within the output directory to store the job results.

### **Directory Structure for Parallel Output**  
The results from running the parallel version of the program will be stored in the following directory structure:

```
output/
└── parallel/
    ├── L10000/
    │   ├── parallel-L10000-T1-R1.out
    │   ├── parallel-L10000-T2-R1.out
    ├── L1000/
    ├── L100/
```

The output files are named using the following convention:

- **Parallel Version**: `parallel-L<length>-T<threads>-R<run>.out`

### **Monitoring Jobs**  
The script monitors the job queue using `squeue` to check how many jobs are running. It ensures that only one job is submitted at a time. The script pauses and waits for jobs to finish before proceeding to the next job.

### **Command to Run the Script**:
To run the parallel version of the algorithm, execute the following command:

```bash
python3 run-parallel.py
```

### **Job Submission**:
The script submits jobs using the Slurm batch script `submit-parallel.sh`. Ensure that this script is properly configured for your system's job submission.

---

## **Run Serial Version**

The `run-serial.py` script automates the submission of serial LCS jobs for execution on a single node using Slurm. It runs the LCS algorithm for different sequence lengths and multiple runs.

### **Script Details**  
1. **User ID**: Set to `"rcm9"` (you can modify this to your own user ID).  
2. **Algorithm**: This script is designed for the serial version (`ALGO = "serial"`).
3. **Sequence Lengths**: The script runs jobs for sequence lengths of 100, 1000, and 10000.
4. **Runs**: Each job will be executed 8 times (`n_runs = 8`).

### **Setup and Execution**:
1. **Ensure Sequences**: Ensure that the sequences are generated in the `data/` directory in the format `L<length>.csv`. The script reads these sequences to run the LCS algorithm.
2. **Job Submission**: The script submits each serial job using Slurm's `sbatch` command. It waits for jobs to finish before submitting new ones by checking the current job queue with `squeue`.
3. **Output Directory**: For each sequence length, the script creates a subdirectory within the output directory to store the job results.
   
### **Directory Structure for Serial Output**  
The results from running the serial version of the program will be stored in the following directory structure:

```
output/
└── serial/
    ├── L100/
    │   ├── serial-L100-R1.out
    │   ├── serial-L100-R2.out
    ├── L1000/
    ├── L10000/
```

The output files are named using the following convention:

- **Serial Version**: `serial-L<length>-R<run>.out`

### **Monitoring Jobs**  
The script monitors the job queue using `squeue` to check how many jobs are running. It ensures that only one job is submitted at a time. The script pauses and waits for jobs to finish before proceeding to the next job.

### **Command to Run the Script**:
To run the serial version of the algorithm, execute the following command:

```bash
python3 run-serial.py
```

### **Job Submission**:
The script submits jobs using the Slurm batch script `submit-serial.sh`. Ensure that this script is properly configured for your system's job submission.

---

## **Run Distributed Version**

The `run-distributed.py` script automates the submission of distributed LCS jobs for execution across multiple nodes using Slurm. It runs the LCS algorithm for different sequence lengths and multiple runs, leveraging distributed resources.

### **Script Details**  
1. **User ID**: Set to `"rcm9"` (you can modify this to your own user ID).
2. **Algorithm**: This script is designed for the distributed version (`ALGO = "distributed"`).
3. **Sequence Lengths**: The script runs jobs for sequence lengths of 10000, 1000, and 100 (`sequence_lengths = [10000, 1000, 100]`).
4. **Runs**: Each job will be executed 8 times (`n_runs = 8`).
5. **Nodes and Tasks**: Distributed runs may involve multiple nodes, and the specific configurations are controlled by the job submission script `submit-distributed.sh`.

### **Setup and Execution**:
1. **Ensure Sequences**: Ensure that the sequences are generated in the `data/` directory in the format `L<length>.csv`. The script reads these sequences to run the LCS algorithm.
2. **Job Submission**: The script submits each distributed job using Slurm's `sbatch` command. It waits for jobs to

 finish before submitting new ones by checking the current job queue with `squeue`.
3. **Output Directory**: For each sequence length, the script creates a subdirectory within the output directory to store the job results.

### **Directory Structure for Distributed Output**  
The results from running the distributed version of the program will be stored in the following directory structure:

```
output/
└── distributed/
    ├── L10000/
    │   ├── distributed-L10000-T1-R1.out
    │   ├── distributed-L10000-T2-R1.out
    ├── L1000/
    ├── L100/
```

The output files are named using the following convention:

- **Distributed Version**: `distributed-L<length>-T<threads>-R<run>.out`

### **Monitoring Jobs**  
The script monitors the job queue using `squeue` to check how many jobs are running. It ensures that only one job is submitted at a time. The script pauses and waits for jobs to finish before proceeding to the next job.

### **Command to Run the Script**:
To run the distributed version of the algorithm, execute the following command:

```bash
python3 run-distributed.py
```

### **Job Submission**:
The script submits jobs using the Slurm batch script `submit-distributed.sh`. Ensure that this script is properly configured for your system's job submission.

---

## **Generating Input Sequences**

The `generate_sequences.py` script generates the input sequence files (`L100.csv`, `L1000.csv`, `L10000.csv`) used for the LCS algorithm. The sequences are generated with random characters, and the CSV files are stored in the `data/` directory.

---

## **Performance Graphs**

The `make_graphs.py` script processes the output data (in CSV format) and generates performance graphs, such as execution time and speedup, for different sequence lengths and algorithm types. The resulting graphs are stored in the `graphs/` directory.

To generate the graphs, run the following:

```bash
python3 make_graphs.py
```

---

## **Conclusion**

This project provides a comprehensive comparison of serial, parallel, and distributed LCS algorithms. The performance metrics, including execution time and speedup, are recorded and visualized for different sequence lengths, offering insights into the efficiency of the different algorithm versions.