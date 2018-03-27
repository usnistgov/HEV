#
#	Makefile for dcmnet/include
#




SHELL = /bin/sh
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def


all:

install:
	$(configdir)/mkinstalldirs $(includedir)/dcmtk
	$(configdir)/mkinstalldirs $(includedir)/dcmtk/dcmnet
	for file in dcmtk/dcmnet/*.h ; \
		do $(INSTALL_DATA) $$file $(includedir)/dcmtk/dcmnet ; \
	done

clean:
	rm -f $(TRASH)

distclean:
	rm -f $(DISTTRASH)
