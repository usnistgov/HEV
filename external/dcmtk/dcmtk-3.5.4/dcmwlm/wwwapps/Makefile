#
#	Makefile for dcmwlm/wwwapps
#




SHELL = /bin/sh
VPATH = ../include:../../config/include
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def

#
# -DNODISPLAY prevents any display code from being called
#
LOCALDEFS = -DNODISPLAY

ofstddir = $(top_srcdir)/../ofstd
ofstdinc = -I$(ofstddir)/include
ofstdlibdir = -L$(ofstddir)/libsrc
ofstdlib = -lofstd

dcmdatadir = $(top_srcdir)/../dcmdata
dcmdatainc = -I$(dcmdatadir)/include
dcmdatalibdir = -L$(dcmdatadir)/libsrc
dcmdatalib = -ldcmdata

LOCALINCLUDES = $(dcmdatainc) $(ofstdinc)
LIBDIRS = $(dcmdatalibdir) $(ofstdlibdir)
LOCALLIBS = $(dcmdatalib) $(ofstdlib) $(ZLIBLIBS)

objs = preplock.o readoviw.o readwlst.o writwlst.o
progs = preplock readoviw readwlst writwlst


all: $(progs)

# Note: preplock does not need $(LOCALLIBS)
preplock: preplock.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBDIRS) -o $@ preplock.o $(LIBS)

readoviw: readoviw.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBDIRS) -o $@ readoviw.o $(LOCALLIBS) $(MATHLIBS) $(LIBS)

readwlst: readwlst.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBDIRS) -o $@ readwlst.o $(LOCALLIBS) $(MATHLIBS) $(LIBS)

writwlst: writwlst.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBDIRS) -o $@ writwlst.o $(LOCALLIBS) $(MATHLIBS) $(LIBS)

install: all
	$(configdir)/mkinstalldirs $(bindir)
	for prog in $(progs); do \
		$(INSTALL_PROGRAM) $$prog$(BINEXT) $(bindir); strip $(bindir)/$$prog$(BINEXT) ;\
	done


clean:
	rm -f $(objs) $(progs) $(TRASH)

distclean:
	rm -f $(objs) $(progs) $(DISTTRASH)


dependencies:
	$(CXX) -MM $(defines) $(includes) $(CPPFLAGS) $(CXXFLAGS) *.cc > $(DEP)

include $(DEP)
