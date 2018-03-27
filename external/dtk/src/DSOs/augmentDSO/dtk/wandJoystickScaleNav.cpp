// scales the world around the pivot point based on Y joystick input
// the wand position is the pivot point if it isn't defined

#define DEAD_ZONE (0.1f)

#define SCALE_SCALE (.2f)
#define MIN_SCALE (.01f)
#define MAX_SCALE (1.f/MIN_SCALE)

#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

class wandJoystickScaleNav : public dtkNav {
public:
  wandJoystickScaleNav(dtkManager *);
  
  int postConfig(void);
  int postFrame(void);
  int activate(void);
  
private:
  
  dtkManager *mgr ;
  // dtkTime is a gettimeofday() wrapper
  dtkTime t ;
  dtkInLocator *wand ;
  dtkInValuator *joystick ;
  dtkCoord wand_xyzhpr ;
  dtkCoord pivot_xyzhpr ;
};


wandJoystickScaleNav::wandJoystickScaleNav(dtkManager *m) :
  dtkNav(m, "wandJoystickScaleNav") {
  mgr = m ;

  setDescription("scales the world around the pivot point based on Y joystick input" );
}

int wandJoystickScaleNav::postConfig(void) {
  
  wand = (dtkInLocator *) mgr->get("wand", DTKINLOCATOR_TYPE);
  joystick = (dtkInValuator *) mgr->get("joystick", DTKINVALUATOR_TYPE);
  
  if(!wand || !joystick)
    return ERROR_;

  if(joystick->number() != 2) {
    dtkMsg.add(DTKMSG_ERROR,
	       "The dtkInValuator named \"joystick\" has %d values"
	       " which is not 2 values.",joystick->number());
    return ERROR_;
  }
  
  // init pivot to be wand position
  float pos[6];
  wand->read(pos);
  defPivot->set(pos) ;
  t.reset(0.0,1);
  return dtkNav::postConfig();
}


int wandJoystickScaleNav::activate(void) {
  t.reset(0.0,1);
  // Install the call-backs.
  return dtkNav::activate() ;
}

int wandJoystickScaleNav::postFrame(void) {
  
  float delta_t = (float) t.delta();
  float joy_xy[2];
  
  // poll current joystick value
  joystick->read(joy_xy);
  
  if (fabs(joy_xy[1]) > DEAD_ZONE) {
    
    // poll wand positon
    float pos[6];
    wand->read(pos);
    wand_xyzhpr.set(pos) ;
    
    // if using the default pivot point, use the wand dtkCoord, otherwise
    // use the pivot's dtkCoord.  if using the default pivot, set it too-
    // see below
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
    
    //////////////////////////////////////////
    // Change scale at pivot point
    //////////////////////////////////////////
    float dx = (joy_xy[1] > 0.0f) ?
      ((joy_xy[1] - DEAD_ZONE) / (1.0f - DEAD_ZONE)) :
      ((joy_xy[1] + DEAD_ZONE) / (1.0f - DEAD_ZONE)) ;
    
    dx *= delta_t*SCALE_SCALE ;
    
    // get the old scale so we can compare change
    float oldScale;
    oldScale = scale ;
    
    // this sets nav scale
    scale += dx ;
    if (scale<MIN_SCALE) scale = MIN_SCALE ;
    if (scale>MAX_SCALE) scale = MAX_SCALE ;
    
    // how far do we slide the location away from the pivot?
    float slide = (scale/oldScale -1.f) ;

    // delta between pivot and location
    dtkCoord delta;
    delta.x = pivot_xyzhpr.x - location.x ;
    delta.y = pivot_xyzhpr.y - location.y ;
    delta.z = pivot_xyzhpr.z - location.z ;
    //printf("delta =  %f %f %f\n",delta.x, delta.y, delta.z) ;
    
    location.x -= delta.x*slide ;
    location.y -= delta.y*slide ;
    location.z -= delta.z*slide ;
    
    
  } // if(fabs(joy_xy[1]) > DEAD_ZONE)
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
  return new wandJoystickScaleNav(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}
