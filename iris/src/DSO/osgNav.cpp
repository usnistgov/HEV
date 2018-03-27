// try to get an OSG manipulator to work as a iris navigation DSO

#include <stdio.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

#include <osgGA/TrackballManipulator>

#include <iris/Augment.h>
#include <iris/SceneGraph.h>
#include <iris/Nav.h>


// do this so this DSO's callbacks don't interfere with other DSO's
namespace osgNav
{

    class eventHandler : public osgGA::TrackballManipulator
    {
    public:

	eventHandler(iris::Nav* nav) :  _nav(nav) {} ;

	bool handle( const osgGA::GUIEventAdapter& ea,
		     osgGA::GUIActionAdapter& aa)
	{

	    if (!_nav)
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::osgNav:: can't get NAV object pointer!\n") ;
	    }
	
	    // every navigation should see if it's active if it wants to honor the active() method
	    if (_nav && !_nav->getActive()) return false ;

	    osgGA::TrackballManipulator::handle(ea,aa) ;
	
	    osg::Matrix m =  getMatrix() ;
	    osg::Vec3d translation ;
	    osg::Quat rotation ;
	    iris::Decompose(m, &translation, &rotation) ;

	    _nav->setPosition(translation) ;
	    _nav->setAttitude(rotation) ;

	    return false ;
	}
    private:
	iris::Nav* _nav ;
	osgGA::GUIEventAdapter::EventType _lastEvent ;
    };
    
    class osgNav : public iris::Augment
    {
    public:

	osgNav():iris::Augment("osgNav")
	{
	    setDescription("osg manipulator Navigation") ;
	    osg::ref_ptr<iris::Nav> nav = new iris::Nav("osgManipulatorNav") ;
	    osg::ref_ptr<eventHandler> eh = new eventHandler(nav.get()) ;
	    eh->setNode(iris::SceneGraph::instance()->getSceneNode()) ;
	    iris::SceneGraph::instance()->getViewer()->setCameraManipulator(eh) ;
	    iris::SceneGraph::instance()->getViewer()->addEventHandler(eh.get());

	    validate(); 
	}

	// every navigation DSO should implement a control method, even if
	// it only passes it along to the base class
	virtual bool control(const std::string& line, const std::vector<std::string>& vec)
	{
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
    return new osgNav::osgNav ;
}

static int dtkDSO_unloader(dtkAugment* augment)
{
    return DTKDSO_UNLOAD_CONTINUE;
}

