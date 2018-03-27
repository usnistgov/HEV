#!/bin/sh
[ -r $1.onOff ] && onoff=$1.onOff
echo WAIT cat $HEV_IDEA_DIR/etc/hev-relativeMove/data/relativeMove.onOff $onoff \| hev-shmOnOff --selector $1 idea/buttons/left \> $IRIS_CONTROL_FIFO \& echo KILL $\! \> $IRIS_CONTROL_FIFO > $IRIS_CONTROL_FIFO
