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

#include "echo.h"
#include "privateEcho.h"

echoPos_t
_echo_SuperquadX_v(echoPos_t x, echoPos_t y, echoPos_t z,
		   echoPos_t A, echoPos_t B) {
  echoPos_t xxb, yya, zza;

  xxb = pow(x*x, 1/B);  yya = pow(y*y, 1/A);  zza = pow(z*z, 1/A);
  return pow(yya + zza, A/B) + xxb - 1;
}

echoPos_t
_echo_SuperquadY_v(echoPos_t x, echoPos_t y, echoPos_t z,
		   echoPos_t A, echoPos_t B) {
  echoPos_t xxa, yyb, zza;

  xxa = pow(x*x, 1/A);  yyb = pow(y*y, 1/B);  zza = pow(z*z, 1/A);
  return pow(xxa + zza, A/B) + yyb - 1;
}

echoPos_t
_echo_SuperquadZ_v(echoPos_t x, echoPos_t y, echoPos_t z,
		   echoPos_t A, echoPos_t B) {
  echoPos_t xxa, yya, zzb;

  xxa = pow(x*x, 1/A);  yya = pow(y*y, 1/A);  zzb = pow(z*z, 1/B);
  return pow(xxa + yya, A/B) + zzb - 1;
}

/* -------------------------------------------------------- */

echoPos_t
_echo_SuperquadX_vg(echoPos_t grad[3],
		    echoPos_t x, echoPos_t y, echoPos_t z,
		    echoPos_t A, echoPos_t B) {
  echoPos_t R, xxb, yya, zza;

  xxb = pow(x*x, 1/B);  yya = pow(y*y, 1/A);  zza = pow(z*z, 1/A);
  R = pow(yya + zza, (A/B)-1);
  ELL_3V_SET(grad, 2*xxb/(B*x), 2*R*yya/(B*y), 2*R*zza/(B*z));
  return pow(yya + zza, A/B) + xxb - 1;
}

echoPos_t
_echo_SuperquadY_vg(echoPos_t grad[3],
		    echoPos_t x, echoPos_t y, echoPos_t z,
		    echoPos_t A, echoPos_t B) {
  echoPos_t R, xxa, yyb, zza;

  xxa = pow(x*x, 1/A);  yyb = pow(y*y, 1/B);  zza = pow(z*z, 1/A);
  R = pow(xxa + zza, (A/B)-1);
  ELL_3V_SET(grad, 2*R*xxa/(B*x), 2*yyb/(B*y), 2*R*zza/(B*z));
  return pow(xxa + zza, A/B) + yyb - 1;
}

echoPos_t
_echo_SuperquadZ_vg(echoPos_t grad[3],
		    echoPos_t x, echoPos_t y, echoPos_t z,
		    echoPos_t A, echoPos_t B) {
  echoPos_t R, xxa, yya, zzb;

  xxa = pow(x*x, 1/A);  yya = pow(y*y, 1/A);  zzb = pow(z*z, 1/B);
  R = pow(xxa + yya, (A/B)-1);
  ELL_3V_SET(grad, 2*R*xxa/(B*x), 2*R*yya/(B*y), 2*zzb/(B*z));
  return pow(xxa + yya, A/B) + zzb - 1;
}

/* -------------------------------------------------------- */

echoPos_t
_echo_SuperquadX_lvg(echoPos_t grad[3],
		     echoPos_t x, echoPos_t y, echoPos_t z,
		     echoPos_t A, echoPos_t B) {
  echoPos_t R, xxb, yya, zza;

  xxb = pow(x*x, 1/B);  yya = pow(y*y, 1/A);  zza = pow(z*z, 1/A);
  R = pow(yya + zza, 1-(A/B))*xxb;
  ELL_3V_SET(grad,
	     2/(B*x*(1 + pow(yya + zza, A/B)/xxb)),
	     2*yya/(B*y*(yya + zza + R)),
	     2*zza/(B*z*(yya + zza + R)));
  return log(pow(yya + zza, A/B) + xxb);
}

echoPos_t
_echo_SuperquadY_lvg(echoPos_t grad[3],
		     echoPos_t x, echoPos_t y, echoPos_t z,
		     echoPos_t A, echoPos_t B) {
  echoPos_t R, xxa, yyb, zza;

  xxa = pow(x*x, 1/A);  yyb = pow(y*y, 1/B);  zza = pow(z*z, 1/A);
  R = pow(xxa + zza, 1-(A/B))*yyb;
  ELL_3V_SET(grad,
	     2*xxa/(B*x*(xxa + zza + R)),
	     2/(B*y*(1 + pow(xxa + zza, A/B)/yyb)),
	     2*zza/(B*z*(xxa + zza + R)));
  return log(pow(xxa + zza, A/B) + yyb);
}

