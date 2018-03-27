/*
 * 
 *  Glyph Toolbox
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


/*********************************************************
*	SAVG-rmcom
*	SAVG Toolbox comment remover
*	Author: Same program as Sean Kelly's gtb-rmcom
*	Date: 8/9/05
*********************************************************/
#define MAX 80

#include<stdio.h>


int main(){

	int ch;
	char s[MAX];

/* init variables */



/* scan for points/faces around comments */

	while(1){
		ch = fgetc(stdin);
		if (ch==EOF){
			break;
		}
	/* check to see if line is comment */
		if ((char)ch=='#'){
			fgets(s,MAX,stdin);
		}
		else
			fprintf(stdout,"%c",(char)ch);
	}
	return(0);
}
