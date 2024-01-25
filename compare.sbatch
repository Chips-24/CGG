#!/bin/bash
#SBATCH --job-name=compare
#SBATCH --output=slurm/slurm-%j.out
#SBATCH --error=slurm/slurm-%j.err
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=16
#SBATCH --export=ALL
#SBATCH --exclusive

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

if [ "$#" -ne 7 ]; then
    echo "usage ./compare.sbatch <mesurement_id> <repetitions> <old_program> <new_program> <lower_bound> <upper_bound> <step> "
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
