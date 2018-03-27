#
#	Makefile for dcmsr/apps
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
LOCALLIBS = -ldcmsr -ldcmdata -lofstd $(XMLLIBS) $(ZLIBLIBS)

objs = dsrdump.o dsr2html.o dsr2xml.o xml2dsr.o
progs = dsrdump dsr2html dsr2xml xml2dsr
support = dsr2xml.xsd


all: $(progs)

dsrdump: dsrdump.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBDIRS) -o $@ $@.o $(LOCALLIBS) $(MATHLIBS) $(LIBS)

dsr2html: dsr2html.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBDIRS) -o $@ $@.o $(LOCALLIBS) $(MATHLIBS) $(LIBS)

dsr2xml: dsr2xml.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBDIRS) -o $@ $@.o $(LOCALLIBS) $(MATHLIBS) $(LIBS)

xml2dsr: xml2dsr.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBDIRS) -o $@ $@.o $(LOCALLIBS) $(MATHLIBS) $(LIBS)


install: install-bin install-support

install-bin: all
	$(configdir)/mkinstalldirs $(bindir)
	for prog in $(progs); do \
		$(INSTALL_PROGRAM) $$prog$(BINEXT) $(bindir); strip $(bindir)/$$prog$(BINEXT) ;\
	done

install-support:
	$(configdir)/mkinstalldirs $(libdir)
	for file in $(support); do \
		$(INSTALL_DATA) $$file $(libdir) ;\
	done

clean:
	rm -f $(objs) $(progs) $(TRASH)

distclean:
	rm -f $(objs) $(progs) $(DISTTRASH)


dependencies:
	$(CXX) -MM $(defines) $(includes) $(CPPFLAGS) $(CXXFLAGS) *.cc  > $(DEP)

include $(DEP)
