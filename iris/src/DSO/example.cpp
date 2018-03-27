#include <stdio.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

#include <iris/Augment.h>
#include <iris/SceneGraph.h>
#include <iris/Nav.h>

// do this so this DSO's callbacks don't interfere with other DSO's
namespace example
{

    ////////////////////////////////////////////////////////////////////////
    class exampleEventHandler : public osgGA::GUIEventHandler 
    {
    public: 

	
	exampleEventHandler(iris::Augment* dso) :  _dso(dso) {} ;

	bool handle( const osgGA::GUIEventAdapter& ea,
		     osgGA::GUIActionAdapter& aa)
	{
	    
	    if (!_dso || _dso->isInvalid())
	    {
		dtkMsg.add(DTKMSG_ERROR, "example: can't get DSO object pointer!") ;
		return false ;
	    }
	
	    //fprintf(stderr,"_dso->getActive() = %d\n",_dso->getActive()) ;

	    // every navigation should see if it's active if it wants to honor the active() method
	    if (_dso && !_dso->getActive()) return false ;

#if 0
	    static int c = 0 ;
	    if (c<8)
	    {
		fprintf(stderr,"exampleEventHandler: c = %d\n",c) ;
		c++ ;
	    }
#endif
	    switch(ea.getEventType())
	    {
	    case osgGA::GUIEventAdapter::PUSH:
		{
		    //fprintf(stderr,"exampleEventHandler::PUSH\n") ;
#if 0
		    int b = ea.getButton() ;
		    fprintf(stderr,"    button %d at mouse %f %f nmouse %f %f\n",b,ea.getXnormalized(),ea.getYnormalized(),ea.getX(),ea.getY()) ;
		    bool s = iris:SceneGraph::instance()->pollButton(1) ;
		    fprintf(stderr,"      button 1 is %d\n        ",s) ;
		    std::vector<int> v = iris:SceneGraph::instance()->getPressedButtons() ;
		    for (unsigned int i=0; i<v.size(); i++) fprintf(stderr,"%d ",v[i]) ;
		    fprintf(stderr,"\n") ;
#endif
		    return(false);
		}
	    
	    case osgGA::GUIEventAdapter::RELEASE:
		{
		    //fprintf(stderr,"exampleEventHandler::RELEASE\n") ;
#if 0		    
		    int b = ea.getButton() ;
		    fprintf(stderr,"    button %d\n",b) ;
		    bool s = iris:SceneGraph::instance()->pollButton(1) ;
		    fprintf(stderr,"      button 1 is %d\n        ",s) ;
		    std::vector<int> v = iris:SceneGraph::instance()->getPressedButtons() ;
		    for (unsigned int i=0; i<v.size(); i++) fprintf(stderr,"%d ",v[i]) ;
		    fprintf(stderr,"\n") ;
#endif
		    return(false);
		}    
	    case osgGA::GUIEventAdapter::KEYDOWN:
		{
#if 1
		    //fprintf(stderr,"exampleEventHandler::KEYDOWN\n") ;
		    int c = ea.getKey() ;
		    fprintf(stderr,"    c = %d\n          ",c) ;
		    std::vector<int> v = iris::SceneGraph::instance()->getPressedKeys() ;
		    for (unsigned int i=0; i<v.size(); i++) fprintf(stderr,"%d ",v[i]) ;
		    fprintf(stderr,"\n") ;
#endif
		    return(false);
		}
	    case osgGA::GUIEventAdapter::KEYUP:
		{
#if 1
		    //fprintf(stderr,"exampleEventHandler::KEYUP\n") ;
		    int c = ea.getKey() ;
		    fprintf(stderr,"    c = %d\n          ",c) ;
		    std::vector<int> v = iris::SceneGraph::instance()->getPressedKeys() ;
		    for (unsigned int i=0; i<v.size(); i++) fprintf(stderr,"%d ",v[i]) ;
		    fprintf(stderr,"\n") ;
#endif
		    return(false);
		}
	    case osgGA::GUIEventAdapter::DRAG:
		{
		    //fprintf(stderr,"exampleEventHandler::DRAG\n") ;
		    return(false);
		}
	    default:
		return(false);
	    }
	}
    private:
	iris::Augment* _dso ;

    } ;

    ////////////////////////////////////////////////////////////////////////
    class nodeCB : public osg::NodeCallback
    {
    public:

	nodeCB(iris::Augment* dso) : _dso(dso), _angle(0.f) {} ;

	virtual void operator()(osg::Node* node,
				osg::NodeVisitor* nv)
	{

	    if (!_dso || _dso->isInvalid())
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::example: can't get DSO object pointer!\n") ;
		return ;
	    }

	    // every navigation should see if it's active if it wants to honor the active() method
	    if (_dso && _dso->isValid() && _dso->getActive())
	    {
		iris::Nav::setAttitude(osg::Quat(_angle, osg::Vec3( 0., 0., 1.))) ;
		_angle+=.01 ;
	    }
	    // Continue traversing so that OSG can process
	    //   any other nodes with callbacks.
	    traverse(node, nv);
	}
    private:
	iris::Augment* _dso ;
	float _angle ;
    };

    class example : public iris::Augment
    {
    public:

	example():iris::Augment("example")
	{
#if 1
	    setDescription("a sample iris:: DSO with a node callback, event handler and control method") ;
	    _nodeCB = new nodeCB(this) ;
	    iris::SceneGraph::instance()->getSceneNode()->addUpdateCallback(_nodeCB.get()) ;
#else
	    setDescription("a sample iris:: DSO with an event handler and control method") ;
#endif
	    _exampleEventHandler = new exampleEventHandler(this) ;
	    iris::SceneGraph::instance()->getViewer()->addEventHandler(_exampleEventHandler.get());

	    validate(); 
	}
    
	bool control(const std::string& line, const std::vector<std::string>& vec)
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::example: in the actual DSO, %s::control passed: \"%s\"\n",line.c_str()) ;
	    // if the DSO defines a control method you have to call the base class's control method too
	    return iris::Augment::control(line,vec) ;
	}

#if 0
	int preConfig()
	{
	    fprintf(stderr,"example::preConfig called\n") ;
	    return DTK_CONTINUE ;
	}

	int postConfig()
	{
	    fprintf(stderr,"example::postConfig called\n") ;
	    return DTK_CONTINUE ;
	}

	int preFrame()
	{
	    fprintf(stderr,"example::preFrame called\n") ;
	    return DTK_CONTINUE ;
	}

	int postFrame()
	{
	    fprintf(stderr,"example::postFrame called\n") ;
	    return DTK_CONTINUE ;
	}

#endif

    private:
	osg::ref_ptr<nodeCB> _nodeCB ;
	osg::ref_ptr<exampleEventHandler> _exampleEventHandler ;
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
    return new example::example ;
}

static int dtkDSO_unloader(dtkAugment* augment)
{
#if 0
    delete augment;
#endif
    return DTKDSO_UNLOAD_CONTINUE;
}

