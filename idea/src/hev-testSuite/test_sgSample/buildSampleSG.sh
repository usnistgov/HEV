#! /bin/bash

hev-savgSphere    | savg-scale .25 .25 .25 | savg-color -r .8   -g 0    -b 0   -a 1 > sg_sphere.savg

hev-savgCube      | savg-scale .25 .25 .25 | savg-color -r  0   -g .8   -b 0   -a 1 > sg_cube.savg

hev-savgCylinder  | savg-scale .25 .25 .25 | savg-color -r  0   -g 0    -b .8  -a 1 > sg_cylinder.savg

hev-savgCone      | savg-scale .25 .25 .25 | savg-color -r .8   -g .8   -b 0   -a 1 > sg_cone.savg


cat > sgSample.iris <<"EOF"
# A sample scenegraph created with iris commands
GROUP sgSample

set TS_DATA  $($HEV_IDEA_DIR)/data/testSuite

LOAD sg_sphere_0   $(TS_DATA)/sg_sphere.savg
LOAD sg_cube_0     $(TS_DATA)/sg_cube.savg
LOAD sg_cylinder_0 $(TS_DATA)/sg_cylinder.savg
LOAD sg_cone_0     $(TS_DATA)/sg_cone.savg

SCS sg_sphere    0 -1  0
SCS sg_cube      1  1  0
SCS sg_cylinder -1  1  0
SCS sg_cone      0  0  1

ADDCHILD sg_sphere_0 sg_sphere
ADDCHILD sg_cube_0 sg_cube
ADDCHILD sg_cylinder_0 sg_cylinder
ADDCHILD sg_cone_0 sg_cone

ADDCHILD sg_sphere   sgSample
ADDCHILD sg_cube     sgSample
ADDCHILD sg_cylinder sgSample
ADDCHILD sg_cone sgSample

RETURN sgSample
EOF