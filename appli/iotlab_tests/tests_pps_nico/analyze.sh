#!/bin/bash
count=0

declare -a deltas1=();
declare -a deltas2=();
declare -a dds=();

for line1 in `cat $1` ; do
    count1=${line1%,*}
    delta1=${line1#*,}
    for line2 in `cat $2` ; do
	count2=${line2%,*}
	delta2=${line2#*,}
	if [ $count1 -eq $count2 ] ; then
	    delta2=${line2#*,}
	    dd=$((delta1-delta2))
	    count=$(($count + 1))
	    deltas1=("${deltas1[@]}" $delta1)
	    deltas2=("${deltas2[@]}" $delta2)
	    dds=("${dds[@]}" $dd)

	    echo "pkt " $count1 " received with deltas " $delta1 " and " $delta2 " diff is " $dd
	fi
    done
done

for ((i=0; i<$count; i++));
do 
    ddsum=$(($ddsum+${dds[$i]}));
    delta1sum=$(($delta1sum+${deltas1[$i]}));
    delta2sum=$(($delta2sum+${deltas2[$i]}));
done

delta1avg=$(($delta1sum / $count))
delta2avg=$(($delta2sum / $count))
ddavg=$(($ddsum / $count))

echo "delta1 avg= " $delta1avg  ", delta2 avg= " $delta2avg  ", dd avg = " $ddavg

for ((i=0; i<$count; i++));
do 
    ddtmp=$(($ddtmp+($ddavg-${dds[$i]})**2));
    delta1tmp=$(($delta1tmp+($delta1avg-${deltas1[$i]})**2));
    delta2tmp=$(($delta2tmp+($delta2avg-${deltas2[$i]})**2));
done
ddeq=`echo "scale = 5; sqrt( $ddtmp / $count)" | bc`
delta1eq=`echo "scale = 5; sqrt( $delta1tmp / $count)" | bc`
delta2eq=`echo "scale = 5; sqrt( $delta2tmp / $count)" | bc`
echo "delta1 eq=" $delta1eq ", delta2 eq=" $delta2eq ", ddeq=" $ddeq 




