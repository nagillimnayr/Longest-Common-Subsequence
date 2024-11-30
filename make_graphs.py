import pandas as pd
import matplotlib as mpl
import matplotlib.pyplot as plt
import os

os.makedirs('graphs', exist_ok=True)

ALGOS = ['serial', 'parallel', 'distributed']

lengths = [100, 1000, 10000]

def make_df(length: int):
  in_file = f'data/L{length}.csv'
  return pd.read_csv(in_file, index_col=0)
  

def make_graph(length: int):
  df = make_df(length)
  print(df)
  ax = df.plot()
  ax.set_title(f'LCS - {length} x {length}')
  ax.set_xlabel('Number of tasks (threads/ processes)')
  ax.set_ylabel('Average Execution Time (s)')
  plt.grid(axis='both', linestyle='--')
  out_file = f'graphs/L{length}.png'
  plt.savefig(out_file)
  plt.xticks([1, 2, 4, 8])
  
def main():
  for length in lengths:
    make_graph(length)
  
if __name__ == '__main__':
  main()
