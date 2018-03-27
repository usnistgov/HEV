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
/****** dtkFilter_loader.h ******/
#ifndef __dtkFilter_loader_h__
#define __dtkFilter_loader_h__

#ifndef DTKFILTER_LOAD_ERROR
// The DTKFILTER_*LOAD_* constants here are almost the same as in
// dtkService.h

/***************** Used to return from dtkFilter_loader() *****************/

// Special dtkFilter_loader() return values.

//! tell the loader to unload the DSO and fault due to error.
#define DTKFILTER_LOAD_ERROR      ((dtkFilter *)  0)
//! tell the loader continue .
// #define DTKFILTER_LOAD_CONTINUE   ((dtkFilter *) -1)
//! tell the loader to unload the DSO and continue.
#define DTKFILTER_LOAD_UNLOAD     ((dtkFilter *) -2)


/*!**************** Used to return from dtkFilter_unloader() *****************/

//! tell the unloader to continue and this was some what successful.
#define DTKFILTER_UNLOAD_CONTINUE  0

//! tell the unloader that the program is hosed.
#define DTKFILTER_UNLOAD_ERROR    -1 /*! error */


/***********************************************************************/
#endif /* #ifndef DTKFILTER_LOAD_ERROR */



#ifndef  __DTKFILTER_NOT_LOADED__

static dtkFilter *dtkFilter_loader(size_t size, int argc, const char **argv);
static int dtkFilter_unloader(dtkFilter *f);

extern "C"
{

#ifdef DTK_ARCH_WIN32_VCPP
  extern __declspec(dllexport) void *__dtkDSO_loader(void);
  extern __declspec(dllexport) void *__dtkDSO_unloader(void);
#endif

  //! Return the loader 
  void *__dtkDSO_loader(void)
  {
    return (void *) dtkFilter_loader;
  }
  //! Return the unloader
  void *__dtkDSO_unloader(void)
  {
    return (void *) dtkFilter_unloader;
  }
}

#endif /* #ifndef __DTKFILTER_NOT_LOADED__ */

#endif /*#ifndef __dtkFilter_loader_h__ */
