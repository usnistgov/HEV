#!/bin/sh
# where to get our OSG loadable data files
export OSG_FILE_PATH=$HEVROOT/idea/src/hev-animator:$IRIS_HOME/examples/tim:$OSG_FILE_PATH 

# load the flipbook file and play it!
irisfly --ex flipbook.iris animation2-exec.iris


