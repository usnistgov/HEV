// directly sets the navigation matrix based on a matrix in dtk shared memory

#include <stdio.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

#include <iris/Augment.h>
#include <iris/Nav.h>
#include <iris/Utils.h>
#include <iris/SceneGraph.h>


// evil global variable to pass state between class and event handler
static dtkSharedMem* shm = NULL ;

// do this so this DSO's callbacks don't interfere with other DSO's
namespace sharedMemNav
{
    class eventHandler : public osgGA::GUIEventHandler
    {
    public:
	eventHandler(iris::Nav* nav) :  _nav(nav),_first(true) {} ;
	
	bool handle( const osgGA::GUIEventAdapter& ea,
		     osgGA::GUIActionAdapter& aa)
	{
	    if (!_nav)
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::sharedMemNav: can't get NAV object pointer!\n") ;
		return false ;
	    }

	    if (_first)
	    {
		memset(_oldMat,0,sizeof(_oldMat)) ;
		_first = false ;
	    }
	    
	    // every navigation should see if it's active if it wants to honor the active() method
	    if (!_nav->getActive()) return false ;
	    
	    osgGA::GUIEventAdapter::EventType et = ea.getEventType() ;
	    
	    switch(et)
	    {
	    case(osgGA::GUIEventAdapter::FRAME):
		{
		    if (shm)
		    {
			shm->read(_mat) ;
			if (memcmp(_oldMat, _mat, sizeof(_mat)))
			{
			    _nav->setMatrix(osg::Matrix(_mat)) ;
			    memcpy(_oldMat, _mat, sizeof(_mat)) ;
			}
		    }

		    return false ;
		}
	    default:
		return false;
	    }
	    
	    return false ;
	    
	}
    private:
	bool _first ;
	iris::Nav* _nav ;
        bool _active ;
	double _mat[16] ;
	double _oldMat[16] ;
    } ;
    
    class sharedMemNav : public iris::Augment
    {
    public:

	sharedMemNav():iris::Augment("sharedMemNav")
	{
	    setDescription("a DSO that navigates via an osg::Matrix in dtk shared memory") ;

	    osg::ref_ptr<iris::Nav> nav = new iris::Nav("sharedMemNav") ;

	    osg::ref_ptr<eventHandler> eh = new eventHandler(nav.get()) ;
	    iris::SceneGraph::instance()->getViewer()->addEventHandler(eh.get());
	    validate(); 
	}

	// every navigation DSO should implement a control method, even if
	// it only passes it along to the base class
	virtual bool control(const std::string& line, const std::vector<std::string>& vec)
	{
	    if (vec.size() == 1)
	    {
		if (shm) delete shm ;
		shm = new dtkSharedMem(sizeof(double)*16,vec[1].c_str()) ;
		if (shm->isInvalid())
		{
		    delete shm ;
		    dtkMsg.add(DTKMSG_ERROR, "iris::sharedMemNav: can't open shared memory file %s, sizeof 16 doubles\n",vec[1].c_str()) ;
		    shm = NULL ;
		    return false ;
		}
		dtkMsg.add(DTKMSG_INFO, "iris::sharedMemNav: using shared memory file %s\n",vec[1].c_str()) ;
		return true ;
	    }
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
    return new sharedMemNav::sharedMemNav ;
}

static int dtkDSO_unloader(dtkAugment* augment)
{
    return DTKDSO_UNLOAD_CONTINUE;
}

