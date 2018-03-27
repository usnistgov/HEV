#
#	Makefile for dcmpstat/include
#




SHELL = /bin/sh
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def


all: 

install: 
	$(configdir)/mkinstalldirs $(includedir)/dcmtk
	$(configdir)/mkinstalldirs $(includedir)/dcmtk/dcmpstat
	for file in dcmtk/dcmpstat/*.h ; do \
		$(INSTALL_DATA) $$file $(includedir)/dcmtk/dcmpstat ; \
	done

clean:
	rm -f $(TRASH)

distclean:
	rm -f $(DISTTRASH)
