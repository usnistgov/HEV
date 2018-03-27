# you need to install iris before building the examples
CXXFLAGS = $(shell pkg-config --cflags iris)
CXXFLAGS := $(CXXFLAGS) $(shell fltk-config --cxxflags)

LIBS = $(shell pkg-config --libs iris)
LIBS := $(LIBS) $(shell fltk-config --ldflags) -lfltk_images -lpng -ljpeg

# assume that $(main).cpp is the source and only cpp file
main = $(patsubst %.cpp,%, $(wildcard *.cpp))

# assume just one fluid file
fl = $(wildcard *.fl)
fld = $(patsubst %.fl,%.d, $(fl))
flcxx := $(patsubst %.fl,%.cxx, $(fl))
flh := $(patsubst %.fl,%.h, $(fl))

.PHONY : test build clean distclean

build:	test $(main)

test:
	@if ! which fltk-config>/dev/null ; then echo; \
          echo "You must put \`fltk-config' in your path" ;\
          exit 1; fi


%.d : %.cpp $(flcxx)
	@dtk-msgNotice -p Makefile creating dependency file $@
	@set -e; rm -f $@; \
	$(CXX) -M $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

sinclude $(main).d

%.o: %.cpp %.d
	@dtk-msgNotice -p Makefile compiling $<
	@$(CXX) -c $(CXXFLAGS) $(INC) -o $@ $<

% : %.o
	@dtk-msgNotice -p Makefile linking to create executable $@
	@$(CXX) $@.o $(LIBS) -o $@

# create c++ program from fluid program
%.cxx: %.fl
	@dtk-msgNotice -p Makefile running fluid on $< to create C++ file $@
	@fluid -c $<

# to get dependency file from compiled fluid file
$(fld): $(flcxx)

clean:
	rm -f $(main) *.d *.d.* *.o $(flcxx) $(flh)

distclean: clean
	-rm -f *~ core \#*
