#!/bin/sh

# this shell script should be loaded with the command
# iris-viewer --ex cow.osg cleanUpOnExit.iris

# all it does is run the dtk-floatRollers program to modify the navigation scale

# the point is that the SceneGraph class kills this shell script, and this
# shell script cleans up the processes it's started

# background the rolllers
dtk-floatRollers -ti scale -N 1 -s 0 .1 10 1 -l scale | while read a
do echo NAVIGATION SCALE $a
done > $IRIS_CONTROL_FIFO &

# on exit kill it off
trap "echo cleanUpOnExit.sh exiting and killing process $(jobs -p); kill $(jobs -p)" exit

# wait around for the kill signal
wait

