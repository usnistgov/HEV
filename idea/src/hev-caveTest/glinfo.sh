#! /bin/bash

tmpPID="/tmp/glinfo_${USER}_pid"
tmpFile="/tmp/glinfo_${USER}_text"

trap "rm -f $tmpFile" 0

case $1 in
on)
	echo "Host: $(hostname)" > $tmpFile
	hev-glinfo  >> $tmpFile
	hev-messageBox -title GLinfo < $tmpFile &
	m=$!
	echo $m > $tmpPID
	echo KILL $m > $IRIS_CONTROL_FIFO
	;;
off)
	if [ -f $tmpPID ]
        then
	    p=$(cat $tmpPID)
	    kill $p
	    rm -f $tmpPID
	fi
	;;
esac


