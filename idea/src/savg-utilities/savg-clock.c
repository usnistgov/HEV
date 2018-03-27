/************************************************
 * Savg-clock					*	
 * Created on: 8-9-05		         	*
 * Current Version: 1.00			*
 *						*
 * Description: 				*
 *    Savg-clock creates an analog clock around	*
 * the y-axis.  It displays hours, minutes, and *
 * seconds.  Each may be entered with command	*
 * line arguments -h hval, -m mval, -s sval, or *
 * -a hval mval sval.	The -c option uses the	*
 * current system time, as provided by date, as *
 * the time to be displayed.			*
 ************************************************/



#include <stdio.h>
#include <stdlib.h>
#include "feed.h"
#include <math.h>
#define PI 3.141592653589793238462643
#include <time.h>
#include <string.h>

void usage();

main(argc,argv)
int argc;
char *argv[];
{
/*
 * INITIALIZATION
 */
        int i;

        int h=12;
        int m=0;
        int s=0;
	int n=1;
	float angle, newX, newZ; 
	char outString[10024], tempString[10024];
struct tm *myLocalTime;
time_t mytime;
mytime=time(NULL);
myLocalTime=localtime(&mytime);

 
//printf("# time = %d\n", myLocalTime->tm_min);
/*
 * COMMAND LINE PROCESSING
 */
        for(i=1;i<argc;i++){
                if(strcmp(argv[i],"-a")==0 && argc > (i+3)){
                        h=(int)atof(argv[++i])%12;
                        m=(int)atof(argv[++i])%60;
                        s=(int)atof(argv[++i])%60;
                }
                else if(strcmp(argv[i],"-h")==0 && argc > (i+1))
                        h=(int)atof(argv[++i])%12;
                else if(strcmp(argv[i],"-m")==0 && argc > (i+1))
                        m=(int)atof(argv[++i])%60;
                else if(strcmp(argv[i],"-s")==0 && argc > (i+1))
                        s=(int)atof(argv[++i])%60;
                else if(strcmp(argv[i],"-c")==0){
                        h=(myLocalTime->tm_hour)%12;
                        m=(myLocalTime->tm_min)%60;
                        s=(myLocalTime->tm_sec)%60;
                }
                else
                {
                        usage();
                        exit(EXIT_FAILURE);
                }
        }

        if(     h > 12 | m > 59 | s > 59 |
                h < 0  | m <  0 | s <  0 )
        {
                usage();
                exit(EXIT_FAILURE);
        }
//printf("# h = %d\n", h);

//printf("lines\n -1 0 0\n 1 0 0\nlines\n0 0 -1\n0 0 1\n");
//printf("# here after arg checks\n");
//create numbers on clock
strcpy(outString, "exec savg-cylinder 30 bottom | savg-scale .5 .5 .1 | savg-rgb .9 .9 .9| savg-translate 0 0 0 | savg-rotate 0 90 0; ");
printf("# here after clyinder making!\n");
for(i=2; i > -10; i--){
	angle=(float)((i)*360/12)* PI/180;
	newX=(4.25)*cos(angle);
	newZ=(4.25)*sin(angle);	
	strcat(outString, " savg-text ");
	sprintf(tempString, "%d", n);
	strcat(outString, tempString);
	strcat(outString, " | savg-translate ");
	sprintf(tempString, "%f",(newX-.2));
	strcat(outString, tempString);
	strcat(outString, " -.2 ");
	sprintf(tempString, "%f", (newZ-.3));
        strcat(outString, tempString);
	strcat(outString, " | savg-scale .1 .005 .1 | savg-rgb .1 .1 .1;");
	n++;
}

//make hands
//hour hand
	strcat(outString, " savg-prism 4 1 .2 | savg-translate 0 -.2  0| savg-scale .04 .05 .2 | savg-rgb .1 .12 .1  | savg-rotate 0 0 ");
	sprintf(tempString, "%d", h*30);
	strcat(outString, tempString);
//minute hand
	strcat(outString, "; savg-prism 4 1 .2 | savg-translate 0 -.2 0 | savg-scale .04 .05 .32 | savg-rgb .1 .1 .1 | savg-rotate 0 0 ");
	sprintf(tempString, "%d", m*6);
        strcat(outString, tempString);
//second hand
	strcat(outString, "; savg-prism 4 1 .2 | savg-translate 0 -.2 0 | savg-scale .025 .05 .4 | savg-rgb .8 0 0 | savg-rotate 0 0 ");
	sprintf(tempString, "%d", s*6);
        strcat(outString, tempString);
	strcat(outString, ";");

system(outString);
}


void usage()
{
  fprintf(stderr,
	"ERROR: Incorrect command line arguments\n"
	"\n"
	"USAGE:\n"
	"\tgtb-clock [-h value | -m value | -s value | -a hval mval sval | -c]\n"
	"\tThe values respesent hours minutes and seconds\n");

}
