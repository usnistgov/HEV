#! /bin/bash

# Kill background children processes on exit
trap 'kill $(jobs -p)' 0


H=$(hev-identifyImmersive)
case $H in
rave)
	s=""
	;;
powerwall)
	s=""
        ;;
*)
	s="-mono"
        ;;
esac

hev-leftRightEyeTest $s  2> /dev/null > /dev/null


