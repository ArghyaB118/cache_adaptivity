#!/bin/bash
IFS=$'\r\n ' GLOBIGNORE='*' command eval  'XYZ=($(cat mem_profile_use.txt))'
PID=$!
STATUS=$(ps ax|grep "$PID"|wc -l)
X=2
CURRENT=0
SUM=0
while [ $STATUS -gt 1 ] && [ $X -lt "${#XYZ[@]}" ]; do
   VAL=${XYZ[$X]%.*}
   CURRENT=$VAL
   let X+=2
   VAL=${XYZ[$X]%.*}
   echo $(($VAL-$CURRENT))
   SUM=$(($SUM+$VAL-$CURRENT))
   let X+=2
   STATUS=$(ps ax|grep "$PID"|wc -l)
done 
echo $SUM