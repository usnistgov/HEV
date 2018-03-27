# assumes nothing in here needs any non-standard packages such as dtk or osg-
# just "vanilla" Unix, whatever that is.

LIBRARY = libIdea.so

INCLUDE = idea/Utils.h

OBJECTS :=  $(patsubst %.cpp,%.o,$(wildcard *.cpp))

%.o: %.cpp
	$(CXX) -fPIC -c -I/usr/X11R6/include $(INC) -o $@ $<

${LIBRARY}: ${OBJECTS}
	$(CXX) -L/usr/X11R6/lib -lX11 -o $@ $^ -shared

distclean: clean
	rm -f *~

clean:
	rm -f $(OBJECTS) $(LIBRARY)

install: ${LIBRARY}
	@mkdir -p ../../lib
	cp ${LIBRARY} ../../lib
	@mkdir -p ../../include/idea
	@if (! cmp -s ../../include/idea/Utils.h idea/Utils.h) ; then sh -xc "cp ${INCLUDE} ../../include/idea" ; fi

