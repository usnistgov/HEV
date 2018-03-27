#!gmake

PUBLIC_BINS = hev-frameGrabberControl

PUBLIC_DATA = grab.mcp

MAN_PAGES = *.1

# always include this!
include $(HEV_IDEA_DIR)/gnumakefile.include

# changes in the cxx file will cause a new dependency file to be built
hev-frameGrabberControl.d: hev-frameGrabberControlGUI.cxx
