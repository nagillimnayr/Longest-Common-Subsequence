import pandas as pd
import matplotlib as mpl
import matplotlib.pyplot as plt
import os

os.makedirs('graphs', exist_ok=True)

lengths = [100, 1000, 10000]

def make_serial_graphs():
  algo = 'serial'
  for length in lengths:
    in_file = f'data/{algo}/L{length}/{algo}-L{length}.csv'
    df = pd.read_csv(in_file, index_col=0)
    print(df)
    ax = df.plot()
    ax.set_title(f'LCS {algo.title()} - L{length}')
    out_file = f'graphs/{algo}-L{length}.png'
    plt.savefig(out_file)
    

def make_parallel_graphs():
  algo = 'parallel'
  for length in lengths:
    in_file = f'data/{algo}/L{length}/{algo}-L{length}.csv'
    df = pd.read_csv(in_file, index_col=0)
    print(df)
    ax = df.plot()
    ax.set_title(f'LCS {algo.title()} - L{length}')
    ax.set_xlabel('Number of threads')
    ax.set_ylabel('Avg. execution time (s)')
    out_file = f'graphs/{algo}-L{length}.png'
    plt.savefig(out_file)

def make_distributed_graphs():
  algo = 'distributed'
  for length in lengths:
    in_file = f'data/{algo}/L{length}/{algo}-L{length}.csv'
    df = pd.read_csv(in_file, index_col=0)
    print(df)
    ax = df.plot()
    ax.set_title(f'LCS {algo.title()} - L{length}')
    ax.set_xlabel('Number of processes')
    ax.set_ylabel('Avg. execution time (s)')
    out_file = f'graphs/{algo}-L{length}.png'
    plt.savefig(out_file)

def main():
  # make_serial_graphs()
  make_distributed_graphs()
  
if __name__ == '__main__':
  main()
