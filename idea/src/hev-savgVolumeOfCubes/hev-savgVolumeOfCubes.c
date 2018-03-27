/*
 * hev-savgVolumeOfCubes - see usage message
 *
 * steve@nist.gov
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>


char *version="20140708";


void *usage()
{

char *message="\n\
\n\
Usage: hev-savgVolumeOfCubes [--scale xs yx zs | -sfile filename] [--color r g b | -cfile filename ]  ix jy kz\n\
\n\
       Output a savg file to stdout that represents a volume of\n\
       size ix by iy by iz made of\n\
       individual unit cubes.\n\
\n\
\n\
   Parameters:\n\
\n\
       --scale xs yx zs    specify a scale factor for the cubes,\n\
                           in the range 0 to 1.  Default is 1.\n\
\n\
       --sfile filename    specify a file to read x y z scale values.\n\
                           Each line has x y z scale values. There should be\n\
                           a line for every cube. This over rides --scale\n\
                           A scale of -1 -1 -1 indicates no cube will be output.\n\
\n\
       --color r g b       specify the cube color in the range 0 to 1.\n\
                           The default is 1 1 1.\n\
\n\
       --cfile filename    specify a file to read r g b color values.\n\
                           Each line has r g b values. There should be\n\
                           a line for every cube. This over rides --color\n\
\n\
       ix jy kz            specify the size of the volume.\n\
\n\
\n\
       Notes:\n\
\n\
                Loop order:\n\
                     Loop kz\n\
                        Loop jy\n\
                           Loop ix\n\
\n\
\n\
             While some command line options are optional,\n\
             they should be specified int the order shown.\n\
\n\
";


 fprintf(stderr,"%s\n",message);

 fprintf(stderr, "Version: %s\n\n", version);

 exit(1);
}


float r=1., g=1., b=1., xs=1., ys=1., zs=1.;
int i=0,j=0,k=0;


void cubeOut();
void polygonOut(float x, float y, float z, float xn, float yn, float zn);
float xtransform(float v);
float ytransform(float v);
float ztransform(float v);



void main(int argc, char **argv)
{
  int ix=-1,jy=-1,kz=-1;
  int iarg=1, debugFlag=0, colorFlag=0, scaleFlag=0;

  char *colorFile="", *scaleFile="";
  FILE *cf, *sf;
  int count;
  int cfLine=1, sfLine=1;

  /* Process  command line */

  if (argc <= 1) {
    usage();
  } else {

    if (argc-iarg < 1) {usage();}
    if (strncasecmp(argv[iarg], "--debug", 3) == 0) {
      debugFlag=1;
      iarg++;
    }


    if (argc-iarg < 1) {usage();}
    if (strncasecmp(argv[iarg], "--scale", 4) == 0) {
      iarg++;
      if (argc-iarg < 3) {
	usage();
      } else {
	xs=atof(argv[iarg++]);
	ys=atof(argv[iarg++]);
	zs=atof(argv[iarg++]);
      }
    }

    if (argc-iarg < 1) {usage();}
    if (strncasecmp(argv[iarg], "--sfile", 4) == 0) {
      iarg++;
      if (argc-iarg < 1) {
	usage();
      } else {
	scaleFile=argv[iarg++];
	scaleFlag=1;
	sf=fopen(scaleFile, "r");
	if (!sf) {
	  fprintf(stderr,"Can not open '%s'\n", scaleFile );
	  exit(1);
	}
      }
    }


    if (argc-iarg < 1) {usage();}
    if (strncasecmp(argv[iarg], "--color", 4) == 0) {
      iarg++;
      if (argc-iarg < 3) {
	usage();
      } else {
	r=atof(argv[iarg++]);
	g=atof(argv[iarg++]);
	b=atof(argv[iarg++]);
      }
    }


    if (argc-iarg < 1) {usage();}
    if (strncasecmp(argv[iarg], "--cfile", 4) == 0) {
      iarg++;
      if (argc-iarg < 1) {
	usage();
      } else {
	colorFile=argv[iarg++];
	colorFlag=1;
	cf=fopen(colorFile, "r");
	if (!cf) {
	  fprintf(stderr,"Can not open '%s'\n", colorFile );
	  exit(1);
	}
      }
    }


    if (argc-iarg != 3) {usage();}
    ix=atof(argv[iarg++]);
    jy=atof(argv[iarg++]);
    kz=atof(argv[iarg++]);



  }


  if (debugFlag) {
    fprintf(stderr, "scale: %g %g %g\n",xs,ys,zs);
    fprintf(stderr, "scaleFile: '%s'\n",scaleFile);
    fprintf(stderr, "color: %g %g %g\n",r,g,b);
    fprintf(stderr, "colorFile: '%s'\n",colorFile);
    fprintf(stderr, "size:  %d %d %d\n",ix,jy,kz);
  }




  for (k=0; k<kz; k++) {
    for (j=0; j<jy; j++) {
      for (i=0; i<ix; i++) {

	if (scaleFlag) {
	  count=fscanf(sf,"%f %f %f",&xs,&ys,&zs);
	  if (count == EOF) {
	    fprintf(stderr, "EOF or error reading '%s' near line %d\n",
		    scaleFile,sfLine);
	    exit(1);
	  }
	  if (count != 3) {
	    fprintf(stderr, "Not enough values read from '%s' near line %d\n",
		    scaleFile, sfLine);
	    exit(1);
	  }
	  sfLine++;
	}

	if (colorFlag) {
	  count=fscanf(cf,"%f %f %f",&r,&g,&b);
	  if (count == EOF) {
	    fprintf(stderr, "EOF or error reading '%s' near line %d\n",
		    colorFile, cfLine);
	    exit(1);
	  }
	  if (count != 3) {
	    fprintf(stderr, "not enough values read from '%s' near line %d\n",
		    colorFile, cfLine);
	    exit(1);
	  }
	}

	if (xs > -1) {
	  cubeOut();
	}

      }
    }
  }

}


