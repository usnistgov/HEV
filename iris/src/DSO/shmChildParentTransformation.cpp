#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

#include <osg/Matrix>
#include <osg/Vec3>
#include <osg/Quat>

#include <iris/SceneGraph.h>
#include <iris/Augment.h>
#include <iris/Utils.h>

// do this so this DSO's callbacks don't interfere with other DSO's
namespace shmChildParentTransformation
{
    
    enum When { GET, STOP, START } ;

    // this is what is stored for every NODETRANSFORMATION
    struct ShmChildParentTransformation
    {
	dtkSharedMem *shm ;
	osg::Node *parent ;
	osg::Node *child ;
	osg::Matrix mat ;
	When when ;
    } ;

    std::map<std::string,ShmChildParentTransformation> shmChildParentTransformations ;

    ////////////////////////////////////////////////////////////////////////
    class shmChildParentTransformationEventHandler : public osgGA::GUIEventHandler
    {
    public: 
	
	shmChildParentTransformationEventHandler(): _first(true) { ; } ;

	
	bool handle( const osgGA::GUIEventAdapter& ea,
		     osgGA::GUIActionAdapter& aa)
	{
	    for (std::map<std::string,ShmChildParentTransformation>::iterator itr = shmChildParentTransformations.begin(); itr != shmChildParentTransformations.end(); itr++)
	    {
		if (itr->second.when != STOP)
		{
		    std::vector<osg::Matrix> matVector =  itr->second.child->getWorldMatrices(itr->second.parent) ;
		    if (matVector.size() != 1)
		    {
			dtkMsg.add(DTKMSG_ERROR, "iris::shmChildParentTransformation::eventHandler: \"%s\" child node has more than one parent, skipping\n",itr->first.c_str()) ;
			return false ;
		    }
		    if (_first || matVector[0] != itr->second.mat)
		    {
			itr->second.mat = matVector[0] ;
			itr->second.shm->write((matVector[0]).ptr()) ;
			if (itr->second.when == GET) itr->second.when = STOP ;
		    }
		}
	    }
	    _first = false ;
	    return false ;
	}
    private:
	bool _first ;
    } ;

    ////////////////////////////////////////////////////////////////////////
    class shmChildParentTransformation : public iris::Augment
    {
    public:
	shmChildParentTransformation():iris::Augment("shmChildParentTransformation") 
	{
	    
	    setDescription("%s- writes to shared memory the transformation between scenegraph nodes",getName()) ;
	    _shmChildParentTransformationEventHandler = new shmChildParentTransformationEventHandler ;
	    iris::SceneGraph::instance()->getViewer()->addEventHandler(_shmChildParentTransformationEventHandler.get());
	    // dtkAugment::dtkAugment() will not validate the object
	    validate() ;
	} ;
	
	bool control(const std::string& line, const std::vector<std::string>& vec)
	{
	    ShmChildParentTransformation scpt ;
	    scpt.when = START ;
	    if (vec.size() == 4 && iris::IsSubstring("init", vec[0], 3))
	    {	
		scpt.shm = new dtkSharedMem(sizeof(double)*16, vec[1].c_str()) ;
		if (!scpt.shm || scpt.shm->isInvalid())
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris::shmChildParentTransformation::control: can't open shared memory \"%s\"\n",vec[1].c_str()) ;
		    return false ;
		}
		scpt.child = iris::SceneGraph::instance()->findNode(vec[2]) ;
		if (!scpt.child)
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris::shmChildParentTransformation::control: can't find child node \"%s\"\n",vec[2].c_str()) ;
		    return false ;
		}
		if (vec[3] =="-")
		{
		    osg::Node::ParentList parentList = scpt.child->getParents() ;
		    if (parentList.size() != 1)
		    {
			dtkMsg.add(DTKMSG_ERROR, "iris::shmChildParentTransformation::control: child node \"%s\" has %d parents\n",vec[2].c_str(),parentList.size()) ;
			return false ;
		    }
		    else
		    {
			scpt.parent = parentList[0] ;
		    }
		}
		else
		{
		    scpt.parent = iris::SceneGraph::instance()->findNode(vec[3]) ;
		}
		if (!scpt.parent)
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris::shmChildParentTransformation::control: can't find parent node \"%s\"\n",vec[3].c_str()) ;
		    return false ;
		}
		
		std::vector<osg::Matrix> matVector = scpt.child->getWorldMatrices(scpt.parent) ;
		if (matVector.size() != 1)
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris::shmChildParentTransformation::control: there are %d paths from child to parent\n",matVector.size()) ;
		    return false ;
		}
		scpt.mat = osg::Matrix(matVector[0]) ;
		dtkMsg.add(DTKMSG_INFO, "iris::shmChildParentTransformation::control INIT shm = %s child = %s parent = %s\n", vec[1].c_str(), vec[2].c_str(), vec[3].c_str()) ;
		
	    }
	    else if (vec.size() >1)
	    {	
		dtkMsg.add(DTKMSG_INFO, "iris::shmChildParentTransformation::control %s shm = %s\n", vec[0].c_str(),vec[1].c_str()) ;
		
		std::map<std::string,ShmChildParentTransformation>::iterator itr = shmChildParentTransformations.find(vec[1]) ;
		if (itr == shmChildParentTransformations.end())
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris::shmChildParentTransformation::control: can't find shmChildParentTransformation \"%s\"\n",vec[1].c_str()) ;
		    return false ;
		}
		scpt = itr->second ;
		if (iris::IsSubstring("get", vec[0], 3) && scpt.when != START) scpt.when = GET ;
		else if (iris::IsSubstring("start", vec[0], 3)) scpt.when = START ;
		else if (iris::IsSubstring("stop", vec[0], 3)) scpt.when = STOP ;
		else return false ;
	    }
	    else
	    {
		return iris::Augment::control(line,vec) ;
	    }

	    shmChildParentTransformations[vec[1]] = scpt ;
	    return true ;
		

	}
	
    private:
	osg::ref_ptr<shmChildParentTransformationEventHandler> _shmChildParentTransformationEventHandler ;
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
    return new shmChildParentTransformation::shmChildParentTransformation ;
}

static int dtkDSO_unloader(dtkAugment* augment)
{
#if 0
    delete augment;
#endif
    return DTKDSO_UNLOAD_CONTINUE;
}

