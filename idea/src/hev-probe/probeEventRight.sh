#!/bin/bash

# get probe orientation
mat=`hev-readDoubles -i -1 idea/worldOffsetWand`

# execute user script if supplied
if [ "$USER_PROBE_RIGHT_SCRIPT" != "" -a -f "$USER_PROBE_RIGHT_SCRIPT" ]
then
    source "$USER_PROBE_RIGHT_SCRIPT" $mat
fi

# instance a glyph at the probe's position
node="irisflyProbeEventRightPlus3d-$RANDOM"
echo MATRIX $node $mat > $IRIS_CONTROL_FIFO
echo NOCLIP $node > $IRIS_CONTROL_FIFO
echo ADDCHILD $node world > $IRIS_CONTROL_FIFO
echo ADDCHILD irisflyPlus3d $node > $IRIS_CONTROL_FIFO

