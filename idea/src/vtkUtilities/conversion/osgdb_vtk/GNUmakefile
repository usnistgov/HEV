#!gmake

############# define MACROS #########################################

# define PROJECT to be the private directory name that this software will
# use, if any.  If not defined, it defaults to name of this directory.
PROJECT = 

# these are the binaries that get compiled AND installed as public binaries
# in $HEV_IDEA_DIR/bin/*
PUBLIC_BINS = 

# these are the binaries that get compiled AND installed as private binaries
# in $HEV_IDEA_DIR/etc/$(PROJECT)/bin/*
PRIVATE_BINS =

# these are DSOs that get compiled AND installed as public DSOs
# in $HEV_IDEA_DIR/lib/*
PUBLIC_DSOS = osgdb_vtk.so

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
PRIVATE_DATA = 

# these are the man pages that get installed in $HEV_IDEA_DIR/man
MAN_PAGES = *.3


################## makefile.include ##################################
# define CXX, CFLAGS, LIBS, etc.


# # # # # # # # # # # # # # # # # # # # # # # # #

# Here are some variables to assist in linking of code that uses VTK
# without using any of the cmake mechanisms that VTK usually uses.

# vtk include files are currently here:
VTK_INC=$(HEVROOT)/external/vtk/installed/include/vtk-5.10
VTK_LIB=$(HEVROOT)/external/vtk/installed/lib/vtk-5.10

# The built-in rule for .cpp uses CXXFLAGS and LDLIBS.  We'll use
# these variables to specify the location of include and library
# files.

# JGH: For some reason, CXXFLAGS does not work but CPPFLAGS does.
# CXXFLAGS +=  -Wno-deprecated  -I$(VTK_INC)

CPPFLAGS +=  -Wno-deprecated  -I$(VTK_INC)

LDLIBS +=   -L$(VTK_LIB)

# All relevant vtk libraries.  We just link to all of them.
LDLIBS +=  \
        -lvtkCommon \
        -lvtkDICOMParser \
        -lvtkexoIIc \
        -lvtkFiltering \
        -lvtkftgl \
        -lvtkGenericFiltering \
        -lvtkGraphics \
        -lvtkHybrid \
        -lvtkImaging \
        -lvtkInfovis \
        -lvtkIO \
        -lvtklibxml2 \
        -lvtkmetaio \
        -lvtkNetCDF \
        -lvtkRendering \
        -lvtksqlite \
        -lvtksys \
        -lvtkverdict \
        -lvtkViews \
        -lvtkVolumeRendering \
        -lvtkWidgets

# end of cmake avoidance section.

# # # # # # # # # # # # # # # # # # # # # # # # #



# creates phoney target build

# creates real target test, and targets for PUBLIC_DSOS, PRIVATE_DSOS,
# PUBLIC_BINS, PRIVATE_BINS, install, clean, cleaner, distclean

# add or modify or make dependencies to them as needed after the include
# statement

include $(HEV_IDEA_DIR)/gnumakefile.include

