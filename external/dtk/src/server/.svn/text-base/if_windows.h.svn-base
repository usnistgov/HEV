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
#ifndef __IF_WINDOZ_H__
#define __IF_WINDOZ_H__

//#include <dtk.h>
#ifdef DTK_ARCH_WIN32_VCPP

# include <winbase.h>

# ifdef DTK_SERVER_DSO_EXPORTS /* if building library */
#  define DTKSERVERAPI __declspec(dllexport)
# else /* if using library */
#  define DTKSERVERAPI __declspec(dllimport)
# endif

typedef HANDLE    pthread_t;



#define MULTI_THREADED


#else  /* #ifdef DTK_ARCH_WIN32_VCPP */

// DTKSERVERAPI is defined as nothing on UNIX.
// Windows DLLs are not written in C++ they required
// this shit to be defined as __declspec(dll??port)
// and put before all exported symbols.
# define DTKSERVERAPI

#endif /* #ifdef DTK_ARCH_WIN32_VCPP #else */

#endif /* #ifndef __IF_WINDOZ_H__ */

