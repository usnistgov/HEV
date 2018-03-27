#! /bin/bash

# This script is example of creating a visualization of the inclusions in the
# .qvisdat file (output of QDPD) using the qvd-toIRIS command.
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
# Make a single "shape 0" sphere
rm -rf rocks
mkdir rocks
radii=1.398
hev-savgSphere 4 | savg-scale $radii $radii $radii > rocks/s.0000.savg

rm -rf pages
mkdir pages
n=0   # set n to number of CPUs, or 0 to automatically determine
      # Note: for large data, this can easily exhaust system memory
      #       and terminate processing
qvd-toIRIS $VERBOSE --parallel $n rocks/s.%04d.savg  $QVISDAT spheres.iris pages spheres

#
# Build the final application
#

# Convert to osgb and add Nick's preferred orientation
iris-convert spheres.iris.90,0,0.rot spheres.osgb

# wirebox and oter files
qvd-wireBox 60 60 60 box.osgb

hev-moo -xsize 80 --examine box.osgb spheres.osgb > moo.iris

hev-animatorMCP nogeometry_spheres.iris > animator.mcp

cat > runapp <<EOF
#! /bin/bash

irisfly --examine moo.iris animator.mcp clip.mcp pointer.mcp

EOF
chmod +x runapp

cat > cleanup.sh <<EOF
#! /bin/bash

rm -fr rocks pages box.osgb moo.iris
rm -fr spheres.iris nogeometry_spheres.iris spheres.osgb
rm -fr animator.mcp runapp
rm -fr cleanup.sh
EOF
chmod +x cleanup.sh

echo
echo "To execute: ./runapp"

