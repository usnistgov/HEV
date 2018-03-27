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

#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#ifdef DTK_ARCH_IRIX
   typedef uint64_t u_int64_t;
#endif

#include "types.h"
#include "dtkBase.h"
#include "dtkAugment.h"
#include "dtkFilter.h"


dtkFilter::dtkFilter(size_t size):
  dtkBase(DTKFILTER_TYPE)
{
  _needTimeStamp = 0;
  _needCount = 0;
  _needQueued = 0;
  _requiredSize = 0;
  _shmSize = size;
  _byteSwapElementSize = 0;
  // This base class object starts out invalid in the dtkBase sense.
}

dtkFilter::~dtkFilter(void) {}

int dtkFilter::read(void *buf, size_t bytes, size_t offset, int diffByteOrder,
		    struct timeval *timeStamp, u_int64_t count)
{
  // base class filter does nothing but remove this callback.
  return REMOVE;
}

int dtkFilter::write(void *buf, size_t bytes, size_t offset,
		     struct timeval *timeStamp)
{
  // base class filter does nothing but remove this callback.
  return REMOVE;
}
