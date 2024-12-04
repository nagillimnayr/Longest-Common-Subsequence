import os
from random import choice
import sys

"""
Usage: python generate_sequences.py <length> <n_pairs>

This will create a .csv file with <n_pairs> number of rows, each row with 2 sequences, 
each of length <length>.

The .csv file will be created under the data/ directory.
"""

os.makedirs('data/', exist_ok=True)


def generate_sequence(length):
  return ''.join([choice('CGTA') for i in range(length)])

def generate_data_set(length, n_pairs):
  lines = [f'{generate_sequence(length)},{generate_sequence(length)}\n' for _ in range(n_pairs)]
  file_name = f'data/sequences_L{length}.csv'
  with open(file_name, 'w') as file:
    file.writelines(lines)
    

def main():
    argv = sys.argv
    argc = len(argv)
    if argc < 3:
        raise Exception("Error: not enough arguments. Usage: python generate_sequences.py <length> <n_pairs>")

    length = int(argv[1])
    n_pairs = int(argv[2])
    
    generate_data_set(length, n_pairs)
  
if __name__ == "__main__":
  main()