void polygonOut(float x, float y, float z, float xn, float yn, float zn)
{
  fprintf(stdout, "%g %g %g   %g %g %g\n",
	  xtransform(x), ytransform(y), ztransform(z), xn, yn, zn);
}

float xtransform(float v)
{
  float t;
  // fprintf(stderr,"1-xs: %g\n",1-xs);
  t=i+xs*v+(1-xs)/2.+.25;
  return(t);
}

float ytransform(float v)
{
  float t;

  t=j+ys*v+(1-ys)/2.+.25;
  return(t);
}

float ztransform(float v)
{
  float t;

  t=k+zs*v+(1-zs)/2.+.25;
  return(t);
}


void cubeOut()
{

  fprintf(stdout, "polygon %g %g %g 1\n", r,g,b);
  polygonOut(-.5,  .5, -.5, 0, 0, -1);
  polygonOut( .5,  .5, -.5, 0, 0, -1);
  polygonOut( .5, -.5, -.5, 0, 0, -1);
  polygonOut(-.5, -.5, -.5, 0, 0, -1);

  fprintf(stdout, "polygon %g %g %g 1\n", r,g,b);
  polygonOut( .5,  .5, .5, 0, 0, 1);
  polygonOut(-.5,  .5, .5, 0, 0, 1);
  polygonOut(-.5, -.5, .5, 0, 0, 1);
  polygonOut( .5, -.5, .5, 0, 0, 1);

  fprintf(stdout, "polygon %g %g %g 1\n", r,g,b);
  polygonOut(-.5, .5,  .5, 0, 1, 0);
  polygonOut( .5, .5,  .5, 0, 1, 0);
  polygonOut( .5, .5, -.5, 0, 1, 0);
  polygonOut(-.5, .5, -.5, 0, 1, 0);

  fprintf(stdout, "polygon %g %g %g 1\n", r,g,b);
  polygonOut(-.5, -.5, -.5, 0, -1, 0);
  polygonOut( .5, -.5, -.5, 0, -1, 0);
  polygonOut( .5, -.5,  .5, 0, -1, 0);
  polygonOut(-.5, -.5,  .5, 0, -1, 0);

  fprintf(stdout, "polygon %g %g %g 1\n", r,g,b);
  polygonOut(.5,  .5, -.5, 1, 0, 0);
  polygonOut(.5,  .5,  .5, 1, 0, 0);
  polygonOut(.5, -.5,  .5, 1, 0, 0);
  polygonOut(.5, -.5, -.5, 1, 0, 0);

  fprintf(stdout, "polygon %g %g %g 1\n", r,g,b);
  polygonOut(-.5,  .5,  .5, -1, 0, 0);
  polygonOut(-.5,  .5, -.5, -1, 0, 0);
  polygonOut(-.5, -.5, -.5, -1, 0, 0);
  polygonOut(-.5, -.5,  .5, -1, 0, 0);

}

