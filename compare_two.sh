#!/bin/bash

ERROR_FILE="error_compare_two.txt"

total_args=$#
if [ $total_args -lt 5 ]; then
    echo "Usage: $0 <prog1> <prog2> <lower_bound> <upper_bound> <samples> [-r reps | -c core | -o]"
    exit 1
fi

REPETITIONS=1
CORE=0

#Test if args -r or/and -c are present
if [ $total_args -gt 5 ]; then
    # Iterate from 6 to the number of arguments
    for ((i = 6; i <= total_args; i++)); do
        if [ "${!i}" == "-r" ]; then        # Set the number of repetitions
            i=$((i+1))
            REPETITIONS=${!i}
        elif [ "${!i}" == "-c" ]; then      # Set the core to use
            i=$((i+1))
            CORE=${!i}
        elif [ "${!i}" == "-o" ]; then      # Override error file
            rm -f $ERROR_FILE
        fi
    done
fi

FILE1=$1
FILE1_NAME=$(basename "${FILE1}")
FILE2=$2
FILE2_NAME=$(basename "${FILE2}")
LOWER_BOUND=$3
UPPER_BOUND=$4
SAMPLES=$5

# Get a list of compiler subdirectories
BINARIES=($FILE1 $FILE2)

# Declare an associative array to store information for each compiler
declare -A RESULTS

echo "# Comparing $FILE1 and $FILE2 :"
echo "# Run with : $LOWER_BOUND $UPPER_BOUND $SAMPLES for $REPETITIONS reps"
echo ""

# Run executables for each compiler
for ((i = 0; i < ${#BINARIES[@]}; i++)); do
    CURRENT_BINARY=${BINARIES[$i]}

    all_estimates=()
    estimatation_diff=false
    all_found=()
    found_diff=false
    total_time=0

    for ((rep = 0; rep < REPETITIONS; rep++)); do
        # Run the executable and capture output
        OUTPUT=$(taskset -c $CORE ./$CURRENT_BINARY $LOWER_BOUND $UPPER_BOUND $SAMPLES)

        # Extract number of zeros and time
        I_ESTIMATE=$(echo "$OUTPUT" | awk '/I estimate I will find/ { print $6 }')
        I_FOUND=$(echo "$OUTPUT" | awk '/I found/ { print $3 }')
        TIME=$(echo "$OUTPUT" | awk '/Zeros in/ { print $(NF-1) }')

        all_estimates+=($I_ESTIMATE)
        all_found+=($I_FOUND)

        if [ $rep -gt 0 ]; then
            if [ $I_ESTIMATE != ${all_estimates[(($rep-1))]} ]; then
                estimatation_diff=true
            fi
            if [ $I_FOUND != ${all_found[(($rep-1))]} ]; then
                found_diff=true
            fi
        fi

        # Accumulate total time
        total_time=$(awk "BEGIN { print $total_time + $TIME }")
    done

    if [ $estimatation_diff == true ] || [ $found_diff == true ] ; then
        echo "$CURRENT_BINARY $LOWER_BOUND $UPPER_BOUND $SAMPLES" >> $ERROR_FILE
        [ $estimatation_diff == true ] && echo "Estimated: ${all_estimates[@]}" >> $ERROR_FILE
        [ $found_diff == true ] && echo "Found: ${all_found[@]}" >> $ERROR_FILE
        echo "" >> $ERROR_FILE
        echo "The estimations or the found number of zeros are different between the repetitions"
    fi

    # Calculate the mean time
    mean_time=$(awk "BEGIN { print $total_time / $REPETITIONS }")

    # Store information in the associative array
    RESULTS["$CURRENT_BINARY"]="${all_estimates[0]}|${all_found[0]}|$mean_time"

    # Print results
    printf "  # $CURRENT_BINARY : %s zeros in %s seconds\n\n" "${all_found[0]}" "$mean_time"

done

# Calculate and display speedup between the last and the first binariy if there are at least three compilers

    FIRST_INFO=(${RESULTS["$FILE1"]//|/ })
    SECOND_INFO=(${RESULTS["$FILE2"]//|/ })

    FIRST_TIME=${FIRST_INFO[2]}
    SECOND_TIME=${SECOND_INFO[2]}

    if (( $(echo "$FIRST_TIME > $SECOND_TIME" | bc -l) )); then
        FIRST_BINARY=$FILE1
        SECOND_BINARY=$FILE2
    else
        FIRST_BINARY=$FILE2
        SECOND_BINARY=$FILE1
        temp=$FIRST_TIME
        FIRST_TIME=$SECOND_TIME
        SECOND_TIME=$temp
    fi

    SPEEDUP=$(awk "BEGIN { print $FIRST_TIME / $SECOND_TIME }")
    echo "Speedup from $FIRST_BINARY to $SECOND_BINARY: $SPEEDUP"

