#!/bin/bash

# execute user script if supplied
if [ "$USER_PROBE_STARTUP_SCRIPT" != "" -a -f "$USER_PROBE_STARTUP_SCRIPT" ]
then
    /bin/bash $USER_PROBE_STARTUP_SCRIPT
fi

# Write probe position to message box
hev-matrixToCoord --xyz --loop --every --usleep 10000 idea/worldOffsetWand | awk '{printf "%g %g %g\n",$1,$2,$3;fflush();}' | hev-messageBox $IRISFLY_MCP_X11_IMMERSIVE_OPTIONS -title probePosition --rows 1 --columns 40 --noescape --dynamic& 
for p in `pgrep -P $$`
do
    echo KILL $p > $IRIS_CONTROL_FIFO
done


