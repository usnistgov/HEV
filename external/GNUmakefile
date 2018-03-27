SUBDIRS = \
		  dtk coin osg gle \
		  vtk teem python3 R node bulletphysics bulletphysics

MAKETARGETS  = build install
CLEANTARGETS = clean distclean

.PHONY: $(MAKETARGETS) $(CLEANTARGETS)

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
	./.fixperm.sh # Set permissions if on Dradis and user in diverse group




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

