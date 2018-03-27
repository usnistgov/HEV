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

// USB port device file

// This just works on GNU/Linux operating systems.

// This service will auto detect the first correct USB device.

// List of possible USB device IDs

static const unsigned short device_id[][4] =
  {
    /* For a given input device ID there are 4 numbers
     *  { bus,  vender,  product,  version }
     */

    //"Microsoft Microsoft SideWinder Precision Pro (USB)"
    //{ 0x3, 0x45e, 0x8, 0x100 } droping bus and version number
    { DTKUSB_ANYID, 0x45e, 0x8, DTKUSB_ANYID },

    // This one will need calibrating.
    //"Microsoft SideWinder Precision 2 Joystick"
    { DTKUSB_ANYID, 0x45e, 0x38,DTKUSB_ANYID },

    // add another here
    // { DTKUSB_ANYID, 0x45e, 0x9, DTKUSB_ANYID },

    { 0, 0, 0, 0 }
  };

	   
// Look at sideWinderJoystick.cpp for the meaning of below stuff or
// better yet don't.

#define JOY_X_REST_VAL       0
#define JOY_Y_REST_VAL       0
#define JOY_Z_REST_VAL       0

#define JOY_X_FUG            20
#define JOY_Y_FUG            20
#define JOY_Z_FUG            3

#define JOY_X_SCALE          (511 - JOY_X_FUG)
#define JOY_Y_SCALE          (511 - JOY_Y_FUG)
#define JOY_Z_SCALE          (32  - JOY_Z_FUG)

#define JOY_DEAD_BAND_WIDTH  10

#define THROTTLE_MIN_VAL     -64
#define THROTTLE_MAX_VAL      63

#ifndef DTK_ARCH_LINUX
# error "This is only ported to GNU/Linux"
#endif


/********************************************************************/
/**************** END --  EDIT TO CONFIGURE *************************/
/********************************************************************/
