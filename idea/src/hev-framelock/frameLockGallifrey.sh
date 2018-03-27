#! /bin/bash

function fl_status_gallifrey {
    nvidia-settings --no-config --ctrl-display=:0 \
	-q [gpu:0]/FrameLockEnable -q [gpu:1]/FrameLockEnable
}

function fl_off_gallifrey {
    nvidia-settings --no-config --ctrl-display=:0 -a [gpu:0]/FrameLockEnable=0
    nvidia-settings --no-config --ctrl-display=:0 -a [gpu:1]/FrameLockEnable=0
}

function fl_on_gallifrey {
    echo on1
    fl_off_gallifrey

    echo on2
    nvidia-settings --config=nvidia-settings.txt --load-config-only --ctrl-display=:0

    echo on3
    nvidia-settings --no-config --ctrl-display=:0 -a [gpu:0]/FrameLockEnable=1
    nvidia-settings --no-config --ctrl-display=:0 -a [gpu:1]/FrameLockEnable=1
}

verbose=0

case $1 in
-v*|--v*)
	verbose=1
	shift
	;;
esac

case $1 in
s|stat|status)
	fl_status_gallifrey
	;;
off|on)
	if [ "$verbose" == "1" ]
	then
	    fl_${1}_gallifrey
	else
	    fl_${1}_gallifrey &> /dev/null
	fi
	;;
*)
	echo "Usage: framelock.sh status | on | off"
	;;
esac

