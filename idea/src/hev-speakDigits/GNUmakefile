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
PUBLIC_SCRIPTS = hev-speakDigits

# these are scrpts that get copied to $HEV_IDEA_DIR/etc/$(PROJECT)/scripts/*
PRIVATE_SCRIPTS =

# these are data files used by the project, and others too
# in $HEV_IDEA_DIR/data
PUBLIC_DATA = 

# these are data files used by the project, private,
# in $HEV_IDEA_DIR/etc/$(PROJECT)/data
PRIVATE_DATA = \
00_zero.wav \
01_one.wav \
02_two.wav \
03_three.wav \
04_four.wav \
05_five.wav \
06_six.wav \
07_seven.wav \
08_eight.wav \
09_nine.wav \
10_positive.wav \
11_negative.wav \
12_point.wav \
13_e.wav \
14_plus.wav \
15_minus.wav

# these are the man pages that get installed in $HEV_IDEA_DIR/man
MAN_PAGES = hev-speakDigits.1

################## makefile.include ##################################
# define CXX, CFLAGS, LIBS, etc.

# creates phoney target build

# creates real target test, and targets for PUBLIC_DSOS, PRIVATE_DSOS,
# PUBLIC_BINS, PRIVATE_BINS, install, clean, cleaner, distclean

# add or modify or make dependencies to them as needed after the include
# statement

include $(HEV_IDEA_DIR)/gnumakefile.include
