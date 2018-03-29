#include <string.h>
#include <iris/ImmersivePane.h>
#include <iris/SceneGraph.h>

namespace iris
{

    void ImmersivePane::setProjectionAndViewMatrix(Pane* np)
    {
	// here's how to get them if you need them
	//osg::ref_ptr<osg::GraphicsContext> gc = np->getGraphicsContext() ;
	//osg::ref_ptr<const osg::GraphicsContext::Traits> traits = gc->getTraits() ;

	osg::Vec3 eye = SceneGraph::instance()->getViewerPosition() ;

	ImmersivePane* nip = np->asImmersive() ;

	// take into account the eye offset and the orientation of the head tracker
	double offset = SceneGraph::instance()->getInterocular()/(2*SceneGraph::instance()->getMeters()) ;

	osg::Vec2 extent = nip->getExtent() ;
	osg::Quat orientation = nip->getOrientation() ;
	osg::Vec3 center = nip->getCenter() ;
	double near = nip->getNear() ;
	double far = nip->getFar() ;

	for (unsigned int i=0; i<np->getCameraList().size(); i++)
	{
	    osg::Camera* c = np->getCameraList()[i].get() ;

	    GLenum buffer = c->getDrawBuffer() ;
	    if (buffer == GL_BACK_LEFT || buffer == GL_FRONT_LEFT)
	    {
		eye -= SceneGraph::instance()->getViewerAttitude() * osg::Vec3(offset, 0.f, 0.f) ;
	    }
	    else if (buffer == GL_BACK_RIGHT || buffer == GL_FRONT_RIGHT)
	    {
		eye += SceneGraph::instance()->getViewerAttitude() * osg::Vec3(offset, 0.f, 0.f) ;
	    }
	    else if (buffer == GL_BACK || buffer == GL_FRONT)
	    {
		// nothing to do for mono
	    }
	    else
	    {
		dtkMsg.add(DTKMSG_ERROR,"iris::ImmersivePane::setImmersiveProjectionAndViewMatrix: unknown OpenGL buffer value\n") ;
		return ;
	    }
	
	    // p is the (0,1,0) vector rotated by the pane orientation, 
	    // so perpendicular to the pane
	    // shouldn't change
	    osg::Vec3 p = orientation * osg::Vec3(0.f,1.f,0.f) ;
	
	    // get position of pane relative to eye position
	    osg::Vec3 v = center - eye ;
	
	    // rotate the eye to face the pane
	    // puts it in pane coordinates?
	    // why inverse??  is this really the inverse rotation?
	    osg::Vec3 eyeTx = orientation.inverse() * eye;
	
	    //temp variables
	    //distance from eye to pane
	    double D = p*v ;
	    double s = near/D;
	
	    osg::Vec3 test = orientation.inverse() * center ;
	
	    double pane_t = test.z() + extent.y()/2.f ;
	    double pane_b = test.z() + extent.y()/-2.f ;
	    double pane_r = test.x() + extent.x()/2.f ;
	    double pane_l = test.x() + extent.x()/-2.f ;
	
	    double top = (pane_t - eyeTx.z()) * s;
	    double bottom = (pane_b - eyeTx.z()) * s;
	    double left = (pane_l - eyeTx.x()) * s;
	    double right = (pane_r - eyeTx.x()) * s;

	    if (dtkMsg.isSeverity(DTKMSG_DEBUG))
	    {
		static bool first = true ;
		if (first) 
		{
		    fprintf(stderr,"%s: before scaling, left = %f right = %f bottom = %f top = %f\n",getName(),left, right, bottom, top) ; 
		    first = false ;
		}
	    }
	    
	    // get generic frustum
	    c->setProjectionMatrixAsFrustum(left, right, bottom, top, near, far) ;

	    // scale it if the window is resized
	    resizeImmersivePane(c) ;

	    osg::Vec3 up = orientation*osg::Vec3(0.f,0.f,1.f) ;
	    c->setViewMatrixAsLookAt(eye, eye+p, up) ;
	}
    } ;

    ////////////////////////////////////////////////////////////////////////
    bool ImmersivePane::control(const std::string& line)
    {
	return control(line, ParseString(line)) ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool ImmersivePane::control(const std::string& line, const std::vector<std::string>& vec)
    {
	dtkMsg.add(DTKMSG_INFO, "iris::ImmersivePane::control passed: \"%s\"\n",line.c_str()) ;

	float x, y, z, w, h, p, r ;
	ImmersivePane* immersivePane ;

	// extent
	if (vec.size()==3 && IsSubstring("extent",vec[0],3) && StringToFloat(vec[1],&x) && StringToFloat(vec[2],&y))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::ImmersivePane::control extent set to %f %f for all immersive panes\n",x,y) ;
	    for (unsigned int i=0 ; i<_paneList.size() ; i++)
	    {
		immersivePane = _paneList[i]->asImmersive() ;
		if (immersivePane) immersivePane->setExtent(osg::Vec2(x,y)) ;	
	    }
	    return true ;
	}

	if (vec.size()==4 && (immersivePane = findImmersivePane(vec[0].c_str())) && IsSubstring("extent",vec[1],3) && StringToFloat(vec[2],&x) && StringToFloat(vec[3],&y))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::ImmersivePane::control extent set to %f %f for immersive pane named %s\n",x,y,vec[0].c_str()) ;
	    immersivePane->setExtent(osg::Vec2(x,y)) ;
	    return true ;
	}

