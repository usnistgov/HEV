#
#	Makefile for dcmqrdb/include
#




SHELL = /bin/sh
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def


all: 

install: 
	$(configdir)/mkinstalldirs $(includedir)/dcmtk
	$(configdir)/mkinstalldirs $(includedir)/dcmtk/dcmqrdb
	for file in dcmtk/dcmqrdb/*.h ; do \
		$(INSTALL_DATA) $$file $(includedir)/dcmtk/dcmqrdb ; \
	done

clean:
	rm -f $(TRASH)

distclean:
	rm -f $(DISTTRASH)
