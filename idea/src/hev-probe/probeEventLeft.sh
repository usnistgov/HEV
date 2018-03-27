#!/bin/bash

# get probe coordinate
coord=`hev-matrixToCoord --xyz idea/worldOffsetWand`

# execute user script if supplied
if [ "$USER_PROBE_LEFT_SCRIPT" != "" -a -f "$USER_PROBE_LEFT_SCRIPT" ]
then
    source "$USER_PROBE_LEFT_SCRIPT" $coord
fi

# put up a window with the probe's position in world coordinates
echo $coord |  awk '{printf "%g %g %g\n",$1,$2,$3;fflush();}' | hev-messageBox $IRISFLY_MCP_X11_IMMERSIVE_OPTIONS -title probe@ --rows 1 --columns 40 & pid=$!
echo KILL $pid > $IRIS_CONTROL_FIFO

