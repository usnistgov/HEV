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
 */

// This file should not be installed.

// We just put defines and like stuff in here that will not change
// by running ../configure.


/* This DTKAPI MACRO is so that Windoz and UNIX's may use the same
 * header include files. On a Windoz system it will be defined as
 * something that will import of export symbols for DLLs.
 * Windoz does not support C++ when using DLLs, it's "extended C++" or
 * a better term is kludged C++. DTKAPI is part of this extention.
 */
#ifndef DTKAPI
# define DTKAPI
#endif

#define DTK_DEFAULT_SERVER_USER       "nobody"


#ifdef DTK_ARCH_WIN32_VCPP
# define DIRCHAR '\\'
# define DIRSTRING "\\"
# ifndef PATH_SEP
#   define PATH_SEP ';'
# endif
#else
# define DIRCHAR '/'
# define DIRSTRING "/"
# ifndef PATH_SEP
#   define PATH_SEP ':'
# endif
#endif
