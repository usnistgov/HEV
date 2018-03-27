#! /bin/bash

function cleanup () {
    if [ "$#" != "0" ]
    then
	kill $*
    fi
}

trap 'cleanup $(jobs -p)'  EXIT

echo KILL $$ > $IRIS_CONTROL_FIFO

#echo "pointerTrigger started" > /dev/tty

# Hard coded to 4th button, the trigger
hev-readButtons buttons | awk '
BEGIN {
    d[0]=.05; d[1]=1; d[2]=2; d[3]=4; d[4]=6; d[5]=8;d[6]=10;
    imax=7;
    i=2; # yes, start at 1 since arrow is already that size
    nm="on";
}
{
    #print "printTrigger",$1,$2 > "/dev/tty";
    if (substr($2,5,1) == 1) {
	if (i == 0) {
	    nm="off";
	} else {
	    nm="on";
	}

	print "SCS ptrGlyph 0 0 0   0 0 0  1",d[i]," 1";
	print "NODEMASK ptrGlyph",nm;
	fflush();

	i=i+1;
	if (i == imax) {
	    i=0;
	}
    }
}' > $IRIS_CONTROL_FIFO
