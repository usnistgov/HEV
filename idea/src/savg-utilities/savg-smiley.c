#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>

enum emotion{error, file, happy, sad, angry, impassive, surprised, dead, evil,
question, foo, bar} mood[3];

enum emotion strtoemo(char[]);
void substr(char[], int, int, char[]);

char customfile[2][200];
int curface=-1;
int debugger;

/* main program */

int main(argc, argv)
int argc;
char *argv[];
{
        //declare variables
 
char outString[5096];
char tempString[5096];
        float deg=1;
        float facew[3], faceh[3], facerot[3], facer[3], faceg[3], faceb[3];
        int facesides[3];
        float eyesw[3], eyesh[3], eyesvert[3], eyessp[3];
        float pupilsx[3], pupilsy[3], pupilsw[3], pupilsh[3], pupilsr[3], pupilsg[3], pupilsb[3];
       float mouthw[3], mouthbend[3], mouthvert[3], mouthopen[3], mouthr;
        float browsangle[3], browssp[3], browlvert[3], browrvert[3], browsrot[3], browsw[3], browsh[3], browsr[3], browsg[3], browsb[3];
        int browssides[3];
        int i, negbend, areteeth[3], istongue[3];
    
        mood[0]=happy;
        mood[1]=happy;
	mouthr = .8;
	negbend =0;


/* process input */

        i = 1;

        while (i < argc)
        {
                if (strcmp(argv[i], "--debug") == 0){
                        debugger=1;
                }

		/* if (strcmp(argv[i], "--vals") == 0){
                        printvals=1;;
                }*/

                else if (argc<=7 && strcmp(argv[i], "--part") == 0){
                        deg=atof(argv[i+1]);
                        mood[0]=strtoemo(argv[i+2]);
                        mood[1]=strtoemo(argv[i+3]);
                        i+=3;
                }

                else if (argc<=4){
                        deg=1;
                        mood[0]=strtoemo(argv[i]);
                        mood[1]=strtoemo(argv[i]);
                }

                if (mood[0]==error || mood[1]==error || argc > 7)
                {
                        fprintf(stderr,"incorrect command line\n");
                        fprintf(stderr,"usage: gtb-smiley happy | sad | angry | impassive\n");
                        fprintf(stderr,"\t | surprised | dead | evil | foo | question\n");
                        fprintf(stderr,"\t | file-filename\n");
                        fprintf(stderr,"\t | -part (degree) (emotion 1) (emotion 2)\n");
                        fprintf(stderr,"\t [--debug] [--vals]\n");
                        exit(0);
                }

                i+=1;
        }

/* set up features based on mood */

        for (i=0; i<2; i++){
        switch (mood[i]){
                case happy:
                        facew[i]=1;
                        faceh[i]=1;
                        facesides[i]=32;
                        facerot[i]=0;
                        facer[i]=1;
                        faceg[i]=1;
                        faceb[i]=0;

                        eyesw[i]=.2;
                        eyesh[i]=.3;
                        eyesvert[i]=.15; /* y dist from center of face to center of eye */
                        eyessp[i]=.15; /* x dist from center of face to center of eye */

                        pupilsx[i]=0; /* x dist from center of eye to center of pupil */
                        pupilsy[i]=-.05; /* y dist from center of eye to center of pupil */
                        pupilsw[i]=.1;
                        pupilsh[i]=.1;
                        pupilsr[i]=0;
                        pupilsg[i]=0;
                        pupilsb[i]=.1;

                        mouthw[i]=.6;
                        mouthbend[i]=.2; /* distance corners of mouth bend up/down */
                        mouthopen[i]=.1; /* y thickness of center of mouth */
                        mouthvert[i]=-.25; /* y dist from center of face to center of mouth */

                        browsangle[i]=0; /* pos = / \ neg = \ / */
                        browssp[i]=.15; /* x dist from center of face to center of eyebrow */
                        browlvert[i]=.35; /* y dist from center of face to center of left eyebrow */
                        browrvert[i]=.35; /* y dist from center of face to center of right eyebrow */
                        browssides[i]=4;
                        browsw[i]=.15;
                        browsh[i]=.05;
                        browsrot[i]=0;
                        browsr[i]=.1;
                        browsg[i]=.1;
                        browsb[i]=0;

                        areteeth[i]=1;
                        istongue[i]=0;

                        break;

    case sad:
                        facew[i]=1;
                        faceh[i]=1;
                        facesides[i]=32;
                        facerot[i]=0;
                        facer[i]=.6;
                        faceg[i]=.7;
                        faceb[i]=1;

                        eyesw[i]=.21;
                        eyesh[i]=.2;
                        eyesvert[i]=.13; /* y dist from center of face to center
of eye */
                        eyessp[i]=.15; /* x dist from center of face to center of eye */

                        pupilsx[i]=0; /* x dist from center of eye to center of pupil */
                        pupilsy[i]=.04; /* y dist from center of eye to center of pupil */
                        pupilsw[i]=.1;
                        pupilsh[i]=.1;
                        pupilsr[i]=0;
                        pupilsg[i]=.1;
                        pupilsb[i]=0;

                        mouthw[i]=.5;
                        mouthbend[i]=-.1; /* distance corners of mouth bend up/down */
                        mouthopen[i]=.1; /* y thickness of center of mouth */
                        mouthvert[i]=-.18; /* y dist from center of face to center of mouth */

                        browsangle[i]=30; /* pos = / \ neg = \ / */
                        browssp[i]=.2; /* x dist from center of face to center of eyebrow */
                        browlvert[i]=.3; /* y dist from center of face to center
of left eyebrow */
                        browrvert[i]=.3; /* y dist from center of face to center
of right eyebrow */
                        browssides[i]=4;
                        browsw[i]=.15;
                        browsh[i]=.05;
                        browsrot[i]=0;
                        browsr[i]=.1;
                        browsg[i]=.1;
                        browsb[i]=0;

                        areteeth[i]=0;
                        istongue[i]=0;

                        break;

        case angry:
                        facew[i]=1;
                        faceh[i]=1;
                        facesides[i]=16;
                        facerot[i]=0;
                        facer[i]=1;
                        faceg[i]=0;
                        faceb[i]=0;

                        eyesw[i]=.3;
                        eyesh[i]=.15;
                        eyesvert[i]=.13; /* y dist from center of face to center
of eye */
                        eyessp[i]=.15; /* x dist from center of face to center of eye */

                        pupilsx[i]=0; /* x dist from center of eye to center of pupil */
                        pupilsy[i]=-.02; /* y dist from center of eye to center of pupil */
                        pupilsw[i]=.07;
                        pupilsh[i]=.1;
                        pupilsr[i]=.1;
                        pupilsg[i]=0;
                        pupilsb[i]=0;

                        mouthw[i]=.5;
                        mouthbend[i]=0; /* distance corners of mouth bend up/down */
                        mouthopen[i]=.3; /* y thickness of center of mouth */
                        mouthvert[i]=-.18; /* y dist from center of face to center of mouth */

                        browsangle[i]=-45; /* pos = / \ neg = \ / */
                        browssp[i]=.15; /* x dist from center of face to center of eyebrow */
                        browlvert[i]=.25; /* y dist from center of face to center of left eyebrow */
                        browrvert[i]=.25; /* y dist from center of face to center of right eyebrow */
                        browssides[i]=3;
                        browsw[i]=.25;
                        browsh[i]=.06;
                        browsrot[i]=180;
                        browsr[i]=0;
                        browsg[i]=0;
                        browsb[i]=0;

                        areteeth[i]=0;
                        istongue[i]=0;

                        break;

    case impassive:
                        facew[i]=1;
                        faceh[i]=1;
                        facesides[i]=5;
                        facerot[i]=199;
                        facer[i]=.5;
                        faceg[i]=.5;
                        faceb[i]=.5;

                        eyesw[i]=.23;
                        eyesh[i]=.16;
                        eyesvert[i]=.12; /* y dist from center of face to center
of eye */
                        eyessp[i]=.15; /* x dist from center of face to center of eye */

                        pupilsx[i]=0; /* x dist from center of eye to center of pupil */
                        pupilsy[i]=-.04; /* y dist from center of eye to center of pupil */
                        pupilsw[i]=.1;
                        pupilsh[i]=.1;
                        pupilsr[i]=0;
                        pupilsg[i]=0;
                        pupilsb[i]=0;

                        mouthw[i]=.4;
                        mouthbend[i]=-.005; /* distance corners of mouth bend up/down */
                        mouthopen[i]=.02; /* y thickness of center of mouth */
                        mouthvert[i]=-.18; /* y dist from center of face to center of mouth */

                        browsangle[i]=0; /* pos = / \ neg = \ / */
                        browssp[i]=.15; /* x dist from center of face to center of eyebrow */
                        browlvert[i]=.21; /* y dist from center of face to center of left eyebrow */
                        browrvert[i]=.21; /* y dist from center of face to center of right eyebrow */
                        browssides[i]=4;
                        browsw[i]=.25;
                        browsh[i]=.05;
                        browsrot[i]=45;
                        browsr[i]=0;
                        browsg[i]=0;
                        browsb[i]=.1;

                        areteeth[i]=0;
                        istongue[i]=0;

                        break;
 

    case surprised:
                        facew[i]=1;
                        faceh[i]=1;
                        facesides[i]=7;
                        facerot[i]=38;
                        facer[i]=1;
                        faceg[i]=1;
                        faceb[i]=.2;

                        eyesw[i]=.3;
                        eyesh[i]=.3;
                        eyesvert[i]=.15; /* y dist from center of face to center
of eye */
                        eyessp[i]=.15; /* x dist from center of face to center of eye */

                        pupilsx[i]=0; /* x dist from center of eye to center of pupil */
                        pupilsy[i]=0; /* y dist from center of eye to center of pupil */
                        pupilsw[i]=.1;
                        pupilsh[i]=.1;
                        pupilsr[i]=.1;
                        pupilsg[i]=.1;
                        pupilsb[i]=.1;

                        mouthw[i]=.4;
                        mouthbend[i]=-.1; /* distance corners of mouth bend up/down */
                        mouthopen[i]=.2; /* y thickness of center of mouth */
                        mouthvert[i]=-.25; /* y dist from center of face to center of mouth */

                        browsangle[i]=30; /* pos = / \ neg = \ / */
                        browssp[i]=.15; /* x dist from center of face to center of eyebrow */
                        browlvert[i]=.35; /* y dist from center of face to center of left eyebrow */
                        browrvert[i]=.35; /* y dist from center of face to center of right eyebrow */
                        browssides[i]=5;
                        browsw[i]=.15;
                        browsh[i]=.07;
                        browsrot[i]=0;
                        browsr[i]=0;
                        browsg[i]=0;
                        browsb[i]=0;

                        areteeth[i]=0;
                        istongue[i]=0;

                        break;


  case dead:
                        facew[i]=1;
                        faceh[i]=1;
                        facesides[i]=4;
                        facerot[i]=0;
                        facer[i]=.4;
                        faceg[i]=.4;
                        faceb[i]=0;

                        eyesw[i]=.2;
                        eyesh[i]=.01;
                        eyesvert[i]=.12; /* y dist from center of face to center
of eye */
                        eyessp[i]=.15; /* x dist from center of face to center of eye */

                        pupilsx[i]=0; /* x dist from center of eye to center of pupil */
                        pupilsy[i]=0; /* y dist from center of eye to center of pupil */
                        pupilsw[i]=.01;
                        pupilsh[i]=.2;
                        pupilsr[i]=1;
                        pupilsg[i]=1;
                        pupilsb[i]=1;

                        mouthw[i]=.3;
                        mouthbend[i]=-.15; /* distance corners of mouth bend up/down */
                        mouthopen[i]=.2; /* y thickness of center of mouth */
                        mouthvert[i]=-.18; /* y dist from center of face to center of mouth */

                        browsangle[i]=0; /* pos = / \ neg = \ / */
                        browssp[i]=.15; /* x dist from center of face to center of eyebrow */
                        browlvert[i]=.21; /* y dist from center of face to center of left eyebrow */
                        browrvert[i]=.21; /* y dist from center of face to center of right eyebrow */
                        browssides[i]=4;
                        browsw[i]=.15;
                        browsh[i]=.05;
                        browsrot[i]=45;
                        browsr[i]=0;
                        browsg[i]=0;
                        browsb[i]=0;

                        areteeth[i]=0;
                        istongue[i]=1;

                        break;


    case evil:
                        facew[i]=1;
                        faceh[i]=1;
                        facesides[i]=16;
                        facerot[i]=0;
                        facer[i]=1;
                        faceg[i]=0;
                        faceb[i]=0;

                        eyesw[i]=.3;
                        eyesh[i]=.1;
                        eyesvert[i]=.13; /* y dist from center of face to center
of eye */
                        eyessp[i]=.15; /* x dist from center of face to center of eye */

                        pupilsx[i]=0; /* x dist from center of eye to center of pupil */
                        pupilsy[i]=-.02; /* y dist from center of eye to center of pupil */
                        pupilsw[i]=.07;
                        pupilsh[i]=.1;
                        pupilsr[i]=.1;
                        pupilsg[i]=0;
                        pupilsb[i]=0;

                        mouthw[i]=.45;
                        mouthbend[i]=.1; /* distance corners of mouth bend up/down */
                        mouthopen[i]=.15; /* y thickness of center of mouth */
                        mouthvert[i]=-.18; /* y dist from center of face to center of mouth */

                        browsangle[i]=35; /* pos = / \ neg = \ / */
                        browssp[i]=.3; /* x dist from center of face to center of eyebrow */
                        browlvert[i]=.40; /* y dist from center of face to center of left eyebrow */
                        browrvert[i]=.40; /* y dist from center of face to center of right eyebrow */
                        browssides[i]=3;
                        browsw[i]=.2;
                        browsh[i]=.4;
                        browsrot[i]=0;
                        browsr[i]=.1;
                        browsg[i]=0;
                        browsb[i]=0;

                        areteeth[i]=1;
                        istongue[i]=0;

                        break;


  case question:
                        facew[i]=1;
                        faceh[i]=1;
                        facesides[i]=7;
                        facerot[i]=193.5;
                        facer[i]=.1;
                        faceg[i]=.5;
                        faceb[i]=.4;

                        eyesw[i]=.23;
                        eyesh[i]=.2;
                        eyesvert[i]=.12; /* y dist from center of face to center
of eye */
                        eyessp[i]=.15; /* x dist from center of face to center of eye */

                        pupilsx[i]=0; /* x dist from center of eye to center of pupil */
                        pupilsy[i]=-.04; /* y dist from center of eye to center of pupil */
                        pupilsw[i]=.1;
                        pupilsh[i]=.1;
                        pupilsr[i]=0;
                        pupilsg[i]=0;
                        pupilsb[i]=0;

                        mouthw[i]=.4;
                        mouthbend[i]=-.01; /* distance corners of mouth bend up/down */
                        mouthopen[i]=.02; /* y thickness of center of mouth */
                        mouthvert[i]=-.18; /* y dist from center of face to center of mouth */

                        browsangle[i]=-15; /* pos = / \ neg = \ / */
                        browssp[i]=.15; /* x dist from center of face to center of eyebrow */
                        browlvert[i]=.3; /* y dist from center of face to center
of left eyebrow */
                        browrvert[i]=.23; /* y dist from center of face to center of right eyebrow */
                        browssides[i]=4;
                        browsw[i]=.25;
                        browsh[i]=.05;
                        browsrot[i]=0;
                        browsr[i]=0;
                        browsg[i]=0;
                        browsb[i]=.1;

                        areteeth[i]=0;
                        istongue[i]=0;

                        break;



  case foo:
                        facew[i]=1;
                        faceh[i]=1;
                        facesides[i]=32;
                        facerot[i]=0;
                        facer[i]=.4;
                        faceg[i]=0;
                        faceb[i]=.4;

                        eyesw[i]=.25;
                        eyesh[i]=.05;
                        eyesvert[i]=.12; /* y dist from center of face to center
of eye */
                        eyessp[i]=.15; /* x dist from center of face to center of eye */

                        pupilsx[i]=0; /* x dist from center of eye to center of pupil */
                        pupilsy[i]=0; /* y dist from center of eye to center of pupil */
                        pupilsw[i]=.1;
                        pupilsh[i]=.05;
                        pupilsr[i]=0;
                        pupilsg[i]=0;
                        pupilsb[i]=0;

                        mouthw[i]=.3;
                        mouthbend[i]=-.05; /* distance corners of mouth bend up/down */
                        mouthopen[i]=.05; /* y thickness of center of mouth */
                        mouthvert[i]=-.18; /* y dist from center of face to center of mouth */

                        browsangle[i]=-15; /* pos = / \ neg = \ / */
                        browssp[i]=.15; /* x dist from center of face to center of eyebrow */
                        browlvert[i]=.18; /* y dist from center of face to center of left eyebrow */
                        browrvert[i]=.18; /* y dist from center of face to center of right eyebrow */
                        browssides[i]=3;
                        browsw[i]=.2;
                        browsh[i]=.05;
                        browsrot[i]=0;
                        browsr[i]=0;
                        browsg[i]=0;
                        browsb[i]=0;

                        areteeth[i]=0;
                        istongue[i]=1;

                        break;


    case bar:
                        facew[i]=2;
                        faceh[i]=.25;
                        facesides[i]=4;
                        facerot[i]=0;
                        facer[i]=.4;
                        faceg[i]=.4;
                        faceb[i]=.4;

                        eyesw[i]=.25;
                        eyesh[i]=.05;
                        eyesvert[i]=.03; /* y dist from center of face to center
of eye */
                        eyessp[i]=.3; /* x dist from center of face to center of
eye */

                        pupilsx[i]=0; /* x dist from center of eye to center of pupil */
                        pupilsy[i]=-.02; /* y dist from center of eye to center of pupil */
                        pupilsw[i]=.1;
                        pupilsh[i]=.05;
                        pupilsr[i]=0;
                        pupilsg[i]=0;
                        pupilsb[i]=0;

                        mouthw[i]=.7;
                        mouthbend[i]=.02; /* distance corners of mouth bend up/down */
                        mouthopen[i]=.05; /* y thickness of center of mouth */
                        mouthvert[i]=-.05; /* y dist from center of face to center of mouth */

                        browsangle[i]=0; /* pos = / \ neg = \ / */
                        browssp[i]=.3; /* x dist from center of face to center of eyebrow */
                        browlvert[i]=.06; /* y dist from center of face to center of left eyebrow */
                        browrvert[i]=.06; /* y dist from center of face to center of right eyebrow */
                        browssides[i]=4;
                        browsw[i]=.4;
                        browsh[i]=.03;
                        browsrot[i]=45;
                        browsr[i]=.8;
                        browsg[i]=.8;
                        browsb[i]=0;

                        areteeth[i]=1;
                        istongue[i]=0;

                        break;
                }}



/* calculate final feature values */
	printf("# %f\n", deg);
        facew[2]=deg*facew[0]+(1-deg)*facew[1];
	faceh[2]=deg*faceh[0]+(1-deg)*faceh[1];
	faceh[2]=deg*faceh[0]+(1-deg)*faceh[1];	
	facesides[2]=(int)(deg*facesides[0])+((1-deg)*facesides[1]);
        facerot[2]=deg*facerot[0]+(1-deg)*facerot[1];
        facer[2]=deg*facer[0]+(1-deg)*facer[1];
        faceg[2]=deg*faceg[0]+(1-deg)*faceg[1];
        faceb[2]=deg*faceb[0]+(1-deg)*faceb[1];
        
printf("# values\n");
printf("# facew %f faceh %f facesides %d facesides[0] %d facesides[1] %d facerot %f facer %f faceg %f faceb %f deg %f\n", facew[2], faceh[2], facesides[2], facesides[0], facesides[1], facerot[2], facer[2], faceg[2],
faceb[2], deg);

	eyesw[2]=deg*eyesw[0]+(1-deg)*eyesw[1];
        eyesh[2]=deg*eyesh[0]+(1-deg)*eyesh[1];
        eyesvert[2]=deg*eyesvert[0]+(1-deg)*eyesvert[1];
        eyessp[2]=deg*eyessp[0]+(1-deg)*eyessp[1];

        pupilsx[2]=deg*pupilsx[0]+(1-deg)*pupilsx[1];
        pupilsy[2]=deg*pupilsy[0]+(1-deg)*pupilsy[1];
        pupilsw[2]=deg*pupilsw[0]+(1-deg)*pupilsw[1];
        pupilsh[2]=deg*pupilsh[0]+(1-deg)*pupilsh[1];
        pupilsr[2]=deg*pupilsr[0]+(1-deg)*pupilsr[1];
        pupilsg[2]=deg*pupilsg[0]+(1-deg)*pupilsg[1];
        pupilsb[2]=deg*pupilsb[0]+(1-deg)*pupilsb[1];

        mouthw[2]=deg*mouthw[0]+(1-deg)*mouthw[1];
        mouthbend[2]=deg*mouthbend[0]+(1-deg)*mouthbend[1];
        mouthopen[2]=deg*mouthopen[0]+(1-deg)*mouthopen[1];
        mouthvert[2]=deg*mouthvert[0]+(1-deg)*mouthvert[1];

        browsangle[2]=deg*browsangle[0]+(1-deg)*browsangle[1];
        browssp[2]=deg*browssp[0]+(1-deg)*browssp[1];
        browlvert[2]=deg*browlvert[0]+(1-deg)*browlvert[1];
        browrvert[2]=deg*browrvert[0]+(1-deg)*browrvert[1];
        browssides[2]=(int)((deg*browssides[0])+((1-deg)*browssides[1]));
	browsw[2]=deg*browsw[0]+(1-deg)*browsw[1];
        browsh[2]=deg*browsh[0]+(1-deg)*browsh[1];
        browsrot[2]=deg*browsrot[0]+(1-deg)*browsrot[1];
        browsr[2]=deg*browsr[0]+(1-deg)*browsr[1];
        browsg[2]=deg*browsg[0]+(1-deg)*browsg[1];
        browsb[2]=deg*browsb[0]+(1-deg)*browsb[1];

 if (deg>.5){
                areteeth[2]=areteeth[0];
                istongue[2]=istongue[0];
        }
        else{
                areteeth[2]=areteeth[1];
                istongue[2]=istongue[1];
        }

        if (mouthbend[2]<0){
                negbend=1;
                mouthbend[2]=-mouthbend[2];
        }



printf("# browsangle %f browssp %f browlvert %f browrvert %f browssides %d browsw %f browsh %f browsrot %f browsr %f browsg %f browsb %f deg %f browssides[0] %d browssides[1] %d\n", browsangle[2], browssp[2], browlvert[2], browrvert[2], browssides[2], browsw[2], browsh[2], browsrot[2], browsr[2], browsg[2], browsb[2], deg, browssides[0], browssides[1]);

if (facer[2] >.5 && faceb[2] < .5 && faceg[2] < .5)
                mouthr=0;


printf("\n#the face\n\n\n");


strcpy(outString, "exec savg-prism ");
sprintf(tempString, "%d", facesides[2]);
strcat(outString, tempString);
strcat(outString, " .1 .9 | savg-rotate ");
sprintf(tempString, "%f", facerot[2]);
strcat(outString, tempString);
strcat(outString, " 0 0 | savg-scale ");
sprintf(tempString, "%f", facew[2]);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", faceh[2]);
strcat(outString, tempString);
strcat(outString, " 1 | savg-rgb ");
sprintf(tempString, "%f", facer[2]);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", faceg[2]);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", faceb[2]);
strcat(outString, tempString);
strcat(outString, ";  savg-prism 16 .05 .5 | savg-scale ");
sprintf(tempString, "%f", eyesw[2]);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", eyesh[2]);
strcat(outString, tempString);
strcat(outString, " 1 | savg-translate ");
sprintf(tempString, "%f", -eyessp[2]);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", eyesvert[2]);
strcat(outString, tempString);
strcat(outString, " .1 |savg-rgb 1 1 1; savg-prism 16 .05 .5 | savg-scale ");
sprintf(tempString, "%f", eyesw[2]);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", eyesh[2]);
strcat(outString, tempString);
strcat(outString, " 1 | savg-translate ");
sprintf(tempString, "%f", eyessp[2]);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", eyesvert[2]);
strcat(outString, tempString);
strcat(outString, " .1| savg-rgb 1 1 1; savg-prism 16 .0125 .5 | savg-scale ");
sprintf(tempString, "%f", pupilsw[2]);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", pupilsh[2]);
strcat(outString, tempString);
strcat(outString, " 1 | savg-translate ");
sprintf(tempString, "%f", (-eyessp[2]+pupilsx[2]));
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", (eyesvert[2]+pupilsy[2]));
strcat(outString, tempString);
strcat(outString, " .15 | savg-rgb  ");
sprintf(tempString, "%f", pupilsr[2]);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", pupilsg[2]);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", pupilsb[2]);
strcat(outString, tempString);
strcat(outString, "; savg-prism 16 .0125 .5 | savg-scale ");
sprintf(tempString, "%f", pupilsw[2]);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", pupilsh[2]);
strcat(outString, tempString);
strcat(outString," 1 | savg-translate ");
sprintf(tempString, "%f", (eyessp[2]+pupilsx[2]));
strcat(outString, tempString);
strcat(outString," ");
sprintf(tempString, "%f", (eyesvert[2]+pupilsy[2]));
strcat(outString, tempString);
strcat(outString," .15 | savg-rgb ");
sprintf(tempString, "%f", pupilsr[2]);
strcat(outString, tempString);
strcat(outString," ");
sprintf(tempString, "%f", pupilsg[2]);
strcat(outString, tempString);
strcat(outString," ");
sprintf(tempString, "%f", pupilsb[2]);
strcat(outString, tempString);
strcat(outString,"; savg-prism ");
sprintf(tempString, "%d", browssides[2]);
strcat(outString, tempString);
strcat(outString," .025 .5 | savg-rotate ");
sprintf(tempString, "%f", browsrot[2]);
strcat(outString, tempString);
strcat(outString, " 0 0 | savg-scale ");
sprintf(tempString, "%f", browsw[2]);
strcat(outString, tempString);
strcat(outString," ");
sprintf(tempString, "%f", browsh[2]);
strcat(outString, tempString);
strcat(outString," 1 | savg-rotate ");
sprintf(tempString, "%f", browsangle[2]);
strcat(outString, tempString);
strcat(outString," 0 0 | savg-translate ");
sprintf(tempString, "%f", -browssp[2]);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", browlvert[2]);
strcat(outString, tempString);
strcat(outString," .1 | savg-rgb ");
sprintf(tempString, "%f", browsr[2]);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", browsg[2]);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", browsb[2]);
strcat(outString, tempString);
strcat(outString, "; savg-prism ");
sprintf(tempString, "%d", browssides[2]);
strcat(outString, tempString);
strcat(outString, " .025 .5 | savg-rotate ");
sprintf(tempString, "%f", browsrot[2]);
strcat(outString, tempString);
strcat(outString, " 0 0 | savg-scale ");
sprintf(tempString, "%f", browsw[2]);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", browsh[2]);
strcat(outString, tempString);
strcat(outString, " 1 | savg-rotate ");
 for(i=0; i<2; i++){
    switch (mood[i]){
    case surprised:
           sprintf(tempString, "%f", (5*browsangle[2]));
           break;
    
    case foo:
           sprintf(tempString, "%f", (11*browsangle[2]));
           break;
    
    case evil:
           sprintf(tempString, "%f", (4*browsangle[2]));
           break;
	  
    case angry:
          sprintf(tempString, "%f", (-5*browsangle[2]));
	  break;

    default:
           sprintf(tempString, "%f", -browsangle[2]);
	     break;
    }}
