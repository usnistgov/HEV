/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2003  Virginia Tech
 * 
 * This is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License (GPL) as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software, in the top level source directory in a file
 * named "COPYING.GPL"; if not, see it at:
 * http://www.gnu.org/copyleft/gpl.html or write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 */
/*********************************************************************
 This is a very simple IS900 DTK tracker service.  It just uses this
 source file and configure.h to build and configure it.
*********************************************************************/

// This just does a is900 head tracker and a is900 tracked wand 
// is900 tracker devices service use many Polhemus protocols.
// dtk-valueInputs cal -s 3 180 -180 0 -s 4 180 -180 0 -s 5 180 -180 0 -l x y z h p r

#include "dtk/_config.h"

#ifdef DTK_ARCH_LINUX

#  define SWAPBYTES     0             /* 0 or 1 = no or yes */
#  define DEVICE_FILE  "/dev/ttyS0"   /* change me */

#else  /* IRIX or other like. */

#  define SWAPBYTES     1             /* 0 or 1 = no or yes */
#  define DEVICE_FILE  "/dev/ttyd10"  /* change me */

#endif


#include <stdio.h> 
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <dtk.h>
#include <dtk/dtkService.h>

// Polhemus station numbers start at 1 my array indexes start at 0 and
// are Polhemus station numbers minus 1.  If you change NUM_STATIONS
// you will need to change (add to or remove) a lot of this code.

#define NUM_STATIONS  2   /* number of stations (trackers) read */


/******* device initializaton string ********/

#define HEAD_RECORDLEN ((size_t) 29)
#define WAND_RECORDLEN ((size_t) 32)

static char setupstring[] =
"f\n"                  // binary output
"u\n"                  // metric units
"l1,1\n"               // tracker 1 on
"l2,1\n"               // tracker 2 on
"l3,0\n"               // tracker 3 off
"l4,0\n"               // tracker 4 off
"R1\n"
"R2\n"
// Alignment Reference Frame oriention = switch x to y and y to x
"A1,0,0,0,0,-1,0,1,0,0\n" // Alignment Reference Frame oriention
"A2,0,0,0,0,-1,0,1,0,0\n" // Alignment Reference Frame oriention
"G1,0,0,0\n"
"G2,0,0,0\n"
"O1,2,4,1\n"           // set tracker 1 "head" output configuration 
"O2,2,4,22,23,1\n"     // set tracker 2 "wand" output configuration
//"ML0\n"                // turn off blue LEDs
"SC\n";                // set continuous spew

#define BUFFER_SIZE   ((size_t) 1023) /* read buffer size */


// #define CALIB
class IS900 : public dtkService
{
public:
  IS900(void);
  virtual ~IS900(void);
  int serve(void);
private:
  dtkSharedMem *tracker_shm[NUM_STATIONS];
  dtkSharedMem *buttons_shm;
  dtkSharedMem *joystick_shm;
#ifdef CALIBRATE
  dtkSharedMem *cal_shm;
  float offset[6];
#endif

  float state[NUM_STATIONS][6];

  unsigned char buffer[BUFFER_SIZE];
  unsigned char *ptr, *end;

  void clean_up(void);

  dtkVRCOSharedMem VRCO;

  //dtkMatrix rotHead;
  dtkMatrix transHead;
};




#if (SWAPBYTES)
static float getFloat(unsigned char * b)
{
  float *fp;
  char   s[4];
  s[3] = b[0];
  s[2] = b[1];
  s[1] = b[2];
  s[0] = b[3];
  fp = (float*)(s);
  return(*fp);
}
#else
#  define getFloat(x)  (*(float *)(x))
#endif


