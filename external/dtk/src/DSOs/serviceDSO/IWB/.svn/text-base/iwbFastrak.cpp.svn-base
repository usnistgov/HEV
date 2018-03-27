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
/* Polhemus 3space Fastrak tracker -- simple DTK service */

/* This is the only file to edit to configure this DTK service. */

/* This works on the Polhemus Fastrak system Immersive Work Bench at
 * Virginia Tech.  1 diverse unit = 36 inches ("=inch) 2.54 cm = inch
 * The origin is 36 inches from the floor, centered 18 inches into the
 * desk.
 */

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

#ifdef DTK_ARCH_LINUX

#  define SWAPBYTES     0             /* 0 or 1 = no or yes */
#  define DEVICE_FILE  "/dev/ttyS0"   /* change me */

#else  /* IRIX or other like. */

#  define SWAPBYTES     1             /* 0 or 1 = no or yes */
#  define DEVICE_FILE  "/dev/ttyd2"   /* change me */

#endif


// Polhemus station numbers start at 1 my array indexes start at 0 and
// are Polhemus station numbers minus 1.  If you change NUM_STATIONS
// you will need to change (add to or remove) a lot of this code.

#define NUM_STATIONS  4       /* number of stations (trackers) read */


/******* device initializaton string ********/
// see your 3space Fastrak Polhemus user's manual for more info on the
// device initializaton string.  I had Revision F dated November 1993.

static char setupstring[] =
"f\n"           // binary output
"u\n"           // metric units
"l1,1\n"        // tracker 1 on
"l2,1\n"        // tracker 2 on
"l3,1\n"        // tracker 3 on
"l4,1\n"        // tracker 4 on
"R1\n"          // tracker 1 reset the alignment reference
"R2\n"          // tracker 2 reset the alignment reference
"R3\n"          // tracker 3 reset the alignment reference
"R4\n"          // tracker 4 reset the alignment reference
"H1,0,0,1\n"    // direction of the operational hemisphere
"H2,0,0,1\n"    // direction of the operational hemisphere
"H3,0,0,1\n"    // direction of the operational hemisphere
"H4,0,0,1\n"    // direction of the operational hemisphere
"O1,2,4,1\n"    // set tracker 1 output configuration
"O2,2,4,1\n"    // set tracker 2 output configuration
"O3,2,4,1\n"    // set tracker 3 output configuration
"O4,2,4,1\n"    // set tracker 4 output configuration
"b1\n"          // set tracker 1 oriention zero to default
"b2\n"          // set tracker 2 oriention zero to default
"b3\n"          // set tracker 3 oriention zero to default
"b4\n"          // set tracker 4 oriention zero to default
"SC\n";         // set continuous spew

static const char *name[] = { "wand", "head", "eye", "stylus", NULL};

/************ Calibration Constants *****************/

static float shift[NUM_STATIONS][6] =
{
  //  x     y     z        H     P     R
  { 0.6f, 62.1f, 10.55f, 90.0f, 0.0f, 180.0f }, // 0 wand
  { 0.0f, 62.0f, 11.22f, 90.0f, 0.0f, 180.0f }, // 1 head
  { 0.0f, 62.1f, 10.55f, 90.0f, 0.0f, 180.0f }, // 2 eye
  { 0.0f, 62.1f, 10.55f, 90.0f, 0.0f, 180.0f }  // 3 stylus
};

static float scale[NUM_STATIONS][6] =
{
  //  x     y     z
  { 92.0f, 92.0f, 92.0f }, // 0 wand
  { 92.0f, 92.0f, 92.0f }, // 1 head
  { 92.0f, 92.0f, 92.0f }, // 2 eye
  { 92.0f, 92.0f, 92.0f }  // 3 stylus
};


#define BUFFER_SIZE   ((size_t) 1023) /* read buffer size */


class Fastrak : public dtkService
{
public:
  Fastrak(void);
  ~Fastrak(void);
  int serve(void);
private:
  dtkSharedMem *shm[NUM_STATIONS];
  float state[NUM_STATIONS][6];

  unsigned char buffer[BUFFER_SIZE];
  unsigned char *ptr, *end;

  void clean_up(void);

