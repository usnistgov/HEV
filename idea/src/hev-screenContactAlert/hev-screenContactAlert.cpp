/*
 * hev-screenContactAlert
 *
 * This code is mostly copied from hev-readFloats.cpp and modified
 *
 * Steve Satterfield
 *
 * October 31 2014
 *
 * 1.94 units / 92 inches => .021 units/inch
 * .021 units/inch * 30 inches => .630 units
 *
 */


#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <dtk.h>
#include <iris.h>

//float stayBackDistance=.63;


/*
void print(char *id, float *data, size_t size)
{
    printf("%s:",id);
    for (unsigned int i=0; i<size/sizeof(float); i++)
    {
      printf("%+g ",*(data+i)) ;
    }
    printf("\n") ;
}
*/



int main(int argc, char **argv) 
{
    
    // send dtkMsg files to stderr
    dtkMsg.setFile(stderr) ;

    bool discardDups = true ;
    int ticks = iris::GetUsleep() ;

    char *iris_control_fifo;
    FILE *icf;

    float wandX, wandY, wandZ, wandH, wandP, wandR;
    float headX, headY, headZ, headH, headP, headR;

    char shmName[8], shmName2[8];

    int stopSignFlag = 0;
    float xLimit, yLimit;

    int audioFlag=1,  altAudioFileFlag=0;
    char *altAudioFile, *altAudioCmd;

    // Test for immediate exit
    if (getenv("SCREEN_CONTACT_ALERT_NOP") != NULL) {
      if (strcmp(getenv("SCREEN_CONTACT_ALERT_NOP"), "1") == 0) {
	//fprintf(stderr, "SCREEN_CONTACT_ALERT_NOP set to 1, exiting\n");
	exit(0);
      }
    }
    //fprintf(stderr, "SCREEN_CONTACT_ALERT_NOP not set\n");

    if ((iris_control_fifo=getenv("IRIS_CONTROL_FIFO")) == NULL) {
      dtkMsg.add(DTKMSG_ERROR, "%s: HEV environment not set\n", argv[0]);
      exit(0);
    }

    if ( ! (icf=fopen(iris_control_fifo, "w"))) {
      dtkMsg.add(DTKMSG_ERROR, "%s: Can't open IRIS confrol fifo\n",argv[0]);
      exit(0);
    }


    if (getenv("SCREEN_CONTACT_ALERT_AUDIO") != NULL) {
      if (strcmp(getenv("SCREEN_CONTACT_ALERT_AUDIO"), "0") == 0) {
	audioFlag=0; // disable audio
      }
    }

    if (getenv("SCREEN_CONTACT_ALERT_AFILE") != NULL) {
      altAudioFile=getenv("SCREEN_CONTACT_ALERT_AFILE");
      if ((altAudioCmd = (char *)malloc(strlen(altAudioFile)+20)) == NULL) {
	dtkMsg.add(DTKMSG_ERROR,"%s: malloc failed\n",argv[0]);
      }
      sprintf(altAudioCmd, "aplay -q %s &", altAudioFile);
      altAudioFileFlag=1;
    }



    // Set limits
    //xLimit = -1+stayBackDistance;
    //yLimit = 1-stayBackDistance;

    xLimit = -.8;   // Hard code for CAVE due to tracker error
    yLimit = .5;

    if (getenv("IRISFLY_IMMERSIVE_ENV") != NULL) {
      if (strcmp(getenv("IRISFLY_IMMERSIVE_ENV"), "powerwall") == 0) {
	xLimit = -1000;    // for PowerWall, no left screen
	yLimit = .5;
      }
    }

    
    // Load stop sign into scene graph
    system("echo LOAD StopSigns ${HEVROOT}/idea/etc/hev-screenContactAlert/data/stopSignModel.osgb > $IRIS_CONTROL_FIFO");
    system("cat ${HEVROOT}/idea/etc/hev-screenContactAlert/data/stopSign.iris > $IRIS_CONTROL_FIFO");


    // Setup to read eand and head

    snprintf(shmName, 5, "wand");
    //fprintf(stderr, "shmName = %s\n",shmName) ;
    
    snprintf(shmName2, 5, "head");
    //fprintf(stderr, "shmName = %s\n",shmName2) ;
    
    //fprintf(stderr,"xLimit = %g  yLimit = %g\n",xLimit,yLimit);

    dtkSharedMem *shm = new dtkSharedMem(shmName, DTK_CONNECT) ;
    if(shm->isInvalid()) return 1 ;

    dtkSharedMem *shm2 = new dtkSharedMem(shmName2, DTK_CONNECT) ;
    if(shm->isInvalid()) return 1 ;

    size_t size = shm->getSize() ;
    size_t size2 = shm->getSize() ;

    float *data = (float*) malloc(size) ;
    float *oldData ;
    if (discardDups) oldData = (float*) malloc(size) ;
    float *data2 = (float*) malloc(size) ;
    float *oldData2 ;
    if (discardDups) oldData2 = (float*) malloc(size) ;
    
    shm->queue(256) ;
    shm2->queue(256) ;

    // you might want to play with this and 
    // see if you want to flush old data or not
    shm->flush() ;
    shm2->flush() ;
    
    while (1)
    {
	if (shm->qread(data)>0)
	{
	  if (!discardDups || memcmp(oldData, data, size))
	    {
	      //print(shmName, data, size) ;
	      wandX=data[0];  wandY=data[1];  wandZ=data[2];  
	      wandH=data[3];  wandP=data[4];  wandR=data[5];  
	      //fprintf(stdout, "wand XYZ: %f %f %f    wand HPR: %f %f %f\n", 
	      //      wandX, wandY, wandZ, wandH, wandP, wandR);
	      //fflush(stdout) ;

	      if (discardDups) memcpy(oldData, data, size) ;
	    }
	}


	if (shm2->qread(data2)>0)
	{
	  if (!discardDups || memcmp(oldData2, data2, size))
	    {
	      //print(shmName2, data2, size) ;
	      headX=data2[0];  headY=data2[1];  headZ=data2[2];  
	      headH=data2[3];  headP=data2[4];  headR=data2[5];  
	      //fprintf(stdout, "head XYZ: %f %f %f    head HPR: %f %f %f\n\n", 
	      //      headX, headY, headZ, headH, headP, headR);
	      //fflush(stdout) ;
	      if (discardDups) memcpy(oldData2, data2, size) ;
	    }
	}

	//fprintf(stderr, "wandX=%g xLimit=%g  wandY=%g yLimit=%g\n",
	//	wandX, xLimit, wandY, yLimit);

	if (((wandX < xLimit) && (wandY > -1.5)) ||
            ((headX < xLimit) && (headY > -1.5)) ||
	    (wandY > yLimit) || (headY > yLimit)) {

	  //fprintf(stdout, "Screen Contact Alert!!\n");
	  //fflush(stdout);


	  if (stopSignFlag == 0) {
	    if (audioFlag) {
	      if (altAudioFileFlag) {
		system(altAudioCmd);
	      } else {
		system("aplay -q ${HEVROOT}/idea/etc/hev-screenContactAlert/data/starTrekCom.wav &");
	      }
	    }

	    //system("echo NODEMASK StopSigns ON > $IRIS_CONTROL_FIFO");
	    fprintf(icf, "NODEMASK StopSigns ON\n");
	    fflush(icf);
	    stopSignFlag = 1;
	  }

	} else {
	  if (stopSignFlag == 1) {
	    //system("echo NODEMASK StopSigns OFF > $IRIS_CONTROL_FIFO");
	    fprintf(icf, "NODEMASK StopSigns OFF\n");
	    fflush(icf);
	    stopSignFlag = 0;
	  }
	}



	usleep(ticks) ;
    }
}

