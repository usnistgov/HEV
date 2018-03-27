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

class Canvas: public Fl_Widget
{
 public:

  Canvas(dtkSharedMem **shm, int x, int y, int w, int h);
  ~Canvas(void);


  inline void rescale(void) { do_rescale = 1; }


  Plot **plot;

 private:

  void draw(void);
  void draw(long double t1, long double t2,
	    int x1, int x2);

  dtkTime Time;

  long double t1;
  int x1;

  int delta_x;

  int old_w, old_h;

  int do_rescale;

};
