#
#	Makefile for dcmtls/include
#




SHELL = /bin/sh
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def


all: 

install: 
	$(configdir)/mkinstalldirs $(includedir)/dcmtk
	$(configdir)/mkinstalldirs $(includedir)/dcmtk/dcmtls
	for file in dcmtk/dcmtls/*.h ; do \
		$(INSTALL_DATA) $$file $(includedir)/dcmtk/dcmtls ; \
	done

clean:
	rm -f $(TRASH)

distclean:
	rm -f $(DISTTRASH)
