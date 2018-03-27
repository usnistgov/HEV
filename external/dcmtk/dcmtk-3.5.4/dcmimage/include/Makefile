#
#	Makefile for dcmimage/include
#




SHELL = /bin/sh
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def


all: 

install: 
	$(configdir)/mkinstalldirs $(includedir)/dcmtk
	$(configdir)/mkinstalldirs $(includedir)/dcmtk/dcmimage
	for file in dcmtk/dcmimage/*.h ; do \
		$(INSTALL_DATA) $$file $(includedir)/dcmtk/dcmimage ; \
	done

clean:
	rm -f $(TRASH)

distclean:
	rm -f $(DISTTRASH)