	// center
	if (vec.size()==4 && IsSubstring("center",vec[0],3) && StringToFloat(vec[1],&x) && StringToFloat(vec[2],&y) && StringToFloat(vec[3],&z))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::ImmersivePane::control center set to %f %f %f for all immersive panes\n",x,y,z) ;
	    for (unsigned int i=0 ; i<_paneList.size() ; i++)
	    {
		immersivePane = _paneList[i]->asImmersive() ;
		if (immersivePane) immersivePane->setCenter(osg::Vec3(x,y,z)) ;	
	    }
	    return true ;
	}

	if (vec.size()==5 && (immersivePane = findImmersivePane(vec[0].c_str())) && IsSubstring("center",vec[1],3) && StringToFloat(vec[2],&x) && StringToFloat(vec[3],&y) && StringToFloat(vec[4],&z))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::ImmersivePane::control center set to %f %f %f for immersive pane named %s\n",x,y,z,vec[0].c_str()) ;
	    immersivePane->setCenter(osg::Vec3(x,y,z)) ;
	    return true ;
	}

	// orientation
	if (vec.size()==4 && IsSubstring("orientation",vec[0],3) && StringToFloat(vec[1],&h) && StringToFloat(vec[2],&p) && StringToFloat(vec[3],&r))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::ImmersivePane::control orientation set to Euler %f %f %f for all immersive panes\n",h,p,r) ;
	    for (unsigned int i=0 ; i<_paneList.size() ; i++)
	    {
		immersivePane = _paneList[i]->asImmersive() ;
		if (immersivePane) immersivePane->setOrientation(EulerToQuat(h,p,r)) ;	
	    }
	    return true ;
	}

	if (vec.size()==5 && (immersivePane = findImmersivePane(vec[0].c_str())) && IsSubstring("orientation",vec[1],3) && StringToFloat(vec[2],&h) && StringToFloat(vec[3],&p) && StringToFloat(vec[4],&r))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::ImmersivePane::control orientation set to Euler %f %f %f for immersive pane named %s\n",h,p,r,vec[0].c_str()) ;
	    immersivePane->setOrientation(EulerToQuat(h,p,r)) ;
	    return true ;
	}

	// attitude
	if (vec.size()==5 && IsSubstring("attitude",vec[0],3) && StringToFloat(vec[1],&x) && StringToFloat(vec[2],&y) && StringToFloat(vec[3],&z) && StringToFloat(vec[4],&w))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::ImmersivePane::control attitude set to quaternion %f %f %f % for all immersive panes\n",x,y,z,w) ;
	    for (unsigned int i=0 ; i<_paneList.size() ; i++)
	    {
		immersivePane = _paneList[i]->asImmersive() ;
		if (immersivePane) immersivePane->setOrientation(osg::Quat(x,y,z,w)) ;	
	    }
	    return true ;
	}

	if (vec.size()==6 && (immersivePane = findImmersivePane(vec[0].c_str())) && IsSubstring("attitude",vec[1],3) && StringToFloat(vec[2],&x) && StringToFloat(vec[3],&y) && StringToFloat(vec[4],&z) && StringToFloat(vec[5],&w))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::ImmersivePane::control attitude set to quaternion %f %f %f %f for immersive pane named %s\n",x,y,z,w,vec[0].c_str()) ;
	    immersivePane->setOrientation(osg::Quat(x,y,z,w)) ;
	    return true ;
	}

	// see if the Pane class has any control commands to process
	else return Pane::control(line,vec) ;

    }

    ////////////////////////////////////////////////////////////////////////
    ImmersivePane* const ImmersivePane::findImmersivePane(const char* name)
    {
	Pane* pane = Pane::findPane(name) ;
	ImmersivePane *immersivePane = NULL ;
	if (pane) immersivePane = pane->asImmersive() ;
	if (immersivePane) return immersivePane ;
	dtkMsg.add(DTKMSG_ERROR, "iris::ImmersivePane::getImmersivePane: no immersive pane named \"%s\" found.\n",name) ;
	return NULL ;
    }
}
