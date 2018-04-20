#!/bin/bash
if [ $# -eq 0 ]
then 
    echo No arguments supplied
    exit -1
elif [ $# -gt 1 ]
then
    echo More arguments than expected
    exit -1
fi

OUTPUT=`awk -F":" -v GROUP_ID=$1 '$3 == GROUP_ID {print $3 "USERS=$4"}' /etc/group`
read GROUP USERS <<< $( echo ${val} | awk -F":" -v GROUP_ID=$1 '$3 == GROUP_ID {print $3 " " $4}' /etc/group  )
if [ "$GROUP" == "" ]
then
    echo -ERROR- $1: no such group
else
    NUMBER_USERS=`echo $USERS | awk -F"," '{print NF}'`
    if [ $NUMBER_USERS -eq 0 ]
    then 
        echo -ERROR- $1: group empty
    else
        for (( I=1; I <= $NUMBER_USERS; ++I ))
        do
            echo $USERS | awk -F"," -v ITERATOR=$I '{print $ITERATOR}'
        done
    fi
fi

