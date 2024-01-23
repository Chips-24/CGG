#!/bin/sh

# This script is used to mesure the time of the execution of a program

if [ "$#" -ne 5 ]; then
    echo "usage ./script.sh <repetitions> <program> <lower_bound> <upper_bound> <step> "
    exit 1
fi

output_dir="output"
if [[ ! -d "$output_dir" ]]; then
    # If it doesn't exist, create it
    mkdir -p "$output_dir"
fi

# Number of times the program will be executed
reps=$1

# Program to execute
prog=./$2

# Program arguments
args="$3 $4 $5"

# Output file
output_file="$output_dir/$(basename $2).out"
echo "" > $output_file

total_time=0.0

for ((i=1; i<=$reps; i++)); do
    # Execute the program and get the output
    output=$($prog $args)
    
    echo "$output" >> $output_file

    # Get the execution time from the output
    execution_time=$(echo "$output" | grep -oP "[0-9]+\.[0-9]+ seconds")

    total_time=$(awk "BEGIN {print $total_time + $execution_time}")
done

# Calculate the average time
average_time=$(awk "BEGIN {print $total_time / $reps}")

echo "Average time for $2 : $average_time seconds"

