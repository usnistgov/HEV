
/********************************
* Savg-Rainbow			*
* Created on 8/11/05		*
* By Alex Thibau		*
*				*
* This program creates a glyph	*
* rainbow.  It takes up to 3	*
* parameters: thickness, angle,	*
* and blended or solid colors.	*
* If the parameters are given 	*
* incorrectly, an error message	*
* displays.			*
********************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define PI 3.141592653589793238462643

/* Prints information on usage*/
void usage();

int main(argc, argv)
int argc;
char *argv[];
{
//char outString[1024], tempString[1024];
int i,j;
int blend=1;
float x=.083333; //coefficient
float spacing;
float thickness=.5,r[2],g[2],b[2],newX, newY, newX2, newY2,newX4, newY4, newX3, newY3,degree,angle=136,degree2;


if(argc>1)
  //first parameter is thickness
      thickness=atof(argv[1]);
if(argc >2)
  angle=atof(argv[2]);

if(argc >3){
  //third parameter is blended or solid
    if(strcmp(argv[3], "blend")==0)
      blend=1;
    if(strcmp(argv[3], "solid")==0)
      blend=0;
}

if(angle > 360 || angle < -360 || angle ==0){
  usage();
  exit(EXIT_FAILURE);
}

if(thickness <=0 || thickness >1){
  usage();
  exit(EXIT_FAILURE);
}

printf("# angle = %f\n", angle);
if(argc > 4){
  usage();
  exit(EXIT_FAILURE);
}



for(j=0; j<7; j++){
for(i=0; i<16; i++){
	if(j<3)
		r[0]=1;	
	if(j>5)
		r[0]=.1;
	if(j>2&& j<6)
		r[0]=0;
	if(j<4)
                b[0]=0;
	if(j==4)
		b[0]=1;
	if(j>4)
		b[0]=.35;
	if(j<1|| j>3)
		g[0]=0;
	if(j==3 || j==2)
		g[0]=1;
	if(j==1)
		g[0]=.4;

	if(j<2)
	  r[1]=1;
	if(j>1 && j<5)
	  r[1]=0;
	if(j>4)
	  r[1]=.1;
	if(j==0)
	  g[1]=.4;
	if(j==1 || j==2)
	  g[1]=1;
	if(j>2)
	  g[1]=0;
	if(j<3)
	  b[1]=0;
	if(j==3)
	  b[1]=1;
	if(j>3)
	  b[1]=.35;
	   	degree=(double)(i*angle/16)*(PI/180);
        degree2=(double)((i+1)*angle/16)*(PI/180);
x=(.5-(.5*(1-thickness)))/7;
	spacing=.5-x;
	  newX=(.5-(j*x))*(cos(degree));
	  newY=(.5-(j*x))*(sin(degree));
	  newX2=(.5-(j*x))*(cos(degree2));
	  newY2=(.5-(j*x))*(sin(degree2));
	  newX3=(spacing-(j*x))*(cos(degree2));
	  newY3=(spacing-(j*x))*(sin(degree2));
	  newX4=(spacing-(j*x))*(cos(degree));
	  newY4=(spacing-(j*x))*(sin(degree));
 
	printf("# i=%d\n", i); //allows for easy checking of polygons in output
	
	printf( "polygons\n");    //print base
	printf("%f %f 0 %f %f %f 1\n", newX2, newY2, r[0],g[0],b[0]);
		printf("%f %f 0 %f %f %f 1\n", newX , newY, r[0],g[0],b[0]);
	printf("%f %f 0 %f %f %f 1\n", newX4, newY4, r[blend],g[blend],b[blend]);
	printf("%f %f 0 %f %f %f 1\n", newX3, newY3, r[blend],g[blend],b[blend]);
	printf("polygons\n");     //print top
	printf("%f %f .1 %f %f %f 1\n", newX , newY, r[0],g[0],b[0]);
	printf("%f %f .1 %f %f %f 1\n", newX2, newY2, r[0],g[0],b[0]);
	printf("%f %f .1 %f %f %f 1\n", newX3, newY3, r[blend],g[blend],b[blend]);
	printf("%f %f .1 %f %f %f 1\n", newX4, newY4, r[blend],g[blend],b[blend]);
	if(i==0){
	  printf("polygons\n"); //print right side
	  printf("%f %f 0 %f %f %f 1\n", newX4, newY4, r[blend],g[blend],b[blend]);
	  printf("%f %f 0 %f %f %f 1\n", newX-.000001 , newY, r[0],g[0],b[0]);
	  printf("%f %f .1 %f %f %f 1\n", newX-.000001 , newY, r[0],g[0],b[0]);
	  printf("%f %f .1 %f %f %f 1\n", newX4, newY4, r[blend],g[blend],b[blend]);
	}
	if(i==15){   //print left side
	  printf("polygons\n");
	  	  printf("%f %f 0 %f %f %f 1\n", newX2, newY2, r[0],g[0],b[0]);
		  printf("%f %f 0 %f %f %f 1\n", newX3-.000001, newY3, r[blend],g[blend],b[blend]);
	  printf("%f %f .1 %f %f %f 1\n", newX3, newY3-.000001, r[blend],g[blend],b[blend]);
	  printf("%f %f .1 %f %f %f 1\n", newX2, newY2, r[0],g[0],b[0]);

	}
	if(j==0){
	  printf("polygons\n");	    	//print violet filler
	  printf("%f %f 0 %f %f %f 1\n", newX , newY, r[0],g[0],b[0]);
	  printf("%f %f 0 %f %f %f 1\n", newX2, newY2, r[0],g[0],b[0]);
	  printf("%f %f .1 %f %f %f 1\n", newX2, newY2,r[0],g[0],b[0] );
	  printf("%f %f .1 %f %f %f 1\n", newX , newY, r[0],g[0],b[0]);
	}
	if(j==6){
	  printf("polygons\n");   //print red filler
	  printf("%f %f 0 %f %f %f 1\n", newX3, newY3, r[0],g[0],b[0]);
	  printf("%f %f 0 %f %f %f 1\n", newX4, newY4, r[0],g[0],b[0]);
	  printf("%f %f .1 %f %f %f 1\n", newX4, newY4, r[0],g[0],b[0]);
	  printf("%f %f .1 %f %f %f 1\n", newX3, newY3, r[0],g[0],b[0]);
	}
}}
 
}
void usage()
{
  fprintf(stderr, "\n\tERROR: Incorrect parameters: [tval (0> <=1)] [aval (360> < 0 > <360)] [solid or blend]\n\t\tUSAGE: savg-rainbow .35 122 blend> rainbow.savg\n\n");
} 

