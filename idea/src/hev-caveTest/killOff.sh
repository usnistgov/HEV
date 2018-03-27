#! /bin/bash

case $# in
1)
	cmd=$1
	;;
*)
	echo "Usage: killOff.sh basename"
	exit
	;;
esac

pkill -HUP -f ${cmd}.sh
