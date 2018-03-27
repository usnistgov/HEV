/****************************************
* SAVG-PIE				*
* Created on 8/11/05			*
* 					*
* This program creates an object that	*
* looks like a pie-slice.  It takes up	*
* to four parameters, angle, number of	*
* sides, height, and radius,		* 
* respectively.  If the parameters are	*
* given incorrectly, an error message 	*
* will display.				*	
****************************************/

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
	int i;	
	float radius = .5, angle = 45, hight = .25, nsides=32;
	float newX, newY, newX2, newY2, newX3, newY3, newX4, newY4,degree,degree2;	

	/*** Processing arguments *****/
if(argc >1)
  //first parameter is angle
    angle=atof(argv[1]);
if(argc>2)
  //second parameter is number of sides
    nsides = atof(argv[2]);
if(argc>3)
  //third parameter is height
    hight=atof(argv[3]);
if(argc>4)
  //fourth parameter is radius
    radius=atof(argv[4]);
if(argc>5){
  usage();
  exit(EXIT_FAILURE);
}

if(radius <= 0 | hight <= 0 | angle > 360 | angle <= 0)
        {
                usage();
                exit(EXIT_FAILURE);
        }

/**** Construction ****/

//create triangles in the base 
for(i=0; i<nsides; i++){
	printf("polygons\n");
	degree=(double)(i*angle/nsides)*(PI/180);
	degree2=(double)((i+1)*angle/nsides)*(PI/180);
	newX=(.8*radius)*(cos(degree));
	newY = (.8*radius)*(sin(degree));
	newX2 = (.8*radius)*(cos(degree2));
	newY2 = (.8*radius)*(sin(degree2));
	printf("%f %f 0\n", newX2, newY2);
        printf("0 0 0\n");
	printf("%f %f 0\n", newX, newY);
}
printf("\n\n# hight = %f\n\n\n", hight);
//create outside polygons
for(i=0; i<nsides; i++){
        printf("polygons\n");
        degree=(double)(i*angle/nsides)*(PI/180);
        degree2=(double)((i+1)*angle/nsides)*(PI/180);
        newX=(.8*radius)*(cos(degree));
        newY = (.8*radius)*(sin(degree));
        newX2 = (.8*radius)*(cos(degree2));
        newY2 = (.8*radius)*(sin(degree2));
	newX3=(radius)*(cos(degree2));
	newY3=(radius)*(sin(degree2));
	newX4=(radius)*(cos(degree));
	newY4=(radius)*(sin(degree));
	printf("\n\n# newX4 = %f newY4 = %f\n\n\n", newX4, newY4);
        printf("%f %f 0\n", newX2, newY2);
	printf("%f %f 0\n", newX, newY);
	printf("%f %f %f\n", newX4, newY4, hight);
	printf("%f %f %f\n", newX3, newY3, hight);
	printf("polygons\n");
	printf("%f %f %f\n", .9*newX3, .9*newY3, (hight*.8));
	printf("%f %f %f\n", .9*newX4, .9*newY4, (hight*.8));
	printf("%f %f %f\n", newX4, newY4, hight);
	printf("%f %f %f\n", newX3, newY3, hight);
	if(angle < 360){
	  if(i==0){
	    printf("polygons\n");
	    printf("0 0 0 \n");
	    printf("%f %f 0\n", newX, newY);
	    printf("%f %f %f\n", 1.125*newX, 1.125*newY, hight*.8);
	    printf("0 0 %f\n", hight*.8);
	    printf("polygons\n");
	    printf("%f %f 0\n", newX, newY);
	    printf("%f %f %f\n", newX4, newY4, hight);
	    printf("%f %f %f\n", 1.125*newX, 1.125*newY, hight*.8);
	  }
	  if(i==nsides-1){
	    printf("polygons\n");
	    printf("0 0 0\n");
	    printf("%f %f 0\n", newX2, newY2);
	    printf("%f %f %f\n", 1.125*newX2, 1.125*newY2, hight*.8);
	    printf("0 0 %f\n", hight*.8);
	    printf("polygons\n");
	    printf("%f %f 0\n", newX2, newY2);
	    printf("%f %f %f\n", newX3, newY3, hight);
	    printf("%f %f %f\n", 1.125*newX2, 1.125*newY2,hight*.8);
	  }
	}
}

//create top
for(i=0; i<nsides; i++){
        printf("polygons\n");
        degree=(double)(i*angle/nsides)*(PI/180);
        degree2=(double)((i+1)*angle/nsides)*(PI/180);
        newX=(.9*radius)*(cos(degree));
        newY = (.9*radius)*(sin(degree));
        newX2 = (.9*radius)*(cos(degree2));
        newY2 = (.9*radius)*(sin(degree2));
        printf("%f %f %f\n", newX2, newY2, hight*.8);
        printf("0 0 %f\n", hight*.8);
        printf("%f %f %f\n", newX, newY, hight*.8);
}
}

void usage()
{
 fprintf(stderr, "\n\tERROR: Incorrect command line parameters: Takes up to 4 parameters: angle, nsides, height, radius\n\t USAGE: savg-pie 122 16 .5 .8 > pie.savg\n\n");
}

	

