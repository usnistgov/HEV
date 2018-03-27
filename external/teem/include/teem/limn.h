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

#ifndef LIMN_HAS_BEEN_INCLUDED
#define LIMN_HAS_BEEN_INCLUDED

#include <stdlib.h>

#include <math.h>

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/hest.h>
#include <teem/ell.h>
#include <teem/nrrd.h>

#if defined(_WIN32) && !defined(TEEM_STATIC)
#define limn_export __declspec(dllimport)
#else
#define limn_export
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define LIMN limnBiffKey
#define LIMN_LIGHT_NUM 16

/*
******** #define LIMN_SPLINE_Q_AVG_EPS
**
** The convergence tolerance used for Buss/Fillmore quaternion
** averaging
*/
#define LIMN_SPLINE_Q_AVG_EPS 0.00001

/*
****** limnCamera struct
**
** for all standard graphics camera parameters.  Image plane is
** spanned by U and V; N always points away from the viewer, U
** always points to the right, V can point up or down, if the
** camera is left- or right-handed, respectively.
**
** Has no dynamically allocated information or pointers.
*/
typedef struct limnCamera_t {
  double from[3],     /* location of eyepoint */
    at[3],            /* what eye is looking at */
    up[3],            /* what is up direction for eye (this is not updated
                         to the "true" up) */
    uRange[2],        /* range of U values to put on horiz. image axis */
    vRange[2],        /* range of V values to put on vert. image axis */
    neer, faar,       /* neer and far clipping plane distances
                         (misspelled for the sake of McRosopht) */
    dist;             /* distance to image plane */
  int atRelative,     /* if non-zero: given neer, faar, and dist
                         quantities indicate distance relative to the
			 _at_ point, instead of the usual (in computer
			 graphics) sense if being relative to the
			 eye point */
    orthographic,     /* no perspective projection: just orthographic */
    rightHanded;      /* if rightHanded, V = NxU (V points "downwards"),
			 otherwise, V = UxN (V points "upwards") */
  /* --------------------------------------------------------------------
     End of user-set parameters.  Things below are set by limnCameraUpdate
     -------------------------------------------------------------------- */
  double W2V[16],     /* World to view transform. The _rows_ of this
                         matrix (its 3x3 submatrix) are the U, V, N
                         vectors which form the view-space coordinate frame.
                         The column-major ordering of elements in the
                         matrix is from ell:
                         0   4   8  12
                         1   5   9  13
                         2   6  10  14
                         3   7  11  15 */
    V2W[16],          /* View to world transform */
    U[4], V[4], N[4], /* View space basis vectors (in world coords)
			 last element always zero */
    vspNeer, vspFaar, /* not usually user-set: neer, far, and image plane
			 distances, in view space */
    vspDist;
} limnCamera;

/*
******** struct limnLight
**
** information for directional lighting and the ambient light
**
** Has no dynamically allocated information or pointers
*/
typedef struct {
  float amb[3],              /* RGB ambient light color */
    _dir[LIMN_LIGHT_NUM][3], /* direction of light[i] (view or world space).
				This is what the user sets via limnLightSet */
    dir[LIMN_LIGHT_NUM][3],  /* direction of light[i] (ONLY world space) 
				Not user-set: calculated/copied from _dir[] */
    col[LIMN_LIGHT_NUM][3];  /* RGB color of light[i] */
  int on[LIMN_LIGHT_NUM],    /* light[i] is on */
    vsp[LIMN_LIGHT_NUM];     /* light[i] lives in view space */
} limnLight;

enum {
  limnDeviceUnknown,
  limnDevicePS,
  limnDeviceGL,
  limnDeviceLast
};

typedef struct {
  float edgeWidth[5],  /* different line thickness for different edge types:
			  0 : non-crease edge, backfacing
			  1 : crease edge, backfacing
			  2 : silhouette edge (crease or non-crease)
			  3 : crease edge: front-facing
			  4 : non-crease edge, front-facing */
    creaseAngle,
    bg[3];             /* background color */
} limnOptsPS;

typedef struct limnWin_t {
  limnOptsPS ps;
  int device;
  float scale, bbox[4];
  int yFlip;
  FILE *file;
} limnWin;

