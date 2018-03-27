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
/****** dtkFLTKOptions.h ******/
/*!
*FLTK options are:
* -d[isplay] host:n.n
* -g[eometry] WxH+X+Y
* -t[itle] windowtitle
* -n[ame] classname
* -i[conic]
* -fg color
* -bg color
* -bg2 color
*/

/*! dtk-getFLTKOptions() returns malloc()ed memory that is pointers to
* the strings that are passed in const char** argv.  You may use
* free() on the returned values *fltk_argv and *other_argv.  argv[0]
* is included in both returned argument arrays. */

//! returns 0 on success
//! returns -1 if malloc() fails.

extern DTKAPI int
dtkFLTKOptions_get(int argc, const char** argv,
		   int *fltk_argc,       char ***fltk_argv,
		   int *other_argc=NULL, char ***other_argv=NULL);

extern DTKAPI void dtkFLTKOptions_print(FILE *file=NULL); // default is stdout
