#!gmake

############# define MACROS #########################################

# define PROJECT to be the directory name containing this software.  If not
# defined, it defaults to name of this directory.
PROJECT = 

# these are the binaries that get compiled AND installed as public binaries
# in $HEV_IDEA_DIR/bin/*
PUBLIC_BINS = 

# these are the binaries that get compiled AND installed as private binaries
# in $HEV_IDEA_DIR/etc/$(PROJECT)/bin/*
PRIVATE_BINS =

# these are DSOs that get compiled AND installed as public DSOs
# in $HEV_IDEA_DIR/lib/*
PUBLIC_DSOS = 

# these are the DSOs that get compiled AND installed as private DSOs
# in $HEV_IDEA_DIR/etc/$(PROJECT)/lib/*
PRIVATE_DSOS =

# same thing for scripts that get installed
PUBLIC_SCRIPTS = 

# these are scrpts that get copied to $HEV_IDEA_DIR/etc/$(PROJECT)/scripts/*
PRIVATE_SCRIPTS = eyeTest.sh killOff.sh

# these are data files used by the project, and others too
# in $HEV_IDEA_DIR/data
PUBLIC_DATA = *.mcp \
tape.osg cinta.png flashlight.iris \
caffeine.osg capsaicin.osg smartgel.osg \
pointerGlyph.osg plus3d.osg box.savg box.osg \
litFatSphere.osg fatglyph.osg fatpoint.osg fatsphere.osg \
lineAA.osg lit.osg litSolid.osg noShader.osg solid.osg \
unlit.osg unlitWire.osg wire.osg \
holodeck.iris holodeck_black.iris holodeck.osgt holodeck_black.osgt \
guestPointer.iris  guestPointerGlyph.osg \
pivotHead.iris pivotWand.iris \
teapot.osg pointerTrigger.sh echoON.iris echoOFF.iris

#holodeck_edges.savg holodeck.iris holodeck.savg


# these are data files used by the project, private,
# in $HEV_IDEA_DIR/etc/$(PROJECT)/data
PRIVATE_DATA = 

# these are the man pages that get installed in $HEV_IDEA_DIR/man
MAN_PAGES = 

################## makefile.include ##################################
# define CXX, CFLAGS, LIBS, etc.

# creates phoney target build

# creates real target test, and targets for PUBLIC_DSOS, PRIVATE_DSOS,
# PUBLIC_BINS, PRIVATE_BINS, install, clean, cleaner, distclean

# add or modify or make dependencies to them as needed after the include
# statement

include $(HEV_IDEA_DIR)/gnumakefile.include

.PHONY:	pivot.mcp pivotHead.iris pivotWand.iris


pointerGlyph.osg: pointerGlyph.savg
	iris-convert pointerGlyph.savg pointerGlyph.osg

guestPointerGlyph.osg: pointerGlyph.osg
	savg-rgb .7 0 .7 < pointerGlyph.savg > guestPointerGlyph.savg
	iris-convert guestPointerGlyph.savg guestPointerGlyph.osg

plus3d.osg: plus3d.savg
	iris-convert plus3d.savg plus3d.osg

## the front of the glyph lines up with the front of wand.osg
#pointerGlyph.savg:
#	savg-arrow | savg-rotate 0 -90 0 | savg-scale .1 .25 .1 | savg-translate 0 -.25 0 | savg-rgb .8 .8  0 > pointerGlyph.savg

# the base of the glyph lines up with the front of wand.osg
pointerGlyph.savg: GNUmakefile
	savg-arrow | savg-rotate 0 -90 0 | savg-scale .1 .25 .1 | savg-translate 0 0  0 | savg-rgb .8 .8  0 > pointerGlyph.savg

#this pops up at the end of the probe
plus3d.savg:
	savg-plus3d | savg-rgb 1 .1 .1 | savg-scale .025 > plus3d.savg

DISTCLEAN_FILES := pointerGlyph.savg plus3d.savg box.savg box.osg

CLEAN_FILES := pointerGlyph.osg guestPointerGlyph.osg plus3d.osg box.savg box.osg

build: pointerGlyph.osg plus3d.osg box.savg box.osg teapot.osg


box.savg:
	/bin/echo -e "# box.savg - wireframe box -1 to +1\n" > box.savg
	savg-cube | savg-rgb .5 .5 .5 | savg-wireframe >> box.savg

box.osg: box.savg
	iris-convert box.savg box.osg

holodeck.osgt: holodeck_black.savg holodeck_edges_black.savg
	iris-convert holodeck.savg holodeck_tmp.osgt
	iris-convert holodeck_edges.savg holodeck_edges_tmp.osgt
	iris-convert holodeck_tmp.osgt holodeck_edges_tmp.osgt holodeck.osgt
	rm -f holodeck_tmp.osgt holodeck_edges_tmp.osgt

holodeck_black.osgt: holodeck_black.savg holodeck_edges_black.savg
	iris-convert holodeck_black.savg holodeck_tmp_black.osgt
	iris-convert holodeck_edges_black.savg holodeck_edges_tmp_black.osgt
	iris-convert holodeck_tmp_black.osgt holodeck_edges_tmp_black.osgt holodeck_black.osgt
	rm -f holodeck_tmp_black.osgt holodeck_edges_tmp_black.osgt

lightsWorld.iris: lightsScene.iris
	sed "s/scene/world/" < lightsScene.iris > lightsWorld.iris

teapot.osg: teapot.obj teapot.mtl
	iris-convert teapot.obj teapot.osg
