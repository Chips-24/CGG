#!/bin/sh

# This script is used to mesure the time of the execution of a program

if [ "$#" -ne 6 ]; then
    echo "usage ./mesure.sh <mesurement_id> <repetitions> <program> <lower_bound> <upper_bound> <step> "
    exit 1
fi

output_dir="output"
if [[ ! -d "$output_dir" ]]; then
    # If it doesn't exist, create it
    mkdir -p "$output_dir"
fi

# Mesurement id
mesurement_id=$1

# Number of times the program will be executed
reps=$2

# Program to execute
prog=./$3

# Program arguments
args="$4 $5 $6"

# Output file
output_file="$output_dir/$mesurement_id$(basename $3).out"
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

echo "$average_time $(basename $3) $args" >> $output_dir/$mesurement_id.times.out

echo "Average time for $2 : $average_time seconds"

