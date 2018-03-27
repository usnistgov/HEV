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

#include "scope.h"


dtkSharedMem **Usage(void)
{
  dtkColor c(stdout);

  printf("  Usage: %s%s%s %sSHM_FILES ...%s [-p|--poll] [%s-s%s|%s--scale%s %sSHM_FILE INDX MIN MAX%s]\\\n"
	 "              [FLTK_OPTIONS]\n",
	 c.vil, PROGRAM_NAME, c.end,
	 c.tur, c.end, c.grn, c.end, c.grn, c.end, c.tur, c.end);
  printf("\n"
	 "   This runs an interactive \"oscilloscope-like\" view of the %s%ss%s that are\n"
	 "  in the DTK shared memory, from the shared memory file(s) %sSHM_FILES%s.\n",
	 c.red, TYPE_STRING, c.end, c.tur, c.end);
  printf("\n"
	 "  %s-h%s|%s--help%s                  print this help and than exit\n",
	 c.grn, c.end, c.grn, c.end);
  printf("  %s-p%s|%s--poll%s                  use polling reads from DTK shared memory.  This may be\n"
	 "                             handy if filters are loaded with the shared memory\n"
	 "                             since filters are not used when reading from a queue.\n"
	 "                             By default the data is read from a shared memory queue.\n",
	 c.grn, c.end, c.grn, c.end);
  printf("  %s-s%s|%s--scale%s %sSHM_FILE INDX MIN MAX%s   make the %sMIN%s and %sMAX%s be the used to make\n"
	 "                             the scale of the plot for the %s%s%s with array\n"
	 "                             index %sINDX%s of shared memory from file %sSHM_FILE%s.\n",
	 c.grn, c.end, c.grn, c.end,   c.tur, c.end, c.tur, c.end, c.tur, c.end,
	 c.red, TYPE_STRING, c.end, c.tur, c.end, 
	 c.tur, c.end);
  // This prints to stdout
  dtkFLTKOptions_print();

  return NULL;
}


