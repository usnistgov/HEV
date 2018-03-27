#!gmake

all: hev-lightEditor


.PHONY : localInstall check


# DTK_SPEW sets the level for display of dtk messages. 
# 4 means all messages that are INFO and above are displayed.
DTK_SPEW=4
export DTK_SPEW


# I am using the following include to provide :
#        rules for building .o and .d files 
#        rules for clean, dist, etc.
#        definition of some variables, such as CXX, LDFLAGS, and LDLIBS
# 
# I am NOT using it for install.
#
include $(HEV_SAVG_DGL_DIR)/gnumakefile.include


# Here are the .o files that are needed
localObjs =   \
			hev-lightEditor.o  \
			sgeComm.o \
			Global_GUI_S.o \
			Color_GUI_S.o \
			Position_GUI_S.o \
			File_GUI_S.o \
			General_GUI_S.o \
			InitLights.o \
			LightModel_GUI_S.o \
			LightSource_GUI_S.o 


# The dependency files for each of the .o files
localDependencyFiles = $(patsubst %.o,%.d, $(localObjs))
# At the end of this file, there is a short discussion of
# use of these dependency files.


#
# For some reason, the inclusion of the dependency files
# causes a problem for some of the phony targets.
# Does anyone know why?
#
# Anyway, to avoid the problem, we only include them for 
# the real targets.
#
ifneq ($(MAKECMDGOALS),clean)
 ifneq ($(MAKECMDGOALS),cleaner)
  ifneq ($(MAKECMDGOALS),distclean)
   ifneq ($(MAKECMDGOALS),tarball)
    ifneq ($(MAKECMDGOALS),dist)

       # -include does not generate an error message if
       # an included file does not exist.
       -include $(localDependencyFiles)

    endif
   endif
  endif
 endif
endif



# files to be deleted 
CLEAN_FILES = hev-lightEditor $(localObjs) $(localDependencyFiles)

# Compilation flags
CPPFLAGS = -g

# Load flags
LDFLAGS = -g


# We also need the fltk library when we link
LIBS := $(LIBS) `fltk-config --ldflags`


hev-lightEditor : $(localObjs) 
	@dtk-msgNotice -p Makefile linking to create executable $@
	$(CXX) $(localObjs) $(library) $(LIBS) $(LDFLAGS) $(LOADLIBS) $(LDLIBS) -o $@
	@echo Executable $@ has been created.
	@echo "At this point, you may want to:"
	@echo "     make check  - check permissions and group"
	@echo "     make perms  - set correct permissions and group"




#
# The following rule adds a prerequisite to the install rule that
# is in the file gnumakefile.include, which is included above.
# This prerequisite simply triggers another rule (defined below)
# that actually does the install. 
#
# We do it this way rather than just re-defining the install rule
# in order to avoid an error message from gmake. (It's the lack
# of any commands in this additional install rule that avoids
# the error message.)
#
# The commands in the install rule in gnumakefile.include will
# be bypassed by the "if" commands in that rule.
#
install : localInstall

localInstall : hev-lightEditor 
	cp --remove-destination -p hev-lightEditor $(HEV_SAVG_DGL_DIR)/bin
	cp --remove-destination -p man/hev-lightEditor.1 $(HEV_SAVG_DGL_DIR)/man/man1
	cp --recursive --remove-destination -p doc $(HEV_SAVG_DGL_DIR)/doc/hev-lightEditor


# set the group and permissions
perms: 
	chgrp -R diverse *
	chmod -R ug+w,a+r,o-w *


# check the group and permissions
check:
	hev-checkPerm
	hev-checkGroup


#############
#
# A Brief Note To Myself About The .d Dependency Files:
#
# If I understand this scheme for using the .d dependency files,
# the .d files are always updated one make-cycle late.
#
# All include and -include commands are done before any rule is 
# acted upon.  
#
# So if a .d file doesn't exist, it (of course) will not be included,
# but it will be generated and it can be included the next time that 
# gmake is run.
#
# If a .d file is out of date, the out of date file is included
# before the rule to remake it is triggered.  So the out of date
# dependencies are used, but the up to date dependencies will be
# generated in the new .d file and they will be available for the 
# next gmake.
#
# But I think that it is typically the case that these dependency
# files need to change much less frequently than the corresponding
# source files.  So, after they are created the first time, they
# will usually be correct.  After all, if you were maintaining the
# dependencies yourself in the make file, you wouldn't expect to
# have to change the make file every time you change a source file.
# In any event, a slightly out of date depencency file is still 
# better than none at all, and the automatic generation of the 
# dependency files saves time and is more reliable.
#
#############


#############
#
# Here are commands that can be used to copy the source
# from my directories to HEV-beta.
#
#   newgrp diverse
#   umask 2
#
#   cd /usr/local/HEV-beta/savg/dgl/src/hev-lightEditor
#   HEV-beta
#   make clean
#
#   \cp -r -f --preserve=timestamps ~/HEV/savg/dgl/src/hev-lightEditor/* .
#         note: you might have to use --remove-destination in the cp command
#               rather than -f
#
#   chgrp -R diverse * ; chmod -R ug+w,a+r,o-w *
#
#
#############

