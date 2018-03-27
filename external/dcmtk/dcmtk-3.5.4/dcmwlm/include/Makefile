#
#	Makefile for dcmwlm/include
#




SHELL = /bin/sh
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def


all:

install:
	$(configdir)/mkinstalldirs $(includedir)/dcmtk
	$(configdir)/mkinstalldirs $(includedir)/dcmtk/dcmwlm
	for file in dcmtk/dcmwlm/*.h ; do \
		$(INSTALL_DATA) $$file $(includedir)/dcmtk/dcmwlm ; \
	done

clean:
	rm -f $(TRASH)

distclean:
	rm -f $(DISTTRASH)
