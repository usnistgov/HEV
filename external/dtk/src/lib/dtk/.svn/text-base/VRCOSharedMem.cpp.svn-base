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


// VRCO trackd does not use any semaphores or read/write locks when
// writing to the system 5 shared memory.  So there is no benefit in
// doing that here.
 
#define NUMBEROFTRACKERS    2
#define NUMBEROFBUTTONS     3
#define NUMBEROFFLOATSINVALUATOR  2

#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "types.h"
#include "dtkBase.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "dtkVRCOSharedMem.h"


dtkVRCOSharedMem::dtkVRCOSharedMem(void) :
  dtkBase(DTKVRCOSHAREDMEM_TYPE)
{
  int id = shmget((key_t)SYSV_SHM_KEY,
                  sizeof(struct _td),
                  IPC_CREAT | 0666);
  if (id < 0) {
    dtkMsg.add(DTKMSG_ERROR, 1,
               "dtkVRCOSharedMem::dtkVRCOSharedMem() "
	       "failed: shmget() failed.\n");
    return;
  }

  struct _td *td  = (struct _td *) shmat(id, (void *) NULL, 0);
  if ( (void *) td == (void *) -1) {
    dtkMsg.add(DTKMSG_ERROR, 1,
               "dtkVRCOSharedMem::dtkVRCOSharedMem() "
	       "failed: shmat() failed.\n");
    return;
  }

  // head = &(td->ts[0]);
  // wand = &(td->ts[1]);
  wand = &(td->ts[1]);
  head = &(td->ts[0]);


  id = shmget((key_t)WAND_SYSV_SHM_KEY,
	      sizeof(struct S2),
	      IPC_CREAT | 0666);
  if (id < 0) {
    /* Oops we did something wrong */
    dtkMsg.add(DTKMSG_ERROR, 1,
               "dtkVRCOSharedMem::dtkVRCOSharedMem() "
	       "failed: shmget() failed.\n");
    return;
  }

  struct S2 *_s2  = (struct S2 *) shmat(id, (void *) NULL, 0);
  if ( (void *) _s2 == (void *) -1) {
    dtkMsg.add(DTKMSG_ERROR, 1,
               "dtkVRCOSharedMem::dtkVRCOSharedMem() "
	       "failed: shmat() failed.\n");
    return;
  }

  // get header time stamps
  struct timeval tv;
  gettimeofday(&tv, NULL);

  /* set pointers to changing data */
  button = _s2->s1.button;
  val = _s2->s1.val;

  /* set values in ?? structure */

  _s2->s1.nv = NUMBEROFFLOATSINVALUATOR;
  _s2->s1.nb = NUMBEROFBUTTONS;
  _s2->s0.button = 40;
  _s2->s0.val = 168;
  _s2->s0.nv = NUMBEROFFLOATSINVALUATOR;
  _s2->s0.nb = NUMBEROFBUTTONS;
  _s2->s0.V  = 1;
  _s2->s0.t[0] =(u_int32_t) tv.tv_sec;
  _s2->s0.t[1] =(u_int32_t) tv.tv_usec;


  /******** initialize the VRCO Trackd header data *********/


  td->tsh.unknown1 = 1;
  td->tsh.unknown2 = 0;
  td->tsh.number_of_sensors = 2;
  td->tsh.size_of_header = 28;
  td->tsh.size_of_sensor = 40;

  td->tsh.timestamp_sec = (u_int32_t) tv.tv_sec;
  td->tsh.timestamp_usec = (u_int32_t) tv.tv_usec;

  td->ts[0].unknown1 = 0;
  td->ts[0].unknown2 = 0;
  td->ts[1].unknown1 = 0;
  td->ts[1].unknown2 = 0;

  validate();
}

dtkVRCOSharedMem::~dtkVRCOSharedMem(void)
{
  invalidate();
}

// With CAVE Trackd CAVE display scaling
void dtkVRCOSharedMem::writeHeadTracker(const float *x)
{
  head->x =  x[0]*5.0f;
  head->y =  (x[2] + 1.0f)*5.0f;
  head->z = -x[1]*5.0f;
  head->h =  x[3];
  head->p = x[4];
  head->r = -x[5] + 90.0f;
}

void dtkVRCOSharedMem::writeWand(const float *tracker,
				 const float *joystick,
				 unsigned char buttons)
{
  wand->x =  tracker[0]*5.0f;
  wand->y =  (tracker[2] + 1.0f)*5.0f;
  wand->z = -tracker[1]*5.0f;
  wand->h =  tracker[3];
  wand->p = tracker[4];
  wand->r = -tracker[5];

  val[0] = joystick[0];
  val[1] = joystick[1];

  button[0] = buttons & 01;
  button[1] = (buttons & 02)?1:0;
  button[2] = (buttons & 04)?1:0;
}

void dtkVRCOSharedMem::writeTracker(const float *x, int i)
{
  if(i == 0) // head
    {
      head->x =  x[0]*5.0f;
      head->y = (x[2] + 1.0f)*5.0f;
      head->z = -x[1]*5.0f;
      head->h =  x[3];
      head->p =  x[4];
      head->r = -x[5] + 90.0f;
    }
  else // wand
    {
      wand->x =  x[0]*5.0f;
      wand->y = (x[2] + 1.0f)*5.0f;
      wand->z = -x[1]*5.0f;
      wand->h =  x[3];
      wand->p =  x[4];
      wand->r = -x[5];
    }
}
