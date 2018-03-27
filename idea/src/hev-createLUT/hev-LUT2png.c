/*
 * hev-LUT2png - create a 255x128 png image file to display the color table
 *
 * Steve Sattefield, steve@nist.gov
 * Original implementation: sfvis-lut2png.c September 5, 2006
 * Rename to hev-LUT2png March 14, 2008
 * Add -a and -h options: May 2, 2009
 * Add -s option and display when no output file provided: April 8, 2011
 *
 */

char *version = "20110408";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 1028
#define LUTSIZE 256
#define SSIZ 100

usage() {
    fprintf(stderr, "Usage: hev-LUT2png [-a] [-s float] lutFile [outImage]\n");
    fprintf(stderr, "\n  Version: %s\n",version);
    exit(1);

}


int f2int(float v, float s) {
  float a;
  
  a=v*s;
  if (a > 1.0) {
    return(255);
  } else {
    return ((int)(a*255));
  }
}





main(int argc, char **argv )
{
  FILE *lutFile, *rawFile;
  char tmpRawName[SSIZ];
  char *inFileName, outFileName[SSIZ];

  char buf[BUFSIZE];

  int i,j, argcount, alphaFlag=0, displayFlag=0, error;
  float tr,tg,tb,ta;
  float lutr[LUTSIZE],lutg[LUTSIZE],lutb[LUTSIZE],luta[LUTSIZE];
  unsigned char r,g,b,a;
  float scale=1.0;

  argcount=argc-1;

  /* Process command line arguements */

  i=1;
  
  while (argcount > 0) {

    /* fprintf(stderr,"argcount=%d argv[%d]=%s\n",argcount,i,argv[i]); */

    /* is it an option or fie name */
    if (strncmp("-",argv[i],1)==0) {
      if (strncmp("-a",argv[i],2)==0) {
	alphaFlag=1;
	argcount--;
      } else if (strncmp("-s",argv[i],2)==0) {
	argcount--;
	if (argcount < 1) usage();
	i++;
	if ((error=sscanf(argv[i],"%g",&scale))!=1) {
	  fprintf(stderr,"scale value incorrect, return code %d\n",error);
	  usage();
	}
	argcount--;
      } else {
	usage();
      }
    } else {
      break;
    }

    i++;
  }

  /* assume remaining arguments are file names */

  if (argcount == 2) {
    inFileName=argv[i];
    snprintf(outFileName,SSIZ,"%s",argv[i+1]);
  } else if (argcount == 1) {
    inFileName=argv[i];
    snprintf(outFileName,SSIZ,"/tmp/LUT2png-%s-%d.png",getenv("USER"),getpid());
    displayFlag=1;
  } else {
    usage();
  }

  snprintf(tmpRawName,SSIZ,"/tmp/LUT2png-%s-%d.raw",getenv("USER"),getpid());

  /*
  fprintf(stderr,"alphaFlag=%d inFileName=%s\n outFileName=%s tmpRawName=%s scale=%g\n",
	  alphaFlag, inFileName, outFileName, tmpRawName,scale);
  */

  if ((lutFile=fopen(inFileName,"r")) == NULL) {
    fprintf(stderr,"file %s not found\n",inFileName);
    exit (1);
  }

  if (! (rawFile=fopen(tmpRawName,"w"))) {
    fprintf(stderr,"Error: %s Can\'t open file %s\n",tmpRawName);
    exit(1);
  }

  /* Read the look up table */
  for (i=0;i<LUTSIZE; i++) {
	fscanf(lutFile,"%d %g %g %g %g",&j,&tr,&tg,&tb,&ta);
	/* fprintf(stderr,"%d %g %g %g %g\n",j,tr,tg,tb,ta); */
	lutr[j] = tr;
	lutg[j] = tg;
	lutb[j] = tb;
        luta[j] = ta;
  }


  for (i=0; i<64; i++) {
    for (j=0; j<256; j++) {
      r=f2int(lutr[j],scale);
      g=f2int(lutg[j],scale);
      b=f2int(lutb[j],scale);
      if (alphaFlag == 0) {
	a=f2int(luta[j],scale);
      } else {
	a=255;
      }
      fputc(r,rawFile); fputc(g,rawFile); fputc(b,rawFile); fputc(a,rawFile);
    }
  }

  fflush(rawFile);
  close(rawFile);

  snprintf(buf,BUFSIZE,"convert -depth 8 -size 256x64 rgba:%s %s",tmpRawName,outFileName);
  system(buf);
  unlink(tmpRawName);

  if (displayFlag == 1) {
    snprintf(buf,BUFSIZE,"display %s &",outFileName);
    system(buf);
    sleep(1);
    unlink(outFileName);
  }


}
