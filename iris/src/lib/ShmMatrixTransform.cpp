#include <iris/ShmMatrixTransform.h>
#include <iris/Utils.h>

namespace iris
{

    // update the matrix in the node
    class ShmMatrixTransformCB : public osg::NodeCallback
    {
    public:
	ShmMatrixTransformCB(ShmMatrixTransform* node) :  _node(node), _waitTwo(0){ ; } ;

	virtual void operator()(osg::Node* node,
				osg::NodeVisitor* nv)
	{

	    if (!_node)
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::ShmMatrixTransformCB: can't get ShmMatrixTransform node pointer!\n") ;
		return ;
	    }

	    if (_waitTwo<2)
	    {
		osg::Matrix matrix ;
		_node->getMatShm()->read(matrix.ptr()) ;
		if (_oldMatrix != matrix)
		{
		    _node->setMatrix(matrix) ;
		    _oldMatrix = matrix ;
		}	
		_waitTwo++ ;
	    }

	    if (_node->getActiveMask() != 0)
	    {
		unsigned char active ;
		bool testMatrix = false ;
		if (!(_node->getActiveShm()))
		{
		    testMatrix = true ;
		}
		else
		{
		    _node->getActiveShm()->read(&active) ;
		    if (active & _node->getActiveMask()) 
		    {
			testMatrix = true ;
		    }
		}
		if (testMatrix)
		{
		    osg::Matrix matrix ;
		    _node->getMatShm()->read(matrix.ptr()) ;
		    if (_oldMatrix != matrix)
		    {
			//PrintMatrix(matrix) ;
			_node->setMatrix(matrix) ;
			_oldMatrix = matrix ;
		    }
		}
	    }
	}
	
    private:
	ShmMatrixTransform* _node ;
	osg::Matrix _oldMatrix ;
	// wait for two graphics frames to initialize the node's
	// transformation.  This ensure other callbacks that write to the
	// shared memory will get a chance to initialize it
	int _waitTwo ;
    } ;
    
    
    ////////////////////////////////////////////////////////////////////////
    ShmMatrixTransform::ShmMatrixTransform()
    {
	_matrixName = "" ;
	_activeName = "" ;
	_activeMask = 0 ;
	_matrixShm = NULL ;
	_activeShm = NULL ;
	setDataVariance(osg::Object::DYNAMIC) ;
    }

    ////////////////////////////////////////////////////////////////////////
    ShmMatrixTransform::ShmMatrixTransform(const std::string& matrixName, const std::string& activeName, unsigned char activeMask) : _valid(false), _matrixShm(NULL), _activeShm(NULL)
    {
	_matrixName = matrixName ;
	_activeName = activeName ;
	_activeMask = activeMask ;
	setDataVariance(osg::Object::DYNAMIC) ;
	
	_matrixShm = new dtkSharedMem(sizeof(double)*16,  _matrixName.c_str()) ;
	if (_matrixShm->isInvalid())
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::ShmMatrixTransform: can't open shared memory %s\n",_matrixName.c_str()) ;
	    return ;
	}
	
	if ( _activeName != "")
	{
	    _activeShm = new dtkSharedMem(sizeof(unsigned char),  _activeName.c_str()) ;
	    if (_activeShm->isInvalid())
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::ShmMatrixTransform: can't open shared memory %s\n", activeName.c_str()) ;
		return ;
	    }
	}
	
	ShmMatrixTransformCB* cb = new ShmMatrixTransformCB(this) ;
	addUpdateCallback(cb) ;
	
    }
    
    ////////////////////////////////////////////////////////////////////////
    ShmMatrixTransform::ShmMatrixTransform(const ShmMatrixTransform& smt, const osg::CopyOp& copyop):
	MatrixTransform(smt,copyop),
	_matrixName(smt._matrixName),
	_activeName(smt._activeName),
	_activeMask(smt._activeMask),
	_valid(smt._valid),
	_matrixShm(smt._matrixShm),
	_activeShm(smt._activeShm)
    {    
	setDataVariance(osg::Object::DYNAMIC) ;
    }
    
}
