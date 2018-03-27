/**************************************
 * SAVG-TABLE                         *
 *                                    *
 * Created on 7/29/05                 *
 *                                    *
 * Description:                       * 
 *                                    *
 * This program creates 5 different   *
 * types of tables, and one type of   *
 * cabinet.  The user must enter      *
 * either "large_plywood, plastic,    *
 * small_plywood, desk, or computer"  *
 * to get the tables, "cabinet" to    *
 * get a cabinet at the origin,       *
 * or "rave_rack".                    *
 *************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "feed.h"
#include "savgLineEvaluator.c"

void usage();

main(argc, argv)
int argc;
char *argv[];
{
	// declarations
	     int i=0,j=0;
	float x,z;
	char outString[10000];
	char tempString[1024];
	int type;

//take in arguments

	if(argc != 2){
		usage();
		exit(EXIT_FAILURE);
	}
	
	else{
	
	if(strcmp(argv[1], "large_plywood")==0)
		type=1;
	else if(strcmp(argv[1], "plastic")==0)
		type=2;
	else if(strcmp(argv[1], "small_plywood")==0)
                type=3;
	else if(strcmp(argv[1], "desk")==0)
	        type=4;
	else if(strcmp(argv[1], "computer")==0)
	        type=5;
	else if(strcmp(argv[1], "cabinet")==0)
	        type=6;
	else if(strcmp(argv[1], "rave_rack")==0)
	        type=7;
	else {
		usage();
		exit(EXIT_FAILURE);
	}
	}

if(type==1){

	strcpy(outString, "exec savg-cube | savg-rgb .3 .1 0 | savg-scale .5 .25 .008333 | savg-translate 0 0 .5; savg-cube | savg-rgb .3 .3 .3 | savg-scale .25 .03333 .016666 | savg-rotate 0 0 90 | savg-translate .42 0 .25; savg-cube | savg-rgb .3 .3 .3 | savg-scale .25 .033333 .0166666 | savg-rotate 0 0 90 | savg-translate -.42 0 .25; savg-cube | savg-rgb .3 .3 .3 | savg-scale .016666 .2 .0083333 | savg-translate .42 0 0; savg-cube | savg-rgb .3 .3 .3 | savg-scale .016666 .2 .0083333 | savg-translate -.42 0 0;");
}

if(type ==2){

	strcpy(outString, "exec savg-cube | savg-rgb .5 .5 .5 | savg-scale .484848 .303030 .0101009 | savg-translate 0 0 .541414; savg-cylinder | savg-rgb .5 .5 .5 | savg-scale .0151515 .0151515 .4969696 | savg-translate .4848484 -.060606 .0565656; savg-cylinder | savg-rgb .5 .5 .5 | savg-scale .0151515 .0151515 .4969696 | savg-translate -.484848 -.060606 .0565656; savg-cylinder | savg-rgb .5 .5 .5 | savg-scale   .0151515 .0151515 .4969696| savg-translate .484848 .2323231 .0565656; savg-cylinder | savg-rgb .5 .5 .5 | savg-scale  .0151515 .0151515 .4969696 | savg-translate -.484848 .2323231 .0565656; savg-cube | savg-rgb .5 .5 .5 | savg-scale .484848 .1616161 .0101009 | savg-translate 0 0.0969696 .1575757; savg-cylinder | savg-rgb .5 .5 .5 | savg-scale   .0151515 .0151515 .6060605 |savg-rotate 0 90 0 |  savg-translate .484848 .303030 .0464646; savg-cylinder | savg-rgb .5 .5 .5 | savg-scale .0151515 .0151515 .6060605 |savg-rotate 0 90 0 |  savg-translate -.484848 .303030 .0464646; savg-cylinder | savg-rgb .5 .5 .5 | savg-scale .0151515 .0151515 .96969696|savg-rotate 0 90 0 |  savg-translate .303030 .484848 .0464646 | savg-rotate 90 0 0;savg-cylinder | savg-rgb .05 .05 .05 | savg-scale .0227272 .0227272 .040404 |savg-rotate 0 90 90 | savg-rotate 65 0 0 | savg-translate .484848 -.2424242 .0121212; savg-cylinder | savg-rgb .05 .05 .05 | savg-scale .0227272 .0227272 .040404|savg-rotate 0 90 90 | savg-rotate 35 0 0 | savg-translate -.484848 -.2424242 .0121212; savg-cylinder | savg-rgb .05 .05 .05 | savg-scale .0227272 .0227272 .040404 |savg-rotate 0 90 90 | savg-rotate 90 0 0 | savg-translate .4363636 .303030 .0121212; savg-cylinder | savg-rgb .05 .05 .05 | savg-scale .0227272 .0227272 .040404| savg-rotate 0 90 90 | savg-rotate 90 0 0 | savg-translate -.4363636 .303030 0.0121212;");
}

if(type==3){
  
  strcpy(outString, "exec savg-cube | savg-rgb .2 .105 0 | savg-scale .5 .416666 .013888 | savg-translate 0 0 .78; savg-cube | savg-rgb .2 .1051 0 | savg-scale .0138888 .416666 .3819443 | savg-translate .4791666 0 .3819443;savg-cube | savg-rgb .2 .1051 0 | savg-scale .0138888 .416666 .3819443 | savg-translate -.4791666 0 .3819443; savg-cube | savg-scale .49 .0138888 .333333| savg-rgb .2 .105 0 | savg-translate 0 .4 .45;");
}

if(type==4){
  
  strcpy(outString, "exec savg-cube | savg-rgb .2 .105 0 | savg-scale .5 .3125 .0104166 | savg-translate 0 0 .585; savg-cube | savg-rgb .2 .1051 0 | savg-scale .0104166 .3125 .2864583 | savg-translate .484375 0 .2864583;savg-cube | savg-rgb .2 .1051 0 | savg-scale .0104166 .3125 .2864583 | savg-translate -.484375 0 .2864583; savg-cube | savg-scale .4925 .0104166 .25| savg-rgb .2 .105 0 | savg-translate 0 .30 .3375;  savg-cube | savg-rgb .2 .1051 0 | savg-scale .00775 .21875 .2864583 | savg-translate .1291666  -.095 .2864583;savg-cube | savg-rgb .2 .1051 0 | savg-scale .00775 .21875 .2864583 | savg-translate .4625  -.095 .2864583; savg-cube | savg-rgb .2 .087 0 | savg-scale .15375 .2175 .125| savg-translate .29625 -.095 0.125;  savg-cube | savg-rgb .2 .087 0 | savg-scale .15375 .2175 .0775| savg-translate .29625 -.095 .3325; savg-cube | savg-rgb .2 .087 0 | savg-scale .15375 .2175 .0775| savg-translate .29625 -.095 .49; savg-cylinder 10  open | savg-rgb .5 .5 .5 | savg-scale .05 .03125 .0052083 | savg-translate .29625 -.3 .125; savg-cylinder 10  open | savg-rgb .5 .5 .5 | savg-scale .05 .03125 .0052083 | savg-translate .29625 -.3 .3325; savg-cylinder 10  open | savg-rgb .5 .5 .5 | savg-scale .05 .03125 .0052083 | savg-translate .29625 -.3 .49;");
	}

if(type==5){
  
  strcpy(outString, "exec savg-cube | savg-rgb .2 .105 0 | savg-scale .5 .416666 .0138888 | savg-translate 0 0 .70666666; savg-cube | savg-rgb .1 .1 .1 | savg-scale .0277777 .0416666 .3333333| savg-translate .4722222 0 .36; savg-cube | savg-rgb .1 .1 .1 | savg-scale .027777777 .04166666 .33333333| savg-translate -.4722222 0 .36; savg-cube | savg-rgb .5 .5 .5 | savg-scale .02777777 .4166666 .0138888 | savg-translate .4722222 0 0.0138888; savg-cube | savg-rgb .5 .5 .5 | savg-scale .0277777 .4166666 .01388888| savg-translate -.4722222 0 0.01388888; savg-cube | savg-rgb .1 .1 .1 | savg-scale .45 .19444444 .0138888 | savg-translate 0 0.15 .25;  savg-cube | savg-scale .45 .01388888 .2166666| savg-rgb .1 .1 .1 | savg-translate 0 .33 .466666666;");
} 

if(type==6){
  
  strcpy(outString, "exec   savg-cube | savg-rgb .2 .051 0 | savg-scale .0135272 .3818182 .5 | savg-translate -.2916364 0 0;savg-cube | savg-rgb .2 .051 0 | savg-scale .0135272 .3818182 .5 | savg-translate .2901818 0 0; savg-cube | savg-rgb .2 .107 0 | savg-scale .2683636 .3796364 .2181818| savg-translate 0 0 -.281818;  savg-cube | savg-rgb .2 .107 0 | savg-scale .2683636 .3796363 .1352727| savg-translate 0 0 .0803634; savg-cube | savg-rgb .2 .107 0 | savg-scale .2683636 .3796363 .1352727| savg-translate 0 0 .3552725;  savg-cylinder 10  open | savg-rgb .5 .5 .5 | savg-scale .0872727 .0545454 .0090908 | savg-translate 0 -.3578182 -.281818; savg-cylinder 10  open | savg-rgb .5 .5 .5 | savg-scale .0872727 .0545454 .0090908 | savg-translate 0 -.3578182 .0803634; savg-cylinder 10  open | savg-rgb .5 .5 .5 | savg-scale .0872727 .0545454 .0090908 | savg-translate 0 -.3578182 .3552725;");
}

if(type==7){

  strcpy(outString, "exec savg-cube | savg-rgb .05 .05 .05 | savg-scale .1558441 .1785713 .5 | savg-translate 0 0 .5; savg-cube | savg-rgb 1 1 .8 | savg-scale .1246752 .1784414 .012987 | savg-translate 0 -.01558441 .2727271; savg-cube | savg-rgb 1 1 .8 | savg-scale .1246752 .1784414 .012987 | savg-translate 0 -.01554881 .3272726; savg-cube | savg-rgb 1 1 .8 | savg-scale .1246752 .1784414 .012987| savg-translate 0 -.01558441 .381818; savg-cube | savg-rgb .005 .005 .005 | savg-scale .116883 .1402596 .0051942 | savg-translate 0 -.0545454 .2727271; savg-cube | savg-rgb .005 .005 .005 | savg-scale .116883 .1402596 .0051942| savg-translate 0 -.0545454 .3272726; savg-cube | savg-rgb .005 .005 .005 | savg-scale .116883 .1402596 .0051942 | savg-translate 0 -.0545454 .381818;  savg-cube | savg-rgb 0 0 0 | savg-scale .1324674 .1784414 .4675323 | savg-translate 0 -.001558441 .4987011; savg-cylinder | savg-rgb .4 .4 .4 | savg-scale .0048701 .0048701 .0974025 |savg-rotate 0 90 0| savg-translate 0 -.0857142 .2025973; savg-cylinder | savg-rgb .4 .4 .4 | savg-scale .0048701 .0048701 .0974025 |savg-rotate 0 90 0| savg-translate 0 -.0857142 .1137661; ");
  for(i=0;i<10;i++){
    for(j=0;j<5;j++){
      strcat(outString, "savg-cylinder 4 top | savg-rgb 0 0 .4 | savg-scale .0031168 .0031168 .0974025 |savg-rotate 0 90 0| savg-translate ");
      x=(-.6 + (.13*i));
      z=(3.75 + (0.0835 * j));
      sprintf(tempString, "%f", (x*.1558441));
      strcat(outString, tempString);
      strcat(outString, " -.0935046 ");
      sprintf(tempString, "%f", (z*.1558441));
      strcat(outString, tempString);
      strcat(outString, ";");
}}
  for(i=0; i<10; i++){
    strcat(outString, "savg-cube | savg-rgb 1 1 .9 | savg-scale .0032727 .0046753 .0077922 | savg-translate ");
     x= (-.6 + (.13*i));
      sprintf(tempString, "%f", (x*.1558441));
      strcat(outString, tempString);
      strcat(outString, " -.1870129 .52987;");
  }
 
      strcat(outString, " savg-cube | savg-rgb 1 1 .7 | savg-scale .0623376 .077922 .0194805| savg-translate 0 -.1090908 .7792205; savg-cube | savg-rgb 1 1 .7 | savg-scale .0623376 .077922 .019480 | savg-translate 0 -.1090908 .8259737; savg-cube | savg-rgb .005 .005 .005 | savg-scale .0545454 .0701298 .01558441| savg-translate 0 -.1184415 .7792205; savg-cube | savg-rgb .005 .005 .005 | savg-scale .0545454 .0701298 .01558441 | savg-translate 0 -.1184415 .8259737; savg-cube | savg-rgb 1 1 .7 | savg-scale .0623376 .077922 .038961| savg-translate 0 -.1090908 .8961035;  savg-cube | savg-rgb .005 .005 .005 | savg-scale .0545454 .0701298 .0311688| savg-translate 0 -.1246752 .8961035 ");

}


	system(outString);


}


void usage()
{ 
  fprintf(stderr, "\n\tERROR - Incorrect command line.  Parameter must equal one of the following:\n\n\t[large_plywood] [plastic] [small_plywood] [desk] [computer] or [cabinet] [rave_rack]\n\n");
	
}









