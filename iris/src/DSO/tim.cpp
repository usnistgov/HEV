#include <stdio.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

#include <osg/MatrixTransform>

#include <iris/SceneGraph.h>
#include <iris/Augment.h>
#include <iris/Utils.h>
#include <iris/SceneGraph.h>

// do this so this DSO's callbacks don't interfere with other DSO's
namespace tim
{

    class tim ;

    ////////////////////////////////////////////////////////////////////////
    class timEventHandler : public osgGA::GUIEventHandler 
    {
    public: 
	timEventHandler(tim* dso) : _dso(dso) { ; } ;
	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) ;
    private:
	tim* _dso ;
    
    } ;
  

    ////////////////////////////////////////////////////////////////////////
    class tim : public iris::Augment
    {
    public:
	
	tim() : iris::Augment("tim"),_readingNodes(false),_validState(true),_running(false) 
	{
	    setDescription("tim:: Things In Motion") ;
	    osg::ref_ptr<timEventHandler> eh = new timEventHandler(this) ;
	    iris::SceneGraph::instance()->getViewer()->addEventHandler(eh.get());
	    validate(); 
	}
	
	bool control(const std::string& line, const std::vector<std::string>& vec)
	{
	    if (!_validState)
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::TIM: can't proceed due to previous errors\n") ;
		return false ;
	    }

	    //dtkMsg.add(DTKMSG_INFO, "iris::TIM: control passed: \"%s\", vec.size() = %d\n",line.c_str(), vec.size()) ;
	    // if the DSO defines a control method you have to call the base class's control method too
	    
	    //if (!active) return false ;
	    
#if 0
	    fprintf(stderr,"line = %s\n",line.c_str()) ;
	    for (int i=0; i<vec.size(); i++)
	    {
		fprintf(stderr,"%d: \"%s\"\n",i,vec[i].c_str()) ;
	    }
#endif
	    if (!_readingNodes && vec.size() == 2 && iris::IsSubstring("begin", vec[0],3)) 
	    {
		dtkMsg.add(DTKMSG_INFO, "iris::TIM: BEGIN %s\n",vec[1].c_str()) ;
		_readingNodes = true ;
		_shmName = vec[1] ;
		_nodesRead = 0 ;
	    }
	    else if (_readingNodes && vec.size() == 2 && iris::IsSubstring("end", vec[0],3) && vec[1] == _shmName) 
	    {
		_shm = new dtkSharedMem(_nodesRead*sizeof(double)*16,vec[1].c_str()) ;
		if (!_shm || _shm->isInvalid())
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris::TIM: can't open shared memory \"%s\"\n",vec[1].c_str()) ;    
		    _validState = false ;
		    return false ;
		}
		dtkMsg.add(DTKMSG_INFO, "iris::TIM: shared memory %s stores %d nodes\n",vec[1].c_str(),_nodesRead) ;
		_matrices =    static_cast<osg::Matrix*>(malloc(_nodesRead*sizeof(double)*16)) ;
		_oldMatrices = static_cast<osg::Matrix*>(malloc(_nodesRead*sizeof(double)*16)) ;
		memset(_oldMatrices,0,sizeof(_oldMatrices)) ;
		dtkMsg.add(DTKMSG_INFO, "iris::TIM: END %s, with %d nodes\n",_shmName.c_str(),_nodesRead) ;
		_readingNodes = false ;
	    }
	    else if (!_readingNodes && vec.size() == 2 && iris::IsSubstring("start", vec[0],3) && vec[1] == _shmName) 
	    { 
		dtkMsg.add(DTKMSG_INFO, "iris::TIM: START %s\n",_shmName.c_str()) ;
		_running = true ;
	    }
	    else if (!_readingNodes && vec.size() == 2 && iris::IsSubstring("stop", vec[0],3) && vec[1] == _shmName) 
	    { 
		dtkMsg.add(DTKMSG_INFO, "iris::TIM: STOP %s\n",_shmName.c_str()) ;
		_running = false ;
	    }
	    else if (!_readingNodes && vec.size() == 2 && iris::IsSubstring("update", vec[0],3) && vec[1] == _shmName) 
	    {
		dtkMsg.add(DTKMSG_INFO, "iris::TIM: UPDATE %s\n",_shmName.c_str()) ;
		update() ;
	    }
	    else if (_readingNodes)
	    {
		for (int i=0; i<vec.size(); i++)
		{
		    osg::Node* node = iris::SceneGraph::instance()->findNode(vec[i]) ;
		    if (node)
		    {
			osg::MatrixTransform* mat = dynamic_cast<osg::MatrixTransform*>(node) ;
			if (mat)
			{
			    dtkMsg.add(DTKMSG_INFO, "iris::TIM: node %d: %s\n",_nodesRead,vec[i].c_str()) ;
			    _nodes.push_back(mat) ;
			}
			else
			{
			    dtkMsg.add(DTKMSG_WARNING, "iris::TIM: node %d: %s is not a osg::MatrixTransform, skipping in UPDATE\n",_nodesRead,vec[i].c_str()) ; 
			    _nodes.push_back(NULL) ;
			}
		    }
		    else
		    {
			dtkMsg.add(DTKMSG_WARNING, "iris::TIM: node %d: %s not found, skipping in UPDATE\n",_nodesRead,vec[i].c_str()) ;
			_nodes.push_back(NULL) ;
		    }
		    _nodesRead++ ;
		}
	    }
	    else
	    {
		return iris::Augment::control(line,vec) ;
	    }
	    return true ;
	}
    
	void update()
	{
	    _shm->read(_matrices) ;
	    for (int i=0; i<_nodesRead; i++)
	    {
		if (_nodes[i])
		{
	      
#if 0
		    if ( (memcmp(_matrices[i].ptr(),_oldMatrices[i].ptr(),sizeof(double)*16) && (_matrices[i] == _oldMatrices[i])) ||
			 (!memcmp(_matrices[i].ptr(),_oldMatrices[i].ptr(),sizeof(double)*16) && (_matrices[i] != _oldMatrices[i])))
		    {
			fprintf(stderr,"compare problem!\n") ;
		    }
#endif
	      
#if 0
		    if (_matrices[i] != _oldMatrices[i])
		    {
			_nodes[i]->setMatrix(_matrices[i]) ;
			_oldMatrices[i] = _matrices[i] ;
		    }
#else
		    if(memcmp(_matrices[i].ptr(), _oldMatrices[i].ptr(),sizeof(double)*16))
		    {
			_nodes[i]->setMatrix(_matrices[i]) ;
			memcpy(_oldMatrices[i].ptr(), _matrices[i].ptr(), sizeof(double)*16) ;
		    }
#endif
		}
	    }
	}
    
	bool getRunning() { return _running ; } ;
 	
    private:
	bool _validState ;
	bool _readingNodes ;
	int _nodesRead ;
	std::vector<osg::MatrixTransform*> _nodes ;
	dtkSharedMem* _shm ;
	std::string _shmName ;
	osg::Matrix* _matrices ;
	osg::Matrix* _oldMatrices ;
	bool _running ;
    } ;
    
    bool timEventHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
	if (!_dso || _dso->isInvalid())
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::tim: can't get DSO object pointer!\n") ;
	    return false ;
	}

	// every DSO should see if it's active if it wants to honor the active() method
	if (_dso && !_dso->getActive()) return false ;
    
	if (ea.getEventType() == osgGA::GUIEventAdapter::FRAME)
	{
	    if (_dso->getRunning()) _dso->update() ;
	}
    
    }
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
    return new tim::tim ;
}

static int dtkDSO_unloader(dtkAugment* augment)
{
#if 0
    delete augment;
#endif
    return DTKDSO_UNLOAD_CONTINUE;
}

