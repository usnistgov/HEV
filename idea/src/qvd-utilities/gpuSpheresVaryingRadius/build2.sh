#! /bin/bash

cat > redPoint.savg <<EOF
POINTS
-4 0 0    1   0    0   4
EOF

cat > bluePoint.savg <<EOF
POINTS
 4 0 0    0   0    1    1
EOF

cat > twoPoints.iris <<EOF
GROUP two
LOAD gpuSphere.iris
MERGESTATE gpuSphere two
UNLOAD gpuSphere
CULLING two OFF

LOAD rp redPoint.savg
LOAD bp bluePoint.savg
ADDCHILD rp two
ADDCHILD bp two

UNIFORM rp fatpointSize -1
UNIFORM bp fatpointSize 2

return two
EOF

echo AFTER REALIZE BOUND world .5 > bound.iris

irisfly --ex twoPoints.iris bound.iris
