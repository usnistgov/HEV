#! /bin/bash

# kludge some data for now

i=0
for f in ../rabExample/*.qvisdat.gz
do
    b=`basename $f`
    t=`echo $b | sed 's/octant.../octant/'`
    if [ "$b" == "vane.qvisdat" ]
    then
	out=$b
    else
	out=section${i}.qvisdat
    fi

    echo $f $out

    zcat $f | \
	sed -e 's/shape 000/shape 301/' | \
	sed -e 's/shape 001/shape 201/' | \
	sed -e 's/shape 010/shape 302/' | \
	sed -e 's/shape 011/shape 202/' | \
	sed -e 's/shape 100/shape 305/' | \
	sed -e 's/shape 101/shape 205/' | \
	sed -e 's/shape 110/shape 306/' | \
	sed -e 's/shape 111/shape 206/' > $out
	

    i=$((i+1))
done
