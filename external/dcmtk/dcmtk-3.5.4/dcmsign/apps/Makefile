#
#	Makefile for dcmsign/apps
#




SHELL = /bin/sh
VPATH = ../include:../../config/include
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def

ofstddir = $(top_srcdir)/../ofstd
dcmdatadir = $(top_srcdir)/../dcmdata

LOCALINCLUDES = -I$(ofstddir)/include -I$(dcmdatadir)/include
LIBDIRS = -L$(top_srcdir)/libsrc -L$(ofstddir)/libsrc -L$(dcmdatadir)/libsrc
LOCALLIBS = -ldcmdsig -ldcmdata -lofstd $(ZLIBLIBS) $(OPENSSLLIBS)

objs = dcmsign.o
progs = dcmsign


all: $(progs)

dcmsign: dcmsign.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBDIRS) -o $@ $@.o $(LOCALLIBS) $(MATHLIBS) $(LIBS)

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
	$(CXX) -MM $(defines) $(includes) $(CPPFLAGS) $(CXXFLAGS) *.cc  > $(DEP)

include $(DEP)
