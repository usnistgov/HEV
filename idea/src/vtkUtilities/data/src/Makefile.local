
all:    mkGrids

.f.o:
	gfortran -fdefault-real-8   -g  -c -o $@ $<

#CFLAGS =   -std=c99 -O3 -I../SEutil 
#CFLAGS =   -std=c99 -g -I../SEutil 
CFLAGS =   -g
LDFLAGS =   -g 






MK_VOL_OBJS = \
                mkGrids.o \
                RGAUSS.o \
                rgaussWrapper.o 



mkGrids: \
                $(MK_VOL_OBJS)
	cc $(LDFLAGS) -o mkGrids \
                $(MK_VOL_OBJS) \
                -lm -lgfortran
        