void IS900::clean_up(void)
{
  if(fd != -1)
    {
      close(fd);
      fd = -1;
    }
  end = ptr = buffer;

  for(int i=0;i<NUM_STATIONS;i++)
    if(tracker_shm[i])
      {
	delete tracker_shm[i];
	tracker_shm[i] = NULL;
      }
  if(buttons_shm)
    {
      delete buttons_shm;
      buttons_shm = NULL;
    }
  if(joystick_shm)
    {
      delete joystick_shm;
      joystick_shm = NULL;
    }

#ifdef CALIBRATE
  if(cal_shm)
    {
      delete cal_shm;
      cal_shm = NULL;
    }
#endif
}


IS900::IS900(void) 
{
  /****************** initialize all object data *****************/
  ptr = end = buffer;
  buttons_shm = NULL;
  joystick_shm = NULL;

#ifdef CALIBRATE
  cal_shm = NULL;
#endif


  int i;

  for(i=0;i<NUM_STATIONS;i++)
    {
      tracker_shm[i] = NULL;
      state[i][0] = 0.0f;
      state[i][1] = 0.0f;
      state[i][2] = 0.0f;
      state[i][3] = 0.0f;
      state[i][4] = 0.0f;
      state[i][5] = 0.0f;
    }

  if(VRCO.isInvalid()) return; // error

  /******************* setup device file for **********************/

  fd = open(DEVICE_FILE,  O_RDWR|O_NDELAY);
  if (fd < 0) 
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "IS900::IS900()"
		 " failed: open(\"%s%s%s\","
		 "O_RDWR|O_NDELAY) failed.\n",
		 dtkMsg.color.tur, DEVICE_FILE,
		 dtkMsg.color.end);
      return;
    }
  
  /* deep voodoo */ 
  struct termios terminfo;

  terminfo.c_iflag = 0;
  terminfo.c_oflag = 0;
  terminfo.c_lflag = 0;
#ifdef DTK_ARCH_IRIX
  terminfo.c_cflag = CS8|CREAD|CLOCAL|CNEW_RTSCTS;
  terminfo.c_ispeed = terminfo.c_ospeed = B38400;
#else /* Linux and what */
  terminfo.c_cflag = CS8|CREAD|CLOCAL;
  cfsetospeed(&terminfo, B115200);
  cfsetispeed(&terminfo, B115200);
#endif

  for (i=0; i<NCCS; i++)
    terminfo.c_cc[i] = 0;

  if(tcsetattr(fd, TCSANOW, &terminfo ) == -1)
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "IS900::IS900() failed: "
		 "tcsetattr(%d,TCSANOW) failed "
		 "for device file %s%s%s.",
		 fd, dtkMsg.color.tur, DEVICE_FILE,
		 dtkMsg.color.end);
      clean_up();
      return;
    }

  if(tcflush(fd, TCIOFLUSH) == -1)
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "IS900::IS900() failed: "
		 "tcsetattr(%d,TCIOFLUSH) failed "
		 "for device file %s%s%s.",
		 fd, dtkMsg.color.tur, DEVICE_FILE,
		 dtkMsg.color.end);
      clean_up();
      return;
    }


  i = write(fd, setupstring, strlen(setupstring));
  if ((size_t) i != strlen(setupstring))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "IS900::IS900() failed: "
		 "error sending configuration "
		 "string to tracker:\n"
		 "write(fd=%d,,) to device file %s%s%s failed.",
		 fd, dtkMsg.color.tur, DEVICE_FILE,
		 dtkMsg.color.end);
      clean_up();
      return;
    }

  /*************** get DTK shared memory *********************/


  tracker_shm[0] = new dtkSharedMem(6*sizeof(float), "head");
  tracker_shm[1] = new dtkSharedMem(6*sizeof(float), "wand");
  buttons_shm = new dtkSharedMem(sizeof(unsigned char), "buttons");
  joystick_shm = new dtkSharedMem(2*sizeof(float), "joystick");

#ifdef CALIBRATE
  cal_shm = new dtkSharedMem(6*sizeof(float), "cal");
