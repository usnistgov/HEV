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
 *
 */

/********************************************************************/
/**************** EDIT TO CONFIGURE *********************************/
/********************************************************************/

// This just works on GNU/Linux operating systems.

// This service will auto detect the first correct USB device.

// List of possible USB device IDs

static const unsigned short device_id[][4] =
  {
    /* For a given input device ID there are 4 numbers
     *  { bus,  vender,  product,  version }
     */

    // Not sure, what this is, will give the name later
    // A $25 Walmart wireless special.  Only one available in 
    // November 2006 in Blacksburg, VA.
    { DTKUSB_ANYID, 0x4d9, 0x3, DTKUSB_ANYID },

    // add another here
    // { DTKUSB_ANYID, 0x45e, 0x9, DTKUSB_ANYID },

    { 0, 0, 0, 0 }
  };


#ifndef DTK_ARCH_LINUX
# error "This is only ported to GNU/Linux"
#endif
		   

/********************************************************************/
/**************** END --  EDIT TO CONFIGURE *************************/
/********************************************************************/