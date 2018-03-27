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
/* This file was originally written by Eric Tester.  Many
 * modifications have been made by Lance Arsenault.
 */

class Tracker;

struct callbackData
{
  Tracker *t;
  int which;
};

class Tracker : public Fl_Group
{
public:

  Tracker(int x, int y,
	  dtkSharedMem *shm, float *state,
#ifndef DTK_ARCH_WIN32_VCPP
	  dtkVRCOSharedMem *sysV, int which_VRCOtracker,
#endif
	  float *resetState,
	  const char *label = 0,
	  Fl_Color labelColor=FL_BLACK);

  void update_sharedMem(void);
  void update_sliders(void);

  void reset(void);
  void init(void);

private:

  Fl_Value_Slider *slider[6];
  friend void init_callback(Fl_Widget *w, void *data);
  friend void reset_callback(Fl_Widget *w, void *data);

  struct callbackData cbData[6];
  dtkSharedMem *shm;
  float *state;
#ifndef DTK_ARCH_WIN32_VCPP
  dtkVRCOSharedMem *sysVstate;
  int which_VRCOtracker;
#endif

  float initState[6];
  float resetState[6];
};

