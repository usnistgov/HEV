#!gmake

PUBLIC_BINS = hev-xyzPositioner

PRIVATE_DATA = box.osg point.osg plus3d.osg xPlusAxis.osg xMinusAxis.osg yPlusAxis.osg yMinusAxis.osg zPlusAxis.osg zMinusAxis.osg

# these are the man pages that get installed in $HEV_IDEA_DIR/man
MAN_PAGES = *.1

include $(HEV_IDEA_DIR)/gnumakefile.include

# add in the name of the cxx file used by your main program
hev-xyzPositioner.d: hev-xyzPositionerGUI.cxx
