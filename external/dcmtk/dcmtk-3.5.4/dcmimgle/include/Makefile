#
#	Makefile for dcmimgle/include
#




SHELL = /bin/sh
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def


all: 

install: 
	$(configdir)/mkinstalldirs $(includedir)/dcmtk
	$(configdir)/mkinstalldirs $(includedir)/dcmtk/dcmimgle
	for file in dcmtk/dcmimgle/*.h ; \
		do $(INSTALL_DATA) $$file $(includedir)/dcmtk/dcmimgle ; \
	done

clean:
	rm -f $(TRASH)

distclean:
	rm -f $(DISTTRASH)
