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
PUBLIC_SCRIPTS = hev-caveTest
PRIVATE_SCRIPTS =

# these are data files used by the project, and others too
# in $HEV_IDEA_DIR/data
PUBLIC_DATA = 

# these are data files used by the project, private,
# in $HEV_IDEA_DIR/etc/$(PROJECT)/data
PRIVATE_DATA = \
alignment.sh \
edgeColorTicksLabeled.png \
eyeTest.sh \
kill.sh \
killOff.sh \
labelDisplays.sh \
left.wav \
models.iris \
right.wav \
smartGel.000100.osg \
tattersallRedBorder.png \
test.mcp \
wandTest.sh \
sgon.iris \
wirecube.savg \
countDown.sh \
countDownDisplay.sh \
fade.iris \
5.savg \
4.savg \
3.savg \
2.savg \
1.savg \
0.savg \
tube.savg \
00_zero.wav \
01_one.wav \
02_two.wav \
03_three.wav \
04_four.wav \
05_five.wav \
\
gpuSpheres.iris \
gpuSpheres.savg \
\
3cubes.savg \
\
glinfo.sh







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

CLEAN_FILES = fade.iris tube.savg 3cubes.savg

# Command specific stuff
fade.iris:
	./mkfade.sh

tube.savg:	fade.iris

3cubes.savg:
	./mk3cubes.sh

