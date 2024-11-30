
import os
import time
import subprocess

USER_ID = "rcm9"
ALGO = "serial"
OUT_DIR =f"output/{ALGO}"
os.makedirs(OUT_DIR, exist_ok=True)

sequence_lengths = [100, 1000, 10000, 100000]
n_runs = 8

def user_jobs_running():
    try:
        result = subprocess.run(["squeue", "-u", USER_ID], stdout = subprocess.PIPE, universal_newlines = True)
        count = len(result.stdout.strip().split("\n")) - 1
        return count
    except subprocess.CalledProcessError as e:
        print(f"Error checking jobs: {e}")
        exit(1)
        
def get_sequences(sequence_length):
    with open(f'data/random/sequences_L{sequence_length}.csv') as csv_file:
      lines = csv_file.read().splitlines()
    sequences = lines[0].split(sep=',')
    sequences = [s.strip() for s in sequences]
    return sequences

def main():
  for sequence_length in sequence_lengths:
    out_dir = f"{OUT_DIR}/L{sequence_length}"
    os.makedirs(out_dir, exist_ok=True)
    sequence_a, sequence_b = get_sequences(sequence_length)
    for run in range(1, n_runs + 1):
      outfile = f"{ALGO}-L{sequence_length}-R{run}.out"
      outfile_path = f"{out_dir}/{outfile}"
      subprocess.run([
        'sbatch', 
        f'--output={outfile_path}', 
        f'submit-{ALGO}.sh',
        sequence_a,
        sequence_b  
      ])
      while (user_jobs_running() > 0):
        time.sleep(3)
      
  
if __name__ == '__main__':
  main()
