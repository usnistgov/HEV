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

#ifndef ECHO_HAS_BEEN_INCLUDED
#define ECHO_HAS_BEEN_INCLUDED

#include <stdio.h>
#include <math.h>

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/ell.h>
#include <teem/nrrd.h>
#include <teem/limn.h>

#if defined(_WIN32) && !defined(TEEM_STATIC)
#define echo_export __declspec(dllimport)
#else
#define echo_export
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ECHO echoBiffKey

/* all position and transform information is kept as ...
** 1: floats
** 0: doubles
*/
#if 0
typedef float echoPos_t;
#define ECHO_POS_FLOAT 1
#else 
typedef double echoPos_t;
#define ECHO_POS_FLOAT 0
#endif


/* all color information is kept as 
** 1: floats
** 0: doubles
*/
#if 1
typedef float echoCol_t;
#define echoCol_nt nrrdTypeFloat
#else
typedef double echoCol_t;
#define echoCol_nt nrrdTypeDouble
#endif

#define ECHO_LIST_OBJECT_INCR 32
#define ECHO_IMG_CHANNELS 5
#define ECHO_EPSILON 0.00005      /* used for adjusting ray positions */
#define ECHO_NEAR0 0.004          /* used for comparing transparency to zero */
#define ECHO_LEN_SMALL_ENOUGH 5   /* to control splitting for split objects */

typedef struct {
  int jitterType,      /* from echoJitter* enum below */
    reuseJitter,       /* don't recompute jitter offsets per pixel */
    permuteJitter,     /* properly permute the various jitter arrays */
    doShadows,         /* do shadowing with shadow rays */
    textureNN,         /* use nearest-neighbor for texture lookups 
			  (rather than bilinear interpolation) */
    numSamples,        /* rays per pixel */
    imgResU, imgResV,  /* horz. and vert. image resolution */
    maxRecDepth,       /* max recursion depth */
    renderLights,      /* render the area lights */
    renderBoxes,       /* faintly render bounding boxes */
    seedRand,          /* call airSrand() (don't if repeatability wanted) */
    sqNRI;             /* how many iterations of newton-raphson we allow for
			  finding superquadric root (within tolorance sqTol) */
  echoPos_t
    sqTol;             /* how close newtwon-raphson must get to zero */
  echoCol_t
    glassC;            /* should really be an additional material parameter:
			  Beer's law attenuation in glass */
  float aperture,      /* shallowness of field */
    timeGamma,         /* gamma for values in time image */
    boxOpac;           /* opacity of bounding boxes with renderBoxes */
  echoCol_t
    maxRecCol[3];      /* color of max recursion depth being hit */
} echoRTParm;

typedef struct {
  int verbose;
  double time;         /* time it took to render image */
} echoGlobalState;

typedef struct {
  int verbose,
    depth;             /* how many recursion levels are we at */
  Nrrd *nperm,         /* ECHO_JITTABLE_NUM x parm->numSamples array 
			  of ints, each column is a (different) random
			  permutation of [0 .. parm->numSamples-1], each
			  row corresponds to the different jittables for
			  a single sample */
    *njitt;            /* 2 x ECHO_JITTABLE_NUM x parm->numSamples array
			  of echoPos_t's in domain [-1/2,1/2]; like the nperm
			  array, each row is comprised of the jitter vectors
			  (for all possible jittables) to use for one sample */
  int *permBuff;       /* temp array for creating permutations */
  echoPos_t *jitt;     /* pointer into njitt, good for current sample */
  echoCol_t *chanBuff; /* for storing ray color and other parameters for each
			  of the parm->numSamples rays in current pixel */
} echoThreadState;

/*
******** echoJitter* enum
** 
** the different jitter patterns that are supported.  This setting is
** global- you can't have different jitter patterns on the lights versus
** the pixels.
*/
enum {
  echoJitterUnknown=-1,
  echoJitterNone,       /* 0: N samples all at the square center */
  echoJitterGrid,       /* 1: N samples exactly on a sqrt(N) x sqrt(N) grid */
  echoJitterJitter,     /* 2: N jittered samples on a sqrt(N) x sqrt(N) grid */
  echoJitterRandom,     /* 3: N samples randomly placed in square */
  echoJitterLast
};
#define ECHO_JITTER_NUM    4

/*
******** echoJittable* enum
**
** the different quantities to which the jitter two-vector may be
** applied.  
*/
enum {
  echoJittableUnknown=-1,
  echoJittablePixel,      /* 0 */
  echoJittableLight,      /* 1 */
  echoJittableLens,       /* 2 */
  echoJittableNormalA,    /* 3 */
  echoJittableNormalB,    /* 4 */
  echoJittableMotionA,    /* 5 */
  echoJittableMotionB,    /* 6 */
  echoJittableLast
};
#define ECHO_JITTABLE_NUM    7