enum {
  limnSpaceUnknown,
  limnSpaceWorld,
  limnSpaceView,
  limnSpaceScreen,
  limnSpaceDevice,
  limnSpaceLast
};

/*
******** struct limnPoint
**
** all the information you might want for a point
**
** Has no dynamically allocated information or pointers
*/
typedef struct limnPoint_t {
  float w[4],        /* world coordinates (homogeneous) */
    v[4],            /* view coordinates */
    s[3],            /* screen coordinates (device independant) */
    d[2],            /* device coordinates */
    n[3];            /* vertex normal (world coords only) */
  int sp;            /* index into parent's SP list */
} limnPoint;

/*
******** struct limnEdge
**
** all the information about an edge
**
** Has no dynamically allocated information or pointers
*/
typedef struct limnEdge_t {
  int v0, v1,        /* two point indices (in parent's point list) */
    f0, f1,          /* two face indices (in parent's face list) */
    sp;              /* index into parent's SP list */
  
  int visib;         /* is edge currently visible (or active) */
} limnEdge;

/*
******** struct limnFace
**
** all the information about a face
**
** Has no dynamically allocated information or pointers
*/
typedef struct limnFace_t {
  float wn[3],       /* normal in world space */
    sn[3];           /* normal in screen space (post-perspective-divide) */
  int vBase, vNum,   /* start and length in parent's vertex array, "v" */
    sp;              /* index into parent's SP list, "s" */
  
  int visib;         /* is face currently visible */
  float z;           /* for depth ordering */
} limnFace;

/*
******** struct limnPart
**
** one connected part of an object
*/
typedef struct limnPart_t {
  int fBase, fNum,   /* start and length in parent's limnFace array, "f" */
    eBase, eNum,     /* start and length in parent's limnEdge array, "e" */
    pBase, pNum,     /* start and length in parent's limnPoint array, "p" */
    origIdx;         /* initial part index of this part */

  float z;           /* assuming that the occlusion graph between
			whole parts is acyclic, one depth value is
			good enough for painter's algorithm ordering
			of drawing */
  float rgba[4];
} limnPart;

/*
******** struct limnSP
**
** "surface" properties: pretty much anything having to do with 
** appearance, for points, edges, faces, etc.
*/
typedef struct limnSP_t {
  float rgba[4];
  float k[3], spec;
} limnSP;

/*
******** struct limnObj
**
** the beast used to represent polygonal objects
**
** Relies on many dynamically allocated arrays
*/
typedef struct limnObj_t {
  limnPoint *p;      /* array of point structs */
  airArray *pA;      /* airArray around "p" */

  int *v;            /* array of vertex indices for all faces */
  airArray *vA;      /* airArray around "v" */

  limnEdge *e;       /* array of edge structs */
  airArray *eA;      /* airArray around "e" */

  limnFace *f;       /* array of face structs */
  airArray *fA;      /* airArray around "f" */
  
  limnPart *r;       /* array of part structs */
  airArray *rA;      /* arrArray around "r" */
  
  limnSP *s;         /* array of surface properties */
  airArray *sA;      /* airArray around "s" */

  limnPart *rCurr;   /* pointer to part under construction */

  int edges;         /* if non-zero, build edges as faces are added */
} limnObj;

/*
******** limnQN enum
**
** the different quantized normal schemes currently supported
*/
enum {
  limnQN_Unknown,     /* 0 */
  limnQN_16checker,   /* 1 */
  limnQN_16simple,    /* 2 */
  limnQN_16border1,   /* 3 */
  limnQN_15checker,   /* 4 */
  limnQN_Last
};
#define LIMN_QN_MAX      4

enum {
  limnSplineTypeUnknown,     /* 0 */
  limnSplineTypeLinear,      /* 1 */
  limnSplineTypeTimeWarp,    /* 2 */
  limnSplineTypeHermite,     /* 3 */
  limnSplineTypeCubicBezier, /* 4 */
  limnSplineTypeBC,          /* 5 */
  limnSplineTypeLast
};
#define LIMN_SPLINE_TYPE_MAX    5

