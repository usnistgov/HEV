/* 
 *       Scientific Applications & Visualization Group
 *            SAVG web page: http://math.nist.gov/mcsd/savg/
 *       Mathematical & Computational Science Division
 *            MCSD web page: http://math.nist.gov/mcsd/
 *       National Institute of Standards and Technology
 *            NIST web page: http://www.nist.gov/
 * 
 *  
 *  This software was developed at the National Institute of Standards
 *  and Technology by employees of the Federal Government in the course
 *  of their official duties. Pursuant to title 17 Section 105 of the
 *  United States Code this software is not subject to copyright
 *  protection and is in the public domain.  isolate is an experimental 
 *  system.  NIST assumes no responsibility whatsoever for its use by 
 *  other parties, and makes no guarantees, expressed or implied, about 
 *  its quality, reliability, or any other characteristic.
 *  
 *  We would appreciate acknowledgement if the software is used.
 * 
 */


/************************************************
 * savg-rgb2hsv					*
 * Created by: Adele Peskin			* 
 * Created on: 3-02-05   			*
 * Current Version: 1.00			*
 *						*
 * Description:					*
 *    savg-rgb2hsv converts color values        *
 * from rgb components to hsv components.	*
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "savgLineEvaluator.c"
#define pi 3.141592653589793238462643

/* Prints information on usage*/
void usage();

float MIN( float r, float g, float b) {
  float mmm = r;
  if (g < mmm) mmm = g;
  if (b < mmm) mmm = b;
  return mmm;
}

float MAX( float r, float g, float b) {
  float mmm = r;
  if (g > mmm) mmm = g;
  if (b > mmm) mmm = b;
  return mmm;
}
void HSVtoRGB( float *r, float *g, float *b, float h, float s, float v )
{
  int i;
  float f, p, q, t;

  if( s == 0 ) {
    // achromatic (grey)
    *r = *g = *b = v;
    return;
  }
  
  h /= 60;			// sector 0 to 5
  //The floor function means round down to the nearest integer.
  i = floor( h );
  
  f = h - i;			// factorial part of h
  p = v * ( 1 - s );
  q = v * ( 1 - s * f );
  t = v * ( 1 - s * ( 1 - f ) );
  
  switch( i ) {
  case 0:
    *r = v;
    *g = t;
    *b = p;
    break;
  case 1:
    *r = q;
    *g = v;
    *b = p;
    break;
  case 2:
    *r = p;
    *g = v;
    *b = t;
    break;
  case 3:
    *r = p;
    *g = q;
    *b = v;
    break;
  case 4:
    *r = t;
    *g = p;
    *b = v;
    break;
  default:		// case 5:
    *r = v;
    *g = p;
    *b = q;
    break;
  } 
}

