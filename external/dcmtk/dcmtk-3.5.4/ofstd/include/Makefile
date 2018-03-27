#
#	Makefile for ofstd/include
#




SHELL = /bin/sh
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def


all:

install:
	$(configdir)/mkinstalldirs $(includedir)/dcmtk
	$(configdir)/mkinstalldirs $(includedir)/dcmtk/ofstd
	for file in dcmtk/ofstd/*.h ; do \
		$(INSTALL_DATA) $$file $(includedir)/dcmtk/ofstd ; \
	done

clean:
	rm -f $(TRASH)

distclean:
	rm -f $(DISTTRASH)
