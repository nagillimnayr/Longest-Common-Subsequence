#!/bin/bash

#SBATCH --cpus-per-task=1
#SBATCH --nodes=1
#SBATCH --mem=10G
#SBATCH --time=10:00
#SBATCH --partition=slow

EXE=lcs_distributed
SEQ_A=$1
SEQ_B=$2

srun echo "----------------- LCS Distributed -----------------"
srun "./${EXE} --sequence_a=${SEQ_A} --sequence_b=${SEQ_B}"
