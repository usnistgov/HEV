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
/* This file is just used on MS Windows systems. */

#ifndef __WINDOWSDEFS_H__
# define __WINDOWSDEFS_H__

typedef ULONGLONG          u_int64_t;
typedef LONGLONG           int64_t;
typedef unsigned int       u_int32_t;
typedef int                int32_t;
typedef unsigned short     u_int16_t;
typedef unsigned char      u_int8_t;
typedef int                ssize_t;
typedef unsigned long      mode_t;
typedef unsigned int       size_t;
typedef int                pid_t;
typedef int                key_t;

#ifndef DTKAPI
# ifdef LIBDTK_EXPORTS
#  define DTKAPI __declspec(dllexport)
# else
#  define DTKAPI __declspec(dllimport)
# endif
#endif

#ifndef DTKFLTKAPI
# ifdef LIBDTKFLTK_EXPORTS
#  define DTKFLTKAPI __declspec(dllexport)
# else
#  define DTKFLTKAPI __declspec(dllimport)
# endif
#endif

#endif

