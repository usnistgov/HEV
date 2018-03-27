// Per-frame update of rave uniform variables
//
// current uniforms set each frame in the scene node
//   hev_wandPos    Position (xyz) and Scale (w) of wand in world space
//   hev_wandQuat   Quaternion orientation of wand in world space
//
// Marc Olano, olano@nist.gov, 7/10
// ported to iris by John Kelso kelso@nist.gov, may 2011

////////////////////////////////////////////////////////////////////////
// this assumes that irisfly is running, so the shared memory file
// idea/worldWand exists and is getting updated
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>
#include <iris.h>

class uniformUpdaterEventHandler : public osgGA::GUIEventHandler 
{
public: 
    
    uniformUpdaterEventHandler() : _first(true)
    {
	
	wandShm = new dtkSharedMem(16*sizeof(double), "idea/worldWand") ;
	if (!wandShm || wandShm->isInvalid())
	{
	    dtkMsg.add(DTKMSG_ERROR, "uniformUpdaterEventHandler: can't open shared memory \"idea/worldWand\", removing callback\n") ;
	    // remove callback
	    iris::RemoveEventHandler(this) ;
	}
    } ;
    
    bool handle( const osgGA::GUIEventAdapter& ea,
		 osgGA::GUIActionAdapter& aa)
    {
	if (_first)
	{
	    _dso = dynamic_cast<iris::Augment*>(getUserData()) ;
	    if (!_dso) return false ;
	    _stateSet = iris::SceneGraph::instance()->getSceneNode()->getOrCreateStateSet() ;
	    _first = false ;
	}
	
	// every DSO should see if it's active if it wants to honor the active() method
	if (!_dso || _dso->isInvalid() || !_dso->getActive()) return false ;
	
	if (ea.getEventType() == osgGA::GUIEventAdapter::FRAME)
	{
	    wandShm->read(wand.ptr()) ;
	    if (oldWand != wand)
	    {
		osg::Vec3d p ;
		osg::Quat q  ;
		osg::Vec3d s ; 
		iris::Decompose(wand, &p, &q, &s) ;

		// update uniform for wand position and orientation
		// this won't be right if scaling isn't uniform
		_stateSet->getOrCreateUniform(
		    "hev_wandPos", osg::Uniform::FLOAT_VEC4)
		    ->set(osg::Vec4(p,s.x()));
		_stateSet->getOrCreateUniform(
		    "hev_wandQuat", osg::Uniform::FLOAT_VEC4)
		    ->set(q.asVec4());

		oldWand = wand ;
	    }
	}
	return false ;
    }
private:
    iris::Augment* _dso ;
    osg::StateSet* _stateSet;
    bool _first ;
    dtkSharedMem* wandShm ;
    osg::Matrix wand ;
    osg::Matrix oldWand ;
    
} ;

////////////////////////////////////////////////////////////////////////
class uniformUpdater : public iris::Augment
{
public:
    
    uniformUpdater():iris::Augment("uniformUpdater")
    {
	setDescription("sets the Uniforms hev_wandPos and hev_wandQuat based on the contents of \"idea/worldWand\"") ;
	_uniformUpdaterEventHandler = new uniformUpdaterEventHandler ;
	_uniformUpdaterEventHandler->setUserData(this) ;
	iris::SceneGraph::instance()->getViewer()->addEventHandler(_uniformUpdaterEventHandler.get());
	validate(); 
    }
    
private:
    osg::ref_ptr<uniformUpdaterEventHandler> _uniformUpdaterEventHandler ;
};


/************ DTK C++ dtkAugment loader/unloader functions ***************
 *
 * All DTK dtkAugment DSO files are required to declare these two
 * functions.  These function are called by the loading program to get
 * your C++ objects loaded.
 *
 *************************************************************************/

static dtkAugment* dtkDSO_loader(dtkManager* manager, void* p)
{
    return new uniformUpdater ;
}

static int dtkDSO_unloader(dtkAugment* augment)
{
#if 0
    delete augment;
#endif
    return DTKDSO_UNLOAD_CONTINUE;
}
