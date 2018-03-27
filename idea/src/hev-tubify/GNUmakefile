#!gmake

OBJECTS = hev-tubify.o savgIOUtils.o gleToSavgOsg.o

hev-tubify:   $(OBJECTS)
	cc -o hev-tubify $(OBJECTS) \
	   -static -L $$HEV_EXTERNAL_GLE -lmodgle -lm

hev-tubify.o:  hev-tubify.c savgIOUtils.h gleToSavgOsg.h
	cc -c hev-tubify.c -I $$HEV_EXTERNAL_GLE

savgIOUtils.o: savgIOUtils.c savgIOUtils.h
	cc -c savgIOUtils.c


gleToSavgOsg.o: gleToSavgOsg.c gleToSavgOsg.h
	cc -c gleToSavgOsg.c \
              -I $$HEV_EXTERNAL_GLE

clean:
	rm -f $(OBJECTS) hev-tubify


distclean: clean
	rm -f *~ *.d.*

install: hev-tubify
	cp -f hev-tubify $(HEV_IDEA_DIR)/bin
	cp -f hev-tubify.1 $(HEV_IDEA_DIR)/man/man1
