#!/bin/sh
filename="/tmp/hev-trackerMonitor-$USER"

if [ -e "$filename" -a ! -f "$filename" ]
then dtk-msgNotice -p irisfly file $filename exists but not a regular file. 2>&1
    exit 1
fi

if [ -f "$filename" -a ! -w "$filename" ]
then dtk-msgNotice -p irisfly file $filename exists but not writeable. 2>&1
    exit 1
fi

if [ -w "$filename" -a -s "$filename" ]
then dtk-msgNotice -p irisfly file $filename exists. Appending new data. 2>&1
    echo ~~~~~~~~~~~~~~~~~~~ start: `date` ~~~~~~~~~~~~~~~~~~~~ >> $filename
fi

echo EXEC hev-trackerMonitor $IRISFLY_HEV_TRACKER_MONITOR_OPTIONS \>\> $filename > $IRIS_CONTROL_FIFO

echo AFTER CLEANUP WAIT if [ -s \"$filename\" ] \; then dtk-msgNotice -p irisfly file $filename has data, not deleted. \; else rm \"$filename\" \; fi  > $IRIS_CONTROL_FIFO 



