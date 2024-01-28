#!/bin/bash

ERROR_FILE="error_compare_compilers.txt"

total_args=$#
if [ $total_args -lt 4 ]; then
    echo "Usage: $0 <source_file> <lower_bound> <upper_bound> <samples> [-r reps | -c core | -o]"
    exit 1
fi

REPETITIONS=1
CORE=2

#Test if args -r or/and -c are present
if [ $total_args -gt 4 ]; then
    # Iterate from 5 to the number of arguments
    for ((i = 5; i <= total_args; i++)); do
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

SOURCE_FILE=$1
FILE_NAME=$(basename "${SOURCE_FILE%.*}")
LOWER_BOUND=$2
UPPER_BOUND=$3
SAMPLES=$4

BUILD_DIR="build"
SRC_DIR="src"

# Get a list of compiler subdirectories
COMPILERS=($(find $BUILD_DIR -mindepth 1 -maxdepth 1 -type d))

# Declare an associative array to store information for each compiler
declare -A RESULTS

echo "# Comparing compilers for $FILE_NAME :"
echo "# Run with : $LOWER_BOUND $UPPER_BOUND $SAMPLES for $REPETITIONS reps"
echo ""

# Run executables for each compiler
for ((i = 0; i < ${#COMPILERS[@]}; i++)); do
    CURRENT_COMPILER=${COMPILERS[$i]}
    CURRENT_COMPILER_NAME=$(basename "$CURRENT_COMPILER")

    # Check if the executable exists
    if [ ! -f "$CURRENT_COMPILER/$FILE_NAME" ]; then
        echo "  #$CURRENT_COMPILER_NAME : executable $FILE_NAME not found"
        echo ""
        continue
    fi

    all_estimates=()
    estimatation_diff=false
    all_found=()
    found_diff=false
    total_time=0

    for ((rep = 0; rep < REPETITIONS; rep++)); do
        # Run the executable and capture output
        OUTPUT=$(taskset -c $CORE ./$CURRENT_COMPILER/$FILE_NAME $LOWER_BOUND $UPPER_BOUND $SAMPLES)

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
        echo "$CURRENT_COMPILER_NAME $FILE_NAME $LOWER_BOUND $UPPER_BOUND $SAMPLES" >> $ERROR_FILE
        [ $estimatation_diff == true ] && echo "Estimated: ${all_estimates[@]}" >> $ERROR_FILE
        [ $found_diff == true ] && echo "Found: ${all_found[@]}" >> $ERROR_FILE
        echo "" >> $ERROR_FILE
        echo "The estimations or the found number of zeros are different between the repetitions"
    fi

    # Calculate the mean time
    mean_time=$(awk "BEGIN { print $total_time / $REPETITIONS }")

    # Store information in the associative array
    RESULTS["$CURRENT_COMPILER_NAME"]="${all_estimates[0]}|${all_found[0]}|$mean_time"

    # Print results
    printf "  # $CURRENT_COMPILER_NAME : %s zeros in %s seconds\n\n" "${all_found[0]}" "$mean_time"

done

COMPILERS_ORDER=()
for key in "${!RESULTS[@]}"; do
    COMPILERS_ORDER+=("$key")
done

# Bubble sort
for ((i = 0; i < ${#COMPILERS_ORDER[@]}; i++)); do
    keyi=${COMPILERS_ORDER[$i]}
    for ((j = i+1; j < ${#COMPILERS_ORDER[@]}; j++)); do
        keyj=${COMPILERS_ORDER[$j]}
        if [ $(awk "BEGIN { print ${RESULTS[$keyi]##*|} < ${RESULTS[$keyj]##*|} }") -eq 1 ]; then
            tmp=${COMPILERS_ORDER[$i]}
            COMPILERS_ORDER[$i]=${COMPILERS_ORDER[$j]}
            COMPILERS_ORDER[$j]=$tmp
        fi
    done
done

# Calculate and display speedup
for ((i = 1; i < ${#COMPILERS_ORDER[@]}; i++)); do
    CURRENT_COMPILER=${COMPILERS_ORDER[$i]}
    PREVIOUS_COMPILER=${COMPILERS_ORDER[$((i-1))]}

    CURRENT_INFO=(${RESULTS["$CURRENT_COMPILER"]//|/ })
    PREVIOUS_INFO=(${RESULTS["$PREVIOUS_COMPILER"]//|/ })

    CURRENT_TIME=${CURRENT_INFO[2]}
    PREVIOUS_TIME=${PREVIOUS_INFO[2]}

    SPEEDUP=$(awk "BEGIN { print $PREVIOUS_TIME / $CURRENT_TIME }")
    echo "Speedup from $PREVIOUS_COMPILER to $CURRENT_COMPILER: $SPEEDUP"
done

# Calculate and display speedup between the last and the first compiler if there are at least three compilers
if [ ${#COMPILERS_ORDER[@]} -ge 3 ]; then
    FIRST_COMPILER=${COMPILERS_ORDER[0]}
    LAST_COMPILER=${COMPILERS_ORDER[${#COMPILERS_ORDER[@]}-1]}

    FIRST_INFO=(${RESULTS["$FIRST_COMPILER"]//|/ })
    LAST_INFO=(${RESULTS["$LAST_COMPILER"]//|/ })

    FIRST_TIME=${FIRST_INFO[2]}
    LAST_TIME=${LAST_INFO[2]}

    SPEEDUP_FIRST_TO_LAST=$(awk "BEGIN { print $FIRST_TIME / $LAST_TIME }")
    echo "Speedup from $FIRST_COMPILER to $LAST_COMPILER: $SPEEDUP_FIRST_TO_LAST"
fi
