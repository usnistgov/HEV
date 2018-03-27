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

#if defined DTK_ARCH_LINUX || defined DTK_ARCH_DARWIN || defined DTK_ARCH_CYGWIN
  typedef socklen_t      dtkSocklen_t;
  typedef unsigned long  DTK_address_type;
  typedef unsigned short DTK_port_type;
#endif

#ifdef DTK_ARCH_WIN32_VCPP
  typedef int            dtkSocklen_t;
  typedef unsigned long  DTK_address_type;
  typedef unsigned short DTK_port_type;
#endif

#ifdef DTK_ARCH_IRIX
  typedef int              dtkSocklen_t;
  typedef in_port_t        DTK_port_type;
#  if (_MIPS_SZLONG == 32)
     typedef unsigned long DTK_address_type;
#  else
     typedef unsigned int  DTK_address_type;
  #endif
#endif
