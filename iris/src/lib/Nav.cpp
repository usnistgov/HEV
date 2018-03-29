#include <dtk.h>

#include <iris/Nav.h>
#include <iris/SceneGraph.h>
#include <iris/Utils.h>
namespace iris
{

    ////////////////////////////////////////////////////////////////////////
    Nav::Nav(const char* name, bool addToList)
    {
	if (name) _name = strdup(name) ;
	else _name = NULL ;

	if (!_nav) _nav = SceneGraph::instance()->getNavNode() ;

	_resetPivotNode = SceneGraph::instance()->getWorldNode() ;
	_pivotNode = _resetPivotNode ;

	if (addToList)
	{
	    if (_currentNav) _currentNav->setActive(false) ;
	    _navList.push_back(this) ;
	    _currentNav = this ;
	}
	_active = addToList ;
    
    }

    ////////////////////////////////////////////////////////////////////////
    // this is for an object that just wants to mess with the class, like
    // the one in the SceneGraph class
    Nav::Nav(bool addToList) : _name(NULL)
    { 
	if (addToList) _navList.push_back(this) ;
	_active = addToList ; 
    } ;

    ////////////////////////////////////////////////////////////////////////
    osg::Matrix Nav::getNormalizedPivotTransformation()
    {
	// pivot transformation matrix in normalized coordinates
	osg::Matrix npm ;
	
	osg::Node* pn = getPivotNode() ;
	
	// if pivot node is NULL either the DSO takes care of doing its
	// own pivot, or there isn't any pivot, but we don't mess with
	// it no matter which
	if (!pn) return npm;
	
	// do every frame as the node could be moved around
	osg::NodePathList npl = pn->getParentalNodePaths(SceneGraph::instance()->getSceneNode()) ;
	if (npl.size()==0)
	{
	    dtkMsg.add(DTKMSG_ERROR,"iris::Nav::getNormalizedPivotTransformation: no path from node to Scene node- setting pivot transformation to identity\n") ;
	    return npm ;
	}
	else if (npl.size()>1)
	{
	    dtkMsg.add(DTKMSG_ERROR,"iris::Nav::getNormalizedPivotTransformation: %d paths from node to Scene node- setting pivot transformation to identity\n") ;
	    return npm ;
	}

	// transformation from pivot node coordinates to nav coordinates
	npm = osg::computeLocalToWorld(npl[0]) ;
	npm.preMultTranslate(getPivotPoint()) ;

	// need to toss the scale
	// I can't find an easier way to do this with the OSG matix class
	osg::Vec3d t ;
	osg::Quat q ;
	iris::Decompose(npm, &t, &q) ;
	npm.makeRotate(q) ;
	npm.postMultTranslate(t) ;

	return npm ;
    }

    ////////////////////////////////////////////////////////////////////////
    osg::Vec3 Nav::getNormalizedPivotPoint()
    {
	osg::Matrix npm = getNormalizedPivotTransformation() ;
	return npm.getTrans() ;
    }

    ////////////////////////////////////////////////////////////////////////
    void Nav::pivotScale(const osg::Vec3d& scale)
    {
	// this will be the matrix with the Nav transformation
	osg::Matrix mat = getMatrix() ;

	// get the pivot point in normalized coordinates
	osg::Vec3 npp = Nav::getNormalizedPivotPoint() ;

	osg::Vec3d oldScale = Nav::getScale() ;
	osg::Vec3d newScale = osg::Vec3(scale.x()/oldScale.x(), scale.y()/oldScale.y(), scale.z()/oldScale.z()) ;
	mat.postMultTranslate(-npp) ;
	mat.postMultScale(newScale) ;
	mat.postMultTranslate(npp) ;
	_nav->setMatrix(mat) ;
	
	
    }
     
    ////////////////////////////////////////////////////////////////////////
    void  Nav::pivot(const osg::Quat& piv) 
    { 
	// this will be the matrix with the Nav transformation
	osg::Matrix mat = getMatrix() ;

	// get the pivot point in normalized coordinates
	osg::Vec3 npp = Nav::getNormalizedPivotPoint() ;
	
	mat.postMultTranslate(-npp) ;
	mat.postMultRotate(piv) ;
	mat.postMultTranslate(npp) ;
	_nav->setMatrix(mat) ;
	
    } ;

