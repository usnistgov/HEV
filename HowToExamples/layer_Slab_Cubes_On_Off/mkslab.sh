#! /bin/bash

case $# in
1)
    k=$1
    ;;
*)
    echo "Usage: mkslab zLayer" > /dev/tty
    exit
esac


n=40
m=50
#n=2
#m=4

slab=slab_${k}

echo GROUP $slab

#hev-savgCube | savg-rgb .8 0 0 > redcube0.savg
#Note, its important to use savg-cube here,
#hev-savg-Cube include a "nopotimization" keyword in its output
savg-cube | savg-rgb .8 0 0 > redcube0.savg

node=red
echo STARTMODELDATA $node savg

for ((i=0; i<n; i++))
do
    for ((j=0; j<m; j++))
    do
	echo $i $j $k> /dev/tty

	hev-savgTransformation --scale .2 .2 .2 --xyz $i $j $k  < redcube0.savg

    done
done

echo ENDMODELDATA
echo ADDCHILD $node $slab
echo NODEMASK $node ON

echo RETURN $slab
