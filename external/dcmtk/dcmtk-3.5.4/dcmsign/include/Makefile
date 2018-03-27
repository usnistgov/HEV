#
#	Makefile for dcmsign/include
#




SHELL = /bin/sh
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def


all: 

install: 
	$(configdir)/mkinstalldirs $(includedir)/dcmtk
	$(configdir)/mkinstalldirs $(includedir)/dcmtk/dcmsign
	for file in dcmtk/dcmsign/*.h ; do \
		$(INSTALL_DATA) $$file $(includedir)/dcmtk/dcmsign ; \
	done

clean:
	rm -f $(TRASH)

distclean:
	rm -f $(DISTTRASH)
