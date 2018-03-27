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
class Canvas;

class Channel
{

 public:

  Channel(int canvasY0, int canvasHeight, int shm_index,
	  TYPE min, TYPE max,
	  // line and point colors
	  uchar r_l, uchar g_l, uchar b_l,
	  uchar r_p, uchar g_p, uchar b_p);
 
  void setScale(int canvasY, int canvasHeight, TYPE min_in, TYPE max_in);
  void setScale(int canvasY, int canvasHeight);
  void setScale(Canvas *canvas, TYPE min_in, TYPE max_in);

  void setPointColor(uchar r, uchar g, uchar b);
  void setLineColor(uchar r, uchar g, uchar b);

  void getPointColor(uchar &r, uchar &g, uchar &b);
  void getLineColor(uchar &r, uchar &g, uchar &b);

  

  inline void setY(TYPE *buffer)
    { y = (int) (scale * buffer[shm_index] + shift); }
  inline int getY(void) { return y; }
  inline void pointColor(void) { fl_color(r_point, g_point, b_point); }
  inline void lineColor(void) { fl_color(r_line, g_line, b_line); }

  int isPlotted;

  int shm_index; // the index in the shared memory array of TYPE
  // For example, if the shared memory is an array of floats
  // index=0 means the first float in the array.

  TYPE max, min, scale, shift;

 private:


  // line and point colors
  uchar r_line, g_line, b_line;
  uchar r_point, g_point, b_point;


  int y;

};
