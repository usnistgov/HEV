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

/*! These magic numbers are used to call:
 *
 * int dtkManager::preFrame(unsigned int magic_num);
 * 
 * and
 *
 * int dtkManager::postFrame(unsigned int magic_num);
 *
 * We added this so general users would not call them.
 */

#define __DTKMANAGER_PREFRAME_MAGIC_     ((unsigned int) 0x0270008B)
#define __DTKMANAGER_POSTFRAME_MAGIC_    ((unsigned int) 0x02700151)
#define __DTKMANAGER_PREPOSTFRAME_MAGIC_ ((unsigned int) 0x02700157)
#define __DTKMANAGER_NOSYNCFORCING_      ((unsigned int) 0x02700197)
