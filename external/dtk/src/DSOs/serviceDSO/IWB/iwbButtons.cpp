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
/* This DTK service reads 3 serial mouse buttons and then serves them
 * to DTK shared memory.  It is used at Virginia Tech to read buttons
 * on a tracked immersive work bench wand that has three buttons.  */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <dtk.h>
#include <dtk/dtkService.h>

#define DEVICE_FILE "/dev/ttyd3"

#define RATE B1200

#define READ_BUFFER_SIZE        ((size_t) 128)


class SerialButtons : public dtkService
{
public:
  SerialButtons(void);
  ~SerialButtons(void);
  int serve();

private:
  unsigned char buttons_state;
  dtkSharedMem buttons_shm;

  unsigned char read_buffer[READ_BUFFER_SIZE];

  // if the termio settings need to be
  // reset on before closing the device.
  int termio_set_flag;
  struct termios original_terminfo;

  void clean_up(void);
};


void SerialButtons::clean_up(void)
{
  if(fd > -1)
    {
      if(termio_set_flag)
	{
	  termio_set_flag = 0;
	  if (tcsetattr(fd, TCSANOW, &original_terminfo) == -1)
	    dtkMsg.add(DTKMSG_WARN, 1,
		       "SerialButtons::*() failed: failed to reset device"
		       " termio settings: "
		       "tcsetattr(%d,,) failed.\n",fd);
	}
      close(fd);
      fd = -1;
    }
}


SerialButtons::~SerialButtons(void)
{
  clean_up();
  dtkMsg.add(DTKMSG_DEBUG,
	     "calling SerialButtons::~SerialButtons().\n");
}


SerialButtons::SerialButtons(void):
  buttons_shm(sizeof(unsigned char),"buttons")
{
  int i;

  // initialize all data

  buttons_state = (unsigned char) 0;
  termio_set_flag = 0;

  /************************************************************/
  /***************** open serial device file ******************/
  /************************************************************/

  if((fd = open(DEVICE_FILE, O_RDWR|O_NONBLOCK)) < 0)
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "service::service() failed: open(\"%s\", "
		 "O_RDWR|O_NONBLOCK)"
		 " failed.\n", DEVICE_FILE);
      clean_up();
      return;
    }

  // get and save original termio info
  if(tcgetattr(fd, &original_terminfo) == -1)
    {
      dtkMsg.add(DTKMSG_ERROR, 1 /* sys err */,
		 "SerialButtons::SerialButtons() failed: "
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
  terminfo.c_ispeed = terminfo.c_ospeed = RATE; // Can this change??

  for (i=0; i<NCCS; i++)
    terminfo.c_cc[i] = 0;
  terminfo.c_cc[VMIN] = 1;

  if (tcsetattr(fd, TCSANOW, &terminfo) == -1)
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "SerialButtons::SerialButtons() failed: "
		 "tcsetattr(%d,,) failed.\n",fd);
      clean_up();
      return;
    }
  termio_set_flag = 1;

  if (tcflush(fd, TCIOFLUSH) == -1)
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "SerialButtons::SerialButtons() failed: "
		 "Could not flush serial port: "
		 "tcflush() failed.\n");
      clean_up();
      return;
    }

  /*************************************************************/
  /****************** check some DTK shared memory ***************/
  /*************************************************************/


  if(buttons_shm.isInvalid())
    {
      // dtkSharedMem::dtkSharedMem() will spew too.
      dtkMsg.add(DTKMSG_ERROR,
		 "SerialButtons::SerialButtons() failed.\n");
      clean_up();
      return;
    }
}


int SerialButtons::serve(void)
{
  ssize_t bytes_read =
    read(fd, read_buffer, READ_BUFFER_SIZE);

  /*************************************************************/
  /*** checking error cases first. Cause it's easyer that way **/
  /*************************************************************/

  if(bytes_read <= 0)
    {
      if(bytes_read == 0) // This should never happen.
	{
	  // But it happens all the time on SGI's.
	  dtkMsg.add(DTKMSG_WARN,
		     "SerialButtons::serve() failed: "
		     "read() read no data.\n");
	  return DTKSERVICE_CONTINUE; // not fatal.
	}
      else // if(bytes_read < 0) // error
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "SerialButtons::serve() failed: "
		     "read() failed.\n");
	  return DTKSERVICE_UNLOAD; // fatal error
	}
    }

  /*************************************************************/
  /***************** process the data read *********************/
  /*************************************************************/


#if(0) /* debug printing */
  {
    printf("read %d: ", bytes_read);
    for(ssize_t i = 0 ; i< bytes_read; i++)
      {
	printf("%3.3o ",read_buffer[i]);
      }
    printf("\n");
  }
#endif

  for(ssize_t i = 0 ; i< bytes_read; i++)
    if(read_buffer[i] & 0200)
      {
	buttons_state = 07 & (~read_buffer[i]);
	
	// which the first and second buttons
	buttons_state =
	  ((buttons_state & 01) << 1) |
	  ((buttons_state & 02) >> 1) | (buttons_state & 04);

	if(buttons_shm.write(&buttons_state))
	  return DTKSERVICE_UNLOAD; // error
      }

  return DTKSERVICE_CONTINUE;
}


// The DTK C++ loader function

static dtkService *dtkDSO_loader(const char *dummy)
{
  return new SerialButtons;
}

static int dtkDSO_unloader(dtkService *serialButtons)
{
  delete serialButtons;
  return DTKDSO_UNLOAD_CONTINUE; // success
}
