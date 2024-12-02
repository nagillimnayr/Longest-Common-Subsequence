import pandas as pd
import matplotlib as mpl
import matplotlib.pyplot as plt
import os

# For rendering Math equations using Tex.
mpl.rcParams["text.usetex"] = True

os.makedirs('graphs', exist_ok=True)

ALGOS = ['serial', 'parallel', 'distributed']

lengths = [100, 1000, 10000]
  

def make_avg_time_graph(length: int):
  in_file = f'data/avg_time_L{length}.csv'
  df = pd.read_csv(in_file, index_col=0)
  print(df)
  ax = df.plot()
  ax.set_title(f'LCS: Execution Time vs Number of Tasks - {length} x {length}')
  ax.set_xlabel('Number of Tasks (threads / processes)')
  ax.set_ylabel('Average Execution Time (s)')
  plt.grid(axis='both', linestyle='--')
  plt.xticks([1, 2, 4, 8])
  out_file = f'graphs/avg_time_L{length}.png'
  plt.savefig(out_file)
  
def make_speedup_graph(length: int):
  in_file = f'data/speedup_L{length}.csv'
  df = pd.read_csv(in_file, index_col=0)
  print(df)
  ax = df.plot()
  ax.set_title(f'LCS: Speedup vs Number of Tasks - {length} x {length}')
  ax.set_xlabel('Number of Tasks (threads / processes)')
  ax.set_ylabel(r'Speedup ($S_p = \frac{T_1}{T_p}$)')
  plt.grid(axis='both', linestyle='--')
  plt.xticks([1, 2, 4, 8])
  out_file = f'graphs/speedup_L{length}.png'
  plt.savefig(out_file)
  
def main():
  for length in lengths:
    make_avg_time_graph(length)
    make_speedup_graph(length)
  
if __name__ == '__main__':
  main()
