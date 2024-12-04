import os
from random import choice
import sys

"""
Usage: python generate_sequences.py <length> 

This will create a .csv file named sequences_L<length>.csv with a single row and 2 columns, each column 
containing a sequence of length <length>. 

The .csv file will be created under the data/ directory.
"""

os.makedirs('data/', exist_ok=True)


def generate_sequence(length):
  return ''.join([choice('CGTA') for i in range(length)])

def generate_data_set(length):
  file_name = f'data/sequences_L{length}.csv'
  with open(file_name, 'w') as file:
    file.write(f'{generate_sequence(length)},{generate_sequence(length)}\n')
    

def main():
    argv = sys.argv
    argc = len(argv)
    if argc < 2:
        raise Exception("Error: not enough arguments. Usage: python generate_sequences.py <length>")

    length = int(argv[1])
    
    generate_data_set(length)
  
if __name__ == "__main__":
  main()
