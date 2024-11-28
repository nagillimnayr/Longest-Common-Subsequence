
import os
import time
import subprocess

USER_ID = "rcm9"

OUT_DIR="output/lcs_distributed"
os.makedirs(OUT_DIR, exist_ok=True)

process_counts = [1, 2, 4, 8]
n_runs = 10

def user_jobs_running():
    try:
        result = subprocess.run(["squeue", "-u", USER_ID], stdout = subprocess.PIPE, universal_newlines = True)
        count = len(result.stdout.strip().split("\n")) - 1
        return count
    except subprocess.CalledProcessError as e:
        print(f"Error checking jobs: {e}")
        exit(1)

def run_algo(sequence_a: str, sequence_b: str):
  for n_processes in process_counts:
    for run in range(1, n_runs + 1):
      outfile = f"distributed-p{n_processes}-r{run}.out"
      outfile_path = f"{OUT_DIR}/{outfile}"
      subprocess.run([
        'sbatch', 
        f'--output={outfile_path}', 
        f'--ntasks={n_processes}', 
        'submit-distributed.sh',
        sequence_a,
        sequence_b  
      ])
      
def main():
  with open('data/random/sequences_10.csv') as csv_file:
    lines = csv_file.read().splitlines()
  
  sequences = lines[0].split(sep=',')
  sequences = [s.strip() for s in sequences]
  sequence_a, sequence_b = sequences
  print(sequence_a, ', ', sequence_b, sep='')
  run_algo(sequence_a, sequence_b)
  
if __name__ == '__main__':
  main()
