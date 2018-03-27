#!/bin/bash
#
# steve@nist.gov
# 1/29/2015

function usage {
   echo "Usage: ./alignment.sh imagefile"
   exit
}

case $# in
1)
   img=$1
   ;;
*)
   usage
   ;;

esac


# Kill background hev-imageDisplay on exit
trap 'kill $(jobs -p)' 0


H=$(hev-identifyImmersive)
case $H in
rave)
        D=":0.3 :0.2 :0.1"
	;;
powerwall)
        D=":0.1"
        ;;
*)
        D="$DISPLAY"
        ;;
esac


if [ -f $img ]
then
    for DISPLAY in $D
    do
        echo "Display $img on " $DISPLAY
	hev-imageDisplay --noborder -geometry +0+0 $img &
    done
#    echo "Press Enter to exit all displays"
#    read x
    wait
else
    echo "Image $img not found"
fi

