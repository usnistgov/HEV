LIBRARY = libmodgle.a
INCLUDE = gle.h

OBJECTS :=  $(patsubst %.c,%.o,$(wildcard *.c))

CFLAGS =   -O2 -Wall -pedantic -Wstrict-prototypes
CC=gcc

%.o: %.cpp
	$(CC)  -c $(INC) -o $@ $<

${LIBRARY}: ${OBJECTS}
	ar cru  $@ $^ 
	ranlib $@

build: ${LIBRARY}

install: ${LIBRARY}

clean:
	rm -f $(OBJECTS) $(LIBRARY)

distclean: clean
	rm -f *~ *.d.*
