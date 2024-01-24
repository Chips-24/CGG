#!/bin/bash
#SBATCH --job-name=quicktest
#SBATCH --output=testres.log
#SBATCH --error=testerr.log
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1

# Load any necessary modules or set environment variables
# (replace these lines with your specific requirements)

# Move to the directory where your script is located
if [ -n "$SLURM_YOUR_SCRIPT_DIR" ]; then
    cd "$SLURM_YOUR_SCRIPT_DIR"
else
    echo "Error: SLURM_YOUR_SCRIPT_DIR is not set."
fi

# Run your script
./"$@"