    ////////////////////////////////////////////////////////////////////////
    void Nav::setScale(const osg::Vec3d& scale) 
    { 
	/*

	  I did this in dgl/DSOs/dgl/shmNavControl.cpp to make scale be
	  visible in the desktop.  For some reason it doesn't seem to be
	  needed now, but it could be needed in the future when I fix the
	  pivot node bug

	  // change location so the scale change seems to be centered around the pivot point

	  // how far do we slide the location away from the pivot?
	  float slide = (scale/oldScale -1.f) ;

	  // delta between pivot and location
	  dtkCoord delta;
	  delta.x = pivot.x - location.x ;
	  delta.y = pivot.y - location.y ;
	  delta.z = pivot.z - location.z ;
	  
	  location.x -= delta.x*slide ;
	  location.y -= delta.y*slide ;
	  location.z -= delta.z*slide ;
	  
	  nav->location = location ;

	*/

#if 0
	osg::Vec3 oldScale =  _nav->getScale() ;
	osg::Vec3 location =  _nav->getPosition() ;

	osg::Vec3 slide ;
	slide.x() = scale.x()/oldScale.x() - 1.0 ;
	slide.y() = scale.y()/oldScale.y() - 1.0 ;
	slide.z() = scale.z()/oldScale.z() - 1.0 ;

	osg::Vec3 delta = _pivotPoint - location ;

	location.x() -= delta.x()*slide.x() ;
	location.y() -= delta.y()*slide.z() ;
	location.z() -= delta.z()*slide.z() ;
	_nav->setPosition(location) ;
	

#endif

	_nav->setScale(scale) ; 
    }

    ////////////////////////////////////////////////////////////////////////
    bool Nav::setPivotNode(osg::Node* node) 
    { 
	if (node == _pivotNode) return true ;

	if (!node)
	{
	    _pivotNode = node ;
	    return true ;
	}

	osg::NodePathList npl = node->getParentalNodePaths(SceneGraph::instance()->getSceneNode()) ;
	if (npl.size()==0)
	{
	    dtkMsg.add(DTKMSG_ERROR,"iris::Nav::setPivotNode: no path from node to Scene node- setting pivot node to NULL\n") ;
	    _pivotNode = NULL ;
	    return false ;
	}
	else if (npl.size()>1)
	{
	    dtkMsg.add(DTKMSG_ERROR,"iris::Nav::setPivotNode: %d paths from node to Scene node- setting pivot node to NULL\n",npl.size()) ;
	    _pivotNode = NULL ;
	    return false ;
	}
    	_pivotNode = node ; 

	return true ;
    } ;

