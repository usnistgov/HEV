#include <stdio.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>
#include <osgDB/ReadFile>
#include <osg/ComputeBoundsVisitor>

#include <iris/Augment.h>
#include <iris/SceneGraph.h>
#include <iris/Nav.h>
#include <iris/ClipNode.h>

// do this so this DSO's callbacks don't interfere with other DSO's
namespace toggleObjects
{

    ////////////////////////////////////////////////////////////////////////
    class toggleObjectsEventHandler : public osgGA::GUIEventHandler
    {
    public: 

	toggleObjectsEventHandler(iris::Augment* dso) : _dso(dso), _first(true), _head(false), _pivotAxis(false), _wand(false), _cubeScene(0), _cubeEther(0), _cubeNav(0), _cubeWorld(0), _boundingObjects(0) {} ;

	
	void togglePivotAxis()
	{
	    _pivotAxis = !_pivotAxis ;
	    _pivotAxisSwitch->setValue(0,_pivotAxis) ;
	}

	void toggleHead()
	{
	    _head = !_head ;
	    _headSwitch->setValue(0,_head) ;
	}

	void toggleWand()
	{
	    _wand = !_wand ;
	    _wandSwitch->setValue(0,_wand) ;
	}

	void toggleSceneCube()
	{
	    _cubeScene = (_cubeScene+1)%4 ;
	    _cubeSceneSwitch->setValue(0,_cubeScene&1) ;
	    _cubeSceneSwitch->setValue(1,_cubeScene&2) ;
	}

	void toggleEtherCube()
	{
	    _cubeEther = (_cubeEther+1)%4 ;
	    _cubeEtherSwitch->setValue(0,_cubeEther&1) ;
	    _cubeEtherSwitch->setValue(1,_cubeEther&2) ;
	}

	void toggleNavCube()
	{
	    _cubeNav = (_cubeNav+1)%4 ;
	    _cubeNavSwitch->setValue(0,_cubeNav&1) ;
	    _cubeNavSwitch->setValue(1,_cubeNav&2) ;
	}

	// special code for toggle objects under world node- remove them so
	// iris::SceneGraph::examine won't include them in its bounding
	// operation
	void toggleWorldCube()
	{
	    _cubeWorld = (_cubeWorld+1)%4 ;
	    _cubeWorldSwitch->setValue(0,_cubeWorld&1) ;
	    _cubeWorldSwitch->setValue(1,_cubeWorld&2) ;
	    if (_cubeWorld == 0 ) iris::SceneGraph::instance()->getWorldNode()->removeChild(_cubeWorldSwitch) ;
	    else iris::SceneGraph::instance()->getWorldNode()->addChild(_cubeWorldSwitch) ;
	}

	void toggleBoundingObjects()
	{
	    osg::BoundingSphere bs ;
	    osg::BoundingBox bb ;
	    for (unsigned int i=0; i<iris::SceneGraph::instance()->getWorldNode()->getNumChildren(); i++)
	    {
		osg::Node* node = iris::SceneGraph::instance()->getWorldNode()->getChild(i) ;
		if (node != _cubeWorldSwitch && node != _boundingObjectsSwitch)
		{
		    bs.expandBy(node->getBound()) ;
		    osg::ComputeBoundsVisitor  cbv;
		    cbv.setNodeMaskOverride(~0) ;
		    node->accept(cbv);
		    osg::BoundingBox cbvbb = cbv.getBoundingBox() ;
		    bb.expandBy(cbvbb) ;
		    //bs.expandBy(osg::BoundingSphere(cbvbb.center(),cbvbb.radius())) ;
		}
	    }
	    if (bs.radius()>0.f)
	    {
		_boundingSphereMatrixTransformNode->setPosition(bs.center(),false) ;
		_boundingSphereMatrixTransformNode->setScale(osg::Vec3(bs.radius(),bs.radius(),bs.radius())) ;
		_boundingCubeMatrixTransformNode->setPosition((bb._max+bb._min)/2.f,false) ;
		_boundingCubeMatrixTransformNode->setScale(osg::Vec3((bb._max.x()-bb._min.x())/2.f, (bb._max.y()-bb._min.y())/2.f, (bb._max.z()-bb._min.z())/2.f)) ;

		_boundingObjects = (_boundingObjects+1)%3 ;
		_boundingObjectsSwitch->setValue(0,_boundingObjects&1) ;
		_boundingObjectsSwitch->setValue(1,_boundingObjects&2) ;
		if (_boundingObjects == 0 ) iris::SceneGraph::instance()->getWorldNode()->removeChild(_boundingObjectsSwitch) ;
		else iris::SceneGraph::instance()->getWorldNode()->addChild(_boundingObjectsSwitch) ;
	    }
	    else dtkMsg.add(DTKMSG_NOTICE, "iris::toggleObjects: No data loaded under world node\n") ;



	}

