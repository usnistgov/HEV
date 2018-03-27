#!/bin/bash

# give it the name of a file which is centered on its origin

if [ $# -eq 0 ] ; then
    file=gnomon.osg
    args="$*"
else
    while [ $# -gt 1 ] ; do
	args="$args $1"
	shift
    done
    file="$1"
fi

# tmp wraper file
tmpfile=/tmp/hev-vectorRotation-$LOGNAME-$RANDOM.iris

# unique node names
tmpdcs=hev-vectorRotation-$LOGNAME-$RANDOM
tmpmodel=hev-vectorRotation-$LOGNAME-$RANDOM

echo QDCS $tmpdcs >> $tmpfile
echo ADDCHILD $tmpdcs world >> $tmpfile
echo LOAD $tmpmodel $file >> $tmpfile
echo ADDCHILD $tmpmodel $tmpdcs >> $tmpfile
echo AFTER REALIZE EXEC ./hev-vectorRotation $tmpdcs \> $IRIS_CONTROL_FIFO >> $tmpfile
irisfly $args --setname --examine $tmpfile

rm $tmpfile
