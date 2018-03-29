#ifndef __IRIS_MATRIX_TRANSFORM__
#define __IRIS_MATRIX_TRANSFORM__

#include <osg/MatrixTransform>
#include <iris/Utils.h>

namespace iris
{

    /**
       \brief The %MatrixTransform class is sub-classed from the
       osg::MatrixTransform class and adds methods to set and get the
       position, attitude and scale in a manner similar to the
       osg::PositionAttitudeTransform node, \b BUT without a pivot point.
       See the iris::Nav class for more about pivot points
    */
    class MatrixTransform : public osg::MatrixTransform
    {
    public:
	
	MatrixTransform() ;
	
	MatrixTransform(const osg::Matrix& matrix) ;
    
	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	MatrixTransform(const MatrixTransform&, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY) ;
    
	//if this is included, the nodes in the OSG files are "iris::MatrixTransform"- Huh?????
	// leaving it out, or changing iris to osg, produces just "MatrixTransform"
	//META_Node(iris, MatrixTransform) ;
    
	inline void setPosition(const osg::Vec3d& pos, bool update=true) { _position = pos ; if (update) _updateMatrix() ; }
	inline const osg::Vec3d& getPosition() { return _position; }
    
	inline void setAttitude(const osg::Quat& quat, bool update=true) { _attitude = quat ; if (update) _updateMatrix() ; }
	inline const osg::Quat& getAttitude() { return _attitude; }
    
	inline void setScale(const osg::Vec3d& scale, bool update=true) { _scale = scale ; if (update) _updateMatrix() ; }
	inline const osg::Vec3d& getScale() { return _scale; }
    
	void setMatrix(const osg::Matrix& mat, bool update=true) { osg::MatrixTransform::setMatrix(mat) ; _updatePosAttScale() ; }
	inline const osg::Matrix& getMatrix() const { return _matrix ; } ;

	void preMult(const osg::Matrix& mat, bool update=true) { osg::MatrixTransform::preMult(mat) ; _updatePosAttScale() ; }

	void postMult(const osg::Matrix& mat, bool update=true) { osg::MatrixTransform::postMult(mat) ; _updatePosAttScale() ; }

	virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const ;
	virtual bool computeWorldToLocalMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const ;
    
    protected: 
	/** update position, attitude and scale, if necessary, based on the value of the matrix */
	void _updatePosAttScale() ;

	/** update the matrix, if necessary,  when position, attitude or scale change */
	void _updateMatrix() ;

	/** the matrix is made from position, attitude and scale; pivot point isn't included */
	osg::Vec3d _position;

	/** the matrix is made from position, attitude and scale; pivot point isn't included */
	osg::Quat _attitude;

	/** the matrix is made from position, attitude and scale; pivot point isn't included */
	osg::Vec3d _scale;

    } ;
}
#endif
