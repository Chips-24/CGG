#!/bin/bash
#SBATCH --job-name=quicktestOMP
#SBATCH --output=slurm/testOMPout-%j.log
#SBATCH --error=slurm/testOMPerr-%j.log
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=16
#SBATCH --time=01:00:00
#SBATCH --export=ALL

echo ${SLURM_SUBMIT_DIR}
cd ${SLURM_SUBMIT_DIR}

# Run your script
./"$@"

