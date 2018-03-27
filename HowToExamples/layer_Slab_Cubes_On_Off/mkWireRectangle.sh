#! /bin/bash

case $# in
3)
	x=`echo $1 | awk '{print $1-1+.4;}'`
	y=`echo $2 | awk '{print $1-1+.4;}'`
	z=`echo $3 | awk '{print $1-1+.4;}'`
	;;
*)
	echo "Usage: mkWireREctangle.sh x y z" > /dev/tty
	exit
esac



(cat  <<EOF
LINES .5 .5 .5 1
0 0 0
$x 0 0
$x $y 0
0 $y 0
0 0 0
0 0 $z
$x 0 $z
$x $y $z
0 $y $z
0 0 $z
LINES .5 .5 .5 1
$x 0 0
$x 0 $z
LINES .5 .5 .5 1
$x $y 0
$x $y $z
LINES .5 .5 .5 1
0 $y 0
0 $y $z
EOF
) | savg-translate -.2 -.2 -.2


