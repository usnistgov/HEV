#!/bin/sh
# where to get our OSG loadable data files
export OSG_FILE_PATH=$HEVROOT/idea/src/hev-animator:$IRIS_HOME/examples/tim:$OSG_FILE_PATH 

# convert the flipbook animation to a timeline file
hev-animationToTimeline --time 1 flipbook.iris > /tmp/hev-flipboook-$USER.timeline

# loop the timeline file three times into an iris file
for f in 1 2 3; do cat /tmp/hev-flipboook-$USER.timeline; done | hev-timelineToIris > /tmp/hev-flipboook-$USER.iris

# load the flipbook file and play it!
irisfly --ex flipbook.iris /tmp/hev-flipboook-$USER.iris

# cleanup
rm /tmp/hev-flipboook-$USER.timeline /tmp/hev-flipboook-$USER.iris