int main(argc, argv)
int argc;
char *argv[];
{
/*
 * INITIALIZATION
 */
	float r = .9, g = .9, b = .9, a = 1; /* Initial color & alpha*/
	float h = 0.0, s = 0.0, v = 0.9;
	char line[STRING_LENGTH];	/* holds the line from the savg file to be processed */  
	int pargc = 0;
	char **pargv  = NULL;
	char assembleNewLine[STRING_LENGTH] = "";
	char tempStringr[20] = "";
	char tempStringg[20] = "";
	char tempStringb[20] = "";
	char tempStringa[20] = "";
	char defaultStringr[20] = "";
	char defaultStringg[20] = "";
	char defaultStringb[20] = "";
	int isTextured = 0;
	int textureDim = 0;
	int next = 0;
	char templine[STRING_LENGTH];
	char tempval[10];

/*
 * COMMAND LINE PROCESSING
 */
	// there is only 1 argument, no parameters
	
	if( 1 != argc ) {	
	  usage();
	  exit(EXIT_FAILURE);
	}

	// put default rgba into strings
	HSVtoRGB(&r, &g, &b, h, s, v );
	sprintf(defaultStringr, "%f", r);
	sprintf(defaultStringg, "%f", g);
	sprintf(defaultStringb, "%f", b);
	sprintf(tempStringa, "%f", a);
/*
 * BODY
 */   
	/*
         * getLine returns 1 if successful (ie not at
         * EOF) and 0 otherwise.  It is necessary to do
         * this test so that a "segmentation fault" is
	 * not encountered.
	 */
	if( !getLine(line) )
	  return 0;
	
        // jump to the nearest key word while not at EOF
	while(jumpToNearestKeyWord(line, pargv, pargc)) {
	  // see how many parameters to know if rgba info is on keyword line
	  dpf_charsToArgcArgv(line, &pargc, &pargv);
	  isTextured = 0;

	  if (pargc > 1) {
	    if (strcasecmp("2DTEXTURE",pargv[1])==0) {
	      isTextured = 1;
	      textureDim = 2;
	    }
	    if (strcasecmp("3DTEXTURE",pargv[1])==0) {
	      isTextured = 1;
	      textureDim = 3;
	    }
	  }

	  if (pargc > 2 && strcasecmp("TEXT",pargv[0])==0 &&
		   strcasecmp("string",pargv[1])==0) {
	    // update a text line if it has rgb info
	    updateTextString(templine, pargc, pargv, 5, r, g, b, tempval);
		
	    printf("%s\n", templine);
	  }
	  else if (pargc == 5 || pargc == 6)
	  {
	    // print the keyword
	    strcpy(assembleNewLine, pargv[0]);
	    strcat(assembleNewLine, " ");
	    if (pargc == 6) {
	      // print texture key word
	      strcat(assembleNewLine, pargv[1]);
	      strcat(assembleNewLine, " ");
	    }

	    // replace the rgb
	    h = atof(pargv[pargc-3]);
	    s = atof(pargv[pargc-2]);
	    v = atof(pargv[pargc-1]);
	    HSVtoRGB(&r, &g, &b, h, s, v );
	    sprintf(tempStringr, "%f", r);
	    sprintf(tempStringg, "%f", g);
	    sprintf(tempStringb, "%f", b);
	    strcat(assembleNewLine, tempStringr);
	    strcat(assembleNewLine, " ");
	    strcat(assembleNewLine, tempStringg);
	    strcat(assembleNewLine, " ");
	    strcat(assembleNewLine, tempStringb);
	    strcat(assembleNewLine, " ");

	    // print the old a value
	    if (pargc == 5)
	      strcat(assembleNewLine, pargv[4]);
	    else
	      strcat(assembleNewLine, pargv[5]);

	    // print the assembled line
	    printf("%s\n", assembleNewLine);
	    if (pargc == 5)
	      isTextured = 0;
          }
	  else
	    printf("%s\n", line);

	  if( !getLine(line) )
	    return 0;
	  
	  while(isCorrectDataLine(line,pargv,pargc) || isBlankLine(line)) {
	    if(!isBlankLine(line)) {

	      // see whats in the line and then update the alpha value
	      dpf_charsToArgcArgv(line, &pargc, &pargv);

	      next = 0;
	      // if not text data
	      if (strcasecmp(pargv[0],"text")!=0) {
		// for all cases, print x,y,z
		strcpy(assembleNewLine, pargv[0]);
		strcat(assembleNewLine, " ");
		strcat(assembleNewLine, pargv[1]);
		strcat(assembleNewLine, " ");
		strcat(assembleNewLine, pargv[2]);
		strcat(assembleNewLine, " ");
		next = 3;

		// if textured, print u,v,(w)
		if (isTextured) {
		  //printf("print uv\n");
		  strcat(assembleNewLine, pargv[3]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[4]);
		  strcat(assembleNewLine, " ");
		  next = 5;
		  if (textureDim == 3) {
		    strcat(assembleNewLine, pargv[5]);
		    strcat(assembleNewLine, " ");
		    next = 6;
		  }
		}

		// print the new rgb values
		if (pargc > 6) {
		  h = atof(pargv[3]);
		  s = atof(pargv[4]);
		  v = atof(pargv[5]);
		  HSVtoRGB(&r, &g, &b, h, s, v );
		  sprintf(tempStringr, "%f", r);
		  sprintf(tempStringg, "%f", g);
		  sprintf(tempStringb, "%f", b);
		  strcat(assembleNewLine, tempStringr);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, tempStringg);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, tempStringb);
		  strcat(assembleNewLine, " ");
		}
		else {
		  // use defaults
		  strcat(assembleNewLine, defaultStringr);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, defaultStringg);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, defaultStringb);
		  strcat(assembleNewLine, " ");
		}

		if (pargc == 3 || pargc == 5) {
		  // case 1: just xyz uv there: print alpha = 1;
		  strcat(assembleNewLine, tempStringa);
		  strcat(assembleNewLine, " ");
		}
		else if (pargc == 7 || pargc == 9) {
		  // case 2: xyz rgba: print old alpha value
		  next = next + 3;
		  strcat(assembleNewLine, pargv[next]);
		  strcat(assembleNewLine, " ");
		}
		else if (pargc == 6 || pargc == 8) { 
		  // case 3: xyz normals 
		  // print alpha = 1
		  strcat(assembleNewLine, "1");
		  strcat(assembleNewLine, " ");
		  // print normals
		  strcat(assembleNewLine, pargv[next]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[next+1]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[next+2]);
		  strcat(assembleNewLine, " ");
		}
		else if (pargc == 10 || pargc == 12) { 
		  // case 4: xyz rgba normals: print old alpha value
		  next = next + 3;
		  strcat(assembleNewLine, pargv[next]);
		  strcat(assembleNewLine, " ");
		  next = next + 1;
		  // print normals
		  strcat(assembleNewLine, pargv[next]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[next+1]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[next+2]);
		}
		printf("%s\n", assembleNewLine);

	      }
	    
	      else {
		// this is text: see if its a rgba line
		if (strcasecmp(pargv[1],"RGBA")==0) {
		  strcpy(assembleNewLine, pargv[0]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[1]);
		  strcat(assembleNewLine, " ");
		  h = atof(pargv[2]);
		  s = atof(pargv[3]);
		  v = atof(pargv[4]);
		  HSVtoRGB(&r, &g, &b, h, s, v );
		  sprintf(tempStringr, "%f", r);
		  sprintf(tempStringg, "%f", g);
		  sprintf(tempStringb, "%f", b);
		  strcat(assembleNewLine, tempStringr);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, tempStringg);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, tempStringb);
		  strcat(assembleNewLine, " ");

		  strcat(assembleNewLine, pargv[5]);
		  strcat(assembleNewLine, " ");
		  printf("%s\n", assembleNewLine);
		}
		else
		  printf("%s\n",line);
	      }
	    }
	   
	    if(!getLine(line))
	      return 0;
	  }
	}
	
	return 0;
}	


void usage()
/*
 * Prints information on usage
 */
{
	fprintf(stderr,
	"ERROR: Incorrect command line arguments\n"
	"\n"
	"USAGE:\n"
	"\tsavg-hsv2rgb/n");
}
