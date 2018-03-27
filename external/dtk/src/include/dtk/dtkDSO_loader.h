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
/****** dtkDSO_loader.h ******/
#ifndef __dtkDSO_loader_h__
#define __dtkDSO_loader_h__

#ifndef DTKDSO_LOAD_ERROR
// The DTKDSO_*LOAD_* constants here are almost the same as in
// dtkService.h

/***************** Used to return from dtkDSO_loader() *****************/

// Special dtkDSO_loader() return values.

//! tell the loader to unload the DSO and fault due to error.
#define DTKDSO_LOAD_ERROR      ((dtkAugment *)  0)
//! tell the loader continue .
#define DTKDSO_LOAD_CONTINUE   ((dtkAugment *) -1)
//! tell the loader to unload the DSO and continue.
#define DTKDSO_LOAD_UNLOAD     ((dtkAugment *) -2)


/*!**************** Used to return from dtkDSO_unloader() *****************/

//! tell the unloader to continue and this was some what successful.
#define DTKDSO_UNLOAD_CONTINUE  0

//! tell the unloader that the program is hosed.
#define DTKDSO_UNLOAD_ERROR    -1 /*! error */


/***********************************************************************/
#endif /* #ifndef DTKDSO_LOAD_ERROR */



#ifndef  __DTKAUGMENT_NOT_LOADED__

static dtkAugment *dtkDSO_loader(dtkManager *m, void *p=NULL);
static int dtkDSO_unloader(dtkAugment *a);

extern "C"
{

#ifdef DTK_ARCH_WIN32_VCPP
  extern __declspec(dllexport) void *__dtkDSO_loader(void);
  extern __declspec(dllexport) void *__dtkDSO_unloader(void);
#endif

  //! Returns the C++ loader 
  void *__dtkDSO_loader(void)
  {
    return (void *) dtkDSO_loader;
  }
  //! Returns the C++ unloader
  void *__dtkDSO_unloader(void)
  {
    return (void *) dtkDSO_unloader;
  }
}

#endif /* #ifndef __DTKAUGMENT_NOT_LOADED__ */

#endif /*#ifndef __dtkDSO_loader_h__ */
