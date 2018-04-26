#!/bin/bash
# Check if the number of arguments is correct
if [ $# -eq 0 ]
then 
    echo No arguments supplied
    exit -1
elif [ $# -gt 2 ]
then
    echo More arguments than expected
    exit -1
elif [ $1 -gt $2 ]
then 
    echo -ERROR- Wrong usage: execute ./exercise3.sh min max
fi

# Iterate through the numbers starting from the smallest one
for (( i=$1; i<=$2; i++ ))
do
    # Get the factorization of the number and save it to a variable to avoid repeating it later
    FACTORIZED_NUMBER=`factor $i`
    # Get the number of factors returned 
    NUMBER_FACTORS=`echo $FACTORIZED_NUMBER | awk -F" " '{print NF}'`
    # If the number of factors is bigger than 2 (one is the number taken as input, 
    # the other one is the factor, they are the same number) then its prime
    if [ $NUMBER_FACTORS -eq 2 ]
    then 
        echo `echo $FACTORIZED_NUMBER | awk -F" " '{print $2}'`
    fi
done