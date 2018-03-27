/************************************************
 * savg-fish                                    *
 * Created on: 8-3-05                           *
 * Current Version: 1.00                        *
 *                                              *
 * Description:                                 *
 *    Savg-fish creates a simple fish with      *
 * a specfic amount of options.  The user may   *
 * chooose whether or not the eyes or fins of   *
 * the fish will be visible.  If this command   *
 * is not used properly, a usage message will   *
 * print.                                       *
 ************************************************/



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "feed.h"

void usage();

main(argc, argv)
int argc;
char *argv[];
{
        int i; 
	int fins = 0;
        int eyes = 0;
	char outString[1024];

	for(i = 1; i<argc; i++){
                if(strcmp(argv[i], "--fins") == 0)
                        fins = 1;
                else if(strcmp(argv[i], "--eyes") == 0)
                        eyes = 1;
                else if(strcmp(argv[i], "--all") == 0){
                        fins = 1;
                        eyes = 1;
                }
                else{
                        usage();
                        exit(EXIT_FAILURE);
                }
        }

	strcpy(outString, "exec savg-sphere | savg-scale .45 .27 .27 | savg-translate -.050 0 0; savg-cone 20 | savg-scale .25 .25 .2 | savg-rotate 0 0 270 | savg-translate .5 0 0;");

	if(fins ==1){
		
		strcat(outString, "savg-sphere 50 | savg-scale .13333 .06666 .03333 | savg-translate .25 0 .3; savg-sphere 50 | savg-scale .13333 .06666 .03333| savg-translate .25 0 -.3;");
}

	if(eyes ==1 ){ //eyes

		strcat(outString, " savg-sphere 50 | savg-scale .03333 .03333 .03333 | savg-translate -.2 .04 .275 | savg-rgb 0 0 0; savg-sphere 50 | savg-scale .03333 .03333 .03333 | savg-translate -.2 .04 -.275 | savg-rgb 0 0 0;");
}


system(outString);

}

void usage(){
        fprintf(stderr, "Incorrect command line\n");
        fprintf(stderr, "Usage: savg-fish --fins --eyes --all > fish.savg\n");
}