	bool handle( const osgGA::GUIEventAdapter& ea,
		     osgGA::GUIActionAdapter& aa)
	{
	    if (!_dso || _dso->isInvalid())
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::writeState: can't get DSO object pointer!\n") ;
		return false ;
	    }

	    if (_first)
	    {

		_headSwitch = new osg::Switch ;
		for (int i=0; i<iris::ClipNode::maxNodes(); i++) _headSwitch->getOrCreateStateSet()->setMode(GL_CLIP_PLANE0+i, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );
		_headSwitch->addChild(osgDB::readNodeFile("head.osg")) ;
		_headSwitch->setValue(0,_head) ;
		iris::SceneGraph::instance()->getHeadNode()->addChild(_headSwitch) ;

		_pivotAxisSwitch = new osg::Switch ;
		for (int i=0; i<iris::ClipNode::maxNodes(); i++) _pivotAxisSwitch->getOrCreateStateSet()->setMode(GL_CLIP_PLANE0+i, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );
		_pivotAxisSwitch->addChild(osgDB::readNodeFile("axis.osg")) ;
		_pivotAxisSwitch->setValue(0,_pivotAxis) ;
		iris::SceneGraph::instance()->getPivotNode()->addChild(_pivotAxisSwitch) ;


		_wandSwitch = new osg::Switch ;
		for (int i=0; i<iris::ClipNode::maxNodes(); i++) _wandSwitch->getOrCreateStateSet()->setMode(GL_CLIP_PLANE0+i, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );
		_wandSwitch->addChild(osgDB::readNodeFile("wand.osg")) ;
		_wandSwitch->setValue(0,_wand) ;
		iris::SceneGraph::instance()->getWandNode()->addChild(_wandSwitch) ;

		osg::ref_ptr<osg::Node> cubeFrame = osgDB::readNodeFile("cubeFrame.osg") ;
		osg::ref_ptr<osg::Node> cubePanel = osgDB::readNodeFile("cubePanel.osg") ;

		_cubeSceneSwitch = new osg::Switch ;
		for (int i=0; i<iris::ClipNode::maxNodes(); i++) _cubeSceneSwitch->getOrCreateStateSet()->setMode(GL_CLIP_PLANE0+i, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );
		_cubeSceneSwitch->addChild(cubeFrame) ;
		_cubeSceneSwitch->setValue(0,_cubeScene&1) ;
		_cubeSceneSwitch->addChild(cubePanel) ;
		_cubeSceneSwitch->setValue(1,_cubeScene&2) ;
		iris::SceneGraph::instance()->getSceneNode()->addChild(_cubeSceneSwitch) ;

		_cubeEtherSwitch = new osg::Switch ;
		for (int i=0; i<iris::ClipNode::maxNodes(); i++) _cubeEtherSwitch->getOrCreateStateSet()->setMode(GL_CLIP_PLANE0+i, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );
		_cubeEtherSwitch->addChild(cubeFrame) ;
		_cubeEtherSwitch->setValue(0,_cubeEther&1) ;
		_cubeEtherSwitch->addChild(cubePanel) ;
		_cubeEtherSwitch->setValue(1,_cubeEther&2) ;
		iris::SceneGraph::instance()->getEtherNode()->addChild(_cubeEtherSwitch) ;

		_cubeNavSwitch = new osg::Switch ;
		for (int i=0; i<iris::ClipNode::maxNodes(); i++) _cubeNavSwitch->getOrCreateStateSet()->setMode(GL_CLIP_PLANE0+i, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );
		_cubeNavSwitch->addChild(cubeFrame) ;
		_cubeNavSwitch->setValue(0,_cubeNav&1) ;
		_cubeNavSwitch->addChild(cubePanel) ;
		_cubeNavSwitch->setValue(1,_cubeNav&2) ;
		iris::SceneGraph::instance()->getNavNode()->addChild(_cubeNavSwitch) ;

		_cubeWorldSwitch = new osg::Switch ;
		for (int i=0; i<iris::ClipNode::maxNodes(); i++) _cubeWorldSwitch->getOrCreateStateSet()->setMode(GL_CLIP_PLANE0+i, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );
		_cubeWorldSwitch->addChild(cubeFrame) ;
		_cubeWorldSwitch->setValue(0,_cubeWorld&1) ;
		_cubeWorldSwitch->addChild(cubePanel) ;
		_cubeWorldSwitch->setValue(1,_cubeWorld&2) ;
		//iris::SceneGraph::instance()->getWorldNode()->addChild(_cubeWorldSwitch) ;

		_boundingObjectsSwitch = new osg::Switch ;
		for (int i=0; i<iris::ClipNode::maxNodes(); i++) _boundingObjectsSwitch->getOrCreateStateSet()->setMode(GL_CLIP_PLANE0+i, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );
		//iris::SceneGraph::instance()->getWorldNode()->addChild(_boundingObjectsSwitch) ;

		_boundingSphereMatrixTransformNode = new iris::MatrixTransform ;
		_boundingObjectsSwitch->addChild(_boundingSphereMatrixTransformNode) ;
		_boundingSphereMatrixTransformNode->addChild(osgDB::readNodeFile("boundingSphere.osg")) ;

		_boundingCubeMatrixTransformNode = new iris::MatrixTransform ;
		_boundingObjectsSwitch->addChild(_boundingCubeMatrixTransformNode) ;
		_boundingCubeMatrixTransformNode->addChild(osgDB::readNodeFile("boundingCube.osg")) ;

		_boundingObjectsSwitch->setValue(0,_boundingObjects&1) ;
		_boundingObjectsSwitch->setValue(1,_boundingObjects&2) ;

		_first = false ;
	    }
	
