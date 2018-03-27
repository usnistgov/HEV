// This navigation was originally written by Lance Arsenault as part of the
// DIVERSE ToolKit package.  Modifications have been made by John Kelso.


// Real-time navagation based on clock time, not frame rate

// Takes control inputs from the cursor and buttons. It makes a navigation
// that is similar to the performer track ball avigation in the program
// perfly.

#include <stdio.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

#include <iris/Augment.h>
#include <iris/Nav.h>
#include <iris/Utils.h>
#include <iris/SceneGraph.h>


#define SPEED (.5f)  // in ether units per second
#define TWIST (45.f) // in degrees per second

// evil global variable to pass state between class and event handler
static bool reset = false ;

// do this so this DSO's callbacks don't interfere with other DSO's
namespace trackballNav
{
    class eventHandler : public osgGA::GUIEventHandler
    {
    public:
	eventHandler(iris::Nav* nav) : _nav(nav), _first(true), _x(false), _z(false), _buttonPressed(false), _activeLastFrame(false), _now(iris::SceneGraph::instance()->getViewer()->elapsedTime()), _then(iris::SceneGraph::instance()->getViewer()->elapsedTime()) {} ;

	bool handle( const osgGA::GUIEventAdapter& ea,
		     osgGA::GUIActionAdapter& aa)
	{
	    if (!_nav)
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::trackballNav: can't get NAV object pointer!\n") ;
		return false ;
	    }
	    
	    if (_first)
	    {
		_lastMouse = osg::Vec2(ea.getXnormalized(),ea.getYnormalized()) ;
		_first = false ;
	    }

	    // every navigation should see if it's active if it wants to honor the active() method
	    if (!_nav || !_nav->getActive()) 
	    {
		_activeLastFrame = false ;
		return false ;
	    }

	    // this'll kill the "momentum" if navigations changed or the navigation position, attitude or scale changes since the last frame
#if 0
	    if (!_activeLastFrame) 
		fprintf(stderr,"navigation changed\n") ;

	    if (_navMatrix != iris::Nav::getMatrix())
		fprintf(stderr,"matrix changed\n") ;
#endif

	    // changed jtk 4/12- don't stop if navigation changed by someone else, lke wiggleNav
	    //if (!_activeLastFrame ||_navMatrix != iris::Nav::getMatrix()) 
	    if (!_activeLastFrame) 
	    {
		_activeLastFrame = true ;
		// scale doesn't have momentum
		_position = osg::Vec3(0.f,0.f,0.f) ;
		_attitude = osg::Vec3(0.f,0.f,0.f) ;
		_navMatrix = iris::Nav::getMatrix() ;
		_buttonPressed = false ;
	    }

#if 0
#if 0
	    if (iris::SceneGraph::pollKey(osgGA::GUIEventAdapter::KEY_Shift_L) || iris::SceneGraph::pollKey(osgGA::GUIEventAdapter::KEY_Shift_R)) _x = true ;
	    else _x = false ;
#else
	    if (iris::SceneGraph::pollKey('x')) _x = true ;
	    else _x = false ;
	    if (iris::SceneGraph::pollKey('z')) _z = true ;
	    else _z = false ;
#endif
#endif

	    _now = iris::SceneGraph::instance()->getViewer()->elapsedTime() ;

	    unsigned int buttons = iris::SceneGraph::instance()->getButtonMask() ;
	    osg::Vec2 thisMouse(ea.getXnormalized(),ea.getYnormalized()) ;
	    osg::Vec2 deltaMouse = thisMouse - _lastMouse ;
	    // reset and return if movement is too fast, indicating an error in event
	    // values or change in screen.
	    if (deltaMouse.length2()>.5*.5) 
	    {
		_lastMouse = thisMouse ;
		return false ;
	    }
	    _deltaTime = _now - _then ;

	    osgGA::GUIEventAdapter::EventType et = ea.getEventType() ;
	    if (reset) 
	    {
		et = osgGA::GUIEventAdapter::RELEASE ;
		reset = false ;
	    }

