// This navigation was originally written by Lance Arsenault as part of the
// DIVERSE ToolKit package.  Modifications have been made by John Kelso.
//
// XYTRANSLATE conditonal code added by Steve Satterfield 5/2016
// XYZTRANSLATEONLY conditonal code added by Steve Satterfield 6/2016

// Real-time navagation based on clock time, not frame rate

#include <stdio.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

#include <iris/Augment.h>
#include <iris/Utils.h>
#include <iris/SceneGraph.h>
#include <iris/Nav.h>


const float SPEED=1.f;  // in ether units per second
const float TWIST=45.f; // in degrees per second

const float TRANS_DEAD_ZONE=0.05f;
const float ROT_DEAD_ZONE=0.1f;

#undef NAVNAME

#ifdef ZROTATE
#  define NAVNAME wandJoystickZNav
#  define NAVNAMESTRING  "wandJoystickZNav"
#endif

#ifdef YROTATE
#  define NAVNAME wandJoystickYNav
#  define NAVNAMESTRING  "wandJoystickYNav"
#endif

#ifdef XYTRANSLATE
#  define NAVNAME wandJoystickXYtranNav
#  define NAVNAMESTRING  "wandJoystickXYtranNav"
  #define GROTATE
#endif

#ifdef XYZTRANSLATEONLY
#  define NAVNAME wandJoystickXYZtranOnlyNav
#  define NAVNAMESTRING  "wandJoystickXYZtranOnlyNav"
  #define GROTATE
#endif

#ifndef NAVNAME
  // the default case
  #define NAVNAME wandJoystickNav
  #define NAVNAMESTRING  "wandJoystickNav"
  #define GROTATE
#endif

// do this so this DSO's callbacks don't interfere with other DSO's
namespace NAVNAME
{

    class eventHandler : public osgGA::GUIEventHandler
    {
    public:

	eventHandler(iris::Nav* nav) : _nav(nav)
	{
	    _now = _then = iris::SceneGraph::instance()->getViewer()->elapsedTime() ;

	    _wandShm = new dtkSharedMem(sizeof(_wand), "wandMatrix") ;
	    if (!_wandShm || _wandShm->isInvalid())
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::%s:: can't open shared memory \"wandMatrix\", removing callback\n",NAVNAMESTRING) ;
		// remove callback
		iris::RemoveEventHandler(this) ;
	    }
		
