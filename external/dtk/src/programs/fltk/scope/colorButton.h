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

#define COLORBUTTON_WIDTH 40

class ColorWidget : public Fl_Widget
{
 public:

  ColorWidget(int x, int y, int w, int h);
  void draw(void);
  uchar r, g, b;
};


class ColorButton : public Fl_Button
{
 public:

  ColorButton(int x ,int y, uchar r, uchar g, uchar b, const char * label=0);

  // Sets the color of this widget.
  void setColor(uchar r, uchar g, uchar b);

  ColorWidget colorWidget;
};