	    switch(et)
	    {
	    case osgGA::GUIEventAdapter::KEYUP:
		{
		    if (ea.getKey() == 'x')
		    {
			_x = false ;
			return true ;
		    }
		    else if (ea.getKey() == 'z')
		    {
			_z = false ;
			return true ;
		    }
		    return false ;
		}
	    case osgGA::GUIEventAdapter::KEYDOWN:
		{
		    if (ea.getKey() == 'x')
		    {
			_x = true ;
			return true ;
		    }
		    else if (ea.getKey() == 'z')
		    {
			_z = true ;
			return true ;
		    }
		    return false ;
		}
	    case(osgGA::GUIEventAdapter::FRAME):
		{

		    // motion is posible every frame even if nothing happens
		    if (!_buttonPressed) _updateNavigation() ;

		    _then = _now ;
		    _lastMouse = thisMouse ;
		
		    //aa.requestRedraw();
		    //aa.requestContinuousUpdate(false);
		
		    return false ;
		}
#if 1
	    case(osgGA::GUIEventAdapter::SCROLL):
		{
		    osgGA::GUIEventAdapter::ScrollingMotion sm = ea.getScrollingMotion() ;
		    if (sm==osgGA::GUIEventAdapter::SCROLL_NONE) fprintf(stderr,"SCROLL_NONE\n") ;
		    else if (sm==osgGA::GUIEventAdapter::SCROLL_LEFT) fprintf(stderr,"SCROLL_LEFT\n") ;
		    else if (sm==osgGA::GUIEventAdapter::SCROLL_RIGHT) fprintf(stderr,"SCROLL_RIGHT\n") ;
		    else if (sm==osgGA::GUIEventAdapter::SCROLL_UP) 
		    {
			//fprintf(stderr,"SCROLL_UP\n") ;
			iris::Nav::pivotScale(iris::Nav::getScale()/1.05f) ;
			return true ;
		    }
		    else if (sm==osgGA::GUIEventAdapter::SCROLL_DOWN) 
		    {
			//fprintf(stderr,"SCROLL_DOWN\n");
			iris::Nav::pivotScale(iris::Nav::getScale()*1.05f) ;
			return true ;
		    }
		    else if (sm==osgGA::GUIEventAdapter::SCROLL_2D) fprintf(stderr,"SCROLL_2D\n") ;
		    else fprintf(stderr,"SCROLL UNKNOWN\n") ;
		    return false ;
		}
#endif
	    case(osgGA::GUIEventAdapter::DRAG):
		{

		    // ignore mouse motions when no button is pressed
		    if (!_buttonPressed) return false ;

		    _position = osg::Vec3(0.f,0.f,0.f) ;
		    _attitude = osg::Vec3(0.f,0.f,0.f) ;

		    // only button 0 pressed while mouse moves
		    // translate in the XZ plane
		    if (buttons == 1)
		    {
			float dx = 0 ;
			float dz = 0 ;
			if (!_z) dx = deltaMouse.x()*SPEED/_deltaTime ;
			if (!_x) dz = deltaMouse.y()*SPEED/_deltaTime ;
			_position = osg::Vec3(dx,0.f,dz) ;

		    }
		
		    // only button 1 pressed, heading and pitch rotation while mouse moves
		    else if (buttons == 2)
		    {
			float dh = 0.f ;
			if (!_x) dh = deltaMouse.x()*TWIST/_deltaTime ;
			float dp = 0.f ;
			if (!_z) dp = -deltaMouse.y()*TWIST/_deltaTime ;
			_attitude = osg::Vec3(dh,dp,0.f) ; //HPR!!!!
		    }
		
		    // chorded navigations:
		    // the X driver will report button 1 when 0 & 2 pressed simultaneously if 
		    // xorg.conf has "emulate3button" set to true
#if 1
		    // buttons 0 & 2 pressed roll only while mouse moves
		    else if (buttons == 5)
		    {
			// get the angle around the unit circle between this mouse and last mouse
			osg::Vec2 p0 = _lastMouse ; p0.normalize() ;
			osg::Vec2 p1 = thisMouse ; p1.normalize() ;
			float  p0t = atan2(p0.y(), p0.x()) ;
			float  p1t = atan2(p1.y(), p1.x()) ;
			float dr = -(p1t - p0t)*(180.f/M_PI)/_deltaTime ;
			_attitude = osg::Vec3(0.f,0.f,dr) ; //HPR!!!!
		    }
#endif

#if 0
		    // buttons 0 and 1 pressed, heading only while mouse moves
		    if (buttons == 3)
		    {
			float dh = (deltaMouse.x()*TWIST/_deltaTime) ;
			_attitude = osg::Vec3(dh,0.f,0.f) ; //HPR!!!!
		    }

		    // buttons 1 and 2 pressed, pitch only while mouse moves
		    if (buttons == 6)
		    {
			float dp = -(deltaMouse.y()*TWIST/_deltaTime) ;
			_attitude = osg::Vec3(0.f,dp,0.f) ; //HPR!!!!
		    }
#endif

		    // only button 2 pressed while mouse moves
		    // translate in Y
		    else if (buttons == 4)
		    {
			float dy = (deltaMouse.y()*SPEED/_deltaTime) ;
			_position = osg::Vec3(0.f,dy,0.f) ;
		    }

		    _updateNavigation() ;
		    return true ;
		}
	    case(osgGA::GUIEventAdapter::PUSH):
		{
		    _buttonPressed = true ;
		    _position = osg::Vec3(0.f,0.f,0.f) ;
		    _attitude = osg::Vec3(0.f,0.f,0.f) ;
		    _then = _now ;
		    //aa.requestContinuousUpdate(false);
		    return true ;
		}
	    case(osgGA::GUIEventAdapter::RELEASE):
		{
		    // any button release turns off that navigation mode
		    _buttonPressed = false ;
		    // just stop if going real slow and button released
		    if (deltaMouse.length2()<.00001f)
		    {
			_position = osg::Vec3(0.f,0.f,0.f) ;
			_attitude = osg::Vec3(0.f,0.f,0.f) ;
		    }
		    //aa.requestRedraw();
		    //aa.requestContinuousUpdate(true);
		    return true ;
		}
	    default:
		return false;
	    }

