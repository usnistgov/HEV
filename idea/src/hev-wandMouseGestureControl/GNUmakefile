#!gmake

# This file can be used as a starting point for your application's
# GNUmakefile. Just copy it to your application's src directory:
#   cp $HEVROOT/idea/GNmakefile.template GNUmakefile
# then fill in the labels and add any extra rules as described below

############# define MACROS #########################################

# define PROJECT to be the private directory name that this software will
# use, if any.  If not defined, it defaults to name of this directory.
#
# in most cases you don't need to set this
#PROJECT =  hev-gestureControl

# These are the binaries that get compiled AND installed
# in $HEV_IDEA_DIR/bin/*
PUBLIC_BINS = hev-wandMouseGestureControl

# These are the binaries that get compiled AND installed
# in $HEV_IDEA_DIR/etc/$(PROJECT)/bin/*
#PRIVATE_BINS =

# These are DSOs that get compiled AND installed
# in $HEV_IDEA_DIR/lib/* The filenames should end in ".so"
#PUBLIC_DSOS = 

# These are the DSOs that get compiled AND installed
# in $HEV_IDEA_DIR/etc/$(PROJECT)/lib/* The filenames should end in ".so"
#PRIVATE_DSOS =

# These are scrpts that get copied to $HEV_IDEA_DIR/scripts/*
#PUBLIC_SCRIPTS = 

# These are scrpts that get copied to $HEV_IDEA_DIR/etc/$(PROJECT)/scripts/*
#PRIVATE_SCRIPTS =

# These are data files used by the project and other programs
# in $HEV_IDEA_DIR/data
#PUBLIC_DATA = 

# These are data files used only by the project
# in $HEV_IDEA_DIR/etc/$(PROJECT)/data
#PRIVATE_DATA =

# These are the man pages that get installed in $HEV_IDEA_DIR/man
MAN_PAGES = hev-wandMouseGestureControl.1 


# This defines the names of programs that will be compiled into object files
# and put into a shared object library which the files specified in
# PUBLIC_BINS, PRIVATE_BINS, PUBLIC_DSOS and PRIVATE_DSOS can link with and
# access at run time.  The filenames should end in ".o". The shared object
# library will be installed in the
# $HEV_IDEA_DIR/etc/$(PROJECT)/lib/directory
#PRIVATE_LIBRARY_FILES = 

# These are shader programs that are either complete or are built from
# corresponding "shader".i files.  "shader".i is a geometry, vertex or
# fragment shader file with optional #include directives. Each "shader".i
# will get preprocessed into "shader", and has a dependency file
# "shader".d. The complete or built shader files will be installed in
# $(HEV_IDEA_DIR)/shader.
#PUBLIC_SHADER_PROGRAMS =

# These are data and include files used by shaders They are just copied to
# $(HEV_IDEA_DIR)/shader, which is in the OSG file search path. This
# directory will be added to cpp's list of include directories, so shaders
# can just #include a file from this library

#PUBLIC_SHADER_DATA =

# $(PRIVATE_SHADER_PROGRAMS) and $(PRIVATE_SHADER_DATA) work the same as
# their public counterparts, but are installed in the directory
# $(HEV_IDEA_DIR)/etc/$(PROJECT)/shader, a directory that isn't
# part of the global OSG file search path- you need to add it yourself
#PRIVATE_SHADER_PROGRAMS = 
#PRIVATE_SHADER_DATA

# add any other data files to be installed for public use. Create targets
# and rules for them if they need to be built. They're installed in
# $(HEV_IDEA_DIR)/data
#PUBLIC_DATA = 

# add any other data files to be installed for private use. Create targets
# and rules for them if they need to be built. They're installed in
# $(HEV_IDEA_DIR)/etc/$(PROJECT)/data
#PRIVATE_DATA = 

# list additional files to be deleted by a "make clean". By default
# dependency files, $(PUBLIC_BINS), $(PUBLIC_DSOS) $(PRIVATE_BINS),
# $(PRIVATE_DSOS) $(PRIVATE_LIBRARY_FILES) and $(PUBLIC_SHADER_PROGRAMS) and
# $(PRIVATE_SHADER_PROGRAMS) built from ".i" files will be deleted.
#CLEAN_FILES = 

# list additional files to be deleted by a "make distclean". By default
# *~, core, *.d.* and *.d will be deleted after running "make clean"
#DISTCLEAN_FILES = 


########################################################################
# always include this!
include $(HEV_IDEA_DIR)/gnumakefile.include

########################################################################
# from here down you can create new targets if needed

# an example of defining a new rule which converts a savg file to an ive file
#%.ive: %.savg
#	@dtk-msgNotice -p Makefile creating file $@
#	@iris-convert $< $@

# an example of adding to the dependencies; changes in the cxx file will
# cause a new dependency file to be built
#anFLTKprogram.d: anFLTKprogramGUI.cxx
