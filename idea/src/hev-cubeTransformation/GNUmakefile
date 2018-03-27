#!gmake

############# define MACROS #########################################

# define PROJECT to be the private directory name that this software will
# use, if any.  If not defined, it defaults to name of this directory.
PROJECT = 

# these are the binaries that get compiled AND installed as public binaries
# in $HEV_IDEA_DIR/bin/*
PUBLIC_BINS = hev-cubeTransformationWand hev-cubeTransformationRollers

# these are the binaries that get compiled AND installed as private binaries
# in $HEV_IDEA_DIR/etc/$(PROJECT)/bin/*
PRIVATE_BINS =

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
PRIVATE_DATA = cube.osgb highlightCube.osgb highlightSquare-X.osgb highlightSquare-Y.osgb highlightSquare-Z.osgb \
	highlightSquare+X.osgb highlightSquare+Y.osgb highlightSquare+Z.osgb 

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
hev-cubeTransformationRollers.d: hev-cubeTransformationRollersGUI.cxx
hev-cubeTransformationWand.d: hev-cubeTransformationWandGUI.cxx

# build the osgb files 
%.osgb: %.savg
	@dtk-msgNotice -p Makefile creating file $@
	@iris-convert $< $@

%.osgb: %.osg
	@dtk-msgNotice -p Makefile creating file $@
	@iris-convert $< $@

CLEAN_FILES = hev-cubeTransformationRollersGUI.cxx hev-cubeTransformationRollersGUI.h hev-cubeTransformationWandGUI.cxx hev-cubeTransformationWandGUI.h

DISTCLEAN_FILES = *.savg *.osgb *.osg

# geometry used by cubeTransformRollers
cube.savg:
	savg-cube | savg-rgb .5 .5 .5 | savg-wireframe > cube.savg

highlightCube.osg:
	savg-cube | savg-rgb .5 .5 .5 | hev-tubify --join ROUND --radius 0.01 > highlightCube.osg

highlightSquare+X.osg: highlightSquare+X.savg.save
	hev-tubify --join ROUND --radius 0.01 < highlightSquare+X.savg.save > highlightSquare+X.osg

highlightSquare-X.osg: highlightSquare-X.savg.save
	hev-tubify --join ROUND --radius 0.01 < highlightSquare-X.savg.save > highlightSquare-X.osg

highlightSquare+Y.osg: highlightSquare+Y.savg.save
	hev-tubify --join ROUND --radius 0.01 < highlightSquare+Y.savg.save > highlightSquare+Y.osg

highlightSquare-Y.osg: highlightSquare-Y.savg.save
	hev-tubify --join ROUND --radius 0.01 < highlightSquare-Y.savg.save > highlightSquare-Y.osg

highlightSquare+Z.osg: highlightSquare+Z.savg.save
	hev-tubify --join ROUND --radius 0.01 < highlightSquare+Z.savg.save > highlightSquare+Z.osg

highlightSquare-Z.osg: highlightSquare-Z.savg.save
	hev-tubify --join ROUND --radius 0.01 < highlightSquare-Z.savg.save > highlightSquare-Z.osg
