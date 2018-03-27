#! /bin/bash

# Pop up a button to kill the specified process


case $# in
1)
	cmd=$1
	;;
*)
	echo "Usage: kill.sh basename"
	exit
	;;
esac

mcp=/tmp/${cmd}_$$.mcp

# Kill background children processes on exit
trap 'rm -f ${mcp}; kill $(jobs -p)' 0

cat > $mcp <<EOF
SIZE 130 20

BUTTON "Kill $cmd" COLOR 1 .2 .2
ON EXEC pkill -HUP -f ${cmd}.sh
ON AFTER TIME 1 EXEC pkill -HUP -f kill.sh

EOF

hev-masterControlPanel $mcp > $IRIS_CONTROL_FIFO