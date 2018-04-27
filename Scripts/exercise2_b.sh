#!/bin/bash
# Check if the number of arguments is correct to end the program when using it incorrectly
if [ $# -ne 2 ]
then
    echo "Usage: ./exercise2_b.sh FILE NUM"
    exit -1
fi
# Execute the code normally
tr -c [:alnum:] [\\n\*] < $1 | sort | uniq -c | sort -nr | head -$2
