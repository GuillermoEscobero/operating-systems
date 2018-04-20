# You are requested to write the exercise4.sh script, that will change the width of JPEG images 
# (.jpg suffix) to 720 pixels (proportional height) for those files whose size is greater than 1 MiB and 
# that are contained in the given directory (passed as a command-line parameter). 
# Additionally, the pathname of each touched file shall be printed to stdout,

# e.g.
# $ cp -R /usr/share/backgrounds/gnome /tmp
# $ ./exercise4.sh /tmp/gnome
# /tmp/gnome/Flowerbed.jpg
# /tmp/gnome/Godafoss_Iceland.jpg
# /tmp/gnome/Signpost_of_the_Shadows.jpg
# /tmp/gnome/Stones.jpg

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
cd $1
ARRAY=( `ls -l | awk -F" " '$5 > 1024000 {print $9}' | awk -F"." '$2 == "jpg" {print $1 "." $2 }'` )
for i in "${ARRAY[@]}"
do 
    echo `pwd`/$i
    mogrify -resize 720x720 `pwd`/$i
done
