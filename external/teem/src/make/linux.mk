#
# teem: Gordon Kindlmann's research software
# Copyright (C) 2003, 2002, 2001, 2000, 1999, 1998 University of Utah
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#

TEEM_SHEXT = so

# NIST mods:
# OPT_CFLAG ?= -O2
# STATIC_CFLAG = -Wl,-Bstatic
OPT_CFLAG ?= -O3 -fPIC
STATIC_CFLAG = 


SHARED_CFLAG = -Wl,-Bdynamic
SHARED_LDFLAG = -shared

TEEM_ENDIAN = 1234
TEEM_QNANHIBIT = 1
TEEM_DIO = 0
ifeq ($(SUBARCH),64)
  TEEM_32BIT = 0
  ARCH_CFLAG = -fPIC -Wall
  ARCH_LDFLAG =
else
  ifeq ($(SUBARCH),32)
    TEEM_32BIT = 1
    ARCH_CFLAG = -Wall
    ARCH_LDFLAG = 
  else
    $(error linux sub-architecture "$(SUBARCH)" not recognized)
  endif
endif
TEEM_BIGBITFIELD = 1

TEEM_ZLIB.IPATH ?=
TEEM_ZLIB.LPATH ?=

TEEM_BZIP2.IPATH ?=
TEEM_BZIP2.LPATH ?=
