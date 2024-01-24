#!/bin/bash

# Check if the number of arguments is correct
if [ "$#" -lt 1 ]; then
    echo "usage ./speedup.sh <mesurement_id> [input_dir]"
    exit 1
fi

# id of the run to be analyzed
id=$1

# Input directory
if [[ "$#" -lt 2 ]]; then
    # If no input directory is specified, use the default one
    input_dir="output/$id"
else
    # Otherwise, use the specified one
    input_dir="$2"
fi

input_file="$input_dir/times.out"
read time1 time2 <<< "$(awk '{print $1}' $input_file | tr '\n' ' ')"
read prog1 prog2 <<< "$(awk '{print $2}' $input_file | tr '\n' ' ')"
   
# Check if both variables are non-empty
if [ -n "$time1" ] && [ -n "$time2" ]; then
    # Calculate speedup
    speedup=$(awk "BEGIN { printf \"%.4f\", $time1 / $time2 }")

    # Use the variables and speedup as needed
    echo "From $prog1 ($time1 seconds) to $prog2 ($time2 seconds)"
    echo "The speedup is $speedup ($(awk "BEGIN { printf \"%.2f\", ($speedup - 1)*100 }")% faster)"
else
    echo "Error: Invalid input line - skipping."
fi