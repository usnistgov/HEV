#! /bin/bash

cat > redPoint.savg <<EOF
POINTS
-1 0 0    1   0    0    1
EOF

cat > bluePoint.savg <<EOF
POINTS
 1 0 0    0   0    1    1
EOF

cat > redPoint.iris <<EOF
LOAD rp redPoint.savg
LOAD gpuSphere.iris
MERGESTATE gpuSphere rp
UNIFORM rp fatpointSize 1
UNLOAD gpuSphere
CULLING rp OFF
RETURN rp
EOF


cat > bluePoint.iris <<EOF
LOAD bp bluePoint.savg
LOAD gpuSphere.iris
MERGESTATE gpuSphere bp
UNIFORM bp fatpointSize .5
UNLOAD gpuSphere
CULLING bp OFF
RETURN bp
EOF

echo AFTER REALIZE BOUND world .5 > bound.iris

irisfly --ex redPoint.iris bluePoint.iris bound.iris

