#!/bin/bash
if [ $# -eq 0 ]
then 
    echo "No arguments supplied"
    exit -1
fi
for ARGUMENT
do
    OUTPUT=`awk -F":" -v USER=$ARGUMENT '$1 == USER {print $1 ": " $7}' /etc/passwd`
    if [ "$OUTPUT" == "" ]
    then
        echo -ERROR- $ARGUMENT: no such user
    else
        echo $OUTPUT
    fi
done
