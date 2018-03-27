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
/****** dtkUSBInput.h ******/

#define DTKUSB_ANYID  ((unsigned short) 0xFFFF)


/* Input device ID
 * device_id[4] = { bus,  vender,  product,  version }
 */

/** \fn int dtkUSBInput_findID(const unsigned short device_id[][4]=NULL,
                     char **deviceFile_ret=NULL)
 *
 * Returns a valid file descriptor on success or -1 on if it fails to
 * find a USB device file that has a matching hardware device IDs. If
 * the char handle \e deviceFile_ret is non-NULL, and a device file
 * was found, it will be set to a string that was allocated with
 * malloc().  The user may call free() to despose of this memory. If
 * no device file was found \e *deviceFile_ret will be set to NULL.
 * The array elements of \e device_id are of the form
 * <pre>{ bus, vender, product, version }</pre> .  The array
 * \e device_id must be
 * terminated with the value <pre>{ 0, 0, 0, 0 }</pre>.  If
 * \e device_id is NULL this will the first USB device that it finds.
 */
extern int dtkUSBInput_findID(const unsigned short device_id[][4]=NULL,
			      char **deviceFile_ret=NULL);
