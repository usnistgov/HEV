/*
 * Wand Joystick Nav re-written to work with a USB gamepad device
 * Designed for the CS CRC Viswall system
 * Copyright (C) 2006 Andrew A. Ray
 *
 */

/* real-time navagation based on clock time, not frame rate */

// Takes control inputs from a dtkInValuator, and a dtkInLocator.

#define TRANS_DEAD_ZONE (0.05f)
#define ROT_DEAD_ZONE (0.1f)

// TWIST = degrees per second
#define TWIST     (45.0f)

#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

#undef NAVNAME

#  define NAVNAME        gameNav
#  define NAVNAMESTRING  "gameNav"



class NAVNAME : public dtkNav
{
public:
  NAVNAME(dtkManager *);

  int postConfig(void);
  int postFrame(void);
  int activate(void);

private:

  dtkManager *mgr ;
  // dtkTime is a gettimeofday() wrapper
  dtkTime t;
  dtkInLocator *wand ;
  dtkSharedMem *joystick ;
  dtkSharedMem* hat;
  dtkCoord wand_xyzhpr ;
  dtkCoord pivot_xyzhpr ;
};


NAVNAME::NAVNAME(dtkManager *m) :
  dtkNav(m, NAVNAMESTRING)
{
  mgr = m ;
  setDescription("simple gamepad navigation"); 
}

int NAVNAME::postConfig(void)
{
  wand = (dtkInLocator *) mgr->get("wand", DTKINLOCATOR_TYPE);
  joystick = new dtkSharedMem(sizeof(float)*4, "gamepad");
  hat = new dtkSharedMem(sizeof(char)*2, "hat");


  // init pivot to be wand position
  float pos[6];
  wand->read(pos);
  defPivot->set(pos) ;
  t.reset(0.0,1);
  return dtkNav::postConfig();
}


int NAVNAME::activate(void)
{
  t.reset(0.0,1);
  // Install the call-backs.
  return dtkNav::activate() ;
}

