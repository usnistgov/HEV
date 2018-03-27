#include <stdio.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

#include <osgGA/GUIEventHandler>

#include <iris/Augment.h>
#include <iris/SceneGraph.h>
#include <iris/ShmState.h>

// do this so this DSO's callbacks don't interfere with other DSO's
namespace writeState
{
    ////////////////////////////////////////////////////////////////////////
    class writeStateEventHandler : public osgGA::GUIEventHandler 
    {
    public: 
	writeStateEventHandler(iris::Augment* dso) : _dso(dso)  {} ;

	////////////////////////////////////////////////////////////////////////
	bool handle( const osgGA::GUIEventAdapter& ea,
		     osgGA::GUIActionAdapter& aa)
	{
	    if (!_dso || _dso->isInvalid())
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::writeState: can't get DSO object pointer!\n") ;
		return false ;
	    }
	    

	    // every DSO should see if it's active if it wants to honor the active() method
	    if (_dso && !_dso->getActive()) return false ;
	
	    if (ea.getEventType() == osgGA::GUIEventAdapter::FRAME)
	    {
		_shmState.update() ;
	    }

	    return false ;
	}

    private:
	iris::Augment* _dso ;
	iris::ShmState _shmState ;

    } ;

    ////////////////////////////////////////////////////////////////////////
    class writeState : public iris::Augment
    {
    public:

	writeState():iris::Augment("writeState")
	{
	    setDescription("writeState:: writes iris state to dtk shared memory files") ;
	    _writeStateEventHandler = new writeStateEventHandler(this) ;
	    iris::SceneGraph::instance()->getViewer()->addEventHandler(_writeStateEventHandler.get());

	    validate(); 
	}
    
#if 0
	bool control(const std::string& line, const std::vector<std::string>& vec)
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::writeState DSO: %s::control passed: \"%s\"\n",line.c_str()) ;
	    // if the DSO defines a control method you have to call the base class's control method too
	    return iris::Augment::control(line,vec) ;
	}
#endif

    private:
	osg::ref_ptr<writeStateEventHandler> _writeStateEventHandler ;
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
    return new writeState::writeState ;
}

static int dtkDSO_unloader(dtkAugment* augment)
{
#if 0
    delete augment;
#endif
    return DTKDSO_UNLOAD_CONTINUE;
}