/*
******** echoMatter* enum
**
** the different materials that are supported.  This setting determines
** the interpretation of the vector of floats/doubles ("mat[]") that
** constitutes material information.  All objects have an rgba[] array
** seperate from material information.  The Light material is currently only
** supported on rectangles.
*/
enum {
  echoMatterUnknown=0,
  echoMatterPhong,      /* 1 */
  echoMatterGlass,      /* 2 */
  echoMatterMetal,      /* 3 */
  echoMatterLight,      /* 4 */
  echoMatterLast
};
#define ECHO_MATTER_MAX    4

enum {
  echoMatterPhongKa,    /* 0 */
  echoMatterPhongKd,    /* 1 */
  echoMatterPhongKs,    /* 2 */
  echoMatterPhongSp,    /* 3 */
};
enum {
  echoMatterGlassIndex, /* 0 */
  echoMatterGlassKa,    /* 1 */
  echoMatterGlassKd,    /* 2 */
  echoMatterGlassFuzzy  /* 3 */
};
enum {
  echoMatterMetalR0,    /* 0 */
  echoMatterMetalKa,    /* 1 */
  echoMatterMetalKd,    /* 2 */
  echoMatterMetalFuzzy  /* 3 */
};
enum {
  echoMatterLightPower, /* 0 */
  echoMatterLightUnit   /* 1 : (takes over role of old parm->refDistance)
			   distance to be considered unity when calculating
			   inverse square fall-off of light intensity, or,
			   use 0.0 to mean "this is a directional light"
			   (with no fall-off at all) */
};

#define ECHO_MATTER_PARM_NUM 4

/*
******** echoType* enum
**
** the types of objects that echo supports
*/
enum {
  echoTypeUnknown=-1,
  echoTypeSphere,         /*  0 */
  echoTypeCylinder,       /*  1 */
  echoTypeSuperquad,      /*  2 */
  echoTypeCube,           /*  3 */
  echoTypeTriangle,       /*  4 */
  echoTypeRectangle,      /*  5 */
  echoTypeTriMesh,        /*  6: only triangles in the mesh */
  echoTypeIsosurface,     /*  7 */
  echoTypeAABBox,         /*  8 */
  echoTypeSplit,          /*  9 */
  echoTypeList,           /* 10 */
  echoTypeInstance,       /* 11 */
  echoTypeLast
};

#define ECHO_TYPE_NUM        12

/*
******** echoObject (generic) and all other object structs
**
** every starts with ECHO_OBJECT_COMMON, and all the "real" objects
** have a ECHO_OBJECT_MATTER following that
*/

#define ECHO_OBJECT_COMMON              \
  signed char type

#define ECHO_OBJECT_MATTER              \
  unsigned char matter;                 \
  echoCol_t rgba[4];                    \
  echoCol_t mat[ECHO_MATTER_PARM_NUM];  \
  Nrrd *ntext

typedef struct {
  ECHO_OBJECT_COMMON;
  ECHO_OBJECT_MATTER;   /* ha! its not actually in every object, but in
			   those cases were we want to access it without
			   knowing object type, then it will be there ... */
} echoObject;

typedef struct {
  ECHO_OBJECT_COMMON;
  ECHO_OBJECT_MATTER;
  echoPos_t pos[3], rad;
} echoSphere;

typedef struct {
  ECHO_OBJECT_COMMON;
  ECHO_OBJECT_MATTER;
  int axis;
} echoCylinder;

typedef struct {
  ECHO_OBJECT_COMMON;
  ECHO_OBJECT_MATTER;
  int axis;
  echoPos_t A, B;
} echoSuperquad;

/* edges are unit length, [-0.5, 0.5] on every edge */
typedef struct {
  ECHO_OBJECT_COMMON;
  ECHO_OBJECT_MATTER;
} echoCube;

typedef struct {
  ECHO_OBJECT_COMMON;
  ECHO_OBJECT_MATTER;
  echoPos_t vert[3][3];  /* e0 = vert[1]-vert[0],
			    e1 = vert[2]-vert[0],
			    normal = e0 x e1 */
} echoTriangle;

typedef struct {
  ECHO_OBJECT_COMMON;
  ECHO_OBJECT_MATTER;
  echoPos_t origin[3], edge0[3], edge1[3];
} echoRectangle;

typedef struct {
  ECHO_OBJECT_COMMON;
  ECHO_OBJECT_MATTER;
  echoPos_t meanvert[3], min[3], max[3];
  int numV, numF;
  echoPos_t *pos;
  int *vert;
} echoTriMesh;

