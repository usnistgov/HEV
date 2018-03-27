#!/bin/sh

# the flipbook has 6 fraems, and we run each for 1 second, so sleep for 6
# seconds after spewing out each batch of commands

while [ 1 ]
do hev-animationToTimeline --time 1 flipbook.iris | hev-timelineToIris > $IRIS_CONTROL_FIFO
    sleep 6 
done
