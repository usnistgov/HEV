#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

#include <osg/Matrix>
#include <osg/Vec3>
#include <osg/Quat>

#include <iris/SceneGraph.h>
#include <iris/Augment.h>
#include <iris/Utils.h>

// do this so this DSO's callbacks don't interfere with other DSO's
namespace shmNodeToNodeTransformation
{
    
    enum When { GET, STOP, START } ;

    // this is what is stored for every NODETRANSFORMATION
    struct ShmNodeToNodeTransformation
    {
	dtkSharedMem *shm ;
	osg::Node* start ;
	osg::Node* finish ;
	osg::Node* common ;
	osg::Matrix startMat ;
	osg::Matrix finishMat ;
	When when ;
    } ;

    std::map<std::string,ShmNodeToNodeTransformation*> shmNodeToNodeTransformations ;

    ////////////////////////////////////////////////////////////////////////
    class shmNodeToNodeTransformationEventHandler : public osgGA::GUIEventHandler
    {
    public: 
	
	shmNodeToNodeTransformationEventHandler(): _first(true) { ; } ;

	
	bool handle( const osgGA::GUIEventAdapter& ea,
		     osgGA::GUIActionAdapter& aa)
	{
	    // look at each map entry
	    for (std::map<std::string,ShmNodeToNodeTransformation*>::iterator itr = shmNodeToNodeTransformations.begin(); itr != shmNodeToNodeTransformations.end(); itr++)
	    {
		ShmNodeToNodeTransformation* sntnt = itr->second ;
		if (sntnt->when != STOP)
		{
		    std::vector<osg::Matrix> startMatVector = sntnt->start->getWorldMatrices(sntnt->common) ;
		    if (startMatVector.size() != 1)
		    {
			dtkMsg.add(DTKMSG_ERROR, "iris::shmNodeToNodeTransformation::control: there are %d paths from start to common parent\n",startMatVector.size()) ;
			return false ;
		    }
		    std::vector<osg::Matrix> finishMatVector = sntnt->finish->getWorldMatrices(sntnt->common) ;
		    if (finishMatVector.size() != 1)
		    {
			dtkMsg.add(DTKMSG_ERROR, "iris::shmNodeToNodeTransformation::control: there are %d paths from finish to common parent\n",finishMatVector.size()) ;
			return false ;
		    }
		    
		    if (_first || startMatVector[0] != sntnt->startMat || finishMatVector[0] != sntnt->finishMat)
		    {
			osg::Matrix mat = startMatVector[0] * osg::Matrix::inverse(finishMatVector[0]) ;
			sntnt->startMat = startMatVector[0] ;
			sntnt->finishMat = finishMatVector[0] ;
			sntnt->shm->write(mat.ptr()) ;
			if (sntnt->when == GET) sntnt->when = STOP ;
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
    class shmNodeToNodeTransformation : public iris::Augment
    {
    public:
	shmNodeToNodeTransformation():iris::Augment("shmNodeToNodeTransformation") 
	{
	    
	    setDescription("%s- writes to shared memory the transformation between scenegraph nodes",getName()) ;
	    _shmNodeToNodeTransformationEventHandler = new shmNodeToNodeTransformationEventHandler ;
	    iris::SceneGraph::instance()->getViewer()->addEventHandler(_shmNodeToNodeTransformationEventHandler.get());
	    // dtkAugment::dtkAugment() will not validate the object
	    validate() ;
	} ;
	
	bool control(const std::string& line, const std::vector<std::string>& vec)
	{
	    ShmNodeToNodeTransformation* sntnt ;

	    // INIT start finish|- [common]
	    if ((vec.size() == 4 || vec.size() == 5) && iris::IsSubstring("init", vec[0], 3))
	    {	
		sntnt = new ShmNodeToNodeTransformation ;
		sntnt->shm = new dtkSharedMem(sizeof(double)*16, vec[1].c_str()) ;
		if (!(sntnt->shm) || sntnt->shm->isInvalid())
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris::shmNodeToNodeTransformation::control: can't open shared memory \"%s\"\n",vec[1].c_str()) ;
		    delete sntnt ;
		    return false ;
		}
		sntnt->start = iris::SceneGraph::instance()->findNode(vec[2]) ;
		if (!(sntnt->start))
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris::shmNodeToNodeTransformation::control: can't find start node \"%s\"\n",vec[2].c_str()) ;
		    delete sntnt ;
		    return false ;
		}

		sntnt->finish = iris::SceneGraph::instance()->findNode(vec[3]) ;
		if (!(sntnt->finish))
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris::shmNodeToNodeTransformation::control: can't find finish node \"%s\"\n",vec[3].c_str()) ;
		    delete sntnt ;
		    return false ;
		}

		std::string common ;
		if (vec.size()==4) 
		{
		    sntnt->common = iris::SceneGraph::instance()->getSceneNode() ;
		    common = "scene" ;
		}
		else
		{
		    common = vec[4] ;
		    sntnt->common = iris::SceneGraph::instance()->findNode(common) ;
		    if (!(sntnt->common))
		    {
			dtkMsg.add(DTKMSG_ERROR, "iris::shmNodeToNodeTransformation::control: can't find common parent node \"%s\"\n",common.c_str()) ;
			delete sntnt ;
			return false ;
		    }
		}

		std::vector<osg::Matrix> startMatVector = sntnt->start->getWorldMatrices(sntnt->common) ;
		if (startMatVector.size() != 1)
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris::shmNodeToNodeTransformation::control: there are %d paths from start to common parent\n",startMatVector.size()) ;
		    delete sntnt ;
		    return false ;
		}
		sntnt->startMat = startMatVector[0] ;
		std::vector<osg::Matrix> finishMatVector = sntnt->finish->getWorldMatrices(sntnt->common) ;
		if (finishMatVector.size() != 1)
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris::shmNodeToNodeTransformation::control: there are %d paths from finish to common parent\n",finishMatVector.size()) ;
		    delete sntnt ;
		    return false ;
		}
		sntnt->finishMat = finishMatVector[0] ;

		dtkMsg.add(DTKMSG_INFO, "iris::shmNodeToNodeTransformation::control INIT shm = %s start = %s finish = %s common = %s\n", vec[1].c_str(), vec[2].c_str(), vec[3].c_str(), common.c_str()) ;
		
	    }

	    // not an INIT command but maybe a GET START or STOP?
	    else if (vec.size() >1)
	    {	
		dtkMsg.add(DTKMSG_INFO, "iris::shmNodeToNodeTransformation::control %s shm = %s\n", vec[0].c_str(),vec[1].c_str()) ;
		
		std::map<std::string,ShmNodeToNodeTransformation*>::iterator itr = shmNodeToNodeTransformations.find(vec[1]) ;
		if (itr == shmNodeToNodeTransformations.end())
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris::shmNodeToNodeTransformation::control: can't find shmNodeToNodeTransformation \"%s\"\n",vec[1].c_str()) ;
		    return false ;
		}
		sntnt = itr->second ;
		if (iris::IsSubstring("get", vec[0], 3) && sntnt->when != START) sntnt->when = GET ;
		else if (iris::IsSubstring("start", vec[0], 3)) sntnt->when = START ;
		else if (iris::IsSubstring("stop", vec[0], 3)) sntnt->when = STOP ;
		else return false ;
	    }

	    // none of the above
	    else
	    {
		return iris::Augment::control(line,vec) ;
	    }

	    // update or add to the data in the map
	    shmNodeToNodeTransformations[vec[1]] = sntnt ;
	    return true ;
		

	}
	
    private:
	osg::ref_ptr<shmNodeToNodeTransformationEventHandler> _shmNodeToNodeTransformationEventHandler ;
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
    return new shmNodeToNodeTransformation::shmNodeToNodeTransformation ;
}

static int dtkDSO_unloader(dtkAugment* augment)
{
#if 0
    delete augment;
#endif
    return DTKDSO_UNLOAD_CONTINUE;
}

