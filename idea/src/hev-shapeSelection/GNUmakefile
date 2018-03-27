#!gmake

############# define MACROS #########################################

# define PROJECT to be the private directory name that this software will
# use, if any.  If not defined, it defaults to name of this directory.
PROJECT = 

# these are the binaries that get compiled AND installed as public binaries
# in $HEV_IDEA_DIR/bin/*
PUBLIC_BINS = hev-shapeSelection

# these are the binaries that get compiled AND installed as private binaries
# in $HEV_IDEA_DIR/etc/$(PROJECT)/bin/*
PRIVATE_BINS = exampleBinaryPoints

# these are DSOs that get compiled AND installed as public DSOs
# in $HEV_IDEA_DIR/lib/*
PUBLIC_DSOS = 

# these are the DSOs that get compiled AND installed as private DSOs
# in $HEV_IDEA_DIR/etc/$(PROJECT)/lib/*
PRIVATE_DSOS =

# these are scrpts that get copied to $HEV_IDEA_DIR/scripts/*
PUBLIC_SCRIPTS = 

# these are scrpts that get copied to $HEV_IDEA_DIR/etc/$(PROJECT)/scripts/*
PRIVATE_SCRIPTS =

# these are data files used by the project and other programs
# in $HEV_IDEA_DIR/data
PUBLIC_DATA = 

# these are data files used only by the project
# in $HEV_IDEA_DIR/etc/$(PROJECT)/data
PRIVATE_DATA = sphere.osgb sphereT.osgb \
	cube.osgb cubeT.osgb \
	pyramid.osgb pyramidT.osgb \
	tetrahedron.osgb tetrahedronT.osgb 

# these are the man pages that get installed in $HEV_IDEA_DIR/man
MAN_PAGES = *.1


################## makefile.include ##################################
# define CXX, CFLAGS, LIBS, etc.

# creates phoney target build

# creates real target test, and targets for PUBLIC_DSOS, PRIVATE_DSOS,
# PUBLIC_BINS, PRIVATE_BINS, install, clean, cleaner, distclean

include $(HEV_IDEA_DIR)/gnumakefile.include

# add or modify or make dependencies to them as needed after the include
# statement

# add this to tie chnges in the fluid file to the main program
hev-shapeSelection.d: hev-shapeSelectionGUI.cxx tetrahedron.array

# build the osgb files 
%.osgb: %.savg
	@dtk-msgNotice -p Makefile creating file $@
	@iris-convert --nolighting $< $@

%.osgb: %.osgt
	@dtk-msgNotice -p Makefile creating file $@
	@iris-convert --nolighting $< $@

CLEAN_FILES = hev-shapeSelectionGUI.cxx hev-shapeSelectionGUI.h

DISTCLEAN_FILES = *.savg *.osgb *.osgt

# geometry used by hev-shapeSelection
tetrahedron.points:
	bc -l < tetrahedron.formula > tetrahedron.points

tetrahedron.array: tetrahedron.points 
	cat tetrahedron.points | sed -e 's/$$/,/' > tetrahedron.array

tetrahedronT.savg: tetrahedron.points
	./makeTetra.sh `cat tetrahedron.points`

tetrahedron.savg: tetrahedronT.savg
	cat tetrahedronT.savg | savg-alpha 1 | savg-wireframe > tetrahedron.savg	

tetrahedron.osgb: tetrahedron.savg
	iris-convert --nolighting tetrahedron.savg tetrahedron.osgb

tetrahedronT.osgt: tetrahedronT.savg
	iris-convert --nolighting tetrahedronT.savg tmp.osgt
	cat tmp.osgt | sed -e 's/GL_DEPTH_TEST PROTECTED|OFF/GL_LIGHTING PROTECTED|OFF/' > tetrahedronT.osgt

sphere.savg:
	savg-sphere | savg-rgb .5 .5 .5 | savg-wireframe > sphere.savg

sphere.osgb: sphere.savg
	iris-convert --nolighting sphere.savg sphere.osgb

sphereT.savg:
	savg-sphere | savg-rgb .5 .5 .5 | savg-alpha .25 > sphereT.savg

sphereT.osgt: sphereT.savg
	iris-convert --nolighting sphereT.savg tmp.osgt
	cat tmp.osgt | sed -e 's/GL_DEPTH_TEST PROTECTED|OFF/GL_LIGHTING PROTECTED|OFF/' > sphereT.osgt

cube.savg:
	savg-cube | savg-rgb .5 .5 .5 | savg-wireframe > cube.savg

cube.osgb: cube.savg
	iris-convert --nolighting cube.savg cube.osgb

cubeT.savg: 
	savg-cube | savg-rgb .5 .5 .5 | savg-alpha .25 > cubeT.savg

cubeT.osgt: cubeT.savg
	iris-convert --nolighting cubeT.savg tmp.osgt
	cat tmp.osgt | sed -e 's/GL_DEPTH_TEST PROTECTED|OFF/GL_LIGHTING PROTECTED|OFF/' > cubeT.osgt

pyramidT.savg: 
	./makePyramid.sh

pyramid.savg: pyramidT.savg
	cat pyramidT.savg | savg-alpha 1 | savg-wireframe > pyramid.savg

pyramid.osgb: pyramid.savg
	iris-convert --nolighting pyramid.savg  pyramid.osgb

pyramidT.osgt: pyramidT.savg
	iris-convert --nolighting pyramidT.savg tmp.osgt
	cat tmp.osgt | sed -e 's/GL_DEPTH_TEST PROTECTED|OFF/GL_LIGHTING PROTECTED|OFF/' > pyramidT.osgt


