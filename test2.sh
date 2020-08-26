#!/bin/bash
head -1 mem_profile_use.txt > File.txt
sed -i '1d' mem_profile_use.txt
shuf -o File.txt < mem_profile_use.txt
IFS=$'\r\n ' GLOBIGNORE='*' command eval  'XYZ=($(cat mem_profile_use.txt))'
X=0
while [ $X -lt "${#XYZ[@]}" ]; do
	TIMEB=$TIME
	TIME=${XYZ[$X]%.*}
	let X+=1
	MEM=${XYZ[$X]%.*}
	let X+=1
done