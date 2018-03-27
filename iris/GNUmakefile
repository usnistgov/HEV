SUBDIRS = src
MAKETARGETS = build install
CLEANTARGETS = clean distclean

.PHONY: $(MAKETARGETS) $(CLEANTARGETS)

$(MAKETARGETS):
	@for dir in $(SUBDIRS) ; do \
		echo ; \
		echo ; \
		echo "# -------------------------------" ; \
		echo ; \
		echo $$dir ; \
		echo ; \
	    $(MAKE) -C $$dir $(MAKECMDGOALS); ret=$$? ; \
	    if [ $$ret -ne 0 ] ; then exit $$ret ; fi  ; \
		echo ; \
	done

$(CLEANTARGETS):
	@for dir in $(SUBDIRS) ; do \
		echo ; \
		echo ; \
		echo "# -------------------------------" ; \
		echo ; \
		echo $$dir ; \
		echo ; \
	    $(MAKE) -C $$dir $@ $(MAKECMDGOALS); ret=$$? ; \
	    if [ $$ret -ne 0 ] ; then exit $$ret ; fi  ; \
		echo ; \
	done
	if [ $@ == "distclean" ] ; \
	then \
		echo iris distclean found `pwd` ; \
		rm -rf bin doc include lib ; \
	fi

idea: iris

