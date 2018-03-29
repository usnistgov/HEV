#ifndef __IRIS_SHM_MATRIX_TRANSFORM__
#define __IRIS_SHM_MATRIX_TRANSFORM__

#include <dtk.h>
#include <osg/MatrixTransform>

namespace iris
{
    /**
       \brief The %ShmMatrixTransform class is sub-classed from the osg::MatrixTransform
       class.  
       \n \n Each %ShmMatrixTransform object is assigned a DTK shared memory
       file that's the size of a osg::Matrix, and is used to set the node's
       matrix transformation.  Each object also has the option to be assigned a
       one byte DTK shared memory file called the "active byte" and one byte
       mask.  
       \n \n When the contents of the mask and active byte are <tt>AND</tt>ed and
       non-zero, the matrix is updated, otherwise the matrix is unchanged.  This
       can allow the matrix to be updated only when a specific wand button is
       pressed, for example.

    */

    class ShmMatrixTransform : public osg::MatrixTransform
    {
    public:

	ShmMatrixTransform() ;
	
	ShmMatrixTransform(const std::string& matrixName, const std::string& activeName = "", unsigned char activeMask = 0xFF) ; 
      
	// need to add a MatrixTransform(const Matrix& matix); constructor?

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	ShmMatrixTransform(const ShmMatrixTransform&, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

	META_Node(iris, ShmMatrixTransform);

	// pick your flavor
	bool isValid() { return _valid ; } ;
	bool isInvalid() { return !_valid ; } ;
	dtkSharedMem* getMatShm() { return _matrixShm ; } ;
	dtkSharedMem* getActiveShm() { return _activeShm ; } ;
	void setActiveMask(char activeMask) { _activeMask = activeMask ; } ;
	unsigned char getActiveMask() { return _activeMask ; } ;

    private:
	std::string _matrixName ;
	std::string _activeName ;
	unsigned char _activeMask ;
	bool _valid ;
	dtkSharedMem* _matrixShm ;
	dtkSharedMem* _activeShm ;
    } ;
}

#endif

