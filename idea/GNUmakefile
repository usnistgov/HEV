FIRST_SUBDIRS = \
				src/Utils/ \
				src/savg/ \
				src/savg-utilities/ \
				src/hev-savgUtilities/ \
				src/volVis/ \
				src/hev-tubify/ \
				src/shader/

LAST_SUBDIRS  = \
				src/hev-screenCautionTape/ \
				src/hev-testSuite/ \
				src/hev-caveTest/ \
				src/doc/

IGNORE_SUBDIRS = src/hev-hydview/ src/hev-triangulation/

ALL_SUBDIRS = $(filter-out $(IGNORE_SUBDIRS),$(sort $(dir $(wildcard src/*/GNUmakefile))))
REST_SUBDIRS = $(filter-out $(FIRST_SUBDIRS),$(filter-out $(LAST_SUBDIRS),$(ALL_SUBDIRS)))

MAKETARGETS  = build install
CLEANTARGETS = clean distclean

.PHONY: $(MAKETARGETS) $(CLEANTARGETS)

all:
	@echo You should run 'make install' from here

$(MAKETARGETS):
	@for dir in $(FIRST_SUBDIRS) ; do \
		echo ; \
		echo ; \
		echo "# --------------------------------" ; \
		echo ; \
		echo $$dir $(MAKECMDGOALS) ; \
		echo ; \
	    cd $$dir && $(MAKE) $(MAKECMDGOALS); ret=$$? ; \
	    if [ $$ret -ne 0 ] ; then exit $$ret ; fi  ; \
		cd $$OLDPWD ; \
		echo ; \
	done && \
	for dir in $(REST_SUBDIRS) ; do \
		echo ; \
		echo ; \
		echo "# --------------------------------" ; \
		echo ; \
		echo $$dir $(MAKECMDGOALS) ; \
		echo ; \
	    cd $$dir && $(MAKE) $(MAKECMDGOALS); ret=$$? ; \
	    if [ $$ret -ne 0 ] ; then exit $$ret ; fi  ; \
		cd $$OLDPWD ; \
		echo ; \
	done && \
	for dir in $(LAST_SUBDIRS) ; do \
		echo ; \
		echo ; \
		echo "# --------------------------------" ; \
		echo ; \
		echo $$dir $(MAKECMDGOALS) ; \
		echo ; \
	    cd $$dir && $(MAKE) $(MAKECMDGOALS); ret=$$? ; \
	    if [ $$ret -ne 0 ] ; then exit $$ret ; fi  ; \
		cd $$OLDPWD ; \
		echo ; \
	done

$(CLEANTARGETS):
	@for dir in $(ALL_SUBDIRS) ; do \
		echo ; \
		echo ; \
		echo "# --------------------------------" ; \
		echo ; \
		echo $$dir $(MAKECMDGOALS) ; \
		echo ; \
	    cd $$dir && $(MAKE) $(MAKECMDGOALS) ; ret=$$? ; \
	    if [ $$ret -ne 0 ] ; then exit $$ret ; fi  ; \
		cd $$OLDPWD ; \
		echo ; \
	done
	if [ $@ == "distclean" ] ; \
	then \
		rm -rf bin data doc etc include lib man shader testSuite ; \
	fi

