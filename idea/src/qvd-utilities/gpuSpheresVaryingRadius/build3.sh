#! /bin/bash

cat > random.iris <<EOF
GROUP rand
LOAD gpuSphere.iris
MERGESTATE gpuSphere rand
UNLOAD gpuSphere
CULLING rand OFF

LOAD r random.savg
ADDCHILD r rand

UNIFORM rand fatpointSize -1

return rand
EOF

echo AFTER REALIZE BOUND world .5 > bound.iris

irisfly --ex random.iris bound.iris
