#
#	Makefile for dcmjpeg/include
#




SHELL = /bin/sh
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def


all: 

install: 
	$(configdir)/mkinstalldirs $(includedir)/dcmtk
	$(configdir)/mkinstalldirs $(includedir)/dcmtk/dcmjpeg
	for file in dcmtk/dcmjpeg/*.h ; do \
		$(INSTALL_DATA) $$file $(includedir)/dcmtk/dcmjpeg ; \
	done

clean:
	rm -f $(TRASH)

distclean:
	rm -f $(DISTTRASH)