    ////////////////////////////////////////////////////////////////////////
    bool Nav::control(const std::string& line)
    {
	return control(line, ParseString(line)) ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool Nav::control(const std::string& line, const std::vector<std::string>& vec)
    {
	// modify it to take [xyz [hpr [s] ] ] as reset location
	if (IsSubstring("reset",vec[0],3) && vec.size() == 1)
	{
	    reset() ;
	    return true ;
	}

	else if (IsSubstring("next",vec[0],3) && vec.size() == 1)
	{
	    next() ;
	    return true ;
	}

	else if (IsSubstring("use",vec[0],3) && vec.size()==2)
	{
	    use(vec[1]) ;
	    return true ;
	}

	else if (IsSubstring("position",vec[0],3) && vec.size()==4)
	{
	    double x, y, z ;
	    if (StringToDouble(vec[1],&x) && StringToDouble(vec[2],&y) && StringToDouble(vec[3],&z))
	    {
		dtkMsg.add(DTKMSG_INFO, "iris::Nav::control position set to %f %f %f\n",x,y,z) ;
		setPosition(osg::Vec3(x,y,z)) ;
		return true ;
	    }
	}

	else if (IsSubstring("orientation",vec[0],3) && vec.size()==4)
	{
	    double h, p, r ;
	    if (StringToDouble(vec[1],&h) && StringToDouble(vec[2],&p) && StringToDouble(vec[3],&r))
	    {
		dtkMsg.add(DTKMSG_INFO, "iris::Nav::control attitude set to Euler angle %f %f\n",h,p,r) ;
		setAttitude(EulerToQuat(h,p,r)) ;
		return true ;
	    }
	}

	else if (IsSubstring("attitude",vec[0],3) && vec.size()==5)
	{
	    double x, y, z, w ;
	    if (StringToDouble(vec[1],&x) && StringToDouble(vec[2],&y) && StringToDouble(vec[3],&z) && StringToDouble(vec[4],&w))
	    {
		dtkMsg.add(DTKMSG_INFO, "iris::Nav::control attitude set to quaternion %f %f %f %q\n",x,y,z,w) ;
		setAttitude(osg::Quat(x,y,z,w)) ;
		return true ;
	    }
	}

	else if (IsSubstring("scale",vec[0],3))
	{
	    double x, y, z ;
	    if (vec.size()==4 && StringToDouble(vec[1],&x) && StringToDouble(vec[2],&y) && StringToDouble(vec[3],&z))
	    {
		dtkMsg.add(DTKMSG_INFO, "iris::Nav::control scale set to %f %f %f\n",x,y,z) ;
		setScale(osg::Vec3(x,y,z)) ;
		return true ;
	    }
	    else if (vec.size()==2 && StringToDouble(vec[1],&x))
	    {
		dtkMsg.add(DTKMSG_INFO, "iris::Nav::control scale set to %f\n",x) ;
		setScale(osg::Vec3(x,x,x)) ;
		return true ;
	    }
	}

	else if (IsSubstring("matrix",vec[0],3) && vec.size()==17)
	{
	    double d[16] ;
	    for (int i=0; i<16; i++)
	    {
		if (!StringToDouble(vec[i+1],d+i)) return false ;
	    }
#if 0
	    dtkMsg.add(DTKMSG_INFO, "iris::Nav::control matrix set to: \n") ;
	    for(int i=0;i<4;i++) dtkMsg.add(DTKMSG_INFO, "iris::Nav:: %+8.8f  %+8.8f  %+8.8f  %+8.8f\n", d[i*4], d[i*4+1], d[i*4+2], d[i*4+3]) ; 
#else
	    dtkMsg.add(DTKMSG_INFO, "iris::Nav::control matrix set\n") ;
#endif
	    setMatrix(osg::Matrix(d)) ;
	    return true ;
	}

	else if (IsSubstring("pivot",vec[0]) && vec.size()==4)
	{
	    double h, p, r;
	    if (StringToDouble(vec[1],&h) && StringToDouble(vec[2],&p) && StringToDouble(vec[3],&r))
	    {
		dtkMsg.add(DTKMSG_INFO, "iris::Nav::control pivot set to orientation %f %f %f\n",h,p,r) ;
		pivot(EulerToQuat(h,p,r)) ;
		return true ;
	    }
	}

	else if (IsSubstring("pivot",vec[0]) && vec.size()==5)
	{
	    double x, y, z, w;
	    if (StringToDouble(vec[1],&x) && StringToDouble(vec[2],&y) && StringToDouble(vec[3],&z) && StringToDouble(vec[4],&w))
	    {
		dtkMsg.add(DTKMSG_INFO, "iris::Nav::control pivot set to attitude %f %f %f %f\n",x, y, z, w) ;
		pivot(osg::Quat(x,y,z,w)) ;
		return true ;
	    }
	}

	else if (IsSubstring("pivotscale",vec[0],6))
	{
	    double x, y, z ;
	    if (vec.size()==4 && StringToDouble(vec[1],&x) && StringToDouble(vec[2],&y) && StringToDouble(vec[3],&z))
	    {
		dtkMsg.add(DTKMSG_INFO, "iris::Nav::control pivot scale set to %f %f %f\n",x,y,z) ;
		pivotScale(osg::Vec3(x,y,z)) ;
		return true ;
	    }
	    else if (vec.size()==2 && StringToDouble(vec[1],&x))
	    {
		dtkMsg.add(DTKMSG_INFO, "iris::Nav::control pivot scale set to %f\n",x) ;
		pivotScale(osg::Vec3(x,x,x)) ;
		return true ;
	    }
	}

	else if (IsSubstring("pivotpoint",vec[0],6) && vec.size()==4)
	{
	    double x, y, z ;
	    if (StringToDouble(vec[1],&x) && StringToDouble(vec[2],&y) && StringToDouble(vec[3],&z))
	    {
		dtkMsg.add(DTKMSG_INFO, "iris::Nav::control pivot point set to %f %f %f\n",x,y,z) ;
		setPivotPoint(osg::Vec3(x,y,z)) ;
		return true ;
	    }
	}

	else if (IsSubstring("pivotnode",vec[0],6) && vec.size()==2)
	{
	    osg::Node* node = SceneGraph::instance()->findNode(vec[1]) ;
	    if (node)
	    {
		dtkMsg.add(DTKMSG_INFO, "iris::Nav::control pivot node set to %s\n",vec[1].c_str()) ;
		setPivotNode(node) ;
		return true ;
	    }
	    else
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::Nav::control node %s not found\n",vec[1].c_str()) ;
		return false ;
	    }
	}

	else if (IsSubstring("response",vec[0],3) && vec.size()==2)
	{
	    double s;
	    if (StringToDouble(vec[1],&s))
	    {
		dtkMsg.add(DTKMSG_INFO, "iris::Nav::control response set to %f\n",s) ;
		setResponse(s) ;
		return true ;
	    }
	}

	else if (IsSubstring("active",vec[0],3) && vec.size()==2)
	{
	    bool onOff ;
	    if (OnOff(vec[1],&onOff))
	    {
		if (_currentNav) _currentNav->_active = onOff ;
		if (onOff) dtkMsg.add(DTKMSG_INFO, "iris::Nav::control: current navigation active\n") ;
		else dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::control: current navigation inactive\n") ;
		return true ;
	    }
	    else return false ;
	    
	    double s;
	    if (StringToDouble(vec[1],&s))
	    {
		dtkMsg.add(DTKMSG_INFO, "iris::Nav::control response set to %f\n",s) ;
		setResponse(s) ;
		return true ;
	    }
	}

	return false ;
    } 

