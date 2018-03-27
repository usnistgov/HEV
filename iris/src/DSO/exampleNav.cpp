#include <stdio.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

// dtk/dtkDSO_loader.h declares the loader and unloader functions so
// this may be loaded with a object dtkManager.
#include <dtk/dtkDSO_loader.h>

#include <iris/Augment.h>
#include <iris/Nav.h>
#include <iris/SceneGraph.h>
#include <iris/Utils.h>

// do this so this DSO's callbacks don't interfere with other DSO's
namespace exampleNav
{

    ////////////////////////////////////////////////////////////////////////
    // set the node's position and attitude from dtk shared memory "wandMatrix"
    class nodeCB : public osg::NodeCallback
    {
    public:

	nodeCB(iris::Nav* nav) : _nav(nav), _first(true), _wandShm(NULL) {} ;

	virtual void operator()(osg::Node* node,
				osg::NodeVisitor* nv)
	{
	    if (_first)
	    {

		if (!_nav)
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris::exampleNav::nodeCB: can't get NAV object pointer!\n") ;
		    return ;
		}

		_wandShm = new dtkSharedMem(sizeof(float)*16, "wandMatrix") ;
		if (!_wandShm || _wandShm->isInvalid())
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris::exampleNav::nodeCB: can't open shared memory \"wand\", removing callback\n") ;
		    // remove node callback
		    node->removeUpdateCallback(this) ;
		    return ;
		}

		_first = false ;
	    }

	    // every navigation should see if it's active if it wants to honor the active() method
	    if (_nav && _nav->getActive())
	    {
		_wandShm->read(_wand.ptr()) ;
		if (_oldWand != _wand)
		{
		    _nav->setMatrix(_wand) ;
		    _oldWand = _wand ;
		}
	    }

	    // Continue traversing so that OSG can process
	    //   any other nodes with callbacks.
	    traverse(node, nv);
	}
    private:
	iris::Nav* _nav ;
	bool _first ;
	dtkSharedMem* _wandShm ;
	osg::Matrix _wand ;
	osg::Matrix _oldWand ;
    };

    class exampleNav : public iris::Augment
    {
    public:

	exampleNav():iris::Augment("exampleNav")
	{
	    setDescription("a sample iris::Nav DSO with a node callback and control method") ;

	    osg::ref_ptr<iris::Nav> nav = new iris::Nav("examplNav") ;

	    _nodeCB = new nodeCB(nav.get()) ;
	    iris::SceneGraph::instance()->getSceneNode()->addUpdateCallback(_nodeCB.get()) ;
	    validate(); 
	}
    
	// every navigation DSO should implement a control method, even if
	// it only passes it along to the base class
	bool control(const std::string& line, const std::vector<std::string>& vec)
	{
#if 0
	    dtkMsg.add(DTKMSG_INFO, "iris::%s::control called with %d args:\n",getName(),vec.size()) ;
	    for (int i=0; i<vec.size(); i++)
	    {
		dtkMsg.add(DTKMSG_INFO, "    %d: %s\n",i,vec[i].c_str()) ;
	    }
#else
	    std::string cmd = vec[0] ;
	    for (int i=1; i<vec.size(); i++)
	    {
		cmd += std::string(" ") + vec[i] ;
	    }
	    dtkMsg.add(DTKMSG_INFO, "iris::%s::control passed: \"%s\"\n",getName(),cmd.c_str()) ;
#endif
	    // pass to Nav class too
	    return iris::Nav::getCurrentNav()->control(line, vec) ;
	}

    private:
	osg::ref_ptr<nodeCB> _nodeCB ;
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
    return new exampleNav::exampleNav ;
}

static int dtkDSO_unloader(dtkAugment* augment)
{
#if 0
    delete augment;
#endif
    return DTKDSO_UNLOAD_CONTINUE;
}

