#! /bin/bash

# mtp.sh - move to position

# Used by Bill to precisely position the Rheometer simulations to conveniently
# visually check for blade object intersections.

# 2/25/2013
# Steve


cat <<EOF | hev-timelineToIris > $IRIS_CONTROL_FIFO
2   0   0  0

2   0   2  2    0  -90.   0  1

2   0   .09  2    0  -90.   0  1

2   0   .09  .7    0  -90.   0  .35
EOF












