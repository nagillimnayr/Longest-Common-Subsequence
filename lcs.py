
# start_seq_a = "amjghexybyrgzczy"
# start_seq_b = "aafcdqbgncrcbihkd"

# start_seq_a = "drfghjk"
# start_seq_b = "dlpkgcqiuyhnjk"

start_seq_a = "dlpkgcqiuyhnjka"
start_seq_b = "drfghjkf"

class Lcs:
  def __init__(self, seq_a: str, seq_b: str):
    self.seq_a = seq_a
    self.seq_b = seq_b
    self.len_a = len(seq_a)
    self.len_b = len(seq_b)
    self.lcs_matrix: list[list[str]] = [ ['' for i in range(self.len_b)] for j in range(self.len_a)  ]
    self.lcs_matrix_count: list[list[int]] = [ [0 for i in range(self.len_b)] for j in range(self.len_a)  ]
    
  def print_matrix_subsequences(self):
    print(" " * 2, end=" ")
    for b in range(self.len_b):
      print(f"{self.seq_b[b]:2}", end=" ")
    print()
    for a in range(self.len_a):
      print(f"{self.seq_a[a]}", end=" ")
      print(self.lcs_matrix[a])
    print()
      
  def print_matrix_counts(self):
    print(" " * 2, end=" ")
    for b in range(self.len_b):
      print(f"{self.seq_b[b]:2}", end=" ")
    print()
    for a in range(self.len_a):
      print(f"{self.seq_a[a]}", end=" ")
      print(self.lcs_matrix_count[a])
    print()
    
  def lcs_iter(self):
    for a in range(self.len_a):
      for b in range(self.len_b):
        # If both characters are the same, add character to the subsequence stored
        # In entry diagonally up and to the left.
        if self.seq_a[a] == self.seq_b[b]:
          sub_seq = self.seq_a[a]
          # If we are in the 0th row or 0th column, then there is no entry to 
          # our top-left.
          if a == 0 or b == 0:
            self.lcs_matrix[a][b] = sub_seq
            self.lcs_matrix_count[a][b] = 1
          else:
            # Get the entry from top-left.
            top_left = self.lcs_matrix[a - 1][b - 1] 
            len_top_left = len(top_left)
            self.lcs_matrix[a][b] = self.lcs_matrix[a - 1][b - 1] + sub_seq
            self.lcs_matrix_count[a][b] = len_top_left + 1
          continue
        # If the elements are not the same, we must get the element from above
        # and the element from the left and use the maximum.
        top = left = ''
        if a > 0:  
          top = self.lcs_matrix[a - 1][b]
        if b > 0: 
          left = self.lcs_matrix[a][b - 1]
          
        len_top = len(top)
        len_left = len(left)
        if len_top >= len_left:
          self.lcs_matrix[a][b] = top
          self.lcs_matrix_count[a][b] = len_top
        else:
          self.lcs_matrix[a][b] = left
          self.lcs_matrix_count[a][b] = len_left
          
    longest_subsequence = self.lcs_matrix[-1][-1]
    # self.print_matrix_subsequences()
    self.print_matrix_counts()
    print('Sequence A: ', self.seq_a)
    print('Sequence B: ', self.seq_b)
    print('Longest common subsequence: ', longest_subsequence)
    print('Longest common subsequence length: ', len(longest_subsequence))
            
    
lcs_obj = Lcs(start_seq_a, start_seq_b)
lcs_obj.lcs_iter()
  