typedef struct {
  ECHO_OBJECT_COMMON;
  ECHO_OBJECT_MATTER;
  /* this needs more stuff, perhaps a gageContext */
  Nrrd *volume;
  float value;
} echoIsosurface;

typedef struct {
  ECHO_OBJECT_COMMON;
  echoObject *obj;
  echoPos_t min[3], max[3];
} echoAABBox;

typedef struct {
  ECHO_OBJECT_COMMON;
  int axis;                    /* which axis was split: 0:X, 1:Y, 2:Z */
  echoPos_t min0[3], max0[3],
    min1[3], max1[3];          /* bboxes of two children */
  echoObject *obj0, *obj1;     /* two splits, or ??? */
} echoSplit;

typedef struct {
  ECHO_OBJECT_COMMON;
  echoObject **obj;
  airArray *objArr;
} echoList;  

typedef struct {
  ECHO_OBJECT_COMMON;
  echoPos_t Mi[16], M[16];
  echoObject *obj;
} echoInstance;

/*
******** echoScene
**
** this is the central list of all objects in a scene, and all nrrds
** used for textures and isosurface volumes.  The scene "owns" all 
** the objects it points to, so that nixing it will cause all objects
** and nrrds to be nixed and nuked, respectively.
*/
typedef struct {
  echoObject **cat;    /* array of ALL objects and all lights */
  airArray *catArr;
  echoObject **rend;   /* array of top-level objects to be rendered */
  airArray *rendArr;
  echoObject **light;  /* convenience pointers to lights within cat[] */
  airArray *lightArr;
  Nrrd **nrrd;         /* nrrds for textures and isosurfaces */
  airArray *nrrdArr;
  Nrrd *envmap;        /* 16checker-based diffuse environment map,
			  not touched by echoSceneNix() */
  echoCol_t ambi[3],   /* color of ambient light */
    bkgr[3];           /* color of background */
} echoScene;

/*
******** echoRay
**
** all info associated with a ray being intersected against a scene
*/
typedef struct {
  echoPos_t from[3],    /* ray comes from this point */
    dir[3],             /* ray goes in this (not normalized) direction */
    neer, faar;         /* look for intx in this interval */
  int shadow;           /* this is a shadow ray */
  echoCol_t transp;     /* for shadow rays, the transparency so far; starts
			   at 1.0, goes down to 0.0 */
} echoRay;

/*
******** echoIntx
**
** all info about nature and location of an intersection 
*/
typedef struct {
  echoObject *obj;      /* computed with every intersection */
  echoPos_t t,          /* computed with every intersection */
    u, v;               /* sometimes needed for texturing */
  echoPos_t norm[3],    /* computed with every intersection */
    view[3],            /* always used with coloring */
    refl[3],            /* reflection of view across line spanned by normal */
    pos[3];             /* always used with coloring (and perhaps texturing) */
  int face,             /* in intx with cube, which face was hit 
			   (used for textures) */
    boxhits;            /* how many bounding boxes we hit */
} echoIntx;

/* enumsEcho.c ------------------------------------------ */
extern echo_export airEnum *echoJitter;
extern echo_export airEnum *echoType;
extern echo_export airEnum *echoMatter;

/* methodsEcho.c --------------------------------------- */
extern echo_export const char *echoBiffKey;
extern echoRTParm *echoRTParmNew();
extern echoRTParm *echoRTParmNix(echoRTParm *parm);
extern echoGlobalState *echoGlobalStateNew();
extern echoGlobalState *echoGlobalStateNix(echoGlobalState *state);
extern echoThreadState *echoThreadStateNew();
extern echoThreadState *echoThreadStateNix(echoThreadState *state);
extern echoScene *echoSceneNew();
extern echoScene *echoSceneNix(echoScene *scene);

/* objmethods.c --------------------------------------- */
extern echoObject *echoObjectNew(echoScene *scene, signed char type);
extern int echoObjectAdd(echoScene *scene, echoObject *obj);
extern echoObject *echoObjectNix(echoObject *obj);

/* model.c ---------------------------------------- */
extern echoObject *echoRoughSphereNew(echoScene *scene, int theRes, int phiRes,
				      echoPos_t *matx);

/* bounds.c --------------------------------------- */
extern void echoBoundsGet(echoPos_t *lo, echoPos_t *hi, echoObject *obj);

/* list.c --------------------------------------- */
extern void echoListAdd(echoObject *parent, echoObject *child);
extern echoObject *echoListSplit(echoScene *scene,
				 echoObject *list, int axis);
extern echoObject *echoListSplit3(echoScene *scene,
				  echoObject *list, int depth);

