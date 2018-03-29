#include <dtk.h>
#include <iris/BillboardTransform.h>
#include <iris/SceneGraph.h>

// #include <iris/Utils.h>



namespace iris
{
    
    BillboardTransform::BillboardTransform()
    {
	memset(_headPos, 0, sizeof(_headPos)) ;
	memset(_oldHeadPos, 0, sizeof(_oldHeadPos)) ;
	_axis = osg::Vec3(0.f, 0.f, 1.f) ;
	_axisRotate = false ;
	_headNode = SceneGraph::instance()->getHeadNode();
        _updateMatrix() ;
    }

    bool BillboardTransform::
         computeLocalToWorldMatrix(osg::Matrix& matrix, osg::NodeVisitor*) const
    {
        _updateMatrix() ;
	matrix.preMult(_mat) ;
	return true;
    }

    bool BillboardTransform::
         computeWorldToLocalMatrix(osg::Matrix& matrix, osg::NodeVisitor*) const
    {
	matrix.postMult(_matInverse) ;

	return true;
    }

    void BillboardTransform::axis(osg::Vec3 axis)
    {
	_axis = axis ;
    }

    osg::Vec3 BillboardTransform::axis() 
    {
	return _axis ;
    }
    
    void BillboardTransform::axisRotate(bool b) 
    {
	_axisRotate = b ;
    }

    bool BillboardTransform::axisRotate() 
    {
	return _axisRotate ;
    }
	
    // given the node, the world matrix and the postion of the head,
    // calculate the matrix to point the node at the head
    osg::Matrix BillboardTransform::
            _getMatrixToHead(osg::Matrix &world, osg::Vec3 xyz) const
    {
        osg::Vec3d t ;
        osg::Quat r ; 
	iris::Decompose (world, &t, &r) ;


	// the following algorithm is from
	// http://www.evl.uic.edu/yg/billboard.html, 
	// written by Alex Hill, ahill@evl.uic.edu
	// acording to Alex, the code is public domain
	// I must admit I get a bit lost in step 4
	// Thanks Alex!!

	// 1- get a vector from the node to the head position
	osg::Vec3 pos = xyz - t;
	pos.normalize();

	// 2- get the cross product of the vector going from the node to
	// the position and the rotation axis. 
	osg::Vec3 axisCrossPos = _axis ^ pos ;
	axisCrossPos.normalize();
  
	// 3- get the cross product of the front axis of the node, 
        // (0, -1, 0), and the rotation axis
	osg::Vec3 axisCrossFront;
	axisCrossFront = _axis ^ osg::Vec3(0.f, -1.f, 0.f);
	axisCrossFront.normalize();
	
	// 4a- get the cross of the axis and the axisCrossFront- 
        // supposed to be a normal towards the viewer, but this one 
        // baffles me
	osg::Vec3 axisCross_AxisCrossFront;
	axisCross_AxisCrossFront = _axis ^ axisCrossFront ;
	axisCross_AxisCrossFront.normalize() ;
        // JGH: axisCross_AxisCrossFront is perp to axis and in opposite
        //      direction of front vector.

	// 4b- get the rotation of the axis to the viewer vector not sure what
	// right triangle we're using for the sides though- part of the mystery
	// above I guess.
	float opposite = axisCross_AxisCrossFront * axisCrossPos ;
	float adjacent = axisCrossFront * axisCrossPos ;
	float axisRot = atan2(opposite, adjacent) ;

	// 5- create a matrix- rotate around the axis
	// if the axis is the default, this changes heading
	osg::Matrix rotMat = osg::Matrix::
                rotate(axisRot, osg::Vec3(_axis[0], _axis[1], _axis[2]));
  
	// if not in axis mode, add another rotation
	if (!_axisRotate)
	{
	    //6- get the rotation from the axis to the vector to the head
	    float crossRot;
	    crossRot = asin(_axis * pos);
	    
	    //7- rotate around the axis normal to the viewer vector and axis
	    osg::Matrix pitchMat = 
                osg::Matrix::rotate (-crossRot,
                                     osg::Vec3(axisCrossPos[0],
                                     axisCrossPos[1],axisCrossPos[2])) ;
	    
	    //add this rotation matrix to the existing rotation matrix
	    rotMat.postMult(pitchMat);
	}
 
	return rotMat * osg::Matrix::rotate(r.inverse()) ;

    }  // end of _getMatrixToHead

    void BillboardTransform::_updateMatrix() const
    {
	if (!_headNode )
	{
	    _headNode = SceneGraph::instance()->getHeadNode();
            if (_headNode == NULL)
                {
		// try again next time
                return ;
                }
	}



	bool _recalculate = false ;

	// get the parent of this node
	osg::Node::ParentList parentVector = getParents() ;
	if (parentVector.size() != 1)
	{
            dtkMsg.add(DTKMSG_DEBUG, 
                "iris::BillboardTransform::_updateMatrix: "
                "number of parents of this node not equal to one!!!!\n"  );
	    return ;
	}
	

	// get the matrix from the scene to parent of this node
	std::vector<osg::Matrix> matVector = 
	    parentVector[0]->
              getWorldMatrices ( SceneGraph::instance()->getSceneNode() );
	
	if (matVector.size() != 1)
	{
            dtkMsg.add(DTKMSG_DEBUG, 
                "iris::BillboardTransform::_updateMatrix: "
                "Number of paths to scene not equal to one!!!!\n" ) ;
	    return ;
	}
	_worldMat = matVector[0] ;
	
	// recalculate if its changed
	if (_worldMat != _oldWorldMat)
	{
	    _recalculate = true ;
	    _oldWorldMat = _worldMat ;
	}

        osg::Vec3 vheadPos = _headNode->getPosition() ;
        _headPos[0] = vheadPos[0];
        _headPos[1] = vheadPos[1];
        _headPos[2] = vheadPos[2];



	// recalculate if its changed- we only care if the head moves, 
        // ignore orientation
	if (memcmp(_oldHeadPos, _headPos, sizeof(float)*3))
	{
	    _recalculate = true ;
#if 0
             dtkMsg.add(DTKMSG_DEBUG, 
                "\nhead = %f %f %f\n",
                "iris::BillboardTransform::_updateMatrix: "
                "head = %f %f %f\n", _headPos[0], _headPos[1], _headPos[2]) ;
#endif
	    memcpy(_oldHeadPos, _headPos, sizeof(_headPos)) ;
	}
	    

	if (_recalculate)
	{
	    _mat = _getMatrixToHead ( 
                        _worldMat, 
                        osg::Vec3(_headPos[0], _headPos[1], _headPos[2])) ;
	    _matInverse = osg::Matrix::inverse(_mat) ;
	}

    }  // end of _updateMatrix

}  // end of namespace iris





