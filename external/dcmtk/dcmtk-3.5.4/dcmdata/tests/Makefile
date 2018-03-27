#
#	Makefile for dcmdata/tests
#




SHELL = /bin/sh
VPATH = ../include:../../config/include
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def

ofstddir = $(top_srcdir)/../ofstd

LOCALINCLUDES = -I$(top_srcdir)/include -I$(ofstddir)/include
LIBDIRS = -L$(top_srcdir)/libsrc -L$(ofstddir)/libsrc
LOCALLIBS = -ldcmdata -lofstd $(ZLIBLIBS)

objs = tvrdatim.o
progs = tvrdatim


all: $(progs)

tvrdatim: tvrdatim.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBDIRS) -o $@ $@.o $(LOCALLIBS) $(MATHLIBS) $(LIBS)


install: all

clean:
	rm -f $(objs) $(progs) $(TRASH)

distclean:
	rm -f $(objs) $(progs) $(DISTTRASH)


dependencies:
	$(CXX) -MM $(defines) $(includes) $(CPPFLAGS) $(CXXFLAGS) *.cc  > $(DEP)

include $(DEP)
