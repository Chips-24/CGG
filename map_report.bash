#!/bin/bash
#SBATCH --job-name=perfReport
#SBATCH --output=perfReport_%J.o
#SBATCH --error=perfReport_%J.e
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=01:00:00
#SBATCH --export=ALL

echo ${SLURM_SUBMIT_DIR}
cd ${SLURM_SUBMIT_DIR}

# Run your script
map --profile -o "${SLURM_JOB_NAME}_${SLURM_JOB_ID}" ./"$@"