	    joystickShm = new dtkSharedMem(sizeof(joystick), "joystick") ;
	    if (!joystickShm || joystickShm->isInvalid())
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::%s: can't open shared memory \"joystick\", removing callback\n",NAVNAMESTRING) ;
		// remove callback
		iris::RemoveEventHandler(this) ;
	    }

	} ;

	bool handle( const osgGA::GUIEventAdapter& ea,
		     osgGA::GUIActionAdapter& aa)
	{

	    if (!_nav)
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::%s: can't get NAV object pointer!\n",NAVNAMESTRING) ;
		return false ;
	    }
	
	    // every navigation should see if it's active if it wants to honor the active() method
	    if (!_nav || !_nav->getActive()) return false ;

	    switch(ea.getEventType())
	    {
	    case osgGA::GUIEventAdapter::FRAME:
		{

		    _now = iris::SceneGraph::instance()->getViewer()->elapsedTime() ;
		    double deltaTime = _now - _then ;

		    _wandShm->read(_wand.ptr()) ;
		    joystickShm->read(joystick) ;

		    float rotation = 0 ;
#ifndef XYZTRANSLATEONLY
		    // rotation is based on joystick left/right
		    if (fabs(joystick[0])>ROT_DEAD_ZONE)
		    {
			rotation = (joystick[0]-ROT_DEAD_ZONE)/(1.f-ROT_DEAD_ZONE) ;
			// cube it- feels more natural
			rotation *= rotation*rotation ;
			rotation *= deltaTime*TWIST ;
		    }

#endif

		    float translation = 0 ;
		    // translation is based on joystick front/back
		    if (fabs(joystick[1])>TRANS_DEAD_ZONE)
		    {
			translation = (joystick[1]-TRANS_DEAD_ZONE)/(1.f-TRANS_DEAD_ZONE) ;
			// cube it- feels more natural
			translation *= translation*translation ;
			translation *= -deltaTime*SPEED*iris::Nav::getResponse() ;
		    }

		    if (translation != 0)
		    {
			osg::Vec3 position ;
			    position = _wand.getRotate() * osg::Vec3(0.f,translation,0.f) ;
#if defined( XYTRANSLATE) || defined( XYZTRANSLATEONLY)
			    double sgs_h, sgs_p, sgs_r;
			    iris::MatrixToEuler(_wand, &sgs_h,  &sgs_p,  &sgs_r);
			    //fprintf(stderr, "wand: %g %g %g\n",sgs_h, sgs_p, sgs_r);
			    if (sgs_p < 45.) { // use x y only
			      position[2]=0.0;
			    } else {
			      if (sgs_p > 60.) { // use z only
			      position[0]=0.0;
			      position[1]=0.0;
			      } else {           // dead spot for transition from walking to elevator
			      position[0]=0.0;
			      position[1]=0.0;
			      position[2]=0.0;
			      }
			    }
			      
#endif
			    iris::Nav::setPosition(iris::Nav::getPosition()+position) ;
		    }

		    if (rotation != 0.f)
		    {
			osg::Quat attitude ;
#ifdef YROTATE
			// this uses the Y coming out of the wand as the axis of rotation
			osg::Vec3 rv = _wand.getRotate() * osg::Vec3(0.f,1.f,0.f) ;
#endif
#ifdef ZROTATE
			// this uses the Z coming out of the wand as the axis of rotation
			osg::Vec3 rv = _wand.getRotate() * osg::Vec3(0.f,0.f,1.f) ;
#endif
#ifdef GROTATE
			// this uses "up" as the axis of rotation
			osg::Vec3 rv = osg::Vec3(0.f,0.f,1.f) ;
#endif
			iris::Nav::pivot(osg::Quat(rotation*M_PI/180.f, rv)) ;
		    }


		    _then = _now ;

		    break ;
		}
	    }

	    return false;
	
	}
    private:
	iris::Nav* _nav ;
	double _now, _then ;
	osg::Vec2 _lastMouse ;

	dtkSharedMem* _wandShm ;
	osg::Matrix _wand ;
	    
	dtkSharedMem* joystickShm ;
	float joystick[2] ;

    };
    
    class NAVNAME : public iris::Augment
    {
    public:

	NAVNAME():iris::Augment(NAVNAMESTRING)
	{
	    setDescription(NAVNAMESTRING"Navigation") ;
	    osg::ref_ptr<iris::Nav> nav = new iris::Nav(NAVNAMESTRING) ;

	    osg::ref_ptr<eventHandler> eh = new eventHandler(nav.get()) ;
	    iris::SceneGraph::instance()->getViewer()->addEventHandler(eh.get());

	    validate(); 
	}

	// every navigation DSO should implement a control method, even if
	// it only passes it along to the base class
	bool control(const std::string& line, const std::vector<std::string>& vec)
	{
	    return iris::Nav::getCurrentNav()->control(line, vec) ;
	}

    };
} ;


/************ DTK C++ dtkAugment loader/unloader functions ***************
 *
 * All DTK dtkAugment DSO files are required to declare these two
 * functions.  These function are called by the loading program to get
 * your C++ objects loaded.
 *
 *************************************************************************/

static dtkAugment* dtkDSO_loader(dtkManager* manager, void* p)
{
    return new NAVNAME::NAVNAME ;
}

static int dtkDSO_unloader(dtkAugment* augment)
{
    return DTKDSO_UNLOAD_CONTINUE;
}

