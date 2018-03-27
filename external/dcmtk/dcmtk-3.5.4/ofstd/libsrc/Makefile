#
#	Makefile for ofstd/libsrc
#




SHELL = /bin/sh
VPATH = ../include:../../config/include
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def

LOCALINCLUDES =
LOCALDEFS =

objs = oflist.o ofstring.o ofcmdln.o ofconapp.o offname.o ofconsol.o ofthread.o \
	oflogfil.o ofcond.o ofstd.o ofcrc32.o ofdate.o oftime.o ofdatime.o ofconfig.o
library = libofstd.$(LIBEXT)


all: $(library)

install: all
	$(configdir)/mkinstalldirs $(libdir)
	$(INSTALL_DATA) $(library) $(libdir)/$(library)

$(library): $(objs)
	$(AR) $(ARFLAGS) $@ $(objs)
	$(RANLIB) $@


clean:
	rm -f $(objs) $(library) $(TRASH)

distclean:
	rm -f $(objs) $(library) $(DISTTRASH)


dependencies:
	$(CXX) -MM $(defines) $(includes) $(CPPFLAGS) $(CXXFLAGS) *.cc  > $(DEP)

include $(DEP)
