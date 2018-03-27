#
#	Makefile for dcmsr/include
#




SHELL = /bin/sh
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def


all: 

install: 
	$(configdir)/mkinstalldirs $(includedir)/dcmtk
	$(configdir)/mkinstalldirs $(includedir)/dcmtk/dcmsr
	for file in dcmtk/dcmsr/*.h ; do \
		$(INSTALL_DATA) $$file $(includedir)/dcmtk/dcmsr ; \
	done

clean:
	rm -f $(TRASH)

distclean:
	rm -f $(DISTTRASH)
