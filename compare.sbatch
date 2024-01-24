#!/bin/bash
#SBATCH --job-name=speedup_calculation
#SBATCH --output=results.log
#SBATCH --error=errors.log
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

if [ "$#" -ne 7 ]; then
    echo "usage ./speedup.sbatch <mesurement_id> <repetitions> <old_program> <new_program> <lower_bound> <upper_bound> <step> "
    exit 1
fi

id=$1
reps=$2
old=$3
new=$4
args="$5 $6 $7"

# Run your script
./mesure.sh $id $reps $old $args
./mesure.sh $id $reps $new $args
