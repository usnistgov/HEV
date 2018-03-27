#
#	Makefile for dcmqrdb/libsrc
#




SHELL = /bin/sh
VPATH = ../include:../../config/include
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def

ofstddir = $(top_srcdir)/../ofstd
dcmdatadir = $(top_srcdir)/../dcmdata
dcmnetdir = $(top_srcdir)/../dcmnet

LOCALINCLUDES = -I$(dcmnetdir)/include -I$(dcmdatadir)/include -I$(ofstddir)/include
LOCALDEFS =

objs = dcmqrcbf.o dcmqrcbg.o dcmqrcbm.o dcmqrcbs.o dcmqrcnf.o dcmqrdbi.o  \
       dcmqrdbs.o dcmqropt.o dcmqrptb.o dcmqrsrv.o dcmqrtis.o

library = libdcmqrdb.$(LIBEXT)

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
