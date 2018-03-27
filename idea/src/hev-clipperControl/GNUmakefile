#!gmake

############# define MACROS #########################################

# define PROJECT to be the private directory name that this software will
# use, if any.  If not defined, it defaults to name of this directory.
PROJECT = 

# these are the binaries that get compiled AND installed as public binaries
# in $HEV_IDEA_DIR/bin/*
PUBLIC_BINS = hev-clipperControl

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
PUBLIC_DATA = clip.mcp

# these are data files used only by the project
# in $HEV_IDEA_DIR/etc/$(PROJECT)/data
PRIVATE_DATA = BoxGlyph.osg CornerGlyph.osg  SliceGlyph.osg  WedgeGlyph.osg WindshieldGlyph.osg

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
hev-clipperControl.d: hev-clipperControlGUI.cxx

# build the osg files 
%.osg: %.savg
	@dtk-msgNotice -p Makefile creating file $@
	@iris-convert $< $@

