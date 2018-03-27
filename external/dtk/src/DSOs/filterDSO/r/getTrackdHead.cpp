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

#include <dtk.h>
#include <dtk/dtkFilter_loader.h>

#define OUTFILE  "getTrackdHead"

class getTrackdHead : public dtkFilter
{
public:

  getTrackdHead(size_t size);

  int read(void *buf, size_t bytes, size_t offset, int diffByteOrder,
	   struct timeval *timeStamp, u_int64_t count);
private:

  dtkVRCOSharedMem sysV;
};


getTrackdHead::getTrackdHead(size_t size) : dtkFilter(size)
{
  if(sysV.isInvalid()) return; // error
  if(size != 6*sizeof(float))
    {
      dtkMsg.add(DTKMSG_WARNING, "loading shared memory read filter "
		 "\""OUTFILE"\" the size is not 6*sizeof(float).\n");
      return;
    }
  validate();
}



int getTrackdHead::read(void *buf, size_t bytes, size_t offset,
			int diffByteOrder,
			struct timeval *timeStamp, u_int64_t count)
{
  if(bytes != 6*sizeof(float) || offset) return dtkFilter::REMOVE;
  float *x = (float *) buf;
  x[0] = (sysV.head->x - 0.0f)/5.0f;
  x[1] = (-sysV.head->z - 0.0f)/5.0f;
  x[2] = (sysV.head->y - 5.0f)/5.0f;
  x[3] = sysV.head->h;
  x[4] = sysV.head->p;
  x[5] = -sysV.head->r;

  return dtkFilter::CONTINUE; // success
}


/************** All dtkFilter DSOs define these two functions ****************
 *
 * If you wish to build your filter DSO from more than one file and
 * you wish to not define dtkFilter_loader and/or dtkFilter_unloader
 * in another source file then you may then define
 * __DTKFILTER_NOT_LOADED__ before including the
 * dtk/dtkFliter_loader.h in that source file.
 *
 *****************************************************************************/


static dtkFilter *dtkFilter_loader(size_t size, int argc, const char **args)
{
  return new getTrackdHead(size);
}

static int dtkFilter_unloader(dtkFilter *f)
{
  delete f;
  return DTKFILTER_UNLOAD_CONTINUE;
}
