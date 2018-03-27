/*
 * 
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
 * savg-split				        *
 * Created by: Adele Peskin     		*
 * Created on: 4-07-05  			*
 * Current Version: 1.00			*
 *						*
 * Description:	                                *
 *    savg-split splits the geometry in a large *
 * savg file into 2 files, leaving the keyword  *
 * lines in both.                               *
 * It outputs 2 files named by the parent file  *
 * file name plus "child1" or "child2".         *
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "savgLineEvaluator.c"

/* Prints information on usage*/
void usage();
void nameNextFile(char* snum, int num, char* rootname, char* newname);

int main(argc, argv)
int argc;
char *argv[];
{
/*
 * INITIALIZATION
 */
  char line[STRING_LENGTH];
  char child1name[STRING_LENGTH];
  char child2name[STRING_LENGTH];
  char intname[50];
  FILE* fp2;
  int pargc = 0;
  char **pargv  = NULL;
  int lines = 1000;
  int nextfile = 0;
  int nextline = 0;

  // define keyword lines
  char styleline[100] = "";
  char pixelline[100] = "";
  char shrinkline[100] = "";
  char stripline[100] = "";
  char octreeline[100] = "";
  char quadsline[100] = "";
  char checkline[100] = "";
  char transline[100] = "";
  char lightline[100] = "";
  char colorline[100] = "";
  char ambline[100] = "";
  char difline[100] = "";
  char specline[100] = "";
  char emissline[100] = "";
  char alphaline[100] = "";
  char shineline[100] = "";
/*
 * COMMAND LINE PROCESSING
 */
  if( argc != 2 && argc != 3) { 
    usage();
    exit(EXIT_FAILURE);
  }

  // the first parameter is the root file name
  strcpy(child1name, argv[1]);

  if (argc > 2) {
    // update the number of lines per file
    lines = atoi(argv[3]);
  }

  /*
   * BODY
   */ 
  // open up the first output file
  // find the next name
  nextfile = nextfile +1;
  nameNextFile(intname, nextfile, child1name, child2name);
  fp2 = fopen(child2name, "w");

   // read the incoming file
   while (getLine(line)) {

     // parse each line and reprint to either one or both new files
     dpf_charsToArgcArgv(line, &pargc, &pargv);
     nextline = nextline + 1;

     if (nextline > lines && pargc > 0) {
       // if it's comment, or keyword, check for new file needed
       if (pargv[0][0] == '#' || isKeyWordTest(pargv[0],pargv[1],pargc)) {
	 nextfile = nextfile + 1;
	 nameNextFile(intname, nextfile, child1name, child2name);
	 // open new file
	 fp2 = fopen(child2name, "w");
	 nextline = 0;

	 // write the current settings into the new file
	 if (strlen(styleline) > 2)
	   fprintf(fp2, "%s\n",styleline);
	 if (strlen(pixelline) > 2)
	   fprintf(fp2, "%s\n",pixelline);
	 if (strlen(shrinkline) > 2)
	   fprintf(fp2, "%s\n",shrinkline);
	 if (strlen(stripline) > 2)
	   fprintf(fp2, "%s\n",stripline);
	 if (strlen(octreeline) > 2)
	   fprintf(fp2, "%s\n",octreeline);
	 if (strlen(quadsline) > 2)
	   fprintf(fp2, "%s\n",quadsline);
	 if (strlen(checkline) > 2)
	   fprintf(fp2, "%s\n",checkline);
	 if (strlen(transline) > 2)
	   fprintf(fp2, "%s\n",transline);
	 if (strlen(lightline) > 2)
	   fprintf(fp2, "%s\n",lightline);
	 if (strlen(colorline) > 2)
	   fprintf(fp2, "%s\n",colorline);
	 if (strlen(ambline) > 2)
	   fprintf(fp2, "%s\n",ambline);
	 if (strlen(difline) > 2)
	   fprintf(fp2, "%s\n",difline);
	 if (strlen(specline) > 2)
	   fprintf(fp2, "%s\n",specline);
	 if (strlen(emissline) > 2)
	   fprintf(fp2, "%s\n",emissline);
	 if (strlen(alphaline) > 2)
	   fprintf(fp2, "%s\n",alphaline);
	 if (strlen(shineline) > 2)
	   fprintf(fp2, "%s\n",shineline);
       }
     }

     fprintf(fp2, "%s\n",line);

     // save any keyword information
     if (pargc > 0 && isKeyWordTest(pargv[0],pargv[1],pargc)) {
       if (strcasecmp(pargv[0],"style")==0)
	 strcpy(styleline, line);
       else if (strcasecmp(pargv[0],"pixelsize")==0)
	 strcpy(pixelline, line);
       else if (strcasecmp(pargv[0],"shrinkage")==0)
	 strcpy(shrinkline, line);
       else if (strcasecmp(pargv[0],"striplength")==0)
	 strcpy(stripline, line);
       else if (strcasecmp(pargv[0],"octree")==0)
	 strcpy(octreeline, line);
       else if (strcasecmp(pargv[0],"preserveplanarquads")==0)
	 strcpy(quadsline, line);
       else if (strcasecmp(pargv[0],"planarpolygonchecking")==0)
	 strcpy(checkline, line);
       else if (strcasecmp(pargv[0],"transparency")==0)
	 strcpy(transline, line);
       else if (strcasecmp(pargv[0],"lighting")==0)
	 strcpy(lightline, line);
       else if (strcasecmp(pargv[0],"colormode")==0)
	 strcpy(colorline, line);
       else if (strcasecmp(pargv[0],"ambient")==0)
	 strcpy(ambline, line);
       else if (strcasecmp(pargv[0],"diffuse")==0)
	 strcpy(difline, line);
       else if (strcasecmp(pargv[0],"specular")==0)
	 strcpy(specline, line);
       else if (strcasecmp(pargv[0],"emission")==0)
	 strcpy(emissline, line);
       else if (strcasecmp(pargv[0],"alpha")==0)
	 strcpy(alphaline, line);
       else if (strcasecmp(pargv[0],"shininess")==0)
	 strcpy(shineline, line);  
     }
   }

   fclose (fp2); 

   return 0;
   
}

/* Prints information on usage*/
void usage()
{
  printf("usage: savg-split filerootname [lines]\n");
  exit(1);
}

void nameNextFile(char* snum, int num, char* rootname, char* newname) {
  strcpy(newname, rootname);
  sprintf(snum, "%i", num);
  if (strlen(snum) == 1)
     strcat(newname,"000");
  else if (strlen(snum) == 2)
     strcat(newname,"00");
  else if (strlen(snum) == 3)
     strcat(newname,"0");
  strcat(newname, snum);
  strcat(newname, ".savg");
}

