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
# Go to the directory taken as input in the script
cd $1
# Save in an array all the filenames whose size is bigger than the required one,
# which is listed in the fifth coulumn of the ls -l output, whose extension is jpg 
# (second columns of the ls -l output)
ARRAY=( `ls -l | awk -F" " '$5 > 1024000 {print $9}' | awk -F"." '$2 == "jpg" {print $1 "." $2 }'` )
# Iterate through the array
for i in "${ARRAY[@]}"
do 
    # Print the complete route of the file
    echo `pwd`/$i
    # Cut it to 720 size maintaining the ratio and override the file
    mogrify -resize 720 `pwd`/$i
done
