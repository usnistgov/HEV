#
#	Makefile for dcmimgle/tests
#




SHELL = /bin/sh
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def

support = camera.lut monitor.lut printer.lut scanner.lut


all:

install: install-support

install-support:
	$(configdir)/mkinstalldirs $(datadir)
	for file in $(support); do \
		$(INSTALL_DATA) $$file $(datadir) ;\
	done

clean:
	rm -f $(TRASH)

distclean:
	rm -f $(DISTTRASH)

dependencies:
