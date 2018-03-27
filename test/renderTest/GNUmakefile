#!gmake

# this GNUmakefile does nothing but make subdirectories

%::
	echo in renderTest
	if [ $@ == "install" ] ; \
	then \
		mkdir -p bin lib man/man1 ; \
	fi
	for d in src/* ; \
	do if [ -d $$d ]  ; \
	   then \
			echo ;         \
			echo ;         \
			echo "# ----------------------"    ;         \
			echo ;         \
			echo $$d ; \
			echo ;         \
			cd $$d ;   \
			$(MAKE) $* ;   \
			status=$$? ;   \
			cd - ;         \
			echo ;         \
			echo ;         \
	   		if [ $$status -ne 0 ] ; then exit $$status ; fi ; \
	   fi ; \
	done
	if [ $@ == "distclean" ] ; \
	then \
		rm -rf bin lib man ; \
	fi