enum {
  limnSplineInfoUnknown,    /* 0 */
  limnSplineInfoScalar,     /* 1 */
  limnSplineInfo2Vector,    /* 2 */
  limnSplineInfo3Vector,    /* 3 */
  limnSplineInfoNormal,     /* 4 */
  limnSplineInfo4Vector,    /* 5 */
  limnSplineInfoQuaternion, /* 6 */
  limnSplineInfoLast
};
#define LIMN_SPLINE_INFO_MAX   6

/*
******** limnSpline
**
** the ncpt nrrd stores control point information in a 3-D nrrd, with
** sizes C by 3 by N, where C is the number of values needed for each 
** point (3 for 3Vecs, 1 for scalars), and N is the number of control
** points.  The 3 things per control point are 0) the pre-point info 
** (either inward tangent or an internal control point), 1) the control
** point itself, 2) the post-point info (e.g., outward tangent).
**
** NOTE: for the sake of simplicity, the ncpt nrrd is always "owned"
** by the limnSpline, that is, it is COPIED from the one given in 
** limnSplineNew() (and is converted to type double along the way),
** and it will is deleted with limnSplineNix.
*/
typedef struct limnSpline_t {
  int type,          /* from limnSplineType* enum */
    info,            /* from limnSplineInfo* enum */
    loop;            /* the last (implicit) control point is the first */
  double B, C;       /* B,C values for BC-splines */
  Nrrd *ncpt;        /* the control point info, ALWAYS a 3-D nrrd */
  double *time;      /* ascending times for non-uniform control points.
			Currently, only used for limnSplineTypeTimeWarp */
} limnSpline;

typedef struct limnSplineTypeSpec_t {
  int type;          /* from limnSplineType* enum */
  double B, C;       /* B,C values for BC-splines */
} limnSplineTypeSpec;

/* defaultsLimn.c */
extern limn_export const char *limnBiffKey;
extern limn_export int limnDefCameraAtRelative;
extern limn_export int limnDefCameraOrthographic;
extern limn_export int limnDefCameraRightHanded;

/* qn.c */
extern limn_export int limnQNBytes[LIMN_QN_MAX+1];
extern limn_export void (*limnQNtoV[LIMN_QN_MAX+1])(float *vec, int qn);
extern limn_export int (*limnVtoQN[LIMN_QN_MAX+1])(float *vec);

/* light.c */
extern void limnLightSet(limnLight *lit, int which, int vsp,
			 float r, float g, float b,
			 float x, float y, float z);
extern void limnLightSetAmbient(limnLight *lit, float r, float g, float b);
extern void limnLightSwitch(limnLight *lit, int which, int on);
extern void limnLightReset(limnLight *lit);
extern int limnLightUpdate(limnLight *lit, limnCamera *cam);

/* env.c */
typedef void (*limnEnvMapCB)(float rgb[3], float vec[3], void *data);
extern int limnEnvMapFill(Nrrd *envMap, limnEnvMapCB cb, 
			  int qnMethod, void *data);
extern void limnLightDiffuseCB(float rgb[3], float vec[3], void *_lit);
extern int limnEnvMapCheck(Nrrd *envMap);

/* methodsLimn.c */
extern limnLight *limnLightNew(void);
extern void limnCameraInit(limnCamera *cam);
extern limnLight *limnLightNix(limnLight *);
extern limnCamera *limnCameraNew(void);
extern limnCamera *limnCameraNix(limnCamera *cam);
extern limnWin *limnWinNew(int device);
extern limnWin *limnWinNix(limnWin *win);

/* hestLimn.c */
extern void limnHestCameraOptAdd(hestOpt **hoptP, limnCamera *cam,
				 char *frDef, char *atDef, char *upDef,
				 char *dnDef, char *diDef, char *dfDef,
				 char *urDef, char *vrDef);

/* cam.c */
extern int limnCameraUpdate(limnCamera *cam);
extern int limnCameraPathMake(limnCamera *cam, int numFrames,
			      limnCamera *keycam, double *time,
			      int numKeys, int trackFrom, 
			      limnSplineTypeSpec *quatType,
			      limnSplineTypeSpec *posType,
			      limnSplineTypeSpec *distType,
			      limnSplineTypeSpec *uvType);

