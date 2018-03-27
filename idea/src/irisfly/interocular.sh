#!/bin/sh
# make the max twice the default
max=`echo "$IRISFLY_INTEROCULAR_DISTANCE*2" | bc -l`
( dtk-floatRollers -ti interocular -N 1 -s 0 0 $max $IRISFLY_INTEROCULAR_DISTANCE  & pid=$! ; echo kill $pid > $IRIS_CONTROL_FIFO ) | sed -u -e 's/^/INTEROCULAR /' > $IRIS_CONTROL_FIFO

