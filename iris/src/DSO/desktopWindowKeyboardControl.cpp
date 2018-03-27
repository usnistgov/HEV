#include <stdio.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

#include <iris/SceneGraph.h>
#include <iris/Utils.h>
#include <iris/Augment.h>

// do this so this DSO's callbacks don't interfere with other DSO's
namespace desktopWindowKeyboardControl
{

    ////////////////////////////////////////////////////////////////////////
    class desktopWindowKeyboardControlEventHandler : public osgGA::GUIEventHandler
    {
    public: 

	desktopWindowKeyboardControlEventHandler(iris::Augment* dso) : _dso(dso), _first(true), _type(0) { } ;

	
	////////////////////////////////////////////////////////////////////////
	bool handle( const osgGA::GUIEventAdapter& ea,
		     osgGA::GUIActionAdapter& aa)
	{
	    if (_first)
	    {
		if (!_dso || _dso->isInvalid())
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris::desktopWindowKeyboardControl: can't get DSO object pointer!\n") ;
		    return false ;
		}
		_desktopWindow = dynamic_cast<iris::Augment*>(iris::SceneGraph::instance()->check("desktopWindow")) ;
		if (!_desktopWindow || _desktopWindow->isInvalid()) 
		{
		    dtkMsg.add(DTKMSG_WARNING, "iris::desktopWindowDSOControl: DSO desktopWindow not loaded\n") ;
		}
		_first = false ;
	    }
	    
	    if (!_desktopWindow || _desktopWindow->isInvalid()) return false ;

	    // every navigation should see if it's active if it wants to honor the active() method
	    if (_dso && !_dso->getActive()) return false ;

	    switch(ea.getEventType())
	    {
	    case osgGA::GUIEventAdapter::KEYDOWN:
		{
		    int c = ea.getKey() ;

		    if (c=='f')
		    {
			_type = (_type+1)%3 ;
			if (_type == 0) _desktopWindow->control("PERSPECTIVE") ;
			else if (_type == 1) _desktopWindow->control("IMMERSIVE") ;
			else if (_type == 2) _desktopWindow->control("ORTHO") ;
			return true ;
		    }

		    return false ;
		}
	    default:
		return false ;
	    }
	}
    private:
	bool _first ;
	iris::Augment* _dso ;
	iris::Augment* _desktopWindow ;
	int _type ;
    } ;
    
    class desktopWindowKeyboardControl : public iris::Augment
    {
    public:
	
	desktopWindowKeyboardControl():iris::Augment("desktopWindowKeyboardControl")
	{
	    setDescription("send control commands using the keyboard") ;
	    _desktopWindowKeyboardControlEventHandler = new desktopWindowKeyboardControlEventHandler(this) ;
	    iris::SceneGraph::instance()->getViewer()->addEventHandler(_desktopWindowKeyboardControlEventHandler.get());
	    validate(); 
	}
	
    private:
	osg::ref_ptr<desktopWindowKeyboardControlEventHandler> _desktopWindowKeyboardControlEventHandler ;
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
    return new desktopWindowKeyboardControl::desktopWindowKeyboardControl ;
}

static int dtkDSO_unloader(dtkAugment* augment)
{
#if 0
    delete augment;
#endif
    return DTKDSO_UNLOAD_CONTINUE;
}

