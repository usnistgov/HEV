#include <stdio.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

#include <osg/PolygonMode>

#include <iris/Augment.h>
#include <iris/SceneGraph.h>
#include <iris/Utils.h>
#include <iris/PerspectivePane.h>
#include <iris/Nav.h>

// how fast to move around in jump mode
const float XYZ=1.f;
const float HPR=60.f ;

// do this so this DSO's callbacks don't interfere with other DSO's
namespace keyboardControl
{

    ////////////////////////////////////////////////////////////////////////
    class keyboardControlEventHandler : public osgGA::GUIEventHandler
    {
    public: 

	keyboardControlEventHandler(iris::Augment* dso) : _dso(dso), _x(1.f), _y(0.f), _z(0.f), _h(90.f), _p(0.f), _r(0.f), _then(iris::SceneGraph::instance()->getViewer()->elapsedTime()), _delta_t(0.f) { } ;

	
	void jump_left(int s)
	{
	    float sine = sinf(iris::DegreesToRadians(HPR*_delta_t));
	    float cosine = cosf(iris::DegreesToRadians(HPR*_delta_t));
	    _h += s*HPR*_delta_t;
	    float savex = (_x*cosine - s*_y*sine);
	    _y = (s*_x*sine + _y*cosine);
	    _x = savex;
	    iris::SceneGraph::instance()->setJumpAttitude(iris::EulerToQuat(_h,_p,_r)) ;
	    iris::SceneGraph::instance()->setJumpPosition(osg::Vec3(_x,_y,_z)) ;
	}
	
	void jump_up(int s)
	{
	    _p -= s*HPR*_delta_t;
	    float d = sqrtf(_x*_x + _y*_y + _z*_z) ;
	    _x =   d*sinf(iris::DegreesToRadians(_h)) * cosf(iris::DegreesToRadians(_p));
	    _y = - d*cosf(iris::DegreesToRadians(_h)) * cosf(iris::DegreesToRadians(_p));
	    _z = - d*sinf(iris::DegreesToRadians(_p));
	    iris::SceneGraph::instance()->setJumpAttitude(iris::EulerToQuat(_h,_p,_r)) ;
	    iris::SceneGraph::instance()->setJumpPosition(osg::Vec3(_x,_y,_z)) ;
	}

	void jump_in(int s)
	{
	    float d = sqrtf(_x*_x + _y*_y + _z*_z) ;
	    if((d > 0.01 || s < 0) && (d < 10000.0f || s> 0))
	    {
		_x -= s*_x*XYZ*_delta_t;
		_y -= s*_y*XYZ*_delta_t;
		_z -= s*_z*XYZ*_delta_t;
		iris::SceneGraph::instance()->setJumpPosition(osg::Vec3(_x,_y,_z)) ;
	    }
	}

#if 0
	void jump_roll()
	{
	    _r += HPR*_delta_t ;
	    iris::SceneGraph::instance()->setJumpAttitude(iris::EulerToQuat(_h,_p,_r)) ;
	}
#endif

	void jump_reset()
	{
	    
	    /// initial value of jump outside view
	    _x = 1.f;
	    _y = 0.f ;
	    _z = 0.f ;
	    _h = 90.f ;
	    _p = 0.f ;
	    _r = 0.f ;
	    iris::SceneGraph::instance()->setJumpAttitude(iris::EulerToQuat(_h,_p,_r)) ;
	    iris::SceneGraph::instance()->setJumpPosition(osg::Vec3(_x,_y,_z)) ;
	} ;

	void iod(float d)
	{
	    float iod = iris::SceneGraph::instance()->getInterocular() ;
	    dtkMsg.add(DTKMSG_INFO, "iris::keyboardNavControl: changing interocular from %f to %f\n",iod,iod+d) ;
	    iris::SceneGraph::instance()->setInterocular(iod+d) ;
	}

	void fusion(float d)
	{
	    float fd = iris::SceneGraph::instance()->getFusion() ;
	    dtkMsg.add(DTKMSG_INFO, "iris::keyboardNavControl: changing fusion from %f to %f\n",fd,fd+d) ;
	    iris::SceneGraph::instance()->setFusion(fd+d) ;
	}

