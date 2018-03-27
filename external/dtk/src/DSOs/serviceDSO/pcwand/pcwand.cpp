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
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <dtk.h>
#include <dtk/dtkService.h>

#include "config.h"

#define READ_BUFFER_SIZE        ((size_t) 128)
#define READ_BUFFER_SIZE_M2     ((size_t) READ_BUFFER_SIZE-2)


/* server_dso_path is extern from `../../server/dtk-server.C'.  Look
 * at but don't touch server_dso_path. */
extern char *server_dso_path;

class PCWand : public dtkService
{
public:
  PCWand(void);
  virtual ~PCWand(void);
  int serve();

private:

  void clean_up(void);

  dtkSharedMem *button_shm;
  dtkSharedMem *joystick_shm;

  float joystick_state[2];
  unsigned char button_state;
#ifdef TRACKD_COMPATABLE 
  dtkVRCOSharedMem VRCO;
#endif
  int termio_set_flag;
  struct termios original_terminfo;

  unsigned char read_buffer[READ_BUFFER_SIZE];
  // end_ptr points to somewhere in read_buffer.
  unsigned char *end_ptr;

};


void PCWand::clean_up(void)
{
  if(button_shm)
    {
      delete button_shm;
      button_shm = NULL;
    }

  if(joystick_shm)
    {
      delete joystick_shm;
      joystick_shm = NULL;
    }

  if(fd > -1)
    {
      if(termio_set_flag)
	{
	  termio_set_flag = 0;
	  if (tcsetattr(fd, TCSANOW, &original_terminfo) == -1)
	    dtkMsg.add(DTKMSG_WARN, 1,
		       "PCWand::~PCWand() failed: failed "
		       "to reset serial device"
		       " termio settings: "
		       "tcsetattr(%d,,) failed.\n",fd);
	}
      close(fd);
      fd = -1;
    }
}


PCWand::~PCWand(void)
{
  clean_up();
}


PCWand::PCWand(void)
{

  button_shm = NULL;
  joystick_shm = NULL;
  joystick_state[0] = 0.0f;
  joystick_state[1] = 0.0f;
  end_ptr = read_buffer;
  termio_set_flag = 0;

#ifdef TRACKD_COMPATABLE
  if(VRCO.isInvalid()) return; // error
#endif

  /************************************************************/
  /***************** open serial device file ******************/
  /************************************************************/

  if((fd = open(DEVICE_FILE, O_RDWR|O_NONBLOCK)) < 0)
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "PCWand::PCWand() failed: open(\"%s\", "
		 "O_RDWR|O_NONBLOCK)"
		 " failed.\n", DEVICE_FILE);
      clean_up();
      return;
    }

  // get and save original termio info
  if(tcgetattr(fd, &original_terminfo) == -1)
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "PCWand::PCWand() failed: "
		 "tcgetattr(%d,,) failed.\n",
		 fd);
      clean_up();
      return;
    }

  struct termios terminfo;
  terminfo.c_iflag = 0;
  terminfo.c_oflag = 0;
  terminfo.c_lflag = 0;
  terminfo.c_cflag = CS8 | CREAD | CLOCAL;
  terminfo.c_ispeed = terminfo.c_ospeed = B9600; // Can this change??

  for (int i=0; i<NCCS; i++)
    terminfo.c_cc[i] = 0;
  terminfo.c_cc[VMIN] = 1;

  if (tcsetattr(fd, TCSANOW, &terminfo) == -1)
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "PCWand::PCWand() failed: "
		 "tcsetattr(%d,,) failed.\n",fd);
      clean_up();
      return;
    }
  termio_set_flag = 1;

  if (tcflush(fd, TCIOFLUSH) == -1)
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "PCWand::PCWand() failed: "
		 "Could not flush serial port: "
		 "tcflush() failed.\n");
      clean_up();
      return;
    }

  /*************************************************************/
  /****************** get some DTK shared memory ***************/
  /*************************************************************/

  button_shm = new
    dtkSharedMem(sizeof(unsigned char),"buttons", &button_state);
  joystick_shm = new
    dtkSharedMem(sizeof(float)*2,"joystick", joystick_state);

  if(!button_shm || !joystick_shm)
    {
      // This will let you know why dtkSharedMem::dtkSharedMem()
      // spewed.
      dtkMsg.add(DTKMSG_ERROR,
		 "PCWand::PCWand() failed.\n");
      clean_up();
      return;
    }
}


int PCWand::serve(void)
{
  // The first two byte may (or not) be unprocessed data from the last
  // read().
  ssize_t bytes_read =
    read(fd, end_ptr,READ_BUFFER_SIZE_M2);

  /*************************************************************/
  /*** checking error cases first. Cause it's easyer that way **/
  /*************************************************************/

  if(bytes_read <= 0)
    {
      if(bytes_read == 0) // This should never happen.
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
                 "PCWand::serve() failed: "
                 "read() read no data.\n");
	  return DTKSERVICE_CONTINUE;
	}
      else // if(bytes_read < 0) // error
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "PCWand::serve() failed: "
		     "read() failed.\n");
	  return DTKSERVICE_CONTINUE;
	}
    }

  /*************************************************************/
  /***************** process the data read *********************/
  /*************************************************************/

  // The first two byte may (or not) be unprocessed data from the last
  // read().
  end_ptr += bytes_read;
  // Now the data is from read_buffer to and not including end_ptr.

