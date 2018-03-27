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
 */

/*! \file
 * 
 * A group of functions that use dlopen() and related functions to
 * enable loading of C++ objects from dynamic shared objects (DSOs)
 * files, so that the DSOs may be plug-ins.
 *
 * Here is an example using dtkDSO_load() and related functions:
 * \include examples/dtk/examplesByClass/dtkDSO_/load.cpp
 */


/*!
 * dtkDSO_load() opens the DSO file \e file and calls a C function
 * void *__dtkDSO_loader(void *) and returns the void pointer that was
 * returned from the C function call. If \e path is non-NULL \e path
 * is the search path to find \e file. Returns NULL on error.
 */
extern DTKAPI void *dtkDSO_load(const char *path, const char *file);

/*!
 * dtkDSO_load() opens the DSO file \e file and calls a C function
 * void *__dtkDSO_loader(void *) and returns the void pointer that was
 * returned from the C function call. If \e path is non-NULL \e path
 * is the search path to find \e file. \e arg is passed as an argment
 * to the __dtkDSO_loader() function. Returns NULL on error.
 */
extern DTKAPI void *dtkDSO_load(const char *path, const char *file,
				const char *arg);
/*!
 * dtkDSO_load() opens the DSO file \e file and calls a C function
 * void *__dtkDSO_loader(void *) and returns the void pointer that was
 * returned from the C function call. If \e path is non-NULL \e path
 * is the search path to find \e file. \e arg is passed as an argment
 * to the __dtkDSO_loader() function. Returns NULL on error.
 */
extern DTKAPI void *dtkDSO_load(const char *path, const char *file,
				void *arg);
/*!
 * dtkDSO_load() opens the DSO file \e file and calls a C function
 * void *__dtkDSO_loader(void *) and returns the void pointer that was
 * returned from the C function call. If \e path is non-NULL \e path
 * is the search path to find \e file. \e arg0 and \e arg1 are passed
 * as argments to the __dtkDSO_loader() function. Returns NULL on
 * error.
 */
extern DTKAPI void *dtkDSO_load(const char *path, const char *file,
				void *arg0, void *arg1);

// For debuging.
extern DTKAPI void  dtkDSO_print(FILE *file); // default is stdout

/*!
 * Unload a given DSO. \e object is a pointer to the value returned
 * from a previous call to dtkDSO_load().  Returns 0
 * on success or -1 on error.
 */
extern DTKAPI int dtkDSO_unload(void *object);

/*!
 * Get a pointer to a pointer from a call to __dtkDSO_unloader().
 * This calls __dtkDSO_unloader() and returns the return value from
 * that call.  This is so we can get a pointer to a C++ function.  C
 * functions do not necessarily let C++ object pointers be passed as
 * augments without braking them.  Returns NULL on error.
 */
extern DTKAPI void *dtkDSO_getUnloader(void *object);

/*!
 * Returns a pointer to malloc()ed memory that will be free()ed when
 * 'object' is unloaded with dtkDSO_unload().  Do not write to the
 * memory returned.  strdup() it if you must.
 */
extern DTKAPI char *dtkDSO_getDSOFilePath(void *object);

