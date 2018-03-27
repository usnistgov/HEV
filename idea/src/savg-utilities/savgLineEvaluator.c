#include <math.h>

#define STRING_LENGTH (250) 

char *gets(char *s)
{
    fgets(s, STRING_LENGTH, stdin) ;
    int i ;
    for (i=0; i<STRING_LENGTH; i++)
    {
	if (s[i] == '\n')
	{
	    s[i] = '\0' ;
	    break ;
	}
	if (s[i] == '\0') break ;
    }
}

#include "parse.c"
#include "feed.h"

int atEOF()
/*
 * Returns 1 if at end of file (EOF) and 0 otherwise
 */ 
{
  int c;
  if( (c = getchar()) != EOF)
  {
    /*
     * The if statement takes the first character: put it back 
     * before reading in the line 
     */ 
    ungetc(c, stdin);   
    return 0; // not at EOF
  }
  else
  {
    ungetc(c, stdin);
    return 1; // at EOF
  }
} 

int jumpToNearestKeyWord(char *pline, char **pargv, int pargc)
/*
 * Assumes that getLine function was called beforehand and
 * pline contains something to examine.  If the current line
 * is a key word line (polygon, point, etc.) then does
 * nothing, esle print the lines to stdout until the first 
 * key word is encountered and stop.
 */
{
  while( !isKeyWord(pline, pargv, pargc) )
  {
    printf("%s\n", pline);
    
    if( !getLine(pline) )
      return 0;
  }   
  return 1;
}


int jumpToNearestDataLine(char *pline, char **pargv, int pargc)
/*
 * Assumes that getLine function was called beforehand and
 * pline contains something to examine.  If the current line is
 * a data line (containing a vertex, RGBA, normals, etc) then does
 * nothing, else print the lines to stdout until the first data line
 * is encountered and stop.  
 */ 
{
  while( isKeyWord(pline, pargv, pargc) ||
         containsComment(pline) ||
	 isBlankLine(pline) )
  {
    printf("%s\n", pline);

    if( !getLine(pline) )
      return 0; 
  }
  return 1;
}

int jumpToNearestLineGeometry(char *pline, char **pargv, int pargc)
/*
 * Assumes that getLine function was called beforehand and
 * pline contains something to examine.  If the current line
 * is a key word line (polygon, point, etc.) then does
 * nothing, esle print the lines to stdout until the first 
 * key word is encountered and stop.
 */
{
  while( !isKeyWordLine(pline, pargv, pargc) )
  {
    printf("%s\n", pline);
    
    if( !getLine(pline) )
      return 0;
  }   
  return 1;
}

int jumpToNearestPolygon(char *pline, char **pargv, int pargc)
/*
 * Assumes that getLine function was called beforehand and
 * pline contains something to examine.  If the current line
 * is the key word "polygon" or "polygons", then does
 * nothing, else print the lines to stdout until the first 
 * key word is encountered and stop.
 */
{
  while( !isKeyWordPoly(pline, pargv, pargc) )
  {
    printf("%s\n", pline);
    
    if( !getLine(pline) )
      return 0;
  }   
  return 1;
}

int isKeyWord(char *pline, char **pargv, int pargc)
/* 
 * Examines a line to see if the first argument is a key word.  If it is a key
 * word then return 1 (for true).  If not, return 0 (for false).
 */
{
  dpf_charsToArgcArgv(pline, &pargc, &pargv);

  /* If line is blank, obviously not a key word. */
  if (pargc == 0) 
    return 0;
  
  if (isKeyWordTest(pargv[0],pargv[1],pargc)) {
    return 1;  // found a key word
  }
  
  return 0;
}

int isKeyWordLine(char *pline, char **pargv, int pargc)
     // Examines a line to see if the first argument is a 
     // line key word.key word.
{
  dpf_charsToArgcArgv(pline, &pargc, &pargv);

  /* If line is blank, obviously not a key word. */
  if (pargc == 0) 
    return 0;

  if(strcasecmp(pargv[0], "line") == 0 || 
     strcasecmp(pargv[0], "lines") == 0 ) {
    return 1;  // found a line key word
  }
  
  return 0;
}


