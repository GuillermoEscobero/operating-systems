#!/bin/bash
# Check if the number of arguments is correct
if [ $# -eq 0 ]
then 
    echo No arguments supplied
    exit -1
elif [ $# -gt 1 ]
then
    echo More arguments than expected
    exit -1
fi

# Get the users (fourth column) and the group (third column, will be used to check if the group exists) and store them in 
# the GROUP and USERS variables
read GROUP USERS <<< $( awk -F":" -v GROUP_ID=$1 '$3 == GROUP_ID {print $3 " " $4}' /etc/group )
# Check that the group exists, if not, throw an error
if [ "$GROUP" == "" ]
then
    echo -ERROR- $1: no such group
else
    # Get the number of users of the group
    NUMBER_USERS=`echo $USERS | awk -F"," '{print NF}'`
    # If there are no users throw an error
    if [ $NUMBER_USERS -eq 0 ]
    then 
        echo -ERROR- $1: group empty
    else
        # Iterate through each user to print them to terminal
        for (( I=1; I <= $NUMBER_USERS; ++I ))
        do
            echo $USERS | awk -F"," -v ITERATOR=$I '{print $ITERATOR}'
        done
    fi
fi

