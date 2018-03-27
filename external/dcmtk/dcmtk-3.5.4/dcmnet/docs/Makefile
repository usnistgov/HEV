#
#	Makefile for dcmnet/docs
#




SHELL = /bin/sh
srcdir = .
top_srcdir = ..
configdir = ../../config

include $(configdir)/Makefile.def

files = asconfig.txt testing.txt


all:

install:
	$(configdir)/mkinstalldirs $(docdir)
	for file in $(files); do \
		$(INSTALL_DATA) $$file $(docdir) ;\
	done

clean:
	rm -f $(TRASH)

distclean:
	rm -f $(DISTTRASH)
