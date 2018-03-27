#! /bin/bash

case $# in
1)
    k=$1
    ;;
*)
    echo "Usage: mklayer zLayer"
    exit
esac


n=40
m=50
#n=2
#m=4

layer=layer_${k}

echo GROUP $layer

savg-cube | savg-scale 1.01 1.01 1.01 | savg-rgb .8 0 0 > redcube.savg
savg-cube | savg-scale 1.01 1.01 1.01 | savg-rgb 0 .8 0 > greencube.savg
savg-cube | savg-scale 1.01 1.01 1.01 | savg-rgb 0 0 .8 > bluecube.savg
#hev-savgCube | savg-rgb .8 0 0 > redcube.savg
#hev-savgCube | savg-rgb 0 .8 0 > greencube.savg
#hev-savgCube | savg-rgb 0 0 .8 > bluecube.savg

for ((i=0; i<n; i++))
do
    for ((j=0; j<m; j++))
    do
	echo $i $j $k> /dev/tty

	node=r_${i}_${j}_${k}
	echo STARTMODELDATA $node savg
	hev-savgTransformation --scale .2 .2 .2 --xyz $i $j $k  < redcube.savg
	echo ENDMODELDATA
	echo ADDCHILD $node $layer
	#echo NODEMASK $node ON
	echo NODEMASK $node OFF

	node=g_${i}_${j}_${k}
	echo STARTMODELDATA $node savg
	hev-savgTransformation --scale .2 .2 .2 --xyz $i $j $k < greencube.savg
	echo ENDMODELDATA
	echo ADDCHILD $node $layer
	echo NODEMASK $node OFF

	node=b_${i}_${j}_${k}
	echo STARTMODELDATA $node savg
	hev-savgTransformation --scale .2 .2 .2 --xyz $i $j $k < bluecube.savg
	echo ENDMODELDATA
	echo ADDCHILD $node $layer
	echo NODEMASK $node OFF

    done
done

echo RETURN $layer
