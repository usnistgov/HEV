SUBDIRS   = external iris idea test
QUICKDIRS =          iris idea test
MAKETARGETS  = build install
CLEANTARGETS = clean distclean

.PHONY: $(MAKETARGETS) $(CLEANTARGETS)

all:
	@echo You should run 'make install' or 'make install-demos' from here

install-demos:
	@echo ; \
	echo ; \
	echo "# --------------------------------" ; \
	echo ; \
	echo demos ; \
	echo ; \
	$(MAKE) -C demos install; ret=$$? ; \
	if [ $$ret -ne 0 ] ; then exit $$ret ; fi  ; \
	    echo ;

distclean-demos:
	@echo ; \
	echo ; \
	echo "# --------------------------------" ; \
	echo ; \
	echo demos ; \
	echo ; \
	$(MAKE) -C demos distclean; ret=$$? ; \
	if [ $$ret -ne 0 ] ; then exit $$ret ; fi  ; \
	    echo ;

install-quick:
	@for dir in $(QUICKDIRS) ; do \
		echo ; \
		echo ; \
		echo "# --------------------------------" ; \
		echo ; \
		echo $$dir ; \
		echo ; \
	    $(MAKE) -C $$dir install; ret=$$? ; \
	    if [ $$ret -ne 0 ] ; then exit $$ret ; fi  ; \
		echo ; \
	done
	@mkdir -p idea/etc/hev-version

$(MAKETARGETS):
	@for dir in $(SUBDIRS) ; do \
		echo ; \
		echo ; \
		echo "# --------------------------------" ; \
		echo ; \
		echo $$dir $(MAKECMDGOALS) ; \
		echo ; \
	    $(MAKE) -C $$dir $(MAKECMDGOALS) ; ret=$$? ; \
	    if [ $$ret -ne 0 ] ; then exit $$ret ; fi  ; \
		echo ; \
	done
	@mkdir -p idea/etc/hev-version

$(CLEANTARGETS):
	@for dir in $(SUBDIRS) ; do \
		echo ; \
		echo ; \
		echo "# --------------------------------" ; \
		echo ; \
		echo $$dir $(MAKECMDGOALS) \
		echo ; \
	    $(MAKE) -C $$dir $(MAKECMDGOALS) ; ret=$$? ; \
	    if [ $$ret -ne 0 ] ; then exit $$ret ; fi  ; \
		echo ; \
	done