	void fov(float d)
	{
	    std::vector<osg::ref_ptr<iris::Pane> > const pl = iris::Pane::getPaneList() ;
	    for (unsigned int i=0; i<pl.size(); i++)
	    {
		iris::Pane* p = pl[i].get() ;
		iris::PerspectivePane* pp = p->asPerspective() ;
		if (pp)
		{
		    float fov = pp->getFov() ;
		    dtkMsg.add(DTKMSG_INFO, "iris::keyboardNavControl: changing fov from %f to %f\n",fov,fov+d) ;
		    pp->setFov(fov+d) ;
		}
	    }
	}

	void toggleAutoAspect()
	{
	    std::vector<osg::ref_ptr<iris::Pane> > const pl = iris::Pane::getPaneList() ;
	    for (unsigned int i=0; i<pl.size(); i++)
	    {
		std::vector<osg::ref_ptr<osg::Camera> > const cl = pl[i]->getCameraList() ;
		for (unsigned int j=0; j<cl.size(); j++)
		{
		    osg::Camera* c = cl[0].get() ;
		    if (c->getProjectionResizePolicy() == osg::Camera::HORIZONTAL) 
		    {
			c->setProjectionResizePolicy(osg::Camera::VERTICAL) ;
			if (j==0) dtkMsg.add(DTKMSG_INFO, "iris::keyboardNavControl: changing autoAspect of pane \"%s\" from horizontal to vertical\n", pl[i]->getName()) ;
		    }
		    else if (c->getProjectionResizePolicy() == osg::Camera::VERTICAL) 
		    {
			c->setProjectionResizePolicy(osg::Camera::HORIZONTAL) ;
			if (j==0) dtkMsg.add(DTKMSG_INFO, "iris::keyboardNavControl: changing autoAspect of pane \"%s\" from vertical to horizontal\n", pl[i]->getName()) ;
		    }
		}
	    }
	}

	void near(float d)
	{
	    std::vector<osg::ref_ptr<iris::Pane> > const pl = iris::Pane::getPaneList() ;
	    for (unsigned int i=0; i<pl.size(); i++)
	    {
		iris::Pane* p = pl[i].get() ;
		float near = p->getNear() ;
		dtkMsg.add(DTKMSG_INFO, "iris::keyboardNavControl: changing near from %f to %f\n",near,near*d) ;
		p->setNear(near*d) ;
	    }
	}

	void far(float d)
	{
	    std::vector<osg::ref_ptr<iris::Pane> > const pl = iris::Pane::getPaneList() ;
	    for (unsigned int i=0; i<pl.size(); i++)
	    {
		iris::Pane* p = pl[i].get() ;
		float far = p->getFar() ;
		dtkMsg.add(DTKMSG_INFO, "iris::keyboardNavControl: changing far from %f to %f\n",far,far*d) ;
		p->setFar(far*d) ;
	    }
	}