int isKeyWordPoly(char *pline, char **pargv, int pargc)
     // Examines a line to see if the first argument is a 
     // line key word.key word.
{
  dpf_charsToArgcArgv(pline, &pargc, &pargv);

  /* If line is blank, obviously not a key word. */
  if (pargc == 0) 
    return 0;

  int len = strlen (pargv[0]);
  if (len < 3)
	{
	return 0;
	}

#if 1
  if (strncasecmp (pargv[0], "polygons", len) == 0)
	{
	return 1;
	}

#else
  if(strcasecmp(pargv[0], "polygon") == 0 || 
     strcasecmp(pargv[0], "polygon") == 0 || 
     strcasecmp(pargv[0], "polygons") == 0 ) {
    return 1;  // found a line key word
  }
#endif
  
  return 0;
}

int isCorrectDataLine(char *pline, char **pargv, int pargc)
/*
 * Examines line to see if it is a data line of the proper format
 * If it is in proper format, then returns 1 -- otherwise 0.
 */
{
  /* 
   * test to see if line has a key word, a comment, is blank, or contains
   * incorrect number of arguments
   */
  dpf_charsToArgcArgv(pline, &pargc, &pargv);
  if (pargc == 0)
    return 0;

  if( !isKeyWordTest(pargv[0],pargv[1],pargc) &&
      !containsComment(pline) &&
      !isBlankLine(pline) &&
      (pargc == 3 || pargc == 6 || pargc == 7 || pargc == 10 ||
       pargc == 5 || pargc == 8 || pargc == 9 || pargc == 12) )
  {
    return 1;
  }
  return 0;
}


int isBlankLine(char *pline)
/* 
 * Examines line to see if it is blank.  If it is
 * return 1; if not, return 0.
 */
{
  if(pline == NULL || strlen(pline) == 0)
    return 1;
  return 0;
}


int containsComment(char *pline)
/*
 * Examines a line to see if it contains a comment.  If it does,
 * return 1 (for true).  If not, return 0 (for false).
 */
{
  int i;

  for(i = 0; i < strlen(pline); i++)
  {
    if(pline[i] == '#')
      return 1;    //comment found!
  }
  return 0;        //no comment found!
}

int getLine(char * pline)
/*  
 * Reads a line from stdin and prints it to pline. 
 * First does a test to see that not at end of file.  
 * If at end of file returns 0 indicating
 * false.  If not EOF The line is placed into "string" and 1
 * is returned indicating true.  This function eats the line.
 */
{
  if( !atEOF() )
  {
    gets(pline);
    return 1; // successful, not at EOF
  }
  else
  {
    return 0; //not successful
  }
}


int isKeyWordTest(char *pgv, char *pgv2, int pgc)
/* 
 * Examines a line to see if the first argument is a key word.  If it is a key
 * word then return 1 (for true).  If not, return 0 (for false).
 */
{
  if(strcasecmp(pgv, "line")        == 0 || 
     strcasecmp(pgv, "lines")       == 0 ||
     strcasecmp(pgv, "tristrip")    == 0 ||
     strcasecmp(pgv, "tristrips")  == 0 ||
     strcasecmp(pgv, "pol")     == 0 ||
     strcasecmp(pgv, "poly")     == 0 ||
     strcasecmp(pgv, "polyg")     == 0 ||
     strcasecmp(pgv, "polygo")     == 0 ||
     strcasecmp(pgv, "polygon")     == 0 ||
     strcasecmp(pgv, "polygons")     == 0 ||
     strcasecmp(pgv, "point")       == 0 ||
     strcasecmp(pgv, "points")      == 0 ||
     strcasecmp(pgv, "style")       == 0 ||
     strcasecmp(pgv, "pixelsize")   == 0 ||
     strcasecmp(pgv, "shrinkage")   == 0 ||
     strcasecmp(pgv, "striplength") == 0 ||
     strcasecmp(pgv, "octree")      == 0 ||
     strcasecmp(pgv, "preserveplanarquads") == 0 ||
     strcasecmp(pgv, "planarpolygonchecking") == 0 ||
     strcasecmp(pgv, "transparency")      == 0 ||
     strcasecmp(pgv, "lighting")      == 0 ||
     strcasecmp(pgv, "colormode")      == 0 ||
     strcasecmp(pgv, "ambient")      == 0 ||
     strcasecmp(pgv, "diffuse")      == 0 ||
     strcasecmp(pgv, "specular")      == 0 ||
     strcasecmp(pgv, "emission")      == 0 ||
     strcasecmp(pgv, "alpha")      == 0 ||
     strcasecmp(pgv, "shininess") == 0 ||
     (strcasecmp(pgv, "text") == 0 && 
                 pgv2 != NULL && strcasecmp(pgv2, "string") == 0))
  {
    return 1;  // found a key word
  }
return 0;
}


