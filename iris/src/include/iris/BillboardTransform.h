#ifndef __IRIS_BILLBOARD_TRANSFORM
#define __IRIS_BILLBOARD_TRANSFORM

#include <osg/Transform>
#include "iris/MatrixTransform.h"
// #include <osg/LineSegment>
// #include <osg/BoundingSphere>
// #include <osg/BoundingBox>
// #include <osg/MatrixTransform>
// #include <osg/NodeCallback>

// #include <dtk.h>
// #include <dgl.h>
// #include <dosg.h>


namespace iris
{	  
    /**

       \brief The %BillboardTransform class is subclassed from the osg::Tranform
       class.  
       \n \n The tranform contained a this node is modified at each update
       to orient child nodes' (0,-1,0) vector toward the location of the head.  
    */


    class BillboardTransform : public osg::Transform
    {
    public:
        BillboardTransform() ;



        /**
           \brief Axis for primary rotation (default is (0, 0, 1).
           (You can think of this as the "up" vector.)
           At each update, a rotation is done
           about this axis to face the children toward the head location
           as closely as possible.
           \n \n
           If axis rotation mode is set to true then 
           this is the only rotation that is done.
           If axis rotation mode is false, then an additional 
           rotation is done to face the children directly toward the head
           location while rotating the axis vector as little as possible.
        */
        void axis(osg::Vec3) ;

        /**
           \brief Returns the axis vector.
        */
        osg::Vec3 axis() ;

        /**
           \brief Sets the axis rotation mode.  Default is false.
        */
        void axisRotate(bool) ;

        /**
           \brief Returns the axis rotation mode.
        */
        bool axisRotate() ;

        virtual bool computeLocalToWorldMatrix ( osg::Matrix& matrix, 
                                                 osg::NodeVisitor* ) const;

        virtual bool computeWorldToLocalMatrix ( osg::Matrix& matrix, 
                                                 osg::NodeVisitor* ) const;


    protected:
        osg::Vec3    _axis ;
        bool         _axisRotate ;

        osg::Matrix  _getMatrixToHead(osg::Matrix &world, osg::Vec3 xyz) const ;
        void         _updateMatrix() const ;
        // mutable dtkInLocator*    _head ;
        mutable MatrixTransform* _headNode;
        mutable osg::Matrix      _worldMat ;
        mutable osg::Matrix      _oldWorldMat ;
        mutable float            _headPos[6] ;
        mutable float            _oldHeadPos[6] ;
        mutable osg::Matrix      _mat ;
        mutable osg::Matrix      _matInverse ;

    } ;

}

#endif