	void togglePolygonMode()
	{
	    static osg::PolygonMode::Mode mode = osg::PolygonMode::FILL ;
	    
	    osg::ref_ptr<osg::PolygonMode> polygonMode = new osg::PolygonMode ;
	    iris::MatrixTransform* node = iris::SceneGraph::instance()->getWorldNode() ;
	    
	    if (mode == osg::PolygonMode::FILL) 
	    {
		dtkMsg.add(DTKMSG_INFO, "iris::keyboardNavControl: changing polygon mode to line\n") ;
		mode = osg::PolygonMode::LINE ;
	    }
	    else if (mode == osg::PolygonMode::LINE) 
	    {
		dtkMsg.add(DTKMSG_INFO, "iris::keyboardNavControl: changing polygon mode to point\n") ;
		mode = osg::PolygonMode::POINT ;
	    }
	    else
	    {
		dtkMsg.add(DTKMSG_INFO, "iris::keyboardNavControl: changing polygon mode to fill\n") ;
		mode = osg::PolygonMode::FILL ;
	    }
	    polygonMode->setMode( osg::PolygonMode::FRONT_AND_BACK, mode);
	    osg::StateSet* stateset = node->getOrCreateStateSet() ;
	    stateset->setAttributeAndModes(polygonMode,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
	}

	////////////////////////////////////////////////////////////////////////
	bool handle( const osgGA::GUIEventAdapter& ea,
		     osgGA::GUIActionAdapter& aa)
	{
	    if (!_dso || _dso->isInvalid())
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::keyboardNavControl: can't get DSO object pointer!\n") ;
		return false ;
	    }
	
	    // every navigation should see if it's active if it wants to honor the active() method
	    if (_dso && !_dso->getActive()) return false ;

	    float now = iris::SceneGraph::instance()->getViewer()->elapsedTime() ;
	    _delta_t = now - _then ;
	    _then = now ;

	    // how much to change response per keypress
	    const float RESPONSE_CHANGE=2.f ;
	    // desktop stereo
	    const float IOD_AND_FUSION_CHANGE=.01 ;
	    // perspective frustum
	    const float FOV_CHANGE=1.f; 

	    // near/far change- a multiplier
	    const float NEAR_FAR_CHANGE=1.05f; 

	    switch(ea.getEventType())
	    {
	    case osgGA::GUIEventAdapter::KEYDOWN:
		{
		    int c = ea.getKey() ;

		    if (c=='?')
		    {
			std::string cmd = std::string("EXEC evince ") + std::string(getenv("IRIS_PREFIX")) + std::string("/doc/pdf/keyboardMouseActions.pdf") ;
			iris::SceneGraph::instance()->control(cmd) ;
			return true ;
		    }
		    else if (c=='C')
		    {
			std::string cmd = std::string("EXEC dtk-caveDeviceSimulator") ;
			iris::SceneGraph::instance()->control(cmd) ;
			return true ;
		    }
		    // navigation
		    else if (c=='0')
		    {
			iris::Nav::getCurrentNav()->reset() ;
			return true ;
		    }
		    else if (c=='n')
		    {
			iris::Nav::getCurrentNav()->next() ;
			return true ;
		    }

		    else if (c=='R') //sgs changed '+'||'=' to 'R'
		    {
			float response = iris::Nav::getResponse() ;
			dtkMsg.add(DTKMSG_INFO, "iris::keyboardNavControl: changing navigation response from %f to %f\n",response,response*RESPONSE_CHANGE) ;
			response *= RESPONSE_CHANGE ;
			iris::Nav::getCurrentNav()->setResponse(response) ;
			return true ;
		    }

		    else if (c=='r')  // sgs changed to 'r'
		    {
			float response = iris::Nav::getResponse() ;
			dtkMsg.add(DTKMSG_INFO, "iris::keyboardNavControl: changing navigation response from %f to %f\n",response,response/RESPONSE_CHANGE) ;
			response /= RESPONSE_CHANGE ;
			iris::Nav::getCurrentNav()->setResponse(response) ;
			return true ;
		    }

		    else if (c=='_')
		    {
			dtkMsg.add(DTKMSG_INFO, "iris::keyboardNavControl: changing navigation response to 1\n") ;
			iris::Nav::getCurrentNav()->setResponse(1.f) ;
			return true ;
		    }

		    // head "jump" position
		    else if (c=='j')
		    {
			if (!iris::SceneGraph::instance()->getJump())
			{
			    iris::SceneGraph::instance()->setJumpPosition(osg::Vec3(_x,_y,_z)) ;
			    iris::SceneGraph::instance()->setJumpAttitude(iris::EulerToQuat(_h,_p,_r)) ;
			}
			iris::SceneGraph::instance()->setJump(!iris::SceneGraph::instance()->getJump()) ;
			return true ;
		    }
		    else if (c==osgGA::GUIEventAdapter::KEY_KP_2 || c==osgGA::GUIEventAdapter::KEY_KP_Down || c=='2') // sgs added 2
		    {
			jump_up(-1) ;
			return true ;
		    }    
		    else if (c==osgGA::GUIEventAdapter::KEY_KP_4 || c==osgGA::GUIEventAdapter::KEY_KP_Left || c=='4') // sgs added 4
		    {
			jump_left(-1) ;
			return true ;
		    }    
		    else if (c==osgGA::GUIEventAdapter::KEY_KP_5 || c==osgGA::GUIEventAdapter::KEY_KP_Begin || c=='5') // sgs added 5
		    {
			jump_reset() ;
			return true ;
		    }    
		    else if (c==osgGA::GUIEventAdapter::KEY_KP_6 || c==osgGA::GUIEventAdapter::KEY_KP_Right || c=='6') // sgs added 6
		    {
			jump_left(1) ;
			return true ;
		    }    
		    else if (c==osgGA::GUIEventAdapter::KEY_KP_8 || c==osgGA::GUIEventAdapter::KEY_KP_Up || c=='8') // sgs aded 8
		    {
			jump_up(1) ;
			return true ;
		    }    
		    else if (c==osgGA::GUIEventAdapter::KEY_KP_Add || c=='+') // sgs added +
		    {
			jump_in(-1) ;
			return true ;
		    }    
		    else if (c==osgGA::GUIEventAdapter::KEY_KP_Subtract || c=='-') // sgs added -
		    {
			jump_in(1) ;
			return true ;
		    }    
#if 0
		    else if (c==osgGA::GUIEventAdapter::KEY_KP_Multiply)
		    {
			jump_roll() ;
			return true ;
		    }    
#endif
		    else if (c==osgGA::GUIEventAdapter::KEY_Left)
		    {
			iod(-IOD_AND_FUSION_CHANGE) ;
			return true ;
		    }
		    else if (c==osgGA::GUIEventAdapter::KEY_Right)
		    {
			iod(IOD_AND_FUSION_CHANGE) ;
			return true ;
		    }
		    else if (c==osgGA::GUIEventAdapter::KEY_Up)
		    {
			fusion(IOD_AND_FUSION_CHANGE) ;
			return true ;
		    }
		    else if (c==osgGA::GUIEventAdapter::KEY_Down)
		    {
			fusion(-IOD_AND_FUSION_CHANGE) ;
		    }
		    
		    else if (c=='<' || c==',')
		    {
			fov(-FOV_CHANGE) ;
			return true ;
		    }

		    else if (c=='a')
		    {
			toggleAutoAspect() ;
			return true ;
		    }

		    else if (c=='>' || c=='.')
		    {
			fov(FOV_CHANGE) ;
			return true ;
		    }

		    else if (c=='}')
		    {
			far(NEAR_FAR_CHANGE) ;
			return true ;
		    }

		    else if (c=='{')
		    {
			far(1/NEAR_FAR_CHANGE) ;
			return true ;
		    }

		    else if (c==']')
		    {
			near(NEAR_FAR_CHANGE) ;
			return true ;
		    }

		    else if (c=='[')
		    {
			near(1/NEAR_FAR_CHANGE) ;
			return true ;
		    }

		    else if (c=='M')
		    {
			togglePolygonMode() ;
			return true ;
		    }
		    
		    return false ;
		}
	    default:
		return false ;
	    }
	}
    private:
	iris::Augment* _dso ;
	float _x, _y, _z, _h, _p, _r ;
	float _delta_t, _then ;
    } ;
    
    class keyboardControl : public iris::Augment
    {
    public:
	
	keyboardControl():iris::Augment("keyboardControl")
	{
	    setDescription("send control commands using the keyboard") ;
	    _keyboardControlEventHandler = new keyboardControlEventHandler(this) ;
	    iris::SceneGraph::instance()->getViewer()->addEventHandler(_keyboardControlEventHandler.get());
	    validate(); 
	}
	
    private:
	osg::ref_ptr<keyboardControlEventHandler> _keyboardControlEventHandler ;
    } ;
    
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
    return new keyboardControl::keyboardControl ;
}

static int dtkDSO_unloader(dtkAugment* augment)
{
#if 0
    delete augment;
#endif
    return DTKDSO_UNLOAD_CONTINUE;
}

