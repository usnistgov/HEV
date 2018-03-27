//
// hev-createLUT.C 
//
//  Input a few lines of an index and float r g b a values on stdin
//
//  Output 256 entry LUT that fills in the full 256 value on stdout
//
// For example, input:
//
//            # comments and blank lines permitted
//            000  0.0 0.0 1.0 1.0
//
//            255  1.0 1.0 0.0 1.1
//
// Output an LUT linear fromn solid blue to solid yellow
//
// Steve Satterfield
// steve@nist.gov
//
// Original implementation:
//          sfvis-createLUT.C  v0.2 November 6, 2006


//char *version = "v1.0 March 14, 2008";
//char *version = const_cast<char*>("v1.1 April 3, 2009");
char *version = const_cast<char*>("October 18, 2012");

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define BUFLEN 1024
char buffer[BUFLEN];

struct floatLUT {
  float r,g,b,a;
};

#define LUTLEN 256

int
main(int argc, char **argv)
{
  float r,r1,r2,rdelta, g,g1,g2,gdelta, b,b1,b2,bdelta, a,a1,a2,adelta;
  int flipflop;
  int i,i1,i2,idelta,iprevious,   j;
  int verboseFlag=0;

  if (argc > 1) {
    if ( strncmp(*(argv+1),"-v",2) == 0) {
      verboseFlag=1;
    } else {
      fprintf(stderr,"Usage: hev-createLUT [-h] | [-v] < file\n\n");
      fprintf(stderr,"Sample Input:\n\t000  0.0 0.0 1.0 1.0\n\t255  1.0 1.0 0.0 1.0\n");
      exit(1);
    }
  }


  flipflop=1;

  struct floatLUT lut[LUTLEN];


  iprevious=-1;
  while (fgets(buffer, BUFLEN, stdin) != NULL) {
    if ((buffer[0] != '#') && (strlen(buffer) > 1)) {
      if (flipflop) {
	if (5 != sscanf(buffer,"%d %g %g %g %g",&i1,&r1,&g1,&b1,&a1)) {
	  fprintf(stderr, "Invalid input, 5 values expected\n");
	  exit(1);
	}
	flipflop=0;
      } else {
	if (5 != sscanf(buffer,"%d %g %g %g %g",&i2,&r2,&g2,&b2,&a2)) {
	  fprintf(stderr, "Invalid input, 5 values expected\n");
	  exit(1);
	}
	if (verboseFlag) {
	  fprintf(stderr,"1: %d %g %g %g %g\n",i1,r1,g1,b1,a1);
	  fprintf(stderr,"2: %d %g %g %g %g\n",i2,r2,g2,b2,a2);
	}
	idelta=i2-i1;
	rdelta=(r2-r1)/idelta;
	gdelta=(g2-g1)/idelta;
	bdelta=(b2-b1)/idelta;
	adelta=(a2-a1)/idelta;
	r=r1;  g=g1;  b=b1; a=a1;
	for (j=i1; j<=i2; j++) {
	  if (j != iprevious) {
	    fprintf(stdout,"%3d %g %g %g %g\n",j,r,g,b,a);
	  }
	  r=r+rdelta;
	  g=g+gdelta;
	  b=b+bdelta;
	  a=a+adelta;
	}

	i1=i2;  r1=r2;  g1=g2;  b1=b2;  a1=a2;
	iprevious=i2;
      }
    }
  }
}
