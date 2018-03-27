#!gmake

############# define MACROS #########################################

# define PROJECT to be the directory name containing this software.  If not
# defined, it defaults to name of this directory.
PROJECT = 

# these are the binaries that get compiled AND installed as public binaries
# in $HEV_SAVG_DIR/bin/*
PUBLIC_BINS = hev-vectorRotation

# these are the binaries that get compiled AND installed as private binaries
# in $HEV_SAVG_DIR/etc/$(PROJECT)/bin/*
PRIVATE_BINS =

# these are DSOs that get compiled AND installed as public DSOs
# in $HEV_SAVG_DIR/lib/*
PUBLIC_DSOS = 

# these are the DSOs that get compiled AND installed as private DSOs
# in $HEV_SAVG_DIR/etc/$(PROJECT)/lib/*
PRIVATE_DSOS =

# same thing for scripts that get installed
PUBLIC_SCRIPTS =
PRIVATE_SCRIPTS =

# these are data files used by the project, and others too
# in $HEV_SAVG_DIR/data
PUBLIC_DATA = 

# these are data files used by the project, private,
# in $HEV_SAVG_DIR/etc/$(PROJECT)/data
PRIVATE_DATA = vector.osg

# these are the man pages that get installed in $HEV_SAVG_DIR/man
MAN_PAGES = hev-vectorRotation.1


################## makefile.include ##################################
# define CXX, CFLAGS, LIBS, etc.

# creates phoney target build

# creates real target test, and targets for PUBLIC_DSOS, PRIVATE_DSOS,
# PUBLIC_BINS, PRIVATE_BINS, install, clean, cleaner, distclean

# add or modify or make dependencies to them as needed after the include
# statement

include $(HEV_IDEA_DIR)/gnumakefile.include

CXXFLAGS += -fPIC
CXXFLAGS := $(CXXFLAGS) `fltk-config --cxxflags`
LIBS := $(LIBS) `fltk-config --ldflags` -lfltk_images -lpng -ljpeg


%.cxx: %.fl
	@dtk-msgNotice -p Makefile running fluid on $< to create C++ file $@
	fluid -c $<

DISTCLEAN_FILES := hev-vectorRotationGUI.cxx  hev-vectorRotationGUI.h

# add in the name of the cxx file used by your main program
hev-vectorRotation.d: hev-vectorRotationGUI.cxx