/* set.c --------------------------------------- */
extern void echoSphereSet(echoObject *sphere,
			  echoPos_t x, echoPos_t y,
			  echoPos_t z, echoPos_t rad);
extern void echoCylinderSet(echoObject *cylind,
			    int axis);
extern void echoSuperquadSet(echoObject *squad,
			     int axis, echoPos_t A, echoPos_t B);
extern void echoRectangleSet(echoObject *rect,
			     echoPos_t ogx, echoPos_t ogy, echoPos_t ogz,
			     echoPos_t x0, echoPos_t y0, echoPos_t z0,
			     echoPos_t x1, echoPos_t y1, echoPos_t z1);
extern void echoTriangleSet(echoObject *tri,
			    echoPos_t x0, echoPos_t y0, echoPos_t z0, 
			    echoPos_t x1, echoPos_t y1, echoPos_t z1, 
			    echoPos_t x2, echoPos_t y2, echoPos_t z2);
extern void echoTriMeshSet(echoObject *trim,
			   int numV, echoPos_t *pos,
			   int numF, int *vert);
extern void echoInstanceSet(echoObject *inst,
			    echoPos_t *M, echoObject *obj);

/* matter.c ------------------------------------------ */
extern echo_export int echoObjectHasMatter[ECHO_TYPE_NUM];
extern void echoColorSet(echoObject *obj,
			 echoCol_t R, echoCol_t G, echoCol_t B, echoCol_t A);
extern void echoMatterPhongSet(echoScene *scene, echoObject *obj,
			       echoCol_t ka, echoCol_t kd,
			       echoCol_t ks, echoCol_t sp);
extern void echoMatterGlassSet(echoScene *scene, echoObject *obj,
			       echoCol_t index, echoCol_t ka,
			       echoCol_t kd, echoCol_t fuzzy);
extern void echoMatterMetalSet(echoScene *scene, echoObject *obj,
			       echoCol_t R0, echoCol_t ka,
			       echoCol_t kd, echoCol_t fuzzy);
extern void echoMatterLightSet(echoScene *scene, echoObject *obj,
			       echoCol_t power, echoCol_t unit);
extern void echoMatterTextureSet(echoScene *scene, echoObject *obj,
				 Nrrd *ntext);

/* lightEcho.c ------------------------------------------- */
extern void echoLightPosition(echoPos_t pos[3], echoObject *light,
			      echoThreadState *tstate);
extern void echoLightColor(echoCol_t rgb[3], echoPos_t Ldist,
			   echoObject *light, echoRTParm *parm,
			   echoThreadState *tstate);
extern void echoEnvmapLookup(echoCol_t rgb[3], echoPos_t norm[3],
			     Nrrd *envmap);

/* color.c ------------------------------------------- */
extern void echoTextureLookup(echoCol_t rgba[4], Nrrd *ntext,
			      echoPos_t u, echoPos_t v, echoRTParm *parm);
extern void echoIntxMaterialColor(echoCol_t rgba[4], echoIntx *intx,
				  echoRTParm *parm);
extern void echoIntxLightColor(echoCol_t ambi[3], echoCol_t diff[3],
			       echoCol_t spec[3], echoCol_t sp,
			       echoIntx *intx, echoScene *scene,
			       echoRTParm *parm, echoThreadState *tstate);
extern void echoIntxFuzzify(echoIntx *intx, echoCol_t fuzz,
			    echoThreadState *tstate);

/* intx.c ------------------------------------------- */
extern int echoRayIntx(echoIntx *intx, echoRay *ray, echoScene *scene,
		       echoRTParm *parm, echoThreadState *tstate);
extern void echoIntxColor(echoCol_t rgba[4], echoIntx *intx,
			  echoScene *scene, echoRTParm *parm,
			  echoThreadState *tstate);

/* renderEcho.c ---------------------------------------- */
extern int echoThreadStateInit(echoThreadState *tstate,
			       echoRTParm *parm, echoGlobalState *gstate);
extern void echoJitterCompute(echoRTParm *parm, echoThreadState *state);
extern void echoRayColor(echoCol_t rgba[4], echoRay *ray,
			 echoScene *scene, echoRTParm *parm,
			 echoThreadState *tstate);
extern void echoChannelAverage(echoCol_t *img,
			       echoRTParm *parm, echoThreadState *tstate);
extern int echoRTRenderCheck(Nrrd *nraw, limnCamera *cam, echoScene *scene,
			     echoRTParm *parm, echoGlobalState *gstate);
extern int echoRTRender(Nrrd *nraw, limnCamera *cam, echoScene *scene,
			echoRTParm *parm, echoGlobalState *gstate);

#ifdef __cplusplus
}
#endif

#endif /* ECHO_HAS_BEEN_INCLUDED */

