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

#include "gage.h"
#include "privateGage.h"

const char *
gageBiffKey = "gage";

int
gageDefVerbose = 0;

gage_t
gageDefGradMagMin = 0.00001;

gage_t
gageDefGradMagCurvMin = 0.0;

int
gageDefRenormalize = AIR_FALSE;

int
gageDefCheckIntegrals = AIR_TRUE;

int
gageDefNoRepadWhenSmaller = AIR_FALSE;

int
gageDefK3Pack = AIR_TRUE;

gage_t
gageDefDefaultSpacing = 1.0;

int
gageDefCurvNormalSide = 1;

double
gageDefKernelIntegralNearZero = 0.0001;

int
gageDefRequireAllSpacings = AIR_TRUE;

int
gageDefRequireEqualCenters = AIR_TRUE;

int
gageDefDefaultCenter = nrrdCenterNode;
