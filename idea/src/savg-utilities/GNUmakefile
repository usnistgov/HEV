#!gmake

############# define MACROS #########################################

# define PROJECT to be the directory name containing this software.  If not
# defined, it defaults to name of this directory.
PROJECT =

# these are the binaries that get compiled AND installed as public binaries
# in $HEV_IDEA_DIR/bin/*
PUBLIC_BINS = \
savg-add-normals \
savg-alpha \
savg-arrow \
savg-bargraph \
savg-bird \
savg-chair \
savg-clock \
savg-color \
savg-computer \
savg-cone \
savg-confetti \
savg-cube \
savg-cylinder \
savg-dualtetra \
savg-ellipsoid \
savg-fish \
savg-flag \
savg-graphFilter \
savg-hsi2rgb \
savg-hsv2rgb \
savg-icecream \
savg-line \
savg-line-arrow \
savg-lineplot \
savg-object \
savg-octa \
savg-output-normals \
savg-pie \
savg-prism \
savg-ptgraph \
savg-rainbow \
savg-regpoly \
savg-reverse \
savg-reverse-normals \
savg-rgb2hsi \
savg-rgb2hsv \
savg-rgb \
savg-rm-normals \
savg-rm-rgba \
savg-rmcom \
savg-rotate \
savg-scale \
savg-scatterplot \
savg-smiley \
savg-sor \
savg-sphere \
savg-split \
savg-square \
savg-surfaceplot \
savg-table \
savg-tetra \
savg-text \
savg-text-font \
savg-text-justification \
savg-translate \
savg-wireframe \
savg-xyzPlanes

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
PUBLIC_SCRIPTS = \
savg-ambient \
savg-colormode \
savg-diffuse \
savg-emission \
savg-lighting \
savg-line-tangents \
savg-octree \
savg-pixelsize \
savg-plus3d \
savg-shininess \
savg-shrinkage \
savg-specular \
savg-striplength \
savg-style \
savg-text-flatten \
savg-text-style \
savg-texture-square \
savg-transparency 

PRIVATE_SCRIPTS = 

# these are data files used by the project, and others too
# in $HEV_IDEA_DIR/data
PUBLIC_DATA = 

# these are data files used by the project, private,
# in $HEV_IDEA_DIR/etc/$(PROJECT)/data
PRIVATE_DATA = 

# these are the man pages that get installed in $HEV_IDEA_DIR/man
MAN_PAGES = \
savg-utilities.3 \
savg-add-normals.1 \
savg-alpha.1 \
savg-arrow.1 \
savg-bargraph.1 \
savg-color.1 \
savg-cone.1 \
savg-cube.1 \
savg-cylinder.1 \
savg-line-arrow.1 \
savg-lineplot.1 \
savg-line-tangents.1 \
savg-octa.1 \
savg-output-normals.1 \
savg-plus3d.1 \
savg-prism.1 \
savg-regpoly.1 \
savg-reverse.1 \
savg-reverse-normals.1 \
savg-rgb.1 \
savg-rm-normals.1 \
savg-rm-rgba.1 \
savg-rotate.1 \
savg-scale.1 \
savg-scatterplot.1 \
savg-sor.1 \
savg-sphere.1 \
savg-split.1 \
savg-square.1 \
savg-surfaceplot.1 \
savg-tetra.1 \
savg-text.1 \
savg-translate.1 \
savg-wireframe.1 \
savg-xyzPlanes.1




################## makefile.include ##################################
# define CXX, CFLAGS, LIBS, etc.

# Compilation flags
# CFLAGS = -g

# Load flags
# LDFLAGS = -g



# creates phoney target build

# creates real target test, and targets for PUBLIC_DSOS, PRIVATE_DSOS,
# PUBLIC_BINS, PRIVATE_BINS, install, clean, cleaner, distclean

# add or modify or make dependencies to them as needed after the include
# statement

include $(HEV_IDEA_DIR)/gnumakefile.include

scripts:
	for f in $(PUBLIC_SCRIPTS) ; \
	do rm -f $$f ; \
	   cp unimplemented.sh $$f ; \
	done ;
