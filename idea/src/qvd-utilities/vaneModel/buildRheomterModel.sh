#! /bin/bash

# buildRheometerModel.sh - build rehometer in two parts:
#                              vane paddle and half sylendar

# April 23, 2012
# Steve


./buildVane > vane.savg

# default cylinder is radius 1, height l=along z is 2
hev-savgCylinder --arc 180 --cap bottom | hev-savgTransformation --scale 11.07 11.07 50 > halfCylinder.savg

OPTIMIZE="\
TRISTRIP_GEOMETRY:\
MERGE_GEOMETRY:\
FLATTEN_STATIC_TRANSFORMS_DUPLICATING_SHARED_SUBGRAPHS:\
REMOVE_REDUNDANT_NODES:\
MERGE_GEODES"


iris-convert --static --optimize $OPTIMIZE vane.savg halfCylinder.savg rheometer.osg