echoPos_t
_echo_SuperquadZ_lvg(echoPos_t grad[3],
		     echoPos_t x, echoPos_t y, echoPos_t z,
		     echoPos_t A, echoPos_t B) {
  echoPos_t R, xxa, yya, zzb;

  xxa = pow(x*x, 1/A);  yya = pow(y*y, 1/A);  zzb = pow(z*z, 1/B);
  R = pow(xxa + yya, 1-(A/B))*zzb;
  ELL_3V_SET(grad,
	     2*xxa/(B*x*(xxa + yya + R)),
	     2*yya/(B*y*(xxa + yya + R)),
	     2/(B*z*(1 + pow(xxa + yya, A/B)/zzb)));
  return log(pow(xxa + yya, A/B) + zzb);
}

/* -------------------------------------------------------- */

int
_echoRayIntx_Superquad(RAYINTX_ARGS(Superquad)) {
  char me[]="_echoRayIntx_Superquad";
  echoPos_t TT=0, Tmin, Tmax, t0, t1, t2, t3, v1, v2, diff, tol,
    saveTmin, Vmin, Vmax, VV=0, dV, dVmin, dVmax, tmp,
    (*v)(echoPos_t, echoPos_t, echoPos_t,
	 echoPos_t, echoPos_t),
    (*vg)(echoPos_t[3],
	  echoPos_t, echoPos_t, echoPos_t,
	  echoPos_t, echoPos_t),
    (*lvg)(echoPos_t[3],
	   echoPos_t, echoPos_t, echoPos_t,
	   echoPos_t, echoPos_t),
    from[3], grad[3], pos[3];  /* these two used only by macros */
  int iter;
  
  if (!_echoRayIntx_CubeSolid(&Tmin, &Tmax,
			      -1-2*ECHO_EPSILON, 1+2*ECHO_EPSILON,
			      -1-2*ECHO_EPSILON, 1+2*ECHO_EPSILON,
			      -1-2*ECHO_EPSILON, 1+2*ECHO_EPSILON, ray)) {
    return AIR_FALSE;
  }
  switch(obj->axis) {
    case 0:
      v = _echo_SuperquadX_v;
      vg = _echo_SuperquadX_vg;
      lvg = _echo_SuperquadX_lvg;
      break;
    case 1:
      v = _echo_SuperquadY_v;
      vg = _echo_SuperquadY_vg;
      lvg = _echo_SuperquadY_lvg;
      break;
    case 2: default:
      v = _echo_SuperquadZ_v;
      vg = _echo_SuperquadZ_vg;
      lvg = _echo_SuperquadZ_lvg;
      break;
  }
  if (tstate->verbose) {
    fprintf(stderr, "%s%s: Tmin, Tmax = %g, %g, ax = %d\n",
	    _echoDot(tstate->depth), me, Tmin, Tmax, obj->axis);
  }

#define VAL(TT)                               \
  (ELL_3V_SCALE_ADD2(pos, 1, from, (TT), ray->dir),  \
   v(pos[0], pos[1], pos[2], obj->A, obj->B))

#define VALGRAD(VV, DV, TT)                                \
  ELL_3V_SCALE_ADD2(pos, 1, from, (TT), ray->dir);                \
  (VV) = vg(grad, pos[0], pos[1], pos[2], obj->A, obj->B); \
  (DV) = ELL_3V_DOT(grad, ray->dir)

#define LVALGRAD(VV, DV, TT)                                \
  ELL_3V_SCALE_ADD2(pos, 1, from, (TT), ray->dir);                 \
  (VV) = lvg(grad, pos[0], pos[1], pos[2], obj->A, obj->B); \
  (DV) = ELL_3V_DOT(grad, ray->dir)

#define RR 0.61803399
#define CC (1.0-RR)
#define SHIFT3(a,b,c,d) (a)=(b); (b)=(c); (c)=(d)
#define SHIFT2(a,b,c)   (a)=(b); (b)=(c)

  /* testing */
  ELL_3V_SCALE_ADD2(from, 1, ray->from, Tmin, ray->dir);
  saveTmin = Tmin;
  Tmin = 0;
  Tmax -= saveTmin;
  /*
  ELL_3V_COPY(from, ray->from);
  saveTmin = 0;
  */

  /* evaluate value and derivatives at Tmin and Tmax */
  VALGRAD(Vmin, dVmin, Tmin);
  VALGRAD(Vmax, dVmax, Tmax);

  /* if the segment start and end are both positive or both negative,
     and if the derivatives also don't change sign, there's no root.
     Also, due to convexity, if values at start and end are negative,
     then there is no root */
  if ( (Vmin*Vmax >= 0 && dVmin*dVmax >= 0) 
       || (Vmin <= 0 && Vmax <= 0) ) {
    return AIR_FALSE;
  }
  if (tstate->verbose) {
    fprintf(stderr, "%s%s: dVmin = %g, dVmax = %g, Vmin = %g, Vmax = %g\n",
	    _echoDot(tstate->depth), me, dVmin, dVmax, Vmin, Vmax);
  }

  /* either the value changed sign, or the derivative changed sign, or
     both.  If, as is common, the derivatives changed sign, but the
     values didn't (which means they are both positive, due to a test
     above), we need to limit the interval by minimizing the value
     until either we see a negative value, or until the minimization
     converged.  Based on Golden Section Search, NR pg.401 */
  if (dVmin*dVmax < 0 && Vmin*Vmax >= 0) {
    t0 = Tmin;
    t1 = RR*Tmin + CC*Tmax;
    t2 = CC*Tmin + RR*Tmax;
    t3 = Tmax;
    v1 = VAL(t1);
    v2 = VAL(t2);
    if (tstate->verbose) {
      fprintf(stderr, "%s%s: \n"
	      "     t0 = % 31.15f\n"
	      "     t1 = % 31.15f  -> v1 = % 31.15f\n"
	      "     t2 = % 31.15f  -> v2 = % 31.15f\n"
	      "     t3 = % 31.15f\n",
	      _echoDot(tstate->depth), me,
	      t0, t1, v1, t2, v2, t3);
    }
    tol = sqrt(ECHO_POS_EPS);
    while ( (t3-t0 > tol*(t1+t2))         /* still haven't converged */
	    && (v1 > 0 && v2 > 0) ) {     /* v1 and v2 both positive */
      diff = v2 - v1;
      if (v1 < v2) {
	SHIFT3(t3, t2, t1, CC*t0 + RR*t2);
	SHIFT2(v2, v1, VAL(t1));
      } else {
	SHIFT3(t0, t1, t2, RR*t1 + CC*t3);
	SHIFT2(v1, v2, VAL(t2));
      }
      if (tstate->verbose) {
	fprintf(stderr, "%s%s: %s ---> \n"
		"     t0 = % 31.15f\n"
		"     t1 = % 31.15f  -> v1 = % 31.15f\n"
		"     t2 = % 31.15f  -> v2 = % 31.15f\n"
		"     t3 = % 31.15f\n",
		_echoDot(tstate->depth), me,
		diff > 0 ? "v1 < v2" : "v1 > v2",
		t0, t1, v1, t2, v2, t3);
      }
    }
    if (v1 > 0 && v2 > 0) {
      /* the minimization stopped, yet both v1 and v2 are still positive,
	 so there's no way we can have a root */
      if (tstate->verbose) {
	fprintf(stderr, "%s%s: minimization found no root\n",
		_echoDot(tstate->depth), me);
      }
      return AIR_FALSE;
    }
    /* else either v1 or v2 <= 0, so there is a root (actually two).
       By construction, f(t0) is positive, so we can now bracket the
       root between t0 and t1 or t2, whichever one is associated with
       a smaller value */
    Tmin = t0;
    Tmax = v1 < v2 ? t1 : t2;
    Vmin = VAL(Tmin);
    Vmax = VAL(Tmax);
  }

  /* the value isn't necessarily monotonic between Tmin and Tmax, but
     we know that there is only one root. Find it with newton-raphson,
     using the log of function, both for values and for derivatives */
  iter = 0;
  TT = (Tmin + Tmax)/2;
  LVALGRAD(VV, dV, TT);
  while (iter < parm->sqNRI && AIR_ABS(VV) > parm->sqTol) {
    if (tstate->verbose) {
      fprintf(stderr, "%s%s: iter = %d: TT = %g, VV = %g, dV = %g\n",
	      _echoDot(tstate->depth), me, iter, TT, VV, dV);
    }
    TT -= VV/dV;
    if (!AIR_IN_OP(Tmin, TT, Tmax)) {
      /* newton-raphson sent us flying out of bounds; find a tighter
	 [Tmin,Tmax] bracket with bisection and try again */
      TT = (Tmin + Tmax)/2;
      VV = VAL(TT);
      if (Vmin*VV < 0) {
	Tmax = TT;
	Vmax = VV;
      } else {
	Tmin = TT;
	Vmin = VV;
      }
      TT = (Tmin + Tmax)/2;
    }
    LVALGRAD(VV, dV, TT);
    iter++;
  }

  intx->t = TT + saveTmin;
  VALGRAD(VV, dV, TT);   /* puts gradient into grad */
  ELL_3V_NORM(intx->norm, grad, tmp);
  intx->obj = OBJECT(obj);
  /* set in intx:
     yes: t, norm
     no: u, v
  */
  return AIR_TRUE;
}
