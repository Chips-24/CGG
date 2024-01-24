#!/bin/bash
#SBATCH --job-name=quicktest
#SBATCH --output=slurm/testout-%j.log
#SBATCH --error=slurm/testerr-%j.log
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1

# Load any necessary modules or set environment variables
# (replace these lines with your specific requirements)

# Move to the directory where your script is located
if [ -n "$SLURM_SUBMIT_DIR" ]; then
    cd "$SLURM_SUBMIT_DIR"
else
    echo "Error: SLURM_SUBMIT_DIR is not set."
fi

if [[ ! -d "slurm" ]]; then
    # If it doesn't exist, create it
    mkdir -p "slurm"
fi

# Run your script
./"$@"


