#!gmake

# Do make in sub-directories and a little bit of top level stuff

%::
	if [ $@ == "install" ] ; \
	then \
		if [ ! -d $(HEV_IDEA_DIR)/testSuite ] ; then \
		      echo "Making $(HEV_IDEA_DIR)/testSuite" ; \
		      mkdir -p "$(HEV_IDEA_DIR)/testSuite" ; \
		fi ; \
		cp hev-testSuite.5 ${HEV_IDEA_DIR}/man/man5  ; \
		cp README_testSuite.txt  $(HEV_IDEA_DIR)/testSuite  ; \
	fi ; \
	for d in test_* ; \
	do if [ -d $$d ]  ; \
	   then \
		echo ;         \
		echo ;         \
		echo "# ----------------------"    ;         \
		echo ;         \
		echo $$d ; \
		echo ;         \
		cd $$d ;   \
		echo working in `pwd` ; \
		if [ -f GNUmakefile ] ; then $(MAKE) $* ; fi;  \
		status=$$? ;   \
		cd - ;         \
		echo ;         \
		echo ;         \
		if [ $$status -ne 0 ] ; then exit $$status ; fi ; \
	   fi ; \
	done