#endif

  if(tracker_shm[0]->isInvalid() || tracker_shm[1]->isInvalid() ||
     buttons_shm->isInvalid() || joystick_shm->isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "IS900::IS900() failed:"
		 " failed to get DTK shared memory.\n");
      clean_up();
      return;
    }

  // Translate the point that is being tracked can be displaced a
  // little from the tracker for calibration.
  transHead.translate(0.0f,-0.06f,-0.035f);

  return;
}


/**********************************************************************
  read a record from the tracker.  returns 1 if data updated,
  0 if no change, and -1 if an error
 **********************************************************************/

#define CHUCK  ((size_t) 120)  /* about the largest buffer read size */

/* data record offsets */

#define X 3
#define Y 7
#define Z 11


#define H 15 /* heading */
#define P 19 /* pitch */
#define R 23 /* roll */

#define BUTTONS 27
#define JOYSTICK 28


int IS900::serve(void)
{
  // end  points just past the end of the data read
  // ptr  points to the data not processed yet

  // check for buffer memory wrapping
  if( BUFFER_SIZE - (size_t) (end - buffer) < CHUCK)
    {
      size_t cpy_size =
        (CHUCK < (size_t) (end - ptr))?CHUCK:(size_t) (end - ptr);
      memcpy(buffer, ptr, cpy_size);
      end = buffer + cpy_size;
      ptr = buffer;
    }

  // read to end of data in the buffer
  int bytes_read = read(fd, end, BUFFER_SIZE - (size_t)(end - buffer));
  
  if (bytes_read < 0)
    {
      dtkMsg.add(DTKMSG_WARN, 1,
                 "tracker::serve(): "
                 "read(fd=%d,,) from device file %s%s%s failed.",
		 fd, dtkMsg.color.tur, DEVICE_FILE,
		 dtkMsg.color.end);
      return DTKSERVICE_CONTINUE;
    }
  else if(bytes_read == 0)
    {
      dtkMsg.add(DTKMSG_WARN,
                 "tracker::serve(): "
                 "read() returned 0.\n");
      return DTKSERVICE_CONTINUE;
    }
  else
    end += bytes_read;

#if(0)
  {
    unsigned char *p = ptr;
    while( (size_t)(end - p) > (size_t) 0)
      {
	printf("%3.3o ",*p);
	p++;
      }
    fflush(stdout);
  }
#endif

  /* look for data records */

  for(;ptr <= end - HEAD_RECORDLEN; ptr++)
    {
      int j; // station number starting at 0

      if(
	 *ptr == '0'
	 && *(ptr+2) == ' '
	 && (j = ((*(ptr+1)-'0')-1)) > -1
	 && j < NUM_STATIONS
	 )
	{
	  // HEAD
	  if(
	     *(ptr+27) == '\r'
	     && *(ptr+28) == '\n'
	     && j == 0
	     )
	    {
	      /** EDIT * SCALING for head tracker **/
	      /******** change shift and offsets *******/

//	      state[j][0] = (getFloat(&ptr[Y]) - 15.522769f)/152.4f;
//	      state[j][1] = (getFloat(&ptr[X]) - 342.853088f)/152.4f;
//	      state[j][2] = -(getFloat(&ptr[Z]) + 164.330032f)/152.4f;
	      state[j][0] = (getFloat(&ptr[Y]) - 0.0f)/121.92f;
	      state[j][1] = (getFloat(&ptr[X]) - 26.0f)/121.92f;
	      state[j][2] = -(getFloat(&ptr[Z]) + 133.12)/121.92f;
	      state[j][3] = -getFloat(&ptr[H]);
	      state[j][4] =  getFloat(&ptr[P]);
	      state[j][5] =  getFloat(&ptr[R]);

	      /** END_EDIT **/
#ifdef CALIBRATE
	      ////////////////////////////////////////////////
	      // used to calibrate the rotation point offset

	      cal_shm->read(offset);

	      // Adjust translation offsets

	      transHead.identity();
	      transHead.translate(offset[0],offset[1],offset[2]);


	      ////////////////////////////////////////////////
#endif

              dtkMatrix tmat;
	      tmat.copy(&transHead);
	      tmat.rotateHPR(state[j][3],state[j][4],state[j][5]);

              tmat.translate(state[j][0],state[j][1],state[j][2]);
              tmat.translate(&state[j][0],&state[j][1],&state[j][2]);

	      tracker_shm[j]->write(state[j]);

	      VRCO.writeHeadTracker((const float *) state[j]);

#if(0) /* debugging print */
	      printf("\n");
	      printf("                                       "
		     "%+2.2f %+2.2f %+2.2f  \r",
		     state[j][3], state[j][4], state[j][5]);
	      printf("               %+2.2f %+2.2f %+2.2f\r",
		     state[j][0], state[j][1], state[j][2]);
	      printf("got station(%d)\n",j);
	      fflush(stdout);
#endif
	      ptr += HEAD_RECORDLEN -1; // ptr++ above
	    }

	  // WAND
	  else if(
		  ptr <= end - WAND_RECORDLEN
		  && *(ptr+30) == '\r'
		  && *(ptr+31) == '\n'
		  && j == 1
		  )
	    {
	      /** EDIT * SCALING for wand tracker **/
	      /******** change shift and offsets *******/

//	      state[j][0] = (getFloat(&ptr[Y]) - 17.608776f)/152.4f;
//	      state[j][1] = (getFloat(&ptr[X]) - 344.141479f)/152.4f;
//	      state[j][2] = -(getFloat(&ptr[Z]) - 159.391495f)/152.4f;
	      state[j][0] = (getFloat(&ptr[Y]) - 3.81f)/121.92f;
	      state[j][1] = (getFloat(&ptr[X]) - 26.0f)/121.92f;
	      state[j][2] = -(getFloat(&ptr[Z]) + 133.12)/121.92f;
	      state[j][3] = -getFloat(&ptr[H]);
	      state[j][4] =  getFloat(&ptr[P]);
	      state[j][5] =  getFloat(&ptr[R]);
	      
	      /** END_EDIT **/
	      
	      unsigned char buttons = ptr[BUTTONS];
	      buttons =
            ((buttons & 0x01) << 1) |
            ((buttons & 0x02) >> 1) |
            ((buttons & 0x04) << 0) |
            ((buttons & 0x08) << 0) |
            ((buttons & 0x010) << 0) |
            ((buttons & 0x020) << 0);

	      float joystick[2] = 
	      { ptr[JOYSTICK] - 127.0f, ptr[JOYSTICK+1] - 127.0f };
	      if(joystick[0] > 0.0f) joystick[0] /= 128.0f;
	      else joystick[0] /= 127.0f;
	      if(joystick[1] > 0.0f) joystick[1] /= 128.0f;
	      else joystick[1] /= 127.0f;
	      
	      if(buttons_shm->write(&buttons) ||
		 joystick_shm->write(joystick) ||
		 tracker_shm[j]->write(state[j]))
		// Failed to write to shared memory, your hosed.
		return DTKSERVICE_ERROR;

	      VRCO.writeWand((const float *) state[j],
			     (const float *) joystick,
			     buttons);

#if(0) /* debugging print */
	      printf("\n");
	      printf("                                       "
		     "                                       "
		     "0%3.3o\r",buttons);
	      printf("                                       "
		     "%+2.2f %+2.2f %+2.2f  \r",
		     state[j][3], state[j][4], state[j][5]);
	      printf("               %+2.2f %+2.2f %+2.2f\r",
		     state[j][0], state[j][1], state[j][2]);
	      printf("got station(%d)\n",j);
	      fflush(stdout);
#endif
	      ptr += WAND_RECORDLEN -1; // ptr++ above
	    }
	}
    }

  return DTKSERVICE_CONTINUE;
}

IS900::~IS900(void)
{
  clean_up();
}


static dtkService *dtkDSO_loader(const char *dummy)
{
  return new IS900;
}

static int dtkDSO_unloader(dtkService *is900)
{
  delete is900;
  return DTKDSO_UNLOAD_CONTINUE;
}
