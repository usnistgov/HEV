/************************************************************** 
 * SAVG-Object                                                *
 *                                                            *
 * Created on 8/2/05                                          *
 *                                                            *
 * This program creates either a display screen with          *
 * input width and height, a phone, cardboard box,            *
 * a flashlight which can take 0 or 3 parameters, r,g,b,      *
 * default is yellow, or the generic NIST overhead light.     *                
 *************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "feed.h"
#include "savgLineEvaluator.c"

void usage();
void usage2();
void usage3();
void usage4();
void usage5();

main(argc, argv)
int argc;
char *argv[];
{
        // declarations
             int i=0,j=0;
        float x,y,z;
  	float x2=1.5,y2=1,z2=.6;	
	float swidth, sheight, sheight2, swidth2;
	float r=1, g=1,b=0;
        char outString[10000];
        char tempString[1024];
        int type;
	int small=0;

	if(argc < 2){
	  usage();
	  exit(EXIT_FAILURE);
	}

	if(strcmp(argv[1], "phone")==0)
	  type=1;
	
	else if(strcmp(argv[1], "screen")==0){
	  if(argc != 4){
	    usage2();
	    exit(EXIT_FAILURE);
	  }
	  else{
	    type=2;
	    swidth=atof(argv[2]);
	    sheight=atof(argv[3]);
	    swidth2=.95*swidth;
	    sheight2=.95*sheight;
	  }
	}
	
	else if(strcmp(argv[1], "box")==0){
		type=3;	
		if(argc > 2)
			x2=atof(argv[2]);
		if(argc >3){
			x2=atof(argv[2]);
			y2=atof(argv[3]);
		}
		if(argc > 4){
			x2=atof(argv[2]);
                        y2=atof(argv[3]);
			z2=atof(argv[4]);
		}
		if(argc >5){
			usage3();
			exit(EXIT_FAILURE);
		}
	}

	

	else if(strcmp(argv[1], "flashlight")==0)

	{

	type=4;

	if(argc>2){
	  if( argc ==5){
	    r=atof(argv[2]);
	    g=atof(argv[3]);
	    b=atof(argv[4]);
	  }
	  else usage4();
	}
	}
	
	else if(strcmp(argv[1], "light")==0){
	  type=5;
		if(argc >2)
			if(strcmp(argv[2], "short")==0)
				small=1;
		else{
			usage5();
			exit(EXIT_FAILURE);
		}	
	}

	else {
	  usage();
	  exit(EXIT_FAILURE);
	}

	if(type==1){

	  strcpy(outString, "exec savg-cube | savg-rgb 1 1 .7 | savg-scale .5 .7142856 .1071428 | savg-rotate 0 15 0 | savg-translate 0 0 0.28836; savg-cube | savg-rgb 1 1 .6 | savg-scale .142587 .1785713 .142587 | savg-rotate 0 30 0 | savg-translate -.3342857 -.5142857 .2914285; savg-cube | savg-rgb 1 1 .6 | savg-scale .142587 .1785713 .142587 | savg-rotate 0 0 0 | savg-translate -.3342857 .4285714 .5485714; savg-cube | savg-rgb 1 1 .6 | savg-scale .1071428 .3685714 .0714284 | savg-rotate 0 15 0 |savg-translate -.3342857 -.0668571 .528 ;");

	  for(i =0; i<3;i++){
	    for(j=0; j<4;j++){
	      x=(.083333+(i*.06));
	      y=(-.2708333+(j*.06));
	      z=(.13+(.016*j));
	      strcat(outString, " savg-cube | savg-rgb 0 0 0 | savg-scale .0342857 .0342857 .0857142 | savg-rotate 0 15 0 | savg-translate ");
	      sprintf(tempString, "%f", (x*1.7142858));
	      strcat(outString, tempString);
	      strcat(outString, " ");
	      sprintf(tempString, "%f", (y*1.7142858));
	      strcat(outString, tempString);
	      strcat(outString, " ");
	      sprintf(tempString, "%f", (z*1.7142858));
	      strcat(outString, tempString);
	      strcat(outString, ";");
	    }}
	  
	   for(i =0; i<4;i++){
	    for(j=0; j<4;j++){
	      strcat(outString, "savg-cube | savg-rgb .5 .5 .5 | savg-scale .0514285 .0285713 .017142858 | savg-rotate 0 15 0 | savg-translate ");
	      x=(-.0+(i*.08));
	      y=(.1+(j*.06));
	      z=(.27+(.015*j));
	      sprintf(tempString, "%f", (x*1.7142858));
	      strcat(outString, tempString);
	      strcat(outString, " ");
	      sprintf(tempString, "%f", (y*1.7142858));
	      strcat(outString, tempString);
	      strcat(outString, " ");
	      sprintf(tempString, "%f", (z*1.7142858));
	      strcat(outString, tempString);
	      strcat(outString, ";");
	    }}
	}

	if(type==2){
	  
	  printf("# values %f %f %f %f", swidth, sheight, swidth2, sheight2);

	  strcpy(outString, "exec savg-cube | savg-rgb .005 .005 .005 | savg-scale ");
	  sprintf(tempString, "%f", (swidth/2));
	  strcat(outString, tempString);
	  strcat(outString, " .01 ");
	  sprintf(tempString, "%f", (sheight/2));
	  strcat(outString, tempString);
	  strcat(outString, ";");

	  strcat(outString, " savg-cube | savg-rgb 0 0 0 | savg-scale ");
	  sprintf(tempString, "%f", (swidth/2));
	  strcat(outString, tempString);
	  strcat(outString, " ");
	  sprintf(tempString, "%f", (swidth/40));
	  strcat(outString, tempString);
	  strcat(outString, " ");
	  sprintf(tempString, "%f", (swidth/40));
	  strcat(outString, tempString);
	  strcat(outString, " | savg-translate 0 0 ");
	  sprintf(tempString, "%f", (sheight/2));
	  strcat(outString, tempString);
	  strcat(outString, ";");

	  strcat(outString, " savg-cube | savg-rgb 1 .9 .9 | savg-scale ");
	  sprintf(tempString, "%f", (swidth2/2));
	  strcat(outString, tempString);
	  strcat(outString, " .01 ");
	  sprintf(tempString, "%f", (sheight2/2));
	  strcat(outString, tempString);
	  strcat(outString, " | savg-translate 0 -.01 0;");
	  
	}

	if(type==3){

		strcpy(outString, "exec savg-cube | savg-rgb .5608 .2902 .09804 | savg-scale ");
		sprintf(tempString, "%f", (x2*.333333333));
		strcat(outString, tempString);
		strcat(outString, " ");
		sprintf(tempString, "%f", (y2*.333333333));
		strcat(outString, tempString);
		strcat(outString, " ");
		sprintf(tempString, "%f", (z2*.333333333));
		strcat(outString, tempString);
		strcat(outString, ";");
	}

	

if(type ==4){

strcpy(outString, "exec savg-cylinder | savg-scale .0625 .0625 .5 | savg-rgb ");
sprintf(tempString, "%f", r);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", g);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", b);
strcat(outString, tempString);
strcat(outString, " ;");

strcat(outString, " savg-cylinder | savg-scale .08 .08 .083333 | savg-translate 0 0 0 | savg-rgb ");
sprintf(tempString, "%f", r);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", g);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", b);
strcat(outString, tempString);
strcat(outString, " ;");

strcat(outString, " savg-cone | savg-scale .08 .08 .083333 | savg-translate 0 0 .083333 | savg-rgb ");
sprintf(tempString, "%f", r);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", g);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", b);
strcat(outString, tempString);
strcat(outString, " ;");

strcat(outString, " savg-cylinder 16 top | savg-rgb .01 .01 .01 | savg-scale .08 .08 .05 | savg-translate 0 0 -.051; savg-cylinder | savg-rgb 1 .8 .5 | savg-scale .01 .01 .03 | savg-translate 0 0 -.0251; savg-cylinder | savg-rgb 1 1 1 | savg-alpha .15 | savg-scale .078 .078 .0025 | savg-translate 0 0 -.045; savg-cube | savg-rgb .01 .01 .01 | savg-scale .033333 .01 .04 | savg-translate 0 -.0625 0.15;");

} 

if(type==5){
 if(small==1){
	 strcpy(outString, "exec savg-cylinder 16 open | savg-rgb 1 1 1| savg-scale .27700707 .27700707 .30778543| savg-rotate 0 45 0| savg-translate 0 0 .19587582;  savg-cylinder 16 open | savg-rgb 0.1 0.1 0.1| savg-scale .27397145 .27397145  .30778543 | savg-rotate 0 45 0| savg-translate 0 0 .19587582 ; savg-cone 16 | savg-scale  .27700707 .27700707 .30356107 |savg-rgb 1 1 1| savg-rotate 0 45 0 | savg-translate 0 -.21704617 .41292198; savg-cylinder 16| savg-scale .049203464 .049203464 .15178053 | savg-rgb .1 .1 .1 | savg-translate 0 -.3035617 .6357267; savg-cylinder 16 top | savg-rgb 1 1 1 | savg-scale .15289271 .15289271 .49245711 | savg-rotate 0 45 0 | savg-translate 0 -.24284886 .43872467; savg-cylinder 16| savg-scale .046167853 .046167853 .30356107 | savg-rgb 1 1 1 | savg-translate 0 -.30356107 .69643891;");
}

else{
	strcpy(outString, "exec savg-cylinder 16 open | savg-rgb 1 1 1| savg-scale .0912525 .0912525 .1013916 | savg-rotate 0 45 0| savg-translate 0 0 .064526;  savg-cylinder 16 open | savg-rgb 0.1 0.1 0.1| savg-scale .0902525 .0902525 .1013916 | savg-rotate 0 45 0| savg-translate 0 0 .06452 ; savg-cone 16 | savg-scale  .0912525 .0912525 .1 |savg-rgb 1 1 1| savg-rotate 0 45 0 | savg-translate 0 -.0715 .136026; savg-cylinder 16| savg-scale .016208753 .016208753 .05 | savg-rgb .1 .1 .1 | savg-translate 0 -.1 .209423; savg-cylinder 16 top | savg-rgb 1 1 1 | savg-scale .0506958 .0506958 .1622267 | savg-rotate 0 45 0 | savg-translate 0 -.08 .144526; savg-cylinder 16| savg-scale .015208753 .015208753 .77057682 | savg-rgb 1 1 1 | savg-translate 0 -.1 .229423;");
}	


}


//printf("# type = %d\n", type);

system(outString);


}



void usage()
{
	fprintf(stderr, "\n\tERROR: Incorrect command line arguments - \n\t Must contain at least 2 arguments [phone], [screen], [box], [flashlight], or [light]\n\n");
}

void usage2()
{
	fprintf(stderr, "\n\tERROR: Incorrect command line arguments - \n\t Screen must have width and height values\nUsage: savg-object screen 10 4 > file.savg\n\n");
}
void usage3()
{
	fprintf(stderr, "\n\tERROR: Incorrect command line arguments - \n\t Box can take up to 3 arguments [length] [width] [height]\n\n");
}
void usage4()
{
       fprintf(stderr, "\n\tERROR: Incorrect command line arguments - \n\t Flashlight can take up to 3 arguments [r] [g] [b]\n\n");
}

void usage5()
{
	fprintf(stderr, "\n\tERROR: Incorrect command line arguments - [light] [short]\n\nUSAGE: savg-object light short > short_light.savg\n\n");
}
