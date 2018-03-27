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

#include "types.h"
#include "dtkBase.h"
#include "dtkAugment.h"
#include "dtkDisplay.h"


dtkDisplay::dtkDisplay(const char *name) : dtkAugment(name)
{
  setType(DTKDISPLAY_TYPE);
}

// virtual destructor.
dtkDisplay::~dtkDisplay(void) {}

int dtkDisplay::config(void) { return DTK_REMOVE_CALLBACK; }

int dtkDisplay::frame(void) { return DTK_REMOVE_CALLBACK; }

int dtkDisplay::sync(void) { return DTK_REMOVE_CALLBACK; }
