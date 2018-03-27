#! /bin/bash

rocksTemplate='savg.66/StBonn-new_1584.struct-sf10.0.poly.%06d.savg'
qvisdat=data.qvisdat
N=7  # 8 frames: 0 to 7

echo run qvd-toTIM
#DEBUG="--debug"
qvd-toTIM $DEBUG --verbose $rocksTemplate $qvisdat demo.timo demo.timf

echo run hev-tim
hev-tim demo.timo demo.timf demo

echo run qvd-wireBox
qvd-wireBox 30 30 30 .5 .5 .5 box.osgb

echo build cleanup.sh and other files
cat > cleanup.sh <<EOF
rm -f box.osgb demo.iris demo.osg demo-posn.mha demo-quat.mha  
rm -f demo.timf demo.timo
rm -f rundemo run_hev-director.sh
rm -f cleanup.sh
EOF
chmod +x cleanup.sh


cat > demo.iris <<EOF
AFTER REALIZE EXEC ./run_hev-director.sh $N > \$IRIS_CONTROL_FIFO
EOF

cat > run_hev-director.sh <<"EOF"
#! /bin/bash

(hev-director --noescape --last $1 & echo kill $! > $IRIS_CONTROL_FIFO) | awk '
{
   print "UNIFORM demo frame uint 1",$1;
   fflush("");
}'

EOF
# quoting heredoc delimeter prevents variable expansion
chmod +x run_hev-director.sh

cat > rundemo <<EOF
#! /bin/bash

irisfly --ex box.osgb demo.osg demo.iris
EOF
chmod +x rundemo





