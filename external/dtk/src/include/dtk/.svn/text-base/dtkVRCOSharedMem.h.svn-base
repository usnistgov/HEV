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
/****** dtkVRCOSharedMem.h ******/
// A C++ class used to access VRCO Trackd (system V) shared memory.

#define SYSV_SHM_KEY        0x0000101e
#define WAND_SYSV_SHM_KEY   0x0000101f

/* I don't know where this number comes from but it seems to make the
 * system V shared memory data structure the correct size for VRCO
 * inc. trackd (tm) compatablity.  */

#define TD_BUTTON_ARRAY_NUM           32
#define TD_VALUATOR_ARRAY_NUM         32

/* paid no mind to these stupid data structures. */
struct S0
{
  u_int32_t V;
  u_int32_t button;
  u_int32_t val;
  u_int32_t nb;
  u_int32_t nv;
  u_int32_t t[2];
  u_int32_t c; /* not used */
};

struct S1
{
  u_int32_t nb, nv;
  int32_t  button[TD_BUTTON_ARRAY_NUM];
  float    val[TD_VALUATOR_ARRAY_NUM];
};

struct S2
{
  struct S0 s0;
  struct S1 s1;
};

#define TRACKD_ARRAY_SENSORS_NUM 8

struct _trackd_header_struct {
  u_int32_t unknown1;
  u_int32_t number_of_sensors;
  u_int32_t size_of_header;
  u_int32_t size_of_sensor;
  u_int32_t timestamp_sec;
  u_int32_t timestamp_usec;
  u_int32_t unknown2;
};
struct _trackd_sensor_struct {
  float x;
  float y;
  float z;
  float h;
  float p;
  float r;
  u_int32_t time_sec;
  u_int32_t time_usec;
  u_int32_t unknown1;
  u_int32_t unknown2;
};

struct _td {
  struct _trackd_header_struct tsh;
  struct _trackd_sensor_struct ts[TRACKD_ARRAY_SENSORS_NUM];
};



// This could be called dtkVRCOTrackdSharedMem but that just too long.

/*! \class dtkVRCOSharedMem dtkVRCOSharedMem.h dtk.h
 * \brief Read and write data to a VRCO CAVE like device
 * Write data to a head tracker, wand tracker and buttons and joystick.
 * On UNIX like systmes VRCO CAVE devices are written to system V
 * shared memory.  This is just a small wrapper to get values to and from
 * that shared memory.
 */
class dtkVRCOSharedMem : public dtkBase
{
 public:
  /*!
   * Constructor
   */
  dtkVRCOSharedMem(void);
  /*!
   * Destructor
   */
  virtual ~dtkVRCOSharedMem(void);
  
  //! Write data to the head tracker
  /*!
   * Write data to the head tracker
   */
  void writeHeadTracker(const float *x);
  //! Write data to the wand 
  /*!
   * Write data to the wand.
   */
  void writeWand(const float *x,
		 const float *joystick,
		 unsigned char buttons);
  //! Write data to the tracker
  /*!
   * Write data to the tracker
   */
  void writeTracker(const float *tracker, int which);
  
  /* pointers to system 5 -- VRCO tracker data */

  //! pointer to the head tracker data
  struct _trackd_sensor_struct *head; //! head tracker
  //! pointer to the wand tracker data
  struct _trackd_sensor_struct *wand; //! wand tracker
  //! pointer to the buttons data
  int32_t *button;    //! wand buttons
  //! pointer to the joystick data
  float *val;         //! wand joystick
};

