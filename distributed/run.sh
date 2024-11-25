#!/bin/bash

set -e
make clean
make lcs_distributed_column
mpirun -n 4 ./lcs_distributed_column