	    return false ;
	
	}
    private:
	bool _first ;
	iris::Nav* _nav ;
	bool _x, _z ;
	// true if a mouse button is pressed
        bool _buttonPressed ;
	bool _activeLastFrame ;
	double _now, _then, _deltaTime ;
	osg::Vec3 _position ;
	osg::Vec3 _attitude ; //HPR!!!!
	osg::Vec2 _lastMouse ;
	osg::Matrix _navMatrix ;

	void _updateNavigation()
	{
	    osg::Vec3 m = _position*_deltaTime*iris::Nav::getResponse() ;
	    if (m != osg::Vec3(0.f, 0.f, 0.f)) iris::Nav::setPosition(iris::Nav::getPosition() + _position*_deltaTime*iris::Nav::getResponse()) ;

	    osg::Vec3 normAttitude = _attitude*_deltaTime*iris::Nav::getResponse() ;
	    osg::Quat q = iris::EulerToQuat(normAttitude.x(), normAttitude.y(), normAttitude.z()) ;
	    if (!q.zeroRotation()) iris::Nav::pivot(q) ;
	    
	    _navMatrix = iris::Nav::getMatrix() ;
	}
    };
    
    class trackballNav : public iris::Augment
    {
    public:

	trackballNav():iris::Augment("trackballNav")
	{
	    setDescription("trackball Navigation") ;

	    osg::ref_ptr<iris::Nav> nav = new iris::Nav("trackballNav") ;

	    osg::ref_ptr<eventHandler> eh = new eventHandler(nav.get()) ;
	    iris::SceneGraph::instance()->getViewer()->addEventHandler(eh.get());
	    validate(); 
	}

	// every navigation DSO should implement a control method, even if
	// it only passes it along to the base class
	virtual bool control(const std::string& line, const std::vector<std::string>& vec)
	{
	    if (iris::IsSubstring("reset",vec[0],3) && vec.size() == 1) reset = true ;
	    // pass to Nav class too
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
    return new trackballNav::trackballNav ;
}

static int dtkDSO_unloader(dtkAugment* augment)
{
    return DTKDSO_UNLOAD_CONTINUE;
}

