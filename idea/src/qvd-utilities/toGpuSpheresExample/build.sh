#! /bin/bash

# This script is example of creating a visualization of the inclusions in the
# .qvisdat file (output of QDPD) using the qvd-toIRIS --gpuSpheres command.
#
# This script assumes a directory with only a data.qvisdat file
# and a copy of this script
#
# There is a full and a short version of the data file.
#
# To run:
#          gunzip data_8frames.qvisdat.gz
#          time ./build.sh data_8frames.qvisdat
#
# To cleanup:
#          ./cleanup.sh
#          gzip data_8frames.qvisdat
#
# May 7, 2012
# January 8, 2013
# Steve

#
# Parse the command line
#
case $1 in
-v*|--v*)
	VERBOSE=--verbose
	shift
	;;
*)
	VERBOSE=""
	;;
esac

if [ $# != 1 ]
then
    echo "Usage: ./build.sh [--verbose] file.qvisdat"
    exit 1
fi

if [ -f $1 ]
then
    QVISDAT=$1
else
    echo "File: $1 not found"
    exit 1
fi

#
# Setup and run the qvd-toIRIS command
#


# This qvisdat file only needs a single rock. i.e in this case a sphere.
# Other qvisdat files will need the appropriate shapes or spheres.
#
# Make a single "shape 0" sphere.
# Since we are using GPU spheres, the shapes are really just single points

rm -rf rocks
mkdir rocks
w=.85
echo "#POINTS   $w $w $w 1" > rocks/s_000.savg

rm -rf pages
mkdir pages
n=0   # set n to number of CPUs, or 0 to automatically determine
      # Note: for large data, this can easily exhaust system memory
      #       and terminate processing

qvd-toIRIS $VERBOSE --gpuSpheres --parallel $n  rocks/s_%03d.savg $QVISDAT points.iris pages points

#
# Now we need to add the GPU shader to convert all the points in
# points.iris to GPU rendered spheres
#

radii=1.398  # Sphere size is hard coded here

cat > spheres.iris <<EOF
LOAD spheres points.iris

LOAD litFatSphere litFatSphere.osg
MERGESTATE litFatSphere spheres
UNLOAD litFatSphere

UNIFORM spheres fatpointSize $radii

CULLING spheres OFF
RETURN spheres
EOF


# Convert to osgb and add Nick's preferred orientation
iris-convert spheres.iris.90,0,0.rot spheres.osgb
rm -f points.iris spheres.iris

#
# Build the final application
#


# wirebox and other files
qvd-wireBox 60 60 60 box.osgb

hev-moo -xsize 80 --examine box.osgb spheres.osgb > moo.iris

hev-animatorMCP nogeometry_points.iris > animator.mcp

cat > runapp <<EOF
#! /bin/bash

irisfly --examine moo.iris animator.mcp clip.mcp pointer.mcp

EOF
chmod +x runapp


cat > cleanup.sh <<EOF
#! /bin/bash

rm -fr rocks pages box.osgb moo.iris
rm -fr nogeometry_points.iris spheres.osgb
rm -fr animator.mcp runapp
rm -fr cleanup.sh
EOF
chmod +x cleanup.sh


echo
echo "To execute: ./runapp"

