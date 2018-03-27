#!gmake

PUBLIC_LIBS = libvolvis.a
PUBLIC_HEADERS = VolVis.h RgbaLut1D.h MetaImg.h GkBoundary.h
OBJECT_FILES = RgbaLut1D.o MetaImg.o GkBoundary.o
CLEAN_FILES = test1.RgbaLut1D

include $(HEV_IDEA_DIR)/gnumakefile.include

# Here are some variables to assist in compiling and linking of code that 
# uses VTK.  This method avoids using any of the cmake mechanisms that 
# VTK usually uses.
#
# vtk include files are currently here:
VTK_INC=$(HEVROOT)/external/vtk/installed/include/vtk-5.10
VTK_LIB=$(HEVROOT)/external/vtk/installed/lib/vtk-5.10
#
# The built-in rule for .cpp uses CXXFLAGS and LDLIBS.  We'll use
# these variables to specify the location of include and library
# files.
#
# For some reason, CXXFLAGS does not work but CPPFLAGS does.
# CXXFLAGS +=  -Wno-deprecated  -I$(VTK_INC)
CPPFLAGS +=  -Wno-deprecated  -I$(VTK_INC)
#
# All relevant vtk libraries.  We just link to all of them.
LDLIBS +=  -L$(VTK_LIB) \
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
        -lvtkParallel \
        -lvtkRendering \
        -lvtksqlite \
        -lvtksys \
        -lvtkverdict \
        -lvtkViews \
        -lvtkVolumeRendering \
        -lvtkWidgets
# end of cmake avoidance section.

test1.RgbaLut1D: test1.RgbaLut1D.cpp libvolvis.a
	$(CXX) -o $@ test1.RgbaLut1D.cpp -L .  -l volvis \
                $(LIBS) $(LDFLAGS) $(LOADLIBS) $(LDLIBS)
