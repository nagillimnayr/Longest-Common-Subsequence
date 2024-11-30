import pandas as pd
import os
from random import choice


pd.options.mode.copy_on_write = True

os.makedirs('data/random', exist_ok=True)

lengths = [100, 1000, 10000]

n_sequences = 10

def generate_sequence(length):
  return ''.join([choice('CGTA') for i in range(length)])

def generate_data_set(length):
  df = pd.DataFrame({
    'sequence_a': [generate_sequence(length) for _ in range(n_sequences)],
    'sequence_b': [generate_sequence(length) for _ in range(n_sequences)]
  })
  df.to_csv(f'project/data/random/sequences_L{length}.csv', header=False, index=False)

def main():
  
  for length in lengths:
    generate_data_set(length)
  
if __name__ == "__main__":
  main()
