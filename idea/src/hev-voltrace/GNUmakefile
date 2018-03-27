#!gmake

############# define MACROS #########################################

# define PROJECT to be the directory name containing this software.  If not
# defined, it defaults to name of this directory.
PROJECT = 

# these are the binaries that get compiled AND installed as public binaries
# in $HEV_IDEA_DIR/bin/*
PUBLIC_BINS =

# hev-volOptimize uses ITK image toolkit, which is not built for 64-bit
ifneq ($(shell uname -m),x86_64)
  PUBLIC_BINS += hev-volOptimize
endif

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
PUBLIC_SCRIPTS = hev-voltrace hev-preintegrate hev-LUT2mhd
PRIVATE_SCRIPTS =

# these are shader programs to be installed in $HEVROOT/idea/shader
PUBLIC_SHADER_PROGRAMS = 

# these are shader programs used by the project, private,
# in $HEV_IDEA_DIR/etc/$(PROJECT)/shader
PRIVATE_SHADER_PROGRAMS = \
	volume.frag			     \
	volume.vert			     \
	pragma.glsl			     \
	uniform.glsl			     \
	varying.glsl			     \
	voltrace-integration-linear.glsl     \
	voltrace-integration-point.glsl	     \
	voltrace-transfer-emit.glsl	     \
	voltrace-transfer-lit.glsl	     \
	voltrace-transfer-preint.glsl	     \
	voltrace-voldata-lookup.glsl         \
	voltrace-voldata-scaled.glsl


# these are the man pages that get installed in $HEV_IDEA_DIR/man
MAN_PAGES = hev-voltrace.1 hev-preintegrate.1 hev-LUT2mhd.1

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

ITK=/usr
CXXFLAGS += -g -I$(ITK)/include/InsightToolkit \
	-I$(ITK)/include/InsightToolkit/Algorithms \
	-I$(ITK)/include/InsightToolkit/BasicFilters \
	-I$(ITK)/include/InsightToolkit/Common \
	-I$(ITK)/include/InsightToolkit/gdcm \
	-I$(ITK)/include/InsightToolkit/IO \
	-I$(ITK)/include/InsightToolkit/Numerics \
	-I$(ITK)/include/InsightToolkit/SpatialObject \
	-I$(ITK)/include/InsightToolkit/Utilities \
	-I$(ITK)/include/InsightToolkit/Utilities/vxl/core \
	-I$(ITK)/include/InsightToolkit/Utilities/vxl/vcl
LDFLAGS += -g -L$(ITK)/lib/InsightToolkit -Wl,-rpath -Wl,$(ITK)/lib/InsightToolkit
LDLIBS += -lITKIO -lITKCommon -litksys
