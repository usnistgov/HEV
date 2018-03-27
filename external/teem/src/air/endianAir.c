/*
  teem: Gordon Kindlmann's research software
  Copyright (C) 2003, 2002, 2001, 2000, 1999, 1998 University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "air.h"
#include <teemEndian.h>

/*
******** int airMyEndian
**
** it gets set to 1234 or 4321
*/
#if TEEM_ENDIAN == 1234
const int airMyEndian = 1234;
#else
const int airMyEndian = 4321;
#endif

char
_airEndianStr[][AIR_STRLEN_SMALL] = {
  "(unknown endian)",
  "little",
  "big"
};

char
_airEndianDesc[][AIR_STRLEN_MED] = {
  "unknown endianness",
  "Intel and compatible",
  "Everyone besides Intel and compatible"
};

int
_airEndianVal[] = {
  airEndianUnknown,
  airEndianLittle,
  airEndianBig,
};

airEnum
_airEndian = {
  "endian",
  2,
  _airEndianStr, _airEndianVal,
  _airEndianDesc,
  NULL, NULL,
  AIR_FALSE
};

airEnum *
airEndian = &_airEndian;

