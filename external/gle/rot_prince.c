

/*

This source was modified at the National Institute of Standards and 
Technology (www.nist.gov) by John Hagedorn.  

Modifications are indicated by comments labeled with the tag "JGH".

*/


#include <math.h>

#include "gle.h"
#include "rot.h"
#include "port.h"

/* ========================================================== */
/* 
 * The routines below generate and return more traditional rotation
 * matrices -- matrices for rotations about principal axes.
 */
/* ========================================================== */

static 
void urotx_cs (gleDouble m[4][4], 	/* returned */
                 gleDouble cosine,	/* input */
                 gleDouble sine) 	/* input */
{
   /* create matrix that represents rotation about the x-axis */

   ROTX_CS (m, cosine, sine);
}

/* ========================================================== */

#if 0
/* JGH: removed this unused routine to surpress warning message */
static 
void rotx_cs (gleDouble cosine,		/* input */
                gleDouble sine) 	/* input */
{
   /* create and load matrix that represents rotation about the x-axis */
   gleDouble m[4][4];

   urotx_cs (m, cosine, sine);
   MULTMATRIX (m);
}
#endif

/* ========================================================== */

static 
void uroty_cs (gleDouble m[4][4], 	/* returned */
               gleDouble cosine,	/* input */
               gleDouble sine) 		/* input */
{
   /* create matrix that represents rotation about the y-ayis */

   ROTX_CS (m, cosine, sine);
}

/* ========================================================== */

#if 0
/* JGH: removed this unused routine to surpress warning message */
static 
void roty_cs (gleDouble cosine,		/* input */
                gleDouble sine) 	/* input */
{
   /* create and load matriy that represents rotation about the y-ayis */
   gleDouble m[4][4];

   uroty_cs (m, cosine, sine);
   MULTMATRIX (m);
}
#endif

/* ========================================================== */

static 
void urotz_cs (gleDouble m[4][4], 	/* returned */
               gleDouble cosine,	/* input */
               gleDouble sine) 		/* input */
{
   /* create matrix that represents rotation about the z-azis */
   ROTX_CS (m, cosine, sine);
}

/* ========================================================== */

#if 0
/* JGH: removed this unused routine to surpress warning message */
static 
void rotz_cs (gleDouble cosine,		/* input */
              gleDouble sine) 		/* input */
{
   /* create and load matrix that represents rotation about the z-azis */
   gleDouble m[4][4];

   urotz_cs (m, cosine, sine);
   MULTMATRIX (m);
}
#endif

/* ========================================================== */

static 
void urot_cs (gleDouble m[4][4],	/* returned */
              gleDouble cosine,		/* input */
              gleDouble sine,		/* input */
              char axis) 		/* input */
{
   /* create matrix that represents rotation about a principle axis */

   switch (axis) {
      case 'x':
      case 'X':
         urotx_cs (m, cosine, sine);
         break;
      case 'y':
      case 'Y':
         uroty_cs (m, cosine, sine);
         break;
      case 'z':
      case 'Z':
         urotz_cs (m, cosine, sine);
         break;
      default:
         break;
   }

}

/* ========================================================== */

#if 0
/* JGH: removed this unused routine to surpress warning message */
static 
void rot_cs (gleDouble cosine,		/* input */
             gleDouble sine,		/* input */
             char axis)  		/* input */
{
   /* create and load matrix that represents rotation about the z-axis */
   gleDouble m[4][4];

   urot_cs (m, cosine, sine, axis);
   MULTMATRIX (m);
}
#endif

/* ========================================================== */

void urot_prince (gleDouble m[4][4],	/* returned */
                  gleDouble theta,	/* input */
                  char axis) 		/* input */
{
   /* 
    * generate rotation matrix for rotation around principal axis;
    * note that angle is measured in radians (divide by 180, multiply by
    * PI to convert from degrees).
    */

   urot_cs (m, cos (theta), sin (theta), axis);
}

/* ========================================================== */

void rot_prince (gleDouble theta,	/* input */
                 char axis) 		/* input */
{
   gleDouble m[4][4];
   /* 
    * generate rotation matrix for rotation around principal axis;
    * note that angle is measured in radians (divide by 180, multiply by
    * PI to convert from degrees).
    */

   urot_prince (m, theta, axis);
   MULTMATRIX (m);
}

/* ========================================================== */