/* obj.c */
extern limnObj *limnObjNew(int incr, int edges);
extern limnObj *limnObjNix(limnObj *obj);
extern int limnObjPointAdd(limnObj *obj, int sp, float x, float y, float z);
extern int limnObjEdgeAdd(limnObj *obj, int sp, int face, int v0, int v1);
extern int limnObjFaceAdd(limnObj *obj, int sp, int numVert, int *vert);
extern int limnObjSPAdd(limnObj *obj);
extern int limnObjPartStart(limnObj *obj);
extern int limnObjPartFinish(limnObj *obj);

/* io.c */
extern int limnObjDescribe(FILE *file, limnObj *obj);

/* shapes.c */
extern int limnObjCubeAdd(limnObj *obj, int sp);
extern int limnObjSquareAdd(limnObj *obj, int sp);
extern int limnObjLoneEdgeAdd(limnObj *obj, int sp);
extern int limnObjCylinderAdd(limnObj *obj, int sp, int axis,int res);
extern int limnObjPolarSphereAdd(limnObj *obj, int sp, int axis,
				 int thetaRes, int phiRes);
extern int limnObjConeAdd(limnObj *obj, int sp, int axis, int res);
extern int limnObjPolarSuperquadAdd(limnObj *obj, int sp, int axis,
				    float A, float B,
				    int thetaRes, int phiRes);

/* transform.c */
extern int limnObjHomog(limnObj *obj, int space);
extern int limnObjNormals(limnObj *obj, int space);
extern int limnObjSpaceTransform(limnObj *obj, limnCamera *cam, limnWin *win,
				 int space);
extern int limnObjPartTransform(limnObj *obj, int ri, float tx[16]);
extern int limnObjDepthSortParts(limnObj *obj);

/* renderLimn.c */
extern int limnObjRender(limnObj *obj, limnCamera *cam, limnWin *win);
extern int limnObjPSDraw(limnObj *obj, limnCamera *cam,
			 Nrrd *envMap, limnWin *win);

/* splineMethods.c */
extern limnSplineTypeSpec *limnSplineTypeSpecNew(int type, ...);
extern limnSplineTypeSpec *limnSplineTypeSpecNix(limnSplineTypeSpec *spec);
extern limnSpline *limnSplineNew(Nrrd *ncpt, int info,
				 limnSplineTypeSpec *spec);
extern limnSpline *limnSplineNix(limnSpline *spline);
extern int limnSplineNrrdCleverFix(Nrrd *nout, Nrrd *nin, int info, int type);
extern limnSpline *limnSplineCleverNew(Nrrd *ncpt, int info,
				       limnSplineTypeSpec *spec);
extern int limnSplineUpdate(limnSpline *spline, Nrrd *ncpt);

/* splineMisc.c */
extern limn_export airEnum *limnSplineType;
extern limn_export airEnum *limnSplineInfo;
extern limnSpline *limnSplineParse(char *str);
extern limnSplineTypeSpec *limnSplineTypeSpecParse(char *str);
extern limn_export hestCB *limnHestSpline;
extern limn_export hestCB *limnHestSplineTypeSpec;
extern limn_export int limnSplineInfoSize[LIMN_SPLINE_INFO_MAX+1];
extern limn_export
  int limnSplineTypeHasImplicitTangents[LIMN_SPLINE_TYPE_MAX+1];
extern int limnSplineNumPoints(limnSpline *spline);
extern double limnSplineMinT(limnSpline *spline);
extern double limnSplineMaxT(limnSpline *spline);
extern void limnSplineBCSet(limnSpline *spline, double B, double C);

/* splineEval.c */
extern void limnSplineEvaluate(double *out, limnSpline *spline, double time);
extern int limnSplineNrrdEvaluate(Nrrd *nout, limnSpline *spline, Nrrd *nin);
extern int limnSplineSample(Nrrd *nout, limnSpline *spline,
			    double minT, int M, double maxT);


#ifdef __cplusplus
}
#endif

#endif /* LIMN_HAS_BEEN_INCLUDED */
