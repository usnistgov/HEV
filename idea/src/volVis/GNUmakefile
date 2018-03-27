#!gmake

# does nothing but make subdirectories

%::
	for d in libvolvis data glsl vol-mhaToNrrd vol-opacityAdjust vol-tfPrep vol-visBuilder  ;\
	do echo $$d ; cd $$d ; $(MAKE) $* ; status=$$? ; cd - ;\
	   if [ $$status -ne 0 ] ; then exit $$status ; fi ;\
	done

