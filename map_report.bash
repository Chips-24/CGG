#!/bin/bash
#SBATCH --job-name=map_report
#SBATCH --output=slurm/map_report_%J.o
#SBATCH --error=slurm/map_report_%J.e
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=64
#SBATCH --time=01:00:00
#SBATCH --export=ALL

echo ${SLURM_SUBMIT_DIR}
cd ${SLURM_SUBMIT_DIR}

# Run your script
map --profile -openmp-threads=64 -o "${SLURM_JOB_NAME}_${SLURM_JOB_ID}" ./"$@"