  dtkMatrix trans[NUM_STATIONS];
  dtkMatrix rot[NUM_STATIONS];
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


void Fastrak::clean_up(void)
{
  if(fd != -1)
    {
      close(fd);
      fd = -1;
    }

  for(int i=0;i<NUM_STATIONS;i++)
    {
      if(shm[i])
	{
	  delete shm[i];
	  shm[i] = NULL;
	}
    } 

  end = ptr = buffer;
}


Fastrak::Fastrak(void)
{
  /****************** initialize all object data *****************/
  ptr = end = buffer;

  int i;

  for(i=0;i<NUM_STATIONS;i++)
    {
      shm[i] = NULL;
      state[i][0] = 0.0f;
      state[i][1] = 0.0f;
      state[i][2] = 0.0f;
      state[i][3] = 0.0f;
      state[i][4] = 0.0f;
      state[i][5] = 0.0f;
    }

  /******************* setup device file for **********************/

  fd = open(DEVICE_FILE,  O_RDWR|O_NDELAY);
  if (fd < 0)
    {
      dtkMsg.add(DTKMSG_ERROR, 1 /* sys err */,
		 "Fastrak::Fastrak()"
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
      dtkMsg.add(DTKMSG_ERROR, 1 /* sys err */,
		 "Fastrak::Fastrak() failed: "
		 "tcsetattr(%d,TCSANOW) failed "
		 "for device file %s%s%s.",
		 fd, dtkMsg.color.tur, DEVICE_FILE,
		 dtkMsg.color.end);
      clean_up();
      return;
    }

  if(tcflush(fd, TCIOFLUSH) == -1)
    {
      dtkMsg.add(DTKMSG_ERROR, 1 /* sys err */,
		 "Fastrak::Fastrak() failed: "
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
      dtkMsg.add(DTKMSG_ERROR, 1 /* sys err */,
		 "Fastrak::Fastrak() failed: "
		 "error sending configuration "
		 "string to tracker:\n"
		 "write(fd=%d,,) to device file %s%s%s failed.",
		 fd, dtkMsg.color.tur, DEVICE_FILE,
		 dtkMsg.color.end);
      clean_up();
      return;
    }

  /*************** get DTK shared memory *********************/


  for(i=0;i<NUM_STATIONS;i++)
    {
      shm[i] = new dtkSharedMem(6*sizeof(float), name[i]);
      if(shm[i]->isInvalid())
	{
	  dtkMsg.add(DTKMSG_ERROR,
		     "Fastrak::Fastrak() failed:"
		     " failed to get DTK shared memory.\n");
	  clean_up();
	  return;
	}
    }

  /*************** more calibration stuff ***************/

  // wand

  rot[0].rotateHPR(-1.376884f,21.992085f,-5.267059f);
  rot[0].invert();
  trans[0].translate(0.0f, 0.0f, 0.0f);

  // head

  rot[1].rotateHPR(0.0f,0.0f, -90.0f);
  rot[1].invert();
  trans[1].translate(3.5f/36.0f, 2.5f/36.0f, -0.5/36.0f);

  // eye

  rot[2].rotateHPR(0.0f,0.0f, -90.0f);
  rot[2].invert();
  trans[2].translate(3.5f/36.0f, 2.5f/36.0f, -0.75/36.0f);

  // stylus

  rot[3].rotateHPR(0.0f,0.0f,0.0f);
  rot[3].invert();
  trans[3].translate(0.0f, 0.0f, 0.0f);

}


/**********************************************************************
  read a record from the tracker.  returns 1 if data updated,
  0 if no change, and -1 if an error
 **********************************************************************/

#define CHUCK  ((size_t) 120)  /* about the largest buffer read size */

/* data record offsets */
#define RECORDLEN ((size_t) 29)
#define X 3
#define Y 7
#define Z 11

#define H 15 /* heading */
#define P 19 /* pitch */
#define R 23 /* roll */


int Fastrak::serve(void)
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
      dtkMsg.add(DTKMSG_WARN, 1 /* sys err */,
                 "tracker::serve(): "
                 "read(fd=%d,,) from device file %s%s%s failed",
		 fd, dtkMsg.color.tur, DEVICE_FILE,
		 dtkMsg.color.end);
      return DTKSERVICE_UNLOAD; // fatal error
    }
  else if(bytes_read == 0)
    {
      dtkMsg.add(DTKMSG_WARN,
                 "tracker::serve(): "
                 "read() returned 0 from device file %s%s%s"
#ifdef DTK_ARCH_IRIX
		 ",\n ??REBOOT YOUR SGI TO FIX YOUR SERIAL DRIVERS??"
#endif
		 ".\n",
		 dtkMsg.color.tur, DEVICE_FILE,
		 dtkMsg.color.end);
      return DTKSERVICE_CONTINUE; // error, not fatal
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
  for(;ptr <= end - RECORDLEN;ptr++)
    {
      int j; // station number starting at 0
      if(
	 (*ptr == '0') &&
	 (*(ptr+2) == ' ') &&
	 (*(ptr+27) == '\r') &&
	 (*(ptr+28) == '\n') &&
	 (j = ((*(ptr+1)-'0')-1)) > -1 &&
	 j < NUM_STATIONS &&
	 j >= 0
	 )
	{
	  /** EDIT * SCALING for head tracker **/
	  /******** change shift and offsets *******/
	  
	  state[j][0] = (getFloat(&ptr[X]) - shift[j][0])/scale[j][0];//x
	  state[j][1] = (getFloat(&ptr[Y]) - shift[j][1])/scale[j][1];//y
	  state[j][2] = (getFloat(&ptr[Z]) - shift[j][2])/scale[j][2];//z
	  state[j][3] =  getFloat(&ptr[H]) - shift[j][3];//H
	  state[j][4] = -getFloat(&ptr[P]) - shift[j][4];//P
	  state[j][5] =  getFloat(&ptr[R]) - shift[j][5];//R

	  dtkMatrix m;
	  m.mult(&rot[j]);
	  m.rotateHPR(state[j][3],state[j][4],state[j][5]);
	  m.rotateHPR(&state[j][3],&state[j][4],&state[j][5]);

	  dtkMatrix tmat;
	  tmat.copy(&trans[j]);
	  tmat.rotateHPR(state[j][3],state[j][4],state[j][5]);

	  tmat.translate(state[j][0],state[j][1],state[j][2]);
	  tmat.translate(&state[j][0],&state[j][1],&state[j][2]);

	  /** END_EDIT **/
	  if(shm[j]->write(state[j]))
	    return DTKSERVICE_UNLOAD; // error

#if(0) /* debugging print */
	  printf("\ngot station(%d) %2.2f %2.2f %2.2f   "
		 "%2.2f %2.2f %2.2f\n",
		 j,
		 state[j][0], state[j][1], state[j][2],
		 state[j][3], state[j][4], state[j][5]);
#endif
	}
    }
  
  return DTKSERVICE_CONTINUE;
}


Fastrak::~Fastrak(void)
{
  clean_up();
}


static dtkService *dtkDSO_loader(const char *dummy)
{
  return new Fastrak;
}

static int dtkDSO_unloader(dtkService *f)
{
  delete f;
  return DTKDSO_UNLOAD_CONTINUE; // success
}
