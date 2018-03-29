#include <iris/MatrixTransform.h>
namespace iris
{
    
    MatrixTransform::MatrixTransform(): 
	osg::MatrixTransform(), 
	_scale(1.0,1.0,1.0)
    { 
	setDataVariance(osg::Object::DYNAMIC) ; 
	getOrCreateStateSet()->setDataVariance(osg::Object::DYNAMIC) ; 
    } ;
    
    MatrixTransform::MatrixTransform(const osg::Matrix& matrix): 
	osg::MatrixTransform(matrix)
    { 
	setDataVariance(osg::Object::DYNAMIC) ; 
	getOrCreateStateSet()->setDataVariance(osg::Object::DYNAMIC) ; 
    } ;
    
    MatrixTransform::MatrixTransform(const MatrixTransform& smt, const osg::CopyOp& copyop):
	osg::MatrixTransform(smt,copyop),
	_position(smt._position), 
	_attitude(smt._attitude), 
	_scale(smt._scale)
    { 
	setDataVariance(osg::Object::DYNAMIC) ; 
	getOrCreateStateSet()->setDataVariance(osg::Object::DYNAMIC) ; 
    } ;
    
    bool MatrixTransform::computeLocalToWorldMatrix(osg::Matrix& matrix, osg::NodeVisitor*) const
    {
	if (_referenceFrame==RELATIVE_RF)
	{
	    matrix.preMult(_matrix);
	}
	else // absolute
	{
	    matrix = _matrix;
	}
	return true;
    }

    bool MatrixTransform::computeWorldToLocalMatrix(osg::Matrix& matrix, osg::NodeVisitor*) const
    {
	const osg::Matrix& inverse = getInverseMatrix();
	if (_referenceFrame==RELATIVE_RF)
	{
	    matrix.postMult(inverse);
	}
	else // absolute
	{
	    matrix = inverse;
	}
	return true;
    }

    void MatrixTransform::_updatePosAttScale()
    {
	// decompose matrix and set position, attitude and scale
	iris::Decompose (_matrix, &_position, &_attitude, &_scale) ;
    }

    void MatrixTransform::_updateMatrix()
    {
	// set matrix based on position, attitude and scale
	_matrix.makeTranslate(_position);
	_matrix.preMultRotate(_attitude);
	_matrix.preMultScale(_scale);
	_inverseDirty = true ;
	dirtyBound();
    }

}
