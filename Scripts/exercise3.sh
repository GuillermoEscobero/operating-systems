#!/bin/bash
if [ $# -eq 0 ]
then 
    echo No arguments supplied
    exit -1
elif [ $# -gt 2 ]
then
    echo More arguments than expected
    exit -1
fi

for (( i=$1; i<=$2; i++ ))
do
    FACTORIZED_NUMBER=`factor $i`
    NUMBER_FACTORS=`echo $FACTORIZED_NUMBER | awk -F" " '{print NF}'`
    if [ $FACTORIZED_NUMBER -eq 2 ]
    then 
        echo `echo $FACTORIZED_NUMBER | awk -F" " '{print $2}'`
    fi
done