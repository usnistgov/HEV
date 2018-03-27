#include <stdio.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

#include <iris/Augment.h>
#include <iris/SceneGraph.h>
#include <iris/Utils.h>


// do this so this DSO's callbacks don't interfere with other DSO's
namespace setHeadWandMatrix
{
    // evil global variables to hopefully speed things up
    bool trackHead ;
    bool trackWand ;

    ////////////////////////////////////////////////////////////////////////
    class setHeadWandMatrixEventHandler : public osgGA::GUIEventHandler 
    {
    public: 
	
	setHeadWandMatrixEventHandler(iris::Augment* dso) : _dso(dso)
	{
	    
	    headShm = new dtkSharedMem(sizeof(head), "head") ;
	    if (!headShm || headShm->isInvalid())
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::setHeadWandMatrix::eventHandler: can't open shared memory \"head\", removing callback\n") ;
		// remove callback
		iris::RemoveEventHandler(this) ;
	    }
	    headMatrixShm = new dtkSharedMem(sizeof(headMatrix), "headMatrix") ;
	    if (!headMatrixShm || headMatrixShm->isInvalid())
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::setHeadWandMatrix::eventHandler: can't open shared memory \"headMatrix\", removing callback\n") ;
		// remove callback
		iris::RemoveEventHandler(this) ;
	    }
	    headNode = iris::SceneGraph::instance()->getHeadNode() ;
	    if (!headNode)
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::setHeadWandMatrix::eventHandler: can't find head node, removing callback\n") ;
		// remove callback
		iris::RemoveEventHandler(this) ;
	    }
	    
	    wandShm = new dtkSharedMem(sizeof(wand), "wand") ;
	    if (!wandShm || wandShm->isInvalid())
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::setHeadWand::eventHandler: can't open shared memory \"wand\", removing callback\n") ;
		// remove callback
		iris::RemoveEventHandler(this) ;
	    }
	    wandMatrixShm = new dtkSharedMem(sizeof(wandMatrix), "wandMatrix") ;
	    if (!wandMatrixShm || wandMatrixShm->isInvalid())
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::setHeadWandMatrix::eventHandler: can't open shared memory \"wandMatrix\", removing callback\n") ;
		// remove callback
		iris::RemoveEventHandler(this) ;
	    }
	    wandNode = iris::SceneGraph::instance()->getWandNode() ;
	    if (!wandNode)
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::setHeadWandMatrix::eventHandler: can't find wand node, removing callback\n") ;
		// remove callback
		iris::RemoveEventHandler(this) ;
	    }
	} ;

	bool handle( const osgGA::GUIEventAdapter& ea,
		     osgGA::GUIActionAdapter& aa)
	{
	    if (!_dso || _dso->isInvalid())
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::setHeadWandMatrix: can't get DSO object pointer!\n") ;
		return false ;
	    }

	    // every navigation should see if it's active if it wants to honor the active() method
	    if (!_dso || _dso->isInvalid() || !_dso->getActive()) return false ;

	    if (ea.getEventType() == osgGA::GUIEventAdapter::FRAME)
	    {
		if (trackHead)
		{
		    // read the head from the tracker
		    headShm->read(head) ;
		    if (memcmp(oldHead, head, sizeof(head)))
		    {
			memcpy(oldHead, head, sizeof(head)) ;
			// if it changes update the node and the shared memory
			headMatrix = iris::CoordToMatrix(head[0], head[1], head[2], head[3], head[4], head[5]) ;
			headNode->setMatrix(headMatrix) ;
			headMatrixShm->write(headMatrix.ptr()) ;
		    }
		}
		
		if (trackWand)
		{
		    // read the wand from the tracker
		    wandShm->read(wand) ;
		    if (memcmp(oldWand, wand, sizeof(wand)))
		    {
			memcpy(oldWand, wand, sizeof(wand)) ;
			// if it changes update the node and the shared memory
			wandMatrix = iris::CoordToMatrix(wand[0], wand[1], wand[2], wand[3], wand[4], wand[5]) ;
			wandNode->setMatrix(wandMatrix) ;
			wandMatrixShm->write(wandMatrix.ptr()) ;
		    }
		}
	    }
	    return false ;
	}
    private:
	iris::Augment* _dso ;

	dtkSharedMem* headShm ;
	dtkSharedMem* headMatrixShm ;
	iris::MatrixTransform* headNode ;
	float head[6] ;
	float oldHead[6] ;
	osg::Matrix headMatrix ;

	dtkSharedMem* wandShm ;
	dtkSharedMem* wandMatrixShm ;
	iris::MatrixTransform* wandNode ;
	float wand[6] ;
	float oldWand[6] ;
	osg::Matrix wandMatrix ;

    } ;

    ////////////////////////////////////////////////////////////////////////
    class setHeadWandMatrix : public iris::Augment
    {
    public:

	setHeadWandMatrix():iris::Augment("setHeadWandMatrix")
	{
	    setDescription("sets the head and wand node matrices based on the dtk shared memory segments \"headMatrix\" and \"wandMatrix\"") ;
	    _setHeadWandMatrixEventHandler = new setHeadWandMatrixEventHandler(this) ;
	    trackHead = trackWand = true ;
	    iris::SceneGraph::instance()->getViewer()->addEventHandler(_setHeadWandMatrixEventHandler.get());
	    validate(); 
	}
    
	bool control(const std::string& line, const std::vector<std::string>& vec)
	{
	    bool onOff ;
	    if (vec.size() == 2 && iris::IsSubstring("head",vec[0],3) && iris::OnOff(vec[1],&onOff))
	    {
		if (onOff)
		{
		    dtkMsg.add(DTKMSG_INFO, "iris::%s::control head tracking on\n",getName()) ;
		}
		else
		{
		    dtkMsg.add(DTKMSG_INFO, "iris::%s::control head tracking off\n",getName()) ;
		}
		trackHead = onOff ;
		return true ;
	    }
	    else if (vec.size() == 2 && iris::IsSubstring("wand",vec[0],3) && iris::OnOff(vec[1],&onOff))
	    {
		if (onOff)
		{
		    dtkMsg.add(DTKMSG_INFO, "iris::%s::control wand tracking on\n",getName()) ;
		}
		else
		{
		    dtkMsg.add(DTKMSG_INFO, "iris::%s::control wand tracking off\n",getName()) ;
		}
		trackWand = onOff ;
		return true ;
	    }
	    else
	    {
		dtkMsg.add(DTKMSG_INFO, "iris::%s::control called with unknown command, %s\n",getName(),line.c_str()) ;
		// if the DSO defines a control method you have to call the base class's control method too
		return iris::Augment::control(line, vec) ;
	    }
	}
	
    private:
	osg::ref_ptr<setHeadWandMatrixEventHandler> _setHeadWandMatrixEventHandler ;
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
    return new setHeadWandMatrix::setHeadWandMatrix ;
}

static int dtkDSO_unloader(dtkAugment* augment)
{
#if 0
    delete augment;
#endif
    return DTKDSO_UNLOAD_CONTINUE;
}

