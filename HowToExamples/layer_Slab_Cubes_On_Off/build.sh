#! /bin/bash

./mkWireRectangle.sh 40 50 50  > wr.savg
#./mkWireRectangle.sh 2 4 4  > wr.savg

./mkslab.sh 0 > slab.iris


f=twolayers.iris

echo "# Two layer example" > $f

echo GROUP layers >> $f

mask="ON"

for i in 0 3
do
    l=layer${i}

    ./mklayer.sh $i > ${l}.iris

    #iris-convert --optimize MERGE_GEODES ${l}.iris ${l}.osgb
    iris-convert ${l}.iris ${l}.osgb


    echo LOAD ${l} ${l}.osgb >> $f
    echo NODEMASK ${l} ${mask} >> $f
    mask="OFF"

    echo ADDCHILD ${l} layers >> $f

done

echo RETURN layers >> $f


##########################


f=oneslab.iris

cat > $f <<EOF
# Position slab
DCS slab 0 0 0
LOAD slab0 slab.iris
ADDCHILD slab0 slab
ADDCHILD slab world
EOF
