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

#include "hest.h"
#include "privateHest.h"

int hestVerbosity = 0;
int hestRespFileEnable = AIR_FALSE;
int hestColumns = 79;
int hestElideSingleEnumType = AIR_FALSE;
int hestElideSingleOtherType = AIR_FALSE;
int hestElideSingleOtherDefault = AIR_FALSE;
int hestElideSingleNonExistFloatDefault = AIR_FALSE;
int hestElideMultipleNonExistFloatDefault = AIR_FALSE;
int hestElideSingleEmptyStringDefault = AIR_FALSE;
int hestElideMultipleEmptyStringDefault = AIR_FALSE;
int hestGreedySingleString = AIR_TRUE;
int hestCleverPluralizeOtherY = AIR_FALSE;
char hestRespFileFlag = '@';
char hestRespFileComment = '#';
char hestVarParamStopFlag = '-';
char hestMultiFlagSep = ',';
