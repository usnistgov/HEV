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

class GnomonWindow : public Fl_Gl_Window
{
 private:

  void drawAxis();
  void drawCube();
  float boxv0[3], boxv1[3];
  float boxv2[3], boxv3[3];
  float boxv4[3], boxv5[3];
  float boxv6[3], boxv7[3];
  float axisXR[3], axisXL[3];
  float axisYB[3], axisYF[3];
  float axisZT[3], axisZB[3];
  GLUquadric* quad;

  public:

  GnomonWindow(int x,int y,int w,int h);

  float xyzhpr[6];  // current coordinates
  float Vert,Horz;  // vertical and horizontal angle of the camera from center
  float Scale;      // Zoom (Scale)

  void resetView();
  void draw();
  void initQuad();
};
