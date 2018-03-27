# you need to install iris before building the examples
CXXFLAGS=$(shell pkg-config --cflags iris)
LIBS=$(shell pkg-config --libs iris)

.PHONY : clean distclean

examples=$(wildcard *.cpp)

examples: $(patsubst %.cpp,%, $(examples))

%.d : %.cpp
	@dtk-msgNotice -p Makefile creating dependency file $@
	@set -e; rm -f $@; \
	$(CXX) -M $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

sinclude $(patsubst %.cpp,%.d, $(examples))

%.o : %.cpp %.d
	@dtk-msgNotice -p Makefile compiling $<
	@$(CXX) -c $(CXXFLAGS) $(INC) -o $@ $<

% : %.o
	@dtk-msgNotice -p Makefile linking to create executable $@
	@$(CXX) $@.o $(LIBS) -o $@

clean:
	rm -f $(patsubst %.cpp,%, $(examples)) $(patsubst %.cpp,%.o, $(examples)) $(patsubst %.cpp,%.so, $(examples))

distclean: clean
	rm -f $(patsubst %.cpp,%.d*, $(examples)) *~ core \#*
