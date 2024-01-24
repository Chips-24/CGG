#!/bin/bash
#SBATCH --job-name=Rieman_OMP
#SBATCH --output=Rieman_OMP_%J.o
#SBATCH --error=Rieman_OMP_%J.e
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=16
#SBATCH --time=01:00:00
#SBATCH --export=ALL

echo ${SLURM_SUBMIT_DIR}
cd ${SLURM_SUBMIT_DIR}

# Run your script
./"$@"

