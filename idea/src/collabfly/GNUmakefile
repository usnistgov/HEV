#!gmake

PUBLIC_BINS = #hev-collabObjectMover

PRIVATE_BINS = hev-collab

PUBLIC_SCRIPTS = collabfly

PRIVATE_DATA = blue*.osg green*.osg hev-collab.mcp hev-collab-main.iris hev-collab-separate.iris sphere.osg cone.osg cube.osg

# these are the man pages that get installed in $HEV_IDEA_DIR/man
MAN_PAGES = *.1


################## makefile.include ##################################
# define CXX, CFLAGS, LIBS, etc.

# creates phoney target build

# creates real target test, and targets for PUBLIC_DSOS, PRIVATE_DSOS,
# PUBLIC_BINS, PRIVATE_BINS, install, clean, cleaner, distclean

# add or modify or make dependencies to them as needed after the include
# statement

include $(HEV_IDEA_DIR)/gnumakefile.include

cube.iris:
	savg-cube | savg-rgb .2 1 .2 > cube.savg ; \
	iris-DCS cube cube.savg > cube.iris
cone.iris:
	savg-cone | savg-rgb .2 .2 1  > cone.savg ; \
	iris-DCS cone --translate 0 0 -.5 --scale 1 1 2 --translate 3 0 0 cone.savg > cone.iris

sphere.iris:
	savg-sphere | savg-rgb 1 .2 .2 > sphere.savg ; \
	iris-DCS sphere --translate -3 0 0 sphere.savg > sphere.iris

%.osg: %.iris
	iris-convert $< $@

CLEAN_FILES = cube.* sphere.* cone.*

hev-collab.d: hev-collabGUI.cxx

#hev-collabObjectMover.d: hev-collabObjectMoverGUI.cxx
