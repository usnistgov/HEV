SUBDIRS = renderTest functionalityTest
MAKETARGETS = build install
CLEANTARGETS = clean distclean

.PHONY: $(MAKETARGETS) $(CLEANTARGETS)

all:
	@echo You should run 'make install' or 'make build' from here


$(MAKETARGETS):
	@for dir in $(SUBDIRS) ; do \
		echo ; \
		echo ; \
		echo "# --------------------------------" ; \
		echo ; \
		echo $$dir ; \
		echo ; \
	    $(MAKE) -C $$dir $@ ; ret=$$? ; \
	    if [ $$ret -ne 0 ] ; then exit $$ret ; fi  ; \
		echo ; \
	done


$(CLEANTARGETS):
	@for dir in $(SUBDIRS) ; do \
		echo ; \
		echo ; \
		echo "# --------------------------------" ; \
		echo ; \
		echo $$dir $@; \
		echo ; \
	    $(MAKE) -C $$dir $@ ; ret=$$? ; \
	    if [ $$ret -ne 0 ] ; then exit $$ret ; fi  ; \
		echo ; \
	done
