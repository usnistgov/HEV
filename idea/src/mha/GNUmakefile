#!gmake

PUBLIC_DSOS = osgdb_mha.so osgdb_mhd.so

MAN_PAGES = mha.5

CLEAN_FILES = test.osg

include $(HEV_IDEA_DIR)/gnumakefile.include

# Since .mha and .mhd are the same, get the mhd loader by just copying
# the mha one. 
osgdb_mhd.o: osgdb_mha.o
	cp -f $< $@