#if(0) /* debug printing */
  {
    printf("read %d need to process: ", bytes_read);
    for(unsigned char *_ptr=read_buffer;_ptr < end_ptr;_ptr++)
      {
	printf("%3.3o ",*_ptr);
      }
    printf("\n");
  }
#endif

  for(unsigned char *ptr=read_buffer;ptr < end_ptr;ptr++)
    {
      // A value (joystick joystick) is marked by one byte that has bits
      // 060 set and bits 0300 are not set, followed by one byte that
      // is zero, followed by a one byte value.
      if( ((unsigned char) 060 == (*ptr & (unsigned char) 060) &&
	   !(*ptr & (unsigned char) 0300)
	   )
	  &&
	  end_ptr > ptr + (size_t) 2 /* long enough */
	  &&
	  *(ptr+1) == (unsigned char) 00
	  )
	{
	  /* there are a possible 017 + 1 = 16 valuators */
	  int j = (*ptr) & (unsigned char) 017;
	  ptr++;
	  if(*ptr == (unsigned char) 00) /* this byte must be zero */
	    {
	      ptr++;
	      if(j < 2)
		{
		  joystick_state[j] = 2.0f * ((float) (*ptr)) / 255.0f - 1.0f;
		  joystick_shm->write(joystick_state);
		  //printf("joystick_state[%d] = %2.2f\n",j,joystick_state[j]);
		}
	      else
		dtkMsg.add(DTKMSG_WARN,
			   "PCWand::serve() read data for valuator number %d\n"
			   "This is setup for only %d valuators\n",
			   j+1, 2);
	    }
	}
      // Button events (press or release) are one byte long and have
      // bits 0330 set.
      else if((unsigned char) 0330 == (*ptr & (unsigned char) 0330))
	{
	  //printf("button event: %3.3o\n",*ptr);
	  if(*ptr & (unsigned char) 0040) /* it's a button release */
	    {
	      /* nuke that bit *//* up to 8 buttons */
	      button_state &= ~(01 << (*ptr & (unsigned char) 07));
	      button_shm->write(&button_state);
	      // printf("released button  %1.1o\n",*ptr & (unsigned char)03);
	    }
	  else /* it's a button press */
	    {
	      /* set that bit *//* up to 8 buttons */
	      button_state |= (01 << (*ptr & (unsigned char) 07)); 
	      button_shm->write(&button_state);
	      // printf("pressed button %1.1o\n",*ptr & (unsigned char)03);
	    }
	}   
    }
  
  /* for VRCO (tm) trackd (tm) compatiablity */
#ifdef TRACKD_COMPATABLE
  {
    int i;
    for(i=0;i<TD_BUTTON_ARRAY_NUM;i++)
      VRCO.button[i] = (button_state & (01 << i)) >> i;
    VRCO.val[0] = joystick_state[0];
    VRCO.val[1] = joystick_state[0];
  }
#endif
  /********************************************************************
   * Put the last two, one or zero bytes in the front, in read_buffer,
   * for processing in the next pass.  In the case that a partial
   * joystick value is received.
   ********************************************************************/

  if(end_ptr > read_buffer + (size_t) 1 /* Have at least 2 bytes. */ &&
     (
      *(end_ptr-2) == (unsigned char) 061 || /* and have a 060 */
      *(end_ptr-2) == (unsigned char) 060   /*          or 061 */
     )
     &&
     *(end_ptr-1) == (unsigned char) 00  /* and then a 00 */
     )
    {
      read_buffer[0] = *(end_ptr-2);
      read_buffer[1] = (unsigned char) 00;
      end_ptr = &read_buffer[2];
    }
  else if(/* read at least 1 byte at this point */
	  *(end_ptr-1) == (unsigned char) 061 ||
	  *(end_ptr-1) == (unsigned char) 060)
    {
      read_buffer[0] = *(end_ptr-1);
      end_ptr = &read_buffer[1];
    }
  else
    end_ptr = read_buffer;

#if(0) /* debug printing */
  {
    printf("left over: ");
    for(unsigned char *_ptr=read_buffer;_ptr < end_ptr;_ptr++)
      {
	printf("%3.3o ",*_ptr);
      }
    printf("\n");
  }
#endif

  return DTKSERVICE_CONTINUE;
}


// The DTK C++ loader/unloader functions

static dtkService *dtkDSO_loader(const char *arg)
{
  return new PCWand;
}

static int dtkDSO_unloader(dtkService *pcwand)
{
  delete pcwand;
  return DTKDSO_UNLOAD_CONTINUE; // success
}
