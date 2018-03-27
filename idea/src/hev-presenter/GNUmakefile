#!gmake

PUBLIC_BINS = hev-presenter

MAN_PAGES = *.1

################## gnumakefile.include ##################################
# always include this!
include $(HEV_IDEA_DIR)/gnumakefile.include

# an example of adding to the dependencies; changes in the cxx file will
# cause a new dependency file to be built
hev-presenter.d: hev-presenterGUI.cxx