strcat(outString, tempString);
strcat(outString, " 0 0 | savg-translate ");
sprintf(tempString, "%f", browssp[2]);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", browrvert[2]);
strcat(outString, tempString);
strcat(outString, " .1 | savg-rgb ");
sprintf(tempString, "%f", browsr[2]);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", browsg[2]);
strcat(outString, tempString);
strcat(outString, " ");
sprintf(tempString, "%f", browsb[2]);
strcat(outString, tempString);
strcat(outString, ";");



if(istongue[2]){
strcat(outString, " savg-sphere 180 | savg-scale .1 .15 .02 | savg-rotate 0 -60 0 | savg-translate 0 ");
sprintf(tempString, "%f", (mouthvert[2]-.03));
strcat(outString, tempString);
strcat(outString, " .15 | savg-rgb 1 0 0;");
}

system(outString);

//printf("#the outstring is: %s \n", outString);
/*the MOUTH */
if(!negbend){
printf("#The Mouth\n");
printf("polygons\n");
        printf("0 %f .01 %f 0 0 1\n",(-mouthopen[2]/2+mouthvert[2]), mouthr);
        printf("%f %f 0.01 %f 0 0 1\n",mouthw[2]/6, (-mouthopen[2]/2+mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",mouthw[2]/3, ((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);       
	printf("%f %f .010 %f 0 0 1\n",mouthw[2]/2, (mouthbend[2]+mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",mouthw[2]/3, ((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",mouthw[2]/6, (mouthopen[2]/2+mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/6, (mouthopen[2]/2+mouthvert[2]), mouthr);
	printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/3, ((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);
	printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/2, (mouthbend[2]+mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/3, ((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/6, (-mouthopen[2]/2+mouthvert[2]), mouthr);
        printf("0 %f .010 %f 0 0 1\n",(-mouthopen[2]/2+mouthvert[2]), mouthr);
printf("polygons\n");
        printf("0 %f 0.11 %f 0 0 1\n",(-mouthopen[2]/2+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/6, (-mouthopen[2]/2+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/3, ((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/2, (mouthbend[2]+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/3, ((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/6, (mouthopen[2]/2+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/6, (mouthopen[2]/2+mouthvert[2]), mouthr);
	printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/3, ((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);      
	printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/2, (mouthbend[2]+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/3, ((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/6, (-mouthopen[2]/2+mouthvert[2]), mouthr);
        printf("0 %f 0.11 %f 0 0 1\n",(-mouthopen[2]/2+mouthvert[2]), mouthr);
printf("polygons\n");
	printf("0 %f .010 %f 0 0 1\n",(-mouthopen[2]/2+mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",mouthw[2]/6, (-mouthopen[2]/2+mouthvert[2]), mouthr);
	printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/6, (-mouthopen[2]/2+mouthvert[2]), mouthr);
        printf("0 %f 0.11 %f 0 0 1\n",(-mouthopen[2]/2+mouthvert[2]), mouthr);
printf("polygons\n");
	printf("%f %f .010 %f 0 0 1\n",mouthw[2]/6, (-mouthopen[2]/2+mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",mouthw[2]/3, ((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/3, ((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",(mouthw[2]/6), ((-mouthopen[2]/2)+mouthvert[2]), mouthr);
printf("polygons\n");
	printf("%f %f .010 %f 0 0 1\n",mouthw[2]/3, ((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",mouthw[2]/2, (mouthbend[2]+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/2, (mouthbend[2]+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/3, ((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);
printf("polygons\n");
	printf("%f %f .010 %f 0 0 1\n",mouthw[2]/2, (mouthbend[2]+mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",mouthw[2]/3, ((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/3, ((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/2, (mouthbend[2]+mouthvert[2]), mouthr);
printf("polygons\n");
	printf("%f %f .010 %f 0 0 1\n",mouthw[2]/3, ((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",mouthw[2]/6, (mouthopen[2]/2+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/6, (mouthopen[2]/2+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/3, ((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);
printf("polygons\n");
	printf("%f %f .010 %f 0 0 1\n",mouthw[2]/6, (mouthopen[2]/2+mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/6, (mouthopen[2]/2+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/6, (mouthopen[2]/2+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/6, (mouthopen[2]/2+mouthvert[2]), mouthr);
printf("polygons\n");
	printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/6, (mouthopen[2]/2+mouthvert[2]), mouthr);
	printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/3, ((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);
	printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/3, ((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);      
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/6, (mouthopen[2]/2+mouthvert[2]), mouthr);
printf("polygons\n");
        printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/3, ((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);
	printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/2, (mouthbend[2]+mouthvert[2]), mouthr);
	printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/2, (mouthbend[2]+mouthvert[2]), mouthr);
	printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/3, ((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);      
printf("polygons\n");
        printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/2, (mouthbend[2]+mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/3, ((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/3, ((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);
	printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/2, (mouthbend[2]+mouthvert[2]), mouthr);
printf("polygons\n");
        printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/3, ((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/6, (-mouthopen[2]/2+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/6, (-mouthopen[2]/2+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/3, ((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]), mouthr);
printf("polygons\n");
        printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/6, (-mouthopen[2]/2+mouthvert[2]), mouthr);
        printf("0 %f .01 %f 0 0 1\n",(-mouthopen[2]/2+mouthvert[2]), mouthr);
        printf("0 %f 0.11 %f 0 0 1\n",(-mouthopen[2]/2+mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/6, (-mouthopen[2]/2+mouthvert[2]), mouthr);
printf("\n");
}
else {

printf("polygons\n");
	printf("0 %f .010 %f 0 0 1\n",-(-mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/6, -(-mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/3, -((-mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/2, -(mouthbend[2]-mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/3, -((mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/6, -(mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",mouthw[2]/6, -(mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",mouthw[2]/3, -((mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",mouthw[2]/2, -(mouthbend[2]-mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",mouthw[2]/3, -((-mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",mouthw[2]/6, -(-mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("0 %f .010 %f 0 0 1\n",-(-mouthopen[2]/2-mouthvert[2]), mouthr);

printf("polygons\n");
	printf("0 %f 0.11 %f 0 0 1\n",-(-mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/6, -(-mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/3, -((-mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/2, -(mouthbend[2]-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/3, -((mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/6, -(mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/6, -(mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/3, -((mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/2, -(mouthbend[2]-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/3, -((-mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/6, -(-mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("0 %f 0.11 %f 0 0 1\n",-(-mouthopen[2]/2-mouthvert[2]), mouthr);
      

printf("polygons\n");
       printf("0 %f .010 %f 0 0 1\n",-(-mouthopen[2]/2-mouthvert[2]), mouthr);  
       printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/6, -(-mouthopen[2]/2-mouthvert[2]), mouthr);
	printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/6, -(-mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("0 %f 0.11 %f 0 0 1\n",-(-mouthopen[2]/2-mouthvert[2]), mouthr);
printf("polygons\n");
	printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/6, -(-mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("%f %f 0.01 %f 0 0 1\n",-mouthw[2]/3, -((-mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/3, -((-mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/6, -(-mouthopen[2]/2-mouthvert[2]), mouthr);
printf("polygons\n");
	printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/3, -((-mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/2, -(mouthbend[2]-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/2, -(mouthbend[2]-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/3, -((-mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
printf("polygons\n");
	printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/2, -(mouthbend[2]-mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/3, -((mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/3, -((mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/2, -(mouthbend[2]-mouthvert[2]), mouthr);
printf("polygons\n");
	printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/3, -((mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/6, -(mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/6, -(mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/3, -((mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
printf("polygons\n");
	printf("%f %f .010 %f 0 0 1\n",-mouthw[2]/6, -(mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",mouthw[2]/6, -(mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/6, -(mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",-mouthw[2]/6, -(mouthopen[2]/2-mouthvert[2]), mouthr);
printf("polygons\n");
	printf("%f %f .010 %f 0 0 1\n",mouthw[2]/6, -(mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",mouthw[2]/3, -((mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/3, -((mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/6, -(mouthopen[2]/2-mouthvert[2]), mouthr);
printf("polygons\n");
	printf("%f %f 0.01 %f 0 0 1\n",mouthw[2]/3, -((mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",mouthw[2]/2, -(mouthbend[2]-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/2, -(mouthbend[2]-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/3, -((mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
printf("polygons\n");
	printf("%f %f .010 %f 0 0 1\n",mouthw[2]/2, -(mouthbend[2]-mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",mouthw[2]/3, -((-mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/3, -((-mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/2, -(mouthbend[2]-mouthvert[2]), mouthr);
printf("polygons\n");
	printf("%f %f .010 %f 0 0 1\n",mouthw[2]/3, -((-mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
        printf("%f %f .010 %f 0 0 1\n",mouthw[2]/6, -(-mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/6, -(-mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/3, -((-mouthopen[2]/4+mouthbend[2]/3)-mouthvert[2]), mouthr);
printf("polygons\n");
	printf("%f %f .010 %f 0 0 1\n",mouthw[2]/6, -(-mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("0 %f .010 %f 0 0 1\n",-(-mouthopen[2]/2-mouthvert[2]), mouthr);
        printf("0 %f 0.11 %f 0 0 1\n",-(-mouthopen[2]/2-mouthvert[2]), mouthr);
	printf("%f %f 0.11 %f 0 0 1\n",mouthw[2]/6, -(-mouthopen[2]/2-mouthvert[2]), mouthr);
	
}
printf("#the teeth\n");

if(areteeth[2])
  {
printf("polygons\n");
        printf("0 %f .11 1.0  1.00  1.00 1\n",((.9)*(-mouthopen[2]/2))+mouthvert[2]);
        printf("%f %f 0.11  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/6, ((.9)*-mouthopen[2]/2)+mouthvert[2]);
        printf("%f %f .110   1.0 1.00  1.00 1\n",(.9)*mouthw[2]/3, ((.9)*(-mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);       
	printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/2, ((.9)*mouthbend[2])+mouthvert[2]);
        printf("%f %f .110   1.0 1.00  1.00 1\n",(.9)*mouthw[2]/3, ((.9)*(mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/6, ((.9)*mouthopen[2]/2)+mouthvert[2]);
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/6, ((.9)*mouthopen[2]/2)+mouthvert[2]);
	printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/3, ((.9)*(mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
	printf("%f %f .110 1.0   1.00  1.00 1\n",(.9)*-mouthw[2]/2, ((.9)*mouthbend[2])+mouthvert[2]);
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/3, ((.9)*(-mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/6, ((.9)*-mouthopen[2]/2)+mouthvert[2]);
        printf("0 %f .110  1.0  1.00  1.00 1\n",((.9)*-mouthopen[2]/2)+mouthvert[2]);
printf("polygons\n");
        printf("0 %f .12 1.0  1.00  1.00 1\n",((.9)*(-mouthopen[2]/2))+mouthvert[2]);
        printf("%f %f 0.12  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/6, ((.9)*-mouthopen[2]/2)+mouthvert[2]);
        printf("%f %f .120   1.0 1.00  1.00 1\n",(.9)*mouthw[2]/3, ((.9)*(-mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);       
	printf("%f %f .120  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/2, ((.9)*mouthbend[2])+mouthvert[2]);
        printf("%f %f .120   1.0 1.00  1.00 1\n",(.9)*mouthw[2]/3, ((.9)*(mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
        printf("%f %f .120  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/6, ((.9)*mouthopen[2]/2)+mouthvert[2]);
        printf("%f %f .120  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/6, ((.9)*mouthopen[2]/2)+mouthvert[2]);
	printf("%f %f .120  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/3, ((.9)*(mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
	printf("%f %f .120 1.0   1.00  1.00 1\n",(.9)*-mouthw[2]/2, ((.9)*mouthbend[2])+mouthvert[2]);
        printf("%f %f .120  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/3, ((.9)*(-mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
        printf("%f %f .120  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/6, ((.9)*-mouthopen[2]/2)+mouthvert[2]);
        printf("0 %f .120  1.0  1.00  1.00 1\n",((.9)*-mouthopen[2]/2)+mouthvert[2]);

printf("polygons\n");
        printf("0 %f .11 1.0  1.00  1.00 1\n",((.9)*(-mouthopen[2]/2))+mouthvert[2]);
        printf("%f %f 0.11  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/6, ((.9)*-mouthopen[2]/2)+mouthvert[2]);
        printf("%f %f 0.12  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/6, ((.9)*-mouthopen[2]/2)+mouthvert[2]);
        printf("0 %f .12 1.0  1.00  1.00 1\n",((.9)*(-mouthopen[2]/2))+mouthvert[2]);
printf("polygons\n");
        printf("%f %f 0.11  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/6, ((.9)*-mouthopen[2]/2)+mouthvert[2]);
        printf("%f %f .110   1.0 1.00  1.00 1\n",(.9)*mouthw[2]/3, ((.9)*(-mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
        printf("%f %f .120   1.0 1.00  1.00 1\n",(.9)*mouthw[2]/3, ((.9)*(-mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
        printf("%f %f 0.12  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/6, ((.9)*-mouthopen[2]/2)+mouthvert[2]);
printf("polygons\n");
        printf("%f %f .110   1.0 1.00  1.00 1\n",(.9)*mouthw[2]/3, ((.9)*(-mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/2, ((.9)*mouthbend[2])+mouthvert[2]);
        printf("%f %f .120  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/2, ((.9)*mouthbend[2])+mouthvert[2]);
        printf("%f %f .120   1.0 1.00  1.00 1\n",(.9)*mouthw[2]/3, ((.9)*(-mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
printf("polygons\n"); 
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/2, ((.9)*mouthbend[2])+mouthvert[2]);
        printf("%f %f .110   1.0 1.00  1.00 1\n",(.9)*mouthw[2]/3, ((.9)*(mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
        printf("%f %f .120   1.0 1.00  1.00 1\n",(.9)*mouthw[2]/3, ((.9)*(mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
        printf("%f %f .120  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/2, ((.9)*mouthbend[2])+mouthvert[2]);
printf("polygons\n");
        printf("%f %f .110   1.0 1.00  1.00 1\n",(.9)*mouthw[2]/3, ((.9)*(mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/6, ((.9)*mouthopen[2]/2)+mouthvert[2]);
        printf("%f %f .120  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/6, ((.9)*mouthopen[2]/2)+mouthvert[2]);
        printf("%f %f .120   1.0 1.00  1.00 1\n",(.9)*mouthw[2]/3, ((.9)*(mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
printf("polygons\n");
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/6, ((.9)*mouthopen[2]/2)+mouthvert[2]);
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/6, ((.9)*mouthopen[2]/2)+mouthvert[2]);
        printf("%f %f .120  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/6, ((.9)*mouthopen[2]/2)+mouthvert[2]);
        printf("%f %f .120  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/6, ((.9)*mouthopen[2]/2)+mouthvert[2]);
printf("polygons\n");
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/6, ((.9)*mouthopen[2]/2)+mouthvert[2]);
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/3, ((.9)*(mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
        printf("%f %f .120  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/3, ((.9)*(mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
        printf("%f %f .120  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/6, ((.9)*mouthopen[2]/2)+mouthvert[2]);
printf("polygons\n");
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/3, ((.9)*(mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
        printf("%f %f .110 1.0   1.00  1.00 1\n",(.9)*-mouthw[2]/2, ((.9)*mouthbend[2])+mouthvert[2]);
        printf("%f %f .120 1.0   1.00  1.00 1\n",(.9)*-mouthw[2]/2, ((.9)*mouthbend[2])+mouthvert[2]);
        printf("%f %f .120  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/3, ((.9)*(mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
printf("polygons\n");
        printf("%f %f .110 1.0   1.00  1.00 1\n",(.9)*-mouthw[2]/2, ((.9)*mouthbend[2])+mouthvert[2]);
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/3, ((.9)*(-mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
        printf("%f %f .120  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/3, ((.9)*(-mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
        printf("%f %f .120 1.0   1.00  1.00 1\n",(.9)*-mouthw[2]/2, ((.9)*mouthbend[2])+mouthvert[2]);
printf("polygons\n");
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/3, ((.9)*(-mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/6, ((.9)*-mouthopen[2]/2)+mouthvert[2]);
        printf("%f %f .120  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/6, ((.9)*-mouthopen[2]/2)+mouthvert[2]);
        printf("%f %f .120  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/3, ((.9)*(-mouthopen[2]/4+mouthbend[2]/3))+mouthvert[2]);
printf("polygons\n");
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/6, ((.9)*-mouthopen[2]/2)+mouthvert[2]);
        printf("0 %f .110  1.0  1.00  1.00 1\n",((.9)*-mouthopen[2]/2)+mouthvert[2]);
        printf("0 %f .120  1.0  1.00  1.00 1\n",((.9)*-mouthopen[2]/2)+mouthvert[2]);
        printf("%f %f .120  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/6, ((.9)*-mouthopen[2]/2)+mouthvert[2]);












/*







printf("polygons\n");
        printf("0 %f .125 1.0  1.00  1.00 1\n",((.9)*(-mouthopen[2]/2+mouthvert[2]));
        printf("%f %f 0.125  1.0  1.00  1.00 1\n",((.9)*mouthw[2]/6), ((.9)*(-mouthopen[2]/2+mouthvert[2]));
        printf("%f %f .1250   1.0 1.00  1.00 1\n",((.9)*mouthw[2]/3), ((.9)*((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]));       
	printf("%f %f .1250  1.0  1.00  1.00 1\n",((.9)*mouthw[2]/2), ((.9)*(mouthbend[2]+mouthvert[2]));
        printf("%f %f .1250   1.0 1.00  1.00 1\n",((.9)*mouthw[2]/3), ((.9)*((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]));
        printf("%f %f .1250  1.0  1.00  1.00 1\n",((.9)*mouthw[2]/6), ((.9)*(mouthopen[2]/2+mouthvert[2]));
        printf("%f %f .1250  1.0  1.00  1.00 1\n",((.9)*-mouthw[2]/6), ((.9)*(mouthopen[2]/2+mouthvert[2]));
	printf("%f %f .1250  1.0  1.00  1.00 1\n",((.9)*-mouthw[2]/3), ((.9)*((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]));
	printf("%f %f .1250 1.0   1.00  1.00 1\n",((.9)*-mouthw[2]/2), ((.9)*(mouthbend[2]+mouthvert[2]));
        printf("%f %f .1250  1.0  1.00  1.00 1\n",((.9)*-mouthw[2]/3), ((.9)*((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]));
        printf("%f %f .1250  1.0  1.00  1.00 1\n",((.9)*-mouthw[2]/6), ((.9)*(-mouthopen[2]/2+mouthvert[2]));
        printf("0 %f .1250  1.0  1.00  1.00 1\n",((.9)*(-mouthopen[2]/2+mouthvert[2]));

printf("polygons\n");
	printf("0 %f .11 1.0  1.00  1.00 1\n",(.9)*(-mouthopen[2]/2)+mouthvert[2);
        printf("%f %f 0.11  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/6, (.9)*(-mouthopen[2]/2+mouthvert[2]);
        printf("%f %f 0.125  1.0  1.00  1.00 1\n",((.9)*mouthw[2]/6), (.9)*(-mouthopen[2]/2)+mouthvert[2);
        printf("0 %f .125 1.0  1.00  1.00 1\n",((.9)*(-mouthopen[2]/2+mouthvert[2]));
printf("polygons\n");
        printf("%f %f 0.11  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/6, (.9)*(-mouthopen[2]/2+mouthvert[2]));
        printf("%f %f .110   1.0 1.00  1.00 1\n",(.9)*mouthw[2]/3, (.9)*((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]));
        printf("%f %f .1250   1.0 1.00  1.00 1\n",((.9)*mouthw[2]/3), ((.9)*((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2])));
        printf("%f %f 0.125  1.0  1.00  1.00 1\n",((.9)*mouthw[2]/6), ((.9)*(-mouthopen[2]/2+mouthvert[2])));
printf("polygons\n");
        printf("%f %f .110   1.0 1.00  1.00 1\n",(.9)*mouthw[2]/3, (.9)*((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]));
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/2, (.9)*(mouthbend[2]+mouthvert[2]));
        printf("%f %f .1250  1.0  1.00  1.00 1\n",((.9)*mouthw[2]/2), ((.9)*(mouthbend[2]+mouthvert[2])));
        printf("%f %f .1250   1.0 1.00  1.00 1\n",((.9)*mouthw[2]/3), ((.9)*((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2])));
printf("polygons\n");
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/2, (.9)*(mouthbend[2]+mouthvert[2]));
        printf("%f %f .110   1.0 1.00  1.00 1\n",(.9)*mouthw[2]/3, (.9)*((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]));
        printf("%f %f .1250   1.0 1.00  1.00 1\n",((.9)*mouthw[2]/3), ((.9)*((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2])));
        printf("%f %f .1250  1.0  1.00  1.00 1\n",((.9)*mouthw[2]/2), ((.9)*(mouthbend[2]+mouthvert[2])));
printf("polygons\n");
        printf("%f %f .110   1.0 1.00  1.00 1\n",(.9)*mouthw[2]/3, (.9)*((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]));
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/6, (.9)*(mouthopen[2]/2+mouthvert[2]));
        printf("%f %f .1250  1.0  1.00  1.00 1\n",((.9)*mouthw[2]/6), ((.9)*(mouthopen[2]/2+mouthvert[2])));
        printf("%f %f .1250   1.0 1.00  1.00 1\n",((.9)*mouthw[2]/3), ((.9)*((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2])));
printf("polygons\n");
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*mouthw[2]/6, (.9)*(mouthopen[2]/2+mouthvert[2]));
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/6, (.9)*(mouthopen[2]/2+mouthvert[2]));
        printf("%f %f .1250  1.0  1.00  1.00 1\n",((.9)*-mouthw[2]/6), ((.9)*(mouthopen[2]/2+mouthvert[2])));
        printf("%f %f .1250  1.0  1.00  1.00 1\n",((.9)*mouthw[2]/6), ((.9)*(mouthopen[2]/2+mouthvert[2])));
printf("polygons\n");
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/6, (.9)*(mouthopen[2]/2+mouthvert[2]));
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/3, (.9)*((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]));
        printf("%f %f .1250  1.0  1.00  1.00 1\n",((.9)*-mouthw[2]/3), ((.9)*((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2])));
        printf("%f %f .1250  1.0  1.00  1.00 1\n",((.9)*-mouthw[2]/6), ((.9)*(mouthopen[2]/2+mouthvert[2])));
printf("polygons\n");
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/3, (.9)*((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]));
        printf("%f %f .110 1.0   1.00  1.00 1\n",(.9)*-mouthw[2]/2, (.9)*(mouthbend[2]+mouthvert[2]));
        printf("%f %f .1250 1.0   1.00  1.00 1\n",((.9)*-mouthw[2]/2), ((.9)*(mouthbend[2]+mouthvert[2])));
        printf("%f %f .1250  1.0  1.00  1.00 1\n",((.9)*-mouthw[2]/3), ((.9)*((mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2])));
printf("polygons\n");
	        printf("%f %f .110 1.0   1.00  1.00 1\n",(.9)*-mouthw[2]/2, (.9)*(mouthbend[2]+mouthvert[2]));
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/3, (.9)*((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]));
        printf("%f %f .1250  1.0  1.00  1.00 1\n",((.9)*-mouthw[2]/3), ((.9)*((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2])));
        printf("%f %f .1250 1.0   1.00  1.00 1\n",((.9)*-mouthw[2]/2), ((.9)*(mouthbend[2]+mouthvert[2])));
printf("polygons\n");
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/3, (.9)*((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2]));
        printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/6, (.9)*(-mouthopen[2]/2+mouthvert[2]));
        printf("%f %f .1250  1.0  1.00  1.00 1\n",((.9)*-mouthw[2]/6), ((.9)*(-mouthopen[2]/2+mouthvert[2])));
        printf("%f %f .1250  1.0  1.00  1.00 1\n",((.9)*-mouthw[2]/3), ((.9)*((-mouthopen[2]/4+mouthbend[2]/3)+mouthvert[2])));
printf("polygons\n");
       printf("%f %f .110  1.0  1.00  1.00 1\n",(.9)*-mouthw[2]/6, (.9)*(-mouthopen[2]/2+mouthvert[2]));
        printf("0 %f .110  1.0  1.00  1.00 1\n",(.9)*(-mouthopen[2]/2+mouthvert[2]));
        printf("0 %f .1250  1.0  1.00  1.00 1\n",(.9)*(-mouthopen[2]/2+mouthvert[2]));   
        printf("%f %f .1250  1.0  1.00  1.00 1\n",((.9)*-mouthw[2]/6), ((.9)*(-mouthopen[2]/2+mouthvert[2])));
*/

}	



  
//printf("#the outstring is: %s \n", outString);






}

enum emotion strtoemo(char str[]){
        char fstr[200];
        curface++;
        substr(str,0,3,fstr);
        if (debugger)
                fprintf(stderr,"%s\n",fstr);
        if (strcmp(str, "happy") == 0){
                return happy;
        }

        else if (strcmp(str, "sad") == 0){
                return sad;
        }

        else if (strcmp(str, "angry") == 0){
                return angry;
        }
                
        else if (strcmp(str, "impassive") == 0){
                return impassive;
        }

        else if (strcmp(str, "surprised") == 0){
                return surprised;
        }

        else if (strcmp(str, "dead") == 0){
                return dead;
        }

        else if (strcmp(str, "evil") == 0){
                return evil;
        }

        else if (strcmp(str, "question") == 0){
                return question;
        }
        
        else if (strcmp(str, "foo") == 0){
                return foo;
        }
        
        else if (strcmp(str, "bar") == 0){
                return bar;
        }
        
        else if (strcmp(fstr, "file") == 0){
                substr(str,5,strlen(str)-1,customfile[curface]);
                if (debugger)
                        fprintf(stderr,"%s\n",customfile[curface]);
                return file;
        }

        else
                return error;
}

void substr(char instr[], int first, int last, char outstr[]){
        if (debugger)
                fprintf(stderr,"%s %d %d %d\n",instr,first,last,strlen(instr));
        if (last>first && first<strlen(instr) && last<strlen(instr)){
                int pos;
                for (pos=first; pos<=last;pos++){
                        outstr[pos-first]=instr[pos];
                }
                outstr[last-first+1]='\0';
        }
        else{
                outstr[1]='\n';
        }
}