    ////////////////////////////////////////////////////////////////////////
    void Nav::reset()
    {
	dtkMsg.add(DTKMSG_NOTICE, "iris::Nav: navigation reset\n") ;
	setPosition(_resetPosition) ;
	setAttitude(_resetAttitude) ;
	setScale(_resetScale) ;
	setPivotPoint(_resetPivotPoint) ;
	setPivotNode(_resetPivotNode) ;
	setResponse(_resetResponse) ;
    }

    ////////////////////////////////////////////////////////////////////////
    void Nav::next()
    {
	dtkMsg.add(DTKMSG_NOTICE, "iris::Nav: navigation next\n") ;
	if (!_currentNav) 
	{
	    dtkMsg.add(DTKMSG_WARNING, "iris::Nav: no current navigation, ignoring \"next\" command.\n") ;
	    return ;
	}
	if (_navList.size()<2)
	{
	    dtkMsg.add(DTKMSG_WARNING, "iris::Nav: only one navigation, ignoring \"next\" command.\n") ;
	    return ;
	}

	const char* thisNav = _currentNav->getName() ;
	unsigned int i ;
	for (i=0; i<_navList.size(); i++)
	{
	    if (_navList[i] == _currentNav) break ;
	}
	if (i==_navList.size())
	{
	    dtkMsg.add(DTKMSG_WARNING, "iris::Nav: current navigation not found in list of navigations.\n") ;
	    return ;
	}
	_currentNav->setActive(false) ;
	unsigned int n = i+1 ;
	if (n==_navList.size()) n=0 ;
	_currentNav = _navList[n] ;

	// is this always a good thing to do?  should it just leave it set to whatever the DSO had it set to?
	_currentNav->setActive(true) ;
	dtkMsg.add(DTKMSG_NOTICE, "iris::Nav: switching from \"%s\" to \"%s\"\n",thisNav,_currentNav->getName()) ;
    
    }

    ////////////////////////////////////////////////////////////////////////
    void Nav::use(const std::string& navName)
    {
	dtkMsg.add(DTKMSG_NOTICE, "iris::Nav: navigation use %s\n",navName.c_str()) ;
	unsigned int i ;
	for (i=0; i<_navList.size(); i++)
	{
	    if (std::string(_navList[i]->getName()) == navName) break ;
	}
	if (i==_navList.size())
	{
	    // should I try to do a getOrLoad(navName)???
	    dtkMsg.add(DTKMSG_WARNING, "iris::Nav: navigation %s not found in list of navigations.\n",navName.c_str()) ;
	    return ;
	}
	if (_currentNav) _currentNav->setActive(false) ;
	_currentNav = _navList[i] ;
	_currentNav->setActive(true) ;
	dtkMsg.add(DTKMSG_NOTICE, "iris::Nav: switching to \"%s\"\n",_currentNav->getName()) ;
    }


    ////////////////////////////////////////////////////////////////////////
    MatrixTransform* Nav::_nav = NULL ;
    Nav* Nav::_currentNav = NULL ;
    std::vector<osg::ref_ptr<Nav> > Nav::_navList ;
    double Nav::_response = 1 ;
    osg::Vec3 Nav::_resetPosition = osg::Vec3(0.f, 0.f, 0.f) ;
    osg::Quat Nav::_resetAttitude =  osg::Quat(0.f,0.f,0.f,1.f) ;
    osg::Vec3 Nav::_resetScale = osg::Vec3(1.f, 1.f, 1.f) ;
    osg::Vec3 Nav::_resetPivotPoint = osg::Vec3(0.f, 0.f, 0.f) ;
    osg::Node* Nav::_resetPivotNode = NULL ;
    double Nav::_resetResponse = 1.f ;
    osg::Vec3 Nav::_pivotPoint = _resetPivotPoint ;
    osg::Node* Nav::_pivotNode = _resetPivotNode ;
}
