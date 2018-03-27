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
 * savg-rotate                                  *
 *                                              *
 * Created by: Brandon M. Smith			*
 * Original "gtb" file created by: Sean Kelly	*
 * Created on: 06-18-04				*
 * Currant Version: 1.00			*
 *						*
 * Description: savg-rotate applies heading,    *
 *    pitch, and roll (HPR) rotations to a savg *
 *    file from stdin and writes the resultant  *
 *    figure to stdout.                         *
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "savgLineEvaluator.c"

#define PI (3.14159265358979)

/* Prints information on usage*/
void usage();

int main(argc, argv)
int argc;
char *argv[];
{

/*
 * INITILIZATION
 */
 	int i;
	int order = -1;
	double headingInDegrees = 0, 
               pitchInDegrees   = 0,
               rollInDegrees    = 0,
	       headingInRadians = 0,
               pitchInRadians   = 0,
               rollInRadians    = 0,
	       xvNew = 0,  /* vertices */
               yvNew = 0,
               zvNew = 0,  
	       xvOld = 0,   
	       yvOld = 0,   
	       zvOld = 0,
	       xnNew = 0,  /* normals */
	       ynNew = 0,
	       znNew = 0,
	       xnOld = 0,
	       ynOld = 0,
	       znOld = 0;

	char line[STRING_LENGTH];

	int changeHeading    = -1, 
            changePitch      = -1,
	    changeRoll       = -1;
	char assembleNewLine[STRING_LENGTH] = "";
	char tempString[20] = "";
	int pargc = 0;
	char **pargv  = NULL;
	int isTextured = 0;
	int textureDim = 0;
	int next = 0;
	char templine[STRING_LENGTH];
	char tempval[10];

/*
 * COMAND LINE PROSSESSING
 */
	// must have 3 parameters, h p r
	if (argc != 4) {
	  usage();
	  exit(EXIT_FAILURE);
	}

	// read in the hpr values
	headingInDegrees = atof(argv[1]);
	pitchInDegrees = atof(argv[2]);
	rollInDegrees = atof(argv[3]);
	if (fabs(headingInDegrees) > 0.000001) {
	  order = order + 1;
	  changeHeading = 2-order;
	}
	if (fabs(pitchInDegrees) > 0.000001) {
	  order = order + 1;
	  changePitch = 2-order;
	}
	if (fabs(rollInDegrees) > 0.000001) {
	  order = order + 1;
	  changeRoll = 2-order;
	}

	headingInRadians = headingInDegrees * PI/180;
	pitchInRadians   = pitchInDegrees * PI/180;
	rollInRadians    = rollInDegrees * PI/180;

/*
 * BODY
 */

	if(!getLine(line))
	  return 0;
	
        // jump to the nearest key word while not at EOF
	while(jumpToNearestKeyWord(line, pargv, pargc)) {

	  dpf_charsToArgcArgv(line, &pargc, &pargv);
	  isTextured = 0;
	  
	  if (pargc > 1) {
	    if (strcasecmp("2DTEXTURE",pargv[1])==0) {
	      //printf("got istextured\n");
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
	    updateTextString(templine, pargc, pargv, 2,
			     headingInDegrees, pitchInDegrees, 
			     rollInDegrees, tempval);
	    printf("%s\n", templine);
	  }
	  else
	    printf("%s\n", line);
	  if( !getLine(line) )
	    return 0;
	  
	  while(isCorrectDataLine(line,pargv,pargc) || isBlankLine(line)) {
	    if(!isBlankLine(line)) {
 
	      dpf_charsToArgcArgv(line, &pargc, &pargv);

	      if (strcasecmp(pargv[0],"text")==0) {
		printf("%s\n", line);
	      }
	      else {
		//printf("next line %s\n",line);
		// if not text data
		if (strcasecmp(pargv[0],"text")!=0) {
		  // in any case, update and then print xyz
		
		  xvOld = atof(pargv[0]); 
		  yvOld = atof(pargv[1]);
		  zvOld = atof(pargv[2]);	  
		
		  // get normals values if they are there
		  if( pargc == 6 || pargc == 10 ||
		      pargc == 8 || pargc == 12) {
		    xnOld = atof(pargv[pargc-3]); 
		    ynOld = atof(pargv[pargc-2]);
		    znOld = atof(pargv[pargc-1]);
		    //printf("xnold %f %f %f\n",xnOld, ynOld, znOld);
		  }
		  else {
		    xnOld = 0; 
		    ynOld = 0;
		    znOld = 0;
		  }	    
		
		  for(i = 0; i < 3; i++) {
		    //printf("print old: %f %f %f\n",xvOld,yvOld,zvOld);
		    if(changeHeading == i) {
		      xvNew = xvOld * cos( headingInRadians ) - 
			      yvOld * sin( headingInRadians );
		      yvNew = yvOld * cos( headingInRadians ) + 
			      xvOld * sin( headingInRadians );
		      zvNew = zvOld;
		      
		      /* Update old values for later rotations */
		      xvOld = xvNew;
		      yvOld = yvNew;
		      xnNew = xnOld * cos( headingInRadians ) - 
			ynOld * sin( headingInRadians );
		      ynNew = ynOld * cos( headingInRadians ) + 
			xnOld * sin( headingInRadians );
		      znNew = znOld;
		      
		      /* Update old values for later rotations */
		      xnOld = xnNew;
		      ynOld = ynNew;
		      //printf("print new head: %f %f %f\n",xvNew,yvNew,zvNew);
		    }
		    else if(changePitch == i) {
		      xvNew = xvOld;
		      yvNew = yvOld * cos( pitchInRadians ) - 
			zvOld * sin( pitchInRadians );
		      zvNew = zvOld * cos( pitchInRadians ) + 
			yvOld * sin( pitchInRadians );
		      
		      yvOld = yvNew;
		      zvOld = zvNew; 	  
		      xnNew = xnOld;
		      ynNew = ynOld * cos( pitchInRadians ) - 
			znOld * sin( pitchInRadians );
		      znNew = znOld * cos( pitchInRadians ) + 
			ynOld * sin( pitchInRadians );
		      
		      ynOld = ynNew;
		      znOld = znNew;
		      //printf("print new pitch: %f %f %f\n",xnNew,ynNew,znNew);
		    }
		    else if(changeRoll == i) {
		      xvNew = xvOld * cos( rollInRadians ) + 
			zvOld * sin( rollInRadians );
		      yvNew = yvOld;
		      zvNew = zvOld * cos( rollInRadians ) - 
			xvOld * sin( rollInRadians );
		      
		      xvOld = xvNew;   
		      zvOld = zvNew;   	      
		      xnNew = xnOld * cos( rollInRadians ) + 
			znOld * sin( rollInRadians );
		      ynNew = ynOld;
		      znNew = znOld * cos( rollInRadians ) - 
			xnOld * sin( rollInRadians );
		      
		      xnOld = xnNew;   
		      znOld = znNew; 
		      //printf("print new roll: %f %f %f\n",xvNew,yvNew,zvNew);
		    }
		    else { 	  
		      xvNew = xvOld; 	  
		      yvNew = yvOld; 	  
		      zvNew = zvOld;

		      xnNew = xnOld; 	  
		      ynNew = ynOld; 	  
		      znNew = znOld;
		    }  
		  }

		  // put all of the info back on the line
		  // first the new xyz values
		  sprintf(tempString, "%f", xvNew);
		  strcpy(assembleNewLine, tempString);
		  strcat(assembleNewLine, " ");
		  sprintf(tempString, "%f", yvNew);
		  strcat(assembleNewLine, tempString);
		  strcat(assembleNewLine, " ");
		  sprintf(tempString, "%f", zvNew);
		  strcat(assembleNewLine, tempString);
		  strcat(assembleNewLine, " ");
		  next = 3;
		  		
		  // print uv if appropriate
		  if (isTextured) {
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

		  // put back the rgb if present
		  if (pargc == 7 || pargc == 10 ||
		      pargc == 9 || pargc == 12 ) {
		    strcat(assembleNewLine, pargv[next]);
		    strcat(assembleNewLine, " ");
		    strcat(assembleNewLine, pargv[next+1]);
		    strcat(assembleNewLine, " ");
		    strcat(assembleNewLine, pargv[next+2]);
		    strcat(assembleNewLine, " ");
		    strcat(assembleNewLine, pargv[next+3]);
		    strcat(assembleNewLine, " ");
		    next = next + 4;
		  }

		  // put back normals if present
		  if (pargc == 6 || pargc == 10 ||
		      pargc == 8 || pargc == 12 ) {
		    sprintf(tempString, "%f", xnNew);
		    strcat(assembleNewLine, tempString);
		    strcat(assembleNewLine, " ");
		    sprintf(tempString, "%f", ynNew);
		    strcat(assembleNewLine, tempString);
		    strcat(assembleNewLine, " ");
		    sprintf(tempString, "%f", znNew);
		    strcat(assembleNewLine, tempString);
		    next = next + 3;
		  }

		  printf("%s\n", assembleNewLine);
		}
	      }
	    }
	    
	    if(!getLine(line))
	      return 0;
	  }
	}
	return(0);
}	

/* Prints information on usage*/
void usage()
{
	fprintf(stderr,
	"ERROR: Incorrect command line arguments\n"
	"Usage: savg-rotate hval pval rval\n");
}




