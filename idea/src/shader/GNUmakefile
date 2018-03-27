#!gmake

############# define MACROS #########################################

# define PROJECT to be the directory name containing this software.  If not
# defined, it defaults to name of this directory.
PROJECT = 

# these are the binaries that get compiled AND installed as public binaries
# in $HEV_IDEA_DIR/bin/*
PUBLIC_BINS = 

# these are DSOs that get compiled AND installed as public DSOs
# in $HEV_IDEA_DIR/lib/*
PUBLIC_DSOS = 

# same thing for scripts that get installed
PUBLIC_SCRIPTS = 

# these are data files used by the project, and others too
# in $HEV_IDEA_DIR/data
PUBLIC_DATA = gpuSphere.iris

# shader files to be put in the "library" and shader data files- put in
# public since used by the include files
PUBLIC_SHADER_DATA = *.glsl *.png *.tif

# public shader programs that use the include files and their data, generated from shader.i files
# PUBLIC_SHADER_PROGRAMS = gpuSphere.frag gpuSphere.geom gpuSphere.vert
PUBLIC_SHADER_PROGRAMS = \
  fatglyph.frag  fatpoint.frag  fatsphere.frag  gpuSphere.frag  lineLight.frag  litFatSphere.frag  red.frag \
  fatglyph.vert  fatpoint.vert  fatsphere.vert  gpuSphere.vert  lineLight.vert  litFatSphere.vert  simple.vert \
  gpuSphere.geom  shrink.geom



# these are the man pages that get installed in $HEV_IDEA_DIR/man
MAN_PAGES = *.7

# additional files to remove with 'make clean'
CLEAN_FILES =

################## makefile.include ##################################
# define CXX, CFLAGS, LIBS, etc.

# creates phoney target build

# creates real target test, and targets for PUBLIC_DSOS, PRIVATE_DSOS,
# PUBLIC_BINS, PRIVATE_BINS, install, clean, cleaner, distclean

# add or modify or make dependencies to them as needed after the include
# statement

include $(HEV_IDEA_DIR)/gnumakefile.include
