#include <limits.h>
#include <iris/SceneGraph.h>
#include <iris/Nav.h>
#include <iris/ShmState.h>
#include <iris/Utils.h>

namespace iris
{
    void ShmState::_init()
    {
	_initTransformNode(&_world, "world") ;
	_initTransformNode(&_nav, "nav") ;
	_initTransformNode(&_head, "head") ;
	_initTransformNode(&_wand, "wand") ;
	_initFloat(&_response, "response") ;
	_initFloat(&_refTime, "refTime") ;
	_initInt(&_frameNumber, "frameNumber") ;
	_initFloat(&_frameRate, "frameRate") ;
	_initString(&_navName, "navName") ;
	_initInt(&_numNavs, "numNavs") ;

    }

    ////////////////////////////////////////////////////////////////////////
    void ShmState::update()
    {
	_updateTransformNode(&_world, SceneGraph::instance()->getWorldNode()) ;
	_updateTransformNode(&_nav, SceneGraph::instance()->getNavNode()) ;
	_updateTransformNode(&_head, SceneGraph::instance()->getHeadNode()) ;
	_updateTransformNode(&_wand, SceneGraph::instance()->getWandNode()) ;
	_updateFloat(&_response, Nav::getResponse()) ;
	osg::FrameStamp* fs = SceneGraph::instance()->getViewer()->getFrameStamp() ;
	_updateFloat(&_refTime, float(fs->getReferenceTime())) ;
	_updateInt(&_frameNumber, fs->getFrameNumber()) ;
	
	// and get the time we're updated
	double thisReferenceTime = fs->getReferenceTime() ;
	_updateFloat(&_frameRate, 1.f/static_cast<float>(thisReferenceTime - _lastReferenceTime)) ;
	_lastReferenceTime = thisReferenceTime ;
	_updateString(&_navName, Nav::getCurrentNav()->getName()) ;
	_updateInt(&_numNavs, Nav::getNavList().size()) ;
    }

    ////////////////////////////////////////////////////////////////////////
    dtkSharedMem* ShmState::getShm(DataType type, std::string label)
    {
	std::map<std::string, DataElement>::iterator pos ;
	pos = _map.find(label) ;
	if (pos == _map.end())
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::ShmState::getShm can't find ShmState item %s\n",label.c_str()) ;
	    return NULL ;
	}
	else if (type != pos->second.type)
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::ShmState::getShm can't find ShmState item %s of specified type\n",label.c_str()) ;
	    return NULL ;
	}
	else return pos->second.shm ;
    }
    

    ////////////////////////////////////////////////////////////////////////
    void ShmState::_initTransformNode(DataElement *d, std::string shmName)
    {
	std::string fullName = shmPrefix + shmName ;

	dtkSharedMem* shm = new dtkSharedMem(sizeof(double)*16, fullName.c_str()) ;
	if (shm->isInvalid())
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::ShmState::_initTransformNode: unable to open shared memory file %s\n",fullName.c_str()) ;
	    delete shm ;
	    shm = NULL ;
	}
	d->matrixValue.set(FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX) ;
	d->shm = shm ;
	d->type = MATRIX ;
	_map.insert(std::make_pair(shmName, *d)) ;

    }
    
    ////////////////////////////////////////////////////////////////////////
    void ShmState::_initFloat(DataElement *d, std::string shmName)
    {
	std::string fullName = shmPrefix + shmName ;

	dtkSharedMem* shm = new dtkSharedMem(sizeof(float), fullName.c_str()) ;
	if (shm->isInvalid())
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::ShmState::_initFloat: unable to open shared memory file %s\n",fullName.c_str()) ;
	    delete shm ;
	    shm = NULL ;
	}

	d->floatValue = FLT_MAX ;
	d->shm = shm ;
	d->type = FLOAT ;
	_map.insert(std::make_pair(shmName, *d)) ;

    }
    
    ////////////////////////////////////////////////////////////////////////
    void ShmState::_initInt(DataElement *d, std::string shmName)
    {
	std::string fullName = shmPrefix + shmName ;

	dtkSharedMem* shm = new dtkSharedMem(sizeof(int), fullName.c_str()) ;
	if (shm->isInvalid())
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::ShmState::_initInt: unable to open shared memory file %s\n",fullName.c_str()) ;
	    delete shm ;
	    shm = NULL ;
	}

	d->intValue = INT_MAX ;
	d->shm = shm ;
	d->type = INT ;
	_map.insert(std::make_pair(shmName, *d)) ;

    }
    
    ////////////////////////////////////////////////////////////////////////
    void ShmState::_initString(DataElement *d, std::string shmName)
    {
	std::string fullName = shmPrefix + shmName ;

	dtkSharedMem* shm = new dtkSharedMem(maxStringValueSize, fullName.c_str()) ;
	if (shm->isInvalid())
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::ShmState::_initString: unable to open shared memory file %s\n",fullName.c_str()) ;
	    delete shm ;
	    shm = NULL ;
	}
	
	d->stringValue = "this is a string that hopefully won't match any other trash value in memory or some other random initialization, but knowing Murphy's law I'll eventually get lucky I figure." ;
	d->shm = shm ;
	d->type = STRING ;
	_map.insert(std::make_pair(shmName, *d)) ;

    }
    
    ////////////////////////////////////////////////////////////////////////
    void ShmState::_updateTransformNode(DataElement *d, MatrixTransform* node)
    {
	osg::Matrix mat = node->getMatrix() ;
	if (d->shm && mat != d->matrixValue)
	{
	    d->matrixValue = mat ;
	    d->shm->write(mat.ptr()) ;
	}	
    }

    ////////////////////////////////////////////////////////////////////////
    void ShmState::_updateFloat(DataElement *d, float shmData)
    {
	if (d->shm && shmData != d->floatValue) 
	{
	    d->floatValue = shmData ;
	    d->shm->write(&shmData) ;
	}
    }

    ////////////////////////////////////////////////////////////////////////
    void ShmState::_updateInt(DataElement *d, int shmData)
    {
	if (d->shm && shmData != d->intValue)
	{
	    d->intValue = shmData ;
	    d->shm->write(&shmData) ;
	}
    }

    ////////////////////////////////////////////////////////////////////////
    void ShmState::_updateString(DataElement *d, const std::string& shmData)
    {
	if (shmData.size() > maxStringValueSize)
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::ShmState::_updateString: string \"%s\" is too long, %d passed, %d allowed\n",shmData.c_str(),shmData.size(),maxStringValueSize) ; 
	    return ;
	}
	if (d->shm && shmData != d->stringValue) 
	{
	    d->stringValue = shmData ;
	    d->shm->write(shmData.c_str()) ;
	}
    }
    const std::string ShmState::shmPrefix = "iris/state/" ;
    const int ShmState::maxStringValueSize = 512 ;
}

