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
#include <stdio.h>
#include <FL/Fl_Box.H>
#include <dtk.h>
#include "scope.h"

#ifdef DTK_ARCH_WIN32_VCPP
# define snprintf _snprintf
#endif


PlotWidget::PlotWidget(int y, Plot *plot_in)
  : Fl_Group(0, y, PLOTWIDGET_WIDTH, PLOTWIDGET_HEIGHT)
{
  plot = plot_in;

  snprintf(label, 199, "DTK shared memory file: \"%s\"\n\n"
	   "Show  Line  Point Indx  Min               Max",
	   plot->shm->getShortName());
  label[199] = '\0';

  Fl_Box *b = 
    new Fl_Box(FL_EMBOSSED_BOX, 0, y, 
	       PLOTWIDGET_WIDTH,
	       PLOTWIDGET_HEIGHT, label);
  b->align(FL_ALIGN_LEFT|FL_ALIGN_BOTTOM|FL_ALIGN_INSIDE);

  end();
}
