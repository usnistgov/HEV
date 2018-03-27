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

 struct dtkBasicDisplay_drawList
  {
    int (*draw)(void);
    struct dtkBasicDisplay_drawList *next;
 };

class dtkBasicDisplay: public dtkDisplay
{
 public:

  dtkBasicDisplay(dtkManager *m, const char *name);
  virtual ~dtkBasicDisplay(void);

  // add a draw callback function.
  void add(int (*drawFunction)(void));
  void remove(int (*drawFunction)(void)=NULL);

 protected:

  dtkManager *manager;

  // This can be made private in the future.
  struct dtkBasicDisplay_drawList *drawList;
};