int keyWordIsAShape(char *pgv) {
  // Examines a keyword to see if the first argument is a shape.  
  // If it is a shape then return 1 (for true).  
  // If not, return 0 (for false).
  if(pgv == NULL)
    return 0;

  if(strcasecmp(pgv, "line")      == 0 ||
     strcasecmp(pgv, "lines")     == 0 ||
     strcasecmp(pgv, "tristrip")  == 0 ||
     strcasecmp(pgv, "tristrips")  == 0 ||
     strcasecmp(pgv, "pol")   == 0 ||
     strcasecmp(pgv, "poly")   == 0 ||
     strcasecmp(pgv, "polyg")   == 0 ||
     strcasecmp(pgv, "polygo")   == 0 ||
     strcasecmp(pgv, "polygon")   == 0 ||
     strcasecmp(pgv, "polygons")   == 0 ||
     strcasecmp(pgv, "point")   == 0 ||
     strcasecmp(pgv, "points")   == 0 )   
  {
    return 1;
  }
return 0;
}

void updateTextString(char* pline, int pgc, char **pgv, int which, 
                      float v1, float v2, float v3, char* newval) {
  // replace pline with an updated pline
  int cnt = 0;
  int found = -1;
  char tempString[20] = ""; 
  int ii;

  // first copy the text string foo;
  strcpy(pline, pgv[0]);
  strcat(pline, " ");
  strcat(pline, pgv[1]);
  strcat(pline, " \"");
  strcat(pline, pgv[2]);
  strcat(pline, "\" ");
  cnt = 3;

  while (cnt < pgc) {
    if (strcasecmp(pgv[cnt],"XYZ")==0) {
      //printf("at xyz: which: %d\n",which);
      
      // just print
      for (ii = cnt; ii < cnt+4; ++ii) {
	strcat(pline, pgv[ii]);
	strcat(pline, " ");
      }
      cnt = cnt + 4;
    }
    else if (strcasecmp(pgv[cnt],"HPR")==0) {
      // just print
      for (ii = cnt; ii < cnt+4; ++ii) {
	strcat(pline, pgv[ii]);
	strcat(pline, " ");
      }
      cnt = cnt + 4;
    }
    else if (strcasecmp(pgv[cnt],"S")==0) {
      // just print
      for (ii = cnt; ii < cnt+2; ++ii) {
	strcat(pline, pgv[ii]);
	strcat(pline, " ");
      }
      cnt = cnt + 2;
    }
    else if (strcasecmp(pgv[cnt],"SXYZ")==0) {
      // just print
      for (ii = cnt; ii < cnt+4; ++ii) {
	strcat(pline, pgv[ii]);
	strcat(pline, " ");
      }
      cnt = cnt + 4;
    }
    else if (strcasecmp(pgv[cnt],"RGBA")==0) {
      if (which == 5) {
	strcat(pline, pgv[cnt]);
	strcat(pline, " ");
	cnt = cnt + 1;
	found = 1;

	// print out the updated rgb values and the old a
	sprintf(tempString, "%f", v1);
	strcat(pline, tempString);
	strcat(pline, " ");
	sprintf(tempString, "%f", v2);
	strcat(pline, tempString);
	strcat(pline, " ");
	sprintf(tempString, "%f", v3);
	strcat(pline, tempString);
	strcat(pline, " ");
	strcat(pline, pgv[cnt+3]);
	strcat(pline, " ");
	cnt = cnt + 4;
	
	// print the rest
	for (ii = cnt; ii < pgc; ++ii) {
	  strcat(pline, pgv[ii]);
	  strcat(pline, " ");
	  cnt = cnt + 1;
	}
      }
      else if (which == 7) {
	// update alpha only
	strcat(pline, pgv[cnt]);
	strcat(pline, " ");
	cnt = cnt + 1;
	found = 1;

	// print out the updated a values and the old rgb
	strcat(pline, pgv[cnt]);
	strcat(pline, " ");
	strcat(pline, pgv[cnt+1]);
	strcat(pline, " ");
	strcat(pline, pgv[cnt+2]);
	strcat(pline, " ");;
	sprintf(tempString, "%f", v1);
	strcat(pline, tempString);
	strcat(pline, " ");
	cnt = cnt + 4;
	
	// print the rest
	for (ii = cnt; ii < pgc; ++ii) {
	  strcat(pline, pgv[ii]);
	  strcat(pline, " ");
	  cnt = cnt + 1;
	}
      }
      else {
	// just print
	for (ii = cnt; ii < cnt+5; ++ii) {
	  strcat(pline, pgv[ii]);
	  strcat(pline, " ");
	}
	cnt = cnt + 5;
      }
    }
    else if (strcasecmp(pgv[cnt],"FONT")==0) {
      //printf("at font\n");
      if (which == 6) {
	strcat(pline, pgv[cnt]);
	strcat(pline, " ");
	strcat(pline, newval);
	strcat(pline, " ");
	cnt = cnt + 2;
	found = 1;
	
	// print the rest
	for (ii = cnt; ii < pgc; ++ii) {
	  strcat(pline, pgv[ii]);
	  strcat(pline, " ");
	  cnt = cnt + 1;
	}
      }
      else {
	// just print
	for (ii = cnt; ii < cnt+2; ++ii) {
	  strcat(pline, pgv[ii]);
	  strcat(pline, " ");
	}
	cnt = cnt + 2;
      }
    }
  }
  
  // if not there, add it
  if (found < 0) {
    if (which == 1)
      strcat(pline, "XYZ ");
    if (which == 2)
      strcat(pline, "HPR ");
    if (which == 3)
      strcat(pline, "S ");
    if (which == 4)
      strcat(pline, "SXYZ ");
    if (which == 5 || which==7)
      strcat(pline, "RGBA ");

    if (which == 6) {
      strcat(pline, "FONT ");
      strcat(pline, newval);
      strcat(pline, " ");
    }

    else {      
      if (which == 7) {
	// add the alpha value to an savg-alpha call
	strcat(pline, "1 1 1 ");
	sprintf(tempString, "%f", v1);
	strcat(pline, tempString);
	strcat(pline, " ");    
      }

      else {
	// print first value in
	sprintf(tempString, "%f", v1);
	strcat(pline, tempString);

	if (which != 3) {
	  strcat(pline, " ");    
	  sprintf(tempString, "%f", v2);
	  strcat(pline, tempString);
	  strcat(pline, " ");    
	  sprintf(tempString, "%f", v3);
	  strcat(pline, tempString);
	  strcat(pline, " ");    
	}
	if (which == 5) {
	  // add the alpha value to an savg-rgb call
	  strcat(pline, " 1 ");
	}
      }
    }
  }
}

void calculateCrossProduct(double *uVector, double *vVector, double *normalVector, double vectorMagnitude)
{
  /* Calculate the cross product */
  normalVector[0] = (uVector[1]) * (vVector[2]) - (uVector[2]) * (vVector[1]);
  normalVector[1] = (uVector[2]) * (vVector[0]) - (uVector[0]) * (vVector[2]);
  normalVector[2] = (uVector[0]) * (vVector[1]) - (uVector[1]) * (vVector[0]);

  vectorMagnitude = sqrt( normalVector[0]*normalVector[0] + 
			  normalVector[1]*normalVector[1] + 
			  normalVector[2]*normalVector[2]);

  if(fabs(vectorMagnitude) > 0.0)
  {
    normalVector[0] = normalVector[0] / vectorMagnitude;
    normalVector[1] = normalVector[1] / vectorMagnitude;
    normalVector[2] = normalVector[2] / vectorMagnitude;
  }
}
