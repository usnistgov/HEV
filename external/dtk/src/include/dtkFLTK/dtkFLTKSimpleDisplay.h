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

#define DTKFLTKSIMPLE_TITLE "fltkSimpleDisplay"


class dtkFLTKSimpleDisplay: public dtkBasicDisplay, public Fl_Gl_Window
{
 public:

  dtkFLTKSimpleDisplay(dtkManager *m, const char *name=DTKFLTKSIMPLE_TITLE,
		      int x=0, int y=0, int w=400, int h=400,
		      const char *title=DTKFLTKSIMPLE_TITLE);
  dtkFLTKSimpleDisplay(dtkManager *m, const char *name,
		      int x, int y,
		      const char *title=DTKFLTKSIMPLE_TITLE);
  dtkFLTKSimpleDisplay(dtkManager *m, const char *name,
		      const char *title);
  virtual ~dtkFLTKSimpleDisplay(void);

  // this is a dtkFLTKSimpleGLWindow child window
  // that is setup in the constructor.
  Fl_Window *fl_window;

  int config(void); // calls Fl_Window::end() and Fl_Window::show()

  int frame(void);
  int sync(void);

  void draw(void); // over-write Fl_Gl_Window::draw()

 private:

  void reshapeWin(int width, int height);

  void init(const char *name,
	    int x, int y, int w, int h,
	    const char *title);
};
