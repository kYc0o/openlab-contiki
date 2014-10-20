#!/bin/bash
count=0

declare -a deltas1=();
declare -a deltas2=();
declare -a dds=();
declare -a pps=();
sumpps=0
cat $1 | cut -d' ' -f 2 > $1_2
sort -n $1_2 | uniq -c > $1_3
gnuplot -persist -e "plot \"$1_3\" using 2:1 w filledcu"
for line in `cat $1_2` ; do
    pps=("${pps[@]}" $line)
    count=$(($count + 1))
    sumpps=$(($sumpps + $line))
done
avgpps=$(($sumpps / $count ))
echo "got " $count " pps, avg="$avgpps 


