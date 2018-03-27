#!/bin/sh
iris-readState --usleep 10000 | hev-messageBox $IRISFLY_MCP_X11_IMMERSIVE_OPTIONS -title status --rows 10 --columns 104 --dynamic --noescape & pid=$!
echo kill $pid > $IRIS_CONTROL_FIFO

