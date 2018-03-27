/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2003  Virginia Tech
 * 
 * This software, the DIVERSE Toolkit library, is free software; you can
 * redistribute it and/or modify it under the terms of the GNU Lesser
 * General Public License (LGPL) as published by the Free Software
 * Foundation; either version 2.1 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software, in the top level source directory in
 * a file named "COPYING.LGPL"; if not, see it at:
 * http://www.gnu.org/copyleft/lesser.html or write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 */
#include "config.h"
#include "_config.h"
#include "privateConstants.h"
#include "dtkConfigure.h"

#ifdef DTK_ARCH_LINUX
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <linux/input.h>

#include "dtkColor.h"
#include "dtkMessage.h"
#include "utils.h"
#include "dtkUSBInput.h"


int dtkUSBInput_findID(const unsigned short device_id[][4],
		       char **deviceFile_ret)
{
  int fd = -1;
  int device_number = 0;
  char fileName[64];
  errno = 0;

  for( ;fd < 0 && errno != ENOENT;)
    {
      sprintf(fileName, "/dev/input/event%d", device_number++);

      struct stat st;
      if(stat(fileName,&st) < 0)
	{
	  dtkMsg.add(DTKMSG_NOTICE, 1,
		     "dtkUSBInput_findID() failed to stat "
		     "USB device file \"%s\":\n "
		     "stat(\"%s\", %p) failed.\n",
		     fileName, fileName, &st);
	}
      else if((fd = open(fileName, O_RDWR)) < 0)
	{
	  dtkMsg.add(DTKMSG_NOTICE, 1,
		     "dtkUSBInput_findID() failed to open "
		     "USB device file \"%s\":\n "
		     "open(\"%s\", O_RDWR) failed.\n",
		     fileName, fileName);
	}
      else // open() succeeded
	{
	  unsigned short id[4];
	  
	  if(ioctl(fd, EVIOCGID, id) == -1)
	    {
	      dtkMsg.add(DTKMSG_NOTICE, 1,
			 "dtkUSBInput_findID() failed "
			 "to get USB device ID on device file \"%s\": "
			 "ioctl(%d, EVIOCGID, %p) failed\n",
			 fileName, fd, id);
	      close(fd);
	      fd = -1;
	    }

	  // They don't care what USB input device.
	  if(!device_id && fd >= 0)
	    break;

	  int j=0;
	  for(;(device_id[j][0] || device_id[j][1] || device_id[j][2] || device_id[j][3]) && fd >= 0;j++)
	    {
	      if((device_id[j][0] == id[0] || device_id[j][0] == DTKUSB_ANYID) &&
		 (device_id[j][1] == id[1] || device_id[j][1] == DTKUSB_ANYID) &&
		 (device_id[j][2] == id[2] || device_id[j][2] == DTKUSB_ANYID) &&
		 (device_id[j][3] == id[3] || device_id[j][3] == DTKUSB_ANYID))
		{
		  // I found IT!!!
		  char name[256] = "Unknown";
		  ioctl(fd, EVIOCGNAME(sizeof(name)), name);
		  dtkMsg.add(DTKMSG_INFO,
			     "dtkUSBInput_findID() found\n "
			     "%sMATCHING USB device named%s \"%s%s%s\"\n on "
			     "device file \"%s\"\n",
			     dtkMsg.color.vil, dtkMsg.color.end,
			     dtkMsg.color.grn, name, dtkMsg.color.end,
			     fileName);
		  break;
		}
	    }

	  if(fd >= 0 && !(device_id[j][0] || device_id[j][1] || device_id[j][2] || device_id[j][3]))
	    {
	      char name[256] = "Unknown";
	      ioctl(fd, EVIOCGNAME(sizeof(name)), name);
	      dtkMsg.add(DTKMSG_INFO,
			 "dtkUSBInput_findID() found non-matching "
			 "USB device\n named \"%s\" on "
			 "device file \"%s\"\n",
			 name, fileName);
	      close(fd);
	      fd = -1;
	    }
	}
    }

  if(deviceFile_ret)
    {
      if(fd >= 0)
	*deviceFile_ret = dtk_strdup(fileName);
      else
	*deviceFile_ret = NULL;
    }

  return fd;
}


#endif /* #ifdef DTK_ARCH_LINUX */
