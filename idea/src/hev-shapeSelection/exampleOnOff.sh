#!/bin/sh

while [ 1 ]
do  bs=`hev-readButtons -1 -i $1 2>/dev/null`
    for i in 1 2 3 4 5 6 7 8
    do b=`echo $bs | cut -d' ' -f $i`
	if [ "$b" == "1" ]
	    then echo NODEMASK p$i ON
	else echo NODEMASK p$i OFF
	fi
    done
    usleep 10000
done
