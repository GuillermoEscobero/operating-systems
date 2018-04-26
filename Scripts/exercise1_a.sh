#!/bin/bash
# Check if the number of arguments is correct
if [ $# -eq 0 ]
then 
    echo "No arguments supplied"
    exit -1
fi
# Iterate through each argument
for ARGUMENT
do
    # Save in OUTPUT the first and seventh column (where the separator of columns is ":") of the /etc/passwd document 
    # if the first column equals the USER (that is the ARGUMENT variable passed to the program) 
    OUTPUT=`awk -F":" -v USER=$ARGUMENT '$1 == USER {print $1 ": " $7}' /etc/passwd`
    # Check that the user exists (if awk returns nothing, the user does not exist)
    if [ "$OUTPUT" == "" ]
    then
        echo -ERROR- $ARGUMENT: no such user
    else
        echo $OUTPUT
    fi
done
