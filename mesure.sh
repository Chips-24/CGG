#!/bin/sh

# This script is used to mesure the time of the execution of a program

if [ "$#" -ne 6 ]; then
    echo "usage ./mesure.sh <mesurement_id> <repetitions> <program> <lower_bound> <upper_bound> <step> "
    exit 1
fi

# Mesurement id
mesurement_id=$1

output_dir="output/$mesurement_id"
if [[ ! -d "$output_dir" ]]; then
    # If it doesn't exist, create it
    mkdir -p "$output_dir"
fi

# Number of times the program will be executed
reps=$2

# Program to execute
prog_path=$3
prog=./$prog_path

# Program arguments
args="$4 $5 $6"

# Output file
output_file="$output_dir/$(echo "$prog_path" | tr '/' '_').out"

# Remove the output file if it exists
if [[ -f "$output_file" ]]; then
    rm "$output_file"
fi

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

echo "$average_time $prog_path $args" >> $output_dir/times.out

echo "Average time for $3 : $average_time seconds"

