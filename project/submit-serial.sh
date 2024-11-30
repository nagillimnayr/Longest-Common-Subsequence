#!/bin/bash

#SBATCH --cpus-per-task=8
#SBATCH --mem=10G
#SBATCH --time=10:00
#SBATCH --partition=slow

DIR=/home/rcm9/project
EXE=lcs_serial
SEQ_A=$1
SEQ_B=$2

srun "${DIR}/${EXE}" --sequence_a=$SEQ_A --sequence_b=$SEQ_B