	    // every navigation should see if it's active if it wants to honor the active() method
	    if (_dso && !_dso->getActive()) return false ;

	    switch(ea.getEventType())
	    {
	    case osgGA::GUIEventAdapter::KEYDOWN:
		{
		    int c = ea.getKey() ;
#if 0
		    if (c>=' ' && c<='~') fprintf(stderr,"c = x%X, \"%c\"\n",c,c) ;
		    else fprintf(stderr,"c = x%X\n",c) ;
		    std::vector<int> keys = iris::SceneGraph::instance()->getPressedKeys() ;
		    fprintf(stderr,"%d keys pressed\n",keys.size()) ;
		    for (unsigned int k=0; k<keys.size(); k++)
		    {
			if (keys[k]>=' ' && keys[k]<='~') fprintf(stderr,"keys[%d] = x%X, \"%c\"\n",k,keys[k],keys[k]) ;
			else fprintf(stderr,"keys[%d] = x%X\n",k,keys[k]) ;
		    }
#endif
		    if (c=='h')
		    {
			toggleHead() ;
			return true ;
		    }

		    if (c=='p')
		    {
			togglePivotAxis() ;
			return true ;
		    }

		    else if (c=='w')
		    {
			toggleWand() ;
			return true ;
		    }

		    else if (c=='c')
		    {
			toggleSceneCube() ;
			return true ;
		    }

		    else if (c=='b')
		    {
			toggleBoundingObjects() ;
			return true ;
		    }

		    // somebody's got a bug in the keyboard stuff!  only happens with left-shift, e, c- caps lock and right shift OK
		    //else if ((c=='C' && iris::SceneGraph::instance()->pollKey('E')) || (c=='e' && iris::SceneGraph::instance()->pollKey('E')))
		    else if (c=='E')
		    {
			toggleEtherCube() ;
			return true ;
		    }

		    else if (c=='N')
		    {
			toggleNavCube() ;
			return true ;
		    }

		    else if (c=='W')
		    {
			toggleWorldCube() ;
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
	bool _head, _wand, _pivotAxis ;
	unsigned int _cubeScene,  _cubeEther,  _cubeNav,  _cubeWorld, _boundingObjects ;
	osg::ref_ptr<osg::Switch> _headSwitch, _pivotAxisSwitch, _wandSwitch, _cubeSceneSwitch, _cubeEtherSwitch, _cubeNavSwitch, _cubeWorldSwitch, _boundingObjectsSwitch ;
	osg::ref_ptr<iris::MatrixTransform> _boundingSphereMatrixTransformNode, _boundingCubeMatrixTransformNode ;
    } ;
    
    class toggleObjects : public iris::Augment
    {
    public:
	
	toggleObjects():iris::Augment("toggleObjects")
	{
	    setDescription("toggle objects using the keyboard") ;
	    _toggleObjectsEventHandler = new toggleObjectsEventHandler(this) ;
	    iris::SceneGraph::instance()->getViewer()->addEventHandler(_toggleObjectsEventHandler.get());
	    validate(); 
	}
	
    private:
	osg::ref_ptr<toggleObjectsEventHandler> _toggleObjectsEventHandler ;
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
    return new toggleObjects::toggleObjects ;
}

static int dtkDSO_unloader(dtkAugment* augment)
{
#if 0
    delete augment;
#endif
    return DTKDSO_UNLOAD_CONTINUE;
}

