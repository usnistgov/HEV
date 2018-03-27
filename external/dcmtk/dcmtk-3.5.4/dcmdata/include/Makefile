#
#	Makefile for dcmdata/include
#




SHELL = /bin/sh
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def


all:

install:
	$(configdir)/mkinstalldirs $(includedir)/dcmtk
	$(configdir)/mkinstalldirs $(includedir)/dcmtk/dcmdata
	for file in dcmtk/dcmdata/*.h ; do \
		$(INSTALL_DATA) $$file $(includedir)/dcmtk/dcmdata ;\
	done

clean:
	rm -f $(TRASH)

distclean:
	rm -f $(DISTTRASH)