int NAVNAME::postFrame(void) {

  float delta_t = (float) t.delta();
  float joy_xy[4];

  // poll current wand and joystick value
  joystick->read(joy_xy);

  float pos[6];
  pos[0]=0;
  pos[1]=0;
  pos[2]=0;
  pos[3]=45;
  pos[4]=0;
  pos[5]=0;
  //wand->read(pos);
  wand_xyzhpr.set(pos) ;

  if(fabs(joy_xy[0]) > TRANS_DEAD_ZONE ||
     fabs(joy_xy[1]) > TRANS_DEAD_ZONE || 
     fabs(joy_xy[3]) > TRANS_DEAD_ZONE || 
     fabs(joy_xy[4]) > TRANS_DEAD_ZONE) 
    {

      // if using the default pivot point, use the wand dtkCoord, otherwise
      // use the pivot's dtkCoord.  if using the default pivot, set it too-
      // see below
#if 0
      printf("_pivot = %p  defPivot = %p\n",_pivot,defPivot) ;
      printf("_pivot = ") ;
      if (_pivot) _pivot->print() ;
      printf("defPivot = ") ;
      if (defPivot) defPivot->print() ;
#endif
      if (_pivot == defPivot) {
	// using the wand as pivot point
	pivot_xyzhpr = wand_xyzhpr ;
      } else {
	// using non-default pivot as the pivot point - don't change it!
	pivot_xyzhpr = *_pivot ;
	// but use the wand hpr for the rotation- we get to decide how to
	// use it.
	pivot_xyzhpr.h = wand_xyzhpr.h ;
	pivot_xyzhpr.p = wand_xyzhpr.p ;
	pivot_xyzhpr.r = wand_xyzhpr.r ;
      }
      // UPDATE THE DEFAULT PIVOT POINT WHETHER IT'S BEING USED OR NOT
      *defPivot = wand_xyzhpr ;
      //Make the pivot be where the user is, not the origin
      pivot_xyzhpr.x = location.x;
      pivot_xyzhpr.y = location.y;
      pivot_xyzhpr.z = location.z;
      pivot_xyzhpr.h = location.h;
      pivot_xyzhpr.p = location.p;
      pivot_xyzhpr.r = location.r;
      

      //////////////////////////////////////////
      // Change heading rotation about the pivot
      // rotation depends on pivot point
      //////////////////////////////////////////
      if (fabs(joy_xy[0])>ROT_DEAD_ZONE)
	{
	  float dy = (joy_xy[0] > 0.0f) ?
	    ((joy_xy[0] - ROT_DEAD_ZONE) / (1.0f - ROT_DEAD_ZONE)):
	    ((joy_xy[0] + ROT_DEAD_ZONE) / (1.0f - ROT_DEAD_ZONE));
	  dy *= dy*dy;

	  dtkMatrix xlateMat;
	  xlateMat.translate( -delta_t*_response*dy, 0.f, 0.f) ;

	  // make the translation go in the direction of the wand
	  xlateMat.rotateHPR(pos[3],
			     pos[4],
			     pos[5]);


	  // add the translation into the location
	  float x, y, z ;
	  xlateMat.translate(&x, &y, &z) ;
	  location.x += x ;
	  location.y += y ;
	  location.z += z ;

	  /*
	  // set an intial heading rotation and then rotate it again about
	  // correct axis
	  float dy = (joy_xy[0] > 0.0f) ?
	    ((joy_xy[0] - ROT_DEAD_ZONE) / (1.0f - ROT_DEAD_ZONE)):
	    ((joy_xy[0] + ROT_DEAD_ZONE) / (1.0f - ROT_DEAD_ZONE));
	  dy *= dy*dy;
	  dtkMatrix rotMat ;

	  // assume that the HEADING, PITCH and ROLL navigations are
	  // user-centric- i.e., users want to feel as if they're
	  // moving through the virtual world.  For HPR, assume that
	  // the navigation is exo-centric- i.e., the object is
	  // manipulated, instead of users moving through the world.
#if defined (HEADING) || defined (XY)
	  rotMat.rotateHPR(delta_t*TWIST*dy, 0.f, 0.f) ;
#endif
#ifdef PITCH
	  rotMat.rotateHPR(0.f, delta_t*TWIST*dy, 0.f) ;
#endif
#ifdef ROLL
	  rotMat.rotateHPR(0.f, 0.f, delta_t*TWIST*dy) ;
#endif
#ifdef HPR
	  // hpr mode sets rotation axis based on wand hpr
	  dtkMatrix hprMat, hprMatInv ;

	  hprMat.rotateHPR(wand_xyzhpr.h,
			   wand_xyzhpr.p,
			   wand_xyzhpr.r) ;
	  hprMatInv =  hprMat ;
	  hprMatInv.invert() ;

	  rotMat = hprMatInv ;
	  rotMat.rotateHPR(0.f, 0.f, delta_t*TWIST*dy) ;
	  rotMat.mult(&hprMat) ;
#endif

	  dtkMatrix locMat;
	  // make nav locator into a matrix
	  locMat.coord(location);

	  // move to pivot position
	  locMat.translate(-pivot_xyzhpr.x,
			   -pivot_xyzhpr.y,
			   -pivot_xyzhpr.z);
	  
	  // rotate nav locator 
	  locMat.mult(&rotMat) ;

	  // move back from pivot position
	  locMat.translate(pivot_xyzhpr.x,
			   pivot_xyzhpr.y,
			   pivot_xyzhpr.z);
	  
	  // make nav locator back to a dtkCoord
	  locMat.coord(&location);
	  */
	}

      /////////////////////////////////////////
      // Change the translation.
      // translation is based on wand hpr, regardless of pivot point
      /////////////////////////////////////////
      if (fabs(joy_xy[1])>TRANS_DEAD_ZONE)
	{

	  float dy = (joy_xy[1] > 0.0f) ? 
	    ((joy_xy[1] - TRANS_DEAD_ZONE) / (1.0f - TRANS_DEAD_ZONE)):
	    ((joy_xy[1] + TRANS_DEAD_ZONE) / (1.0f - TRANS_DEAD_ZONE));
	  dy *= dy*dy;
	  
	  dtkMatrix xlateMat;
	  // get the translation distance along y
	  xlateMat.translate(0.f, -delta_t*_response*dy, 0.f) ;

	  // make the translation go in the direction of the wand
	  xlateMat.rotateHPR(pos[3],
			     pos[4],
			     pos[5]);


	  // add the translation into the location
	  float x, y, z ;
	  xlateMat.translate(&x, &y, &z) ;
	  location.x += x ;
	  location.y += y ;
	  location.z += z ;
	}
      if (fabs(joy_xy[3])>TRANS_DEAD_ZONE)
	{

	  float dy = (joy_xy[3] > 0.0f) ? 
	    ((joy_xy[3] - TRANS_DEAD_ZONE) / (1.0f - TRANS_DEAD_ZONE)):
	    ((joy_xy[3] + TRANS_DEAD_ZONE) / (1.0f - TRANS_DEAD_ZONE));
	  dy *= dy*dy;
	  
	  dtkMatrix xlateMat;
	  // get the translation distance along y
	  xlateMat.translate(0.f, 0.f,  delta_t*_response*dy); 

	  // make the translation go in the direction of the wand
	  xlateMat.rotateHPR(pos[3],
			     pos[4],
			     pos[5]);


	  // add the translation into the location
	  float x, y, z ;
	  xlateMat.translate(&x, &y, &z) ;
	  location.x += x ;
	  location.y += y ;
	  location.z += z ;
	}
    }

    //Now the hat...
    char hatVal[2];
    float heading=0;
    float pitch=0;

    hat->read(hatVal);
    if (hatVal[0] == 1)
    	heading=180;
    if (hatVal[0] == -1)
    	heading=-180;
    if (hatVal[1] == -1)
    	pitch=-180;
    if (hatVal[1] == 1)
    	pitch=180;
	  // set an intial heading rotation and then rotate it again about
	  // correct axis
	  if (heading !=0)
	  {
		  float dy = (hatVal[0] > 0.0f) ?
		    ((1  - ROT_DEAD_ZONE) / (1.0f - ROT_DEAD_ZONE)):
		    ((-1 + ROT_DEAD_ZONE) / (1.0f - ROT_DEAD_ZONE));
		  dy *= dy*dy;
		  dtkMatrix rotMat ;

		  // assume that the HEADING, PITCH and ROLL navigations are
		  // user-centric- i.e., users want to feel as if they're
		  // moving through the virtual world.  For HPR, assume that
		  // the navigation is exo-centric- i.e., the object is
		  // manipulated, instead of users moving through the world.
		  rotMat.rotateHPR(delta_t*TWIST*dy, 0.f, 0.f) ;

		  dtkMatrix locMat;
		  // make nav locator into a matrix
		  locMat.coord(location);

		  // move to pivot position
		  locMat.translate(-pivot_xyzhpr.x,
				   -pivot_xyzhpr.y,
				   -pivot_xyzhpr.z);
		  
		  // rotate nav locator 
		  locMat.mult(&rotMat) ;

		  // move back from pivot position
		  locMat.translate(pivot_xyzhpr.x,
				   pivot_xyzhpr.y,
				   pivot_xyzhpr.z);
		  
		  // make nav locator back to a dtkCoord
		  locMat.coord(&location);
	  }
	  if (pitch !=0)
	  {
		  float dy = (hatVal[1] > 0.0f) ?
		    ((1  - ROT_DEAD_ZONE) / (1.0f - ROT_DEAD_ZONE)):
		    ((-1 + ROT_DEAD_ZONE) / (1.0f - ROT_DEAD_ZONE));
		  dy *= dy*dy;
		  dtkMatrix rotMat ;

		  // assume that the HEADING, PITCH and ROLL navigations are
		  // user-centric- i.e., users want to feel as if they're
		  // moving through the virtual world.  For HPR, assume that
		  // the navigation is exo-centric- i.e., the object is
		  // manipulated, instead of users moving through the world.
		  rotMat.rotateHPR(0.f, delta_t*TWIST*dy ,0.f) ;

		  dtkMatrix locMat;
		  // make nav locator into a matrix
		  locMat.coord(location);

		  // move to pivot position
		  locMat.translate(-pivot_xyzhpr.x,
				   -pivot_xyzhpr.y,
				   -pivot_xyzhpr.z);
		  
		  // rotate nav locator 
		  locMat.mult(&rotMat) ;

		  // move back from pivot position
		  locMat.translate(pivot_xyzhpr.x,
				   pivot_xyzhpr.y,
				   pivot_xyzhpr.z);
		  
		  // make nav locator back to a dtkCoord
		  locMat.coord(&location);
	  }

  return dtkNav::postFrame() ;
}



/************ DTK C++ dtkAugment loader/unloader functions ***************
 *
 * All DTK dtkAugment DSO files are required to declare these two
 * functions.  These function are called by the loading program to get
 * your C++ objects loaded.
 *
 *************************************************************************/

static dtkAugment *dtkDSO_loader(dtkManager *manager, void *p)
{
  return new NAVNAME(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}
