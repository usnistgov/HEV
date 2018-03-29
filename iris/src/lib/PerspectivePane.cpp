#include <string.h>
#include <iris/PerspectivePane.h>
#include <iris/SceneGraph.h>

//#define IRIS_VERSION_1_3

namespace iris
{

    // calculate perspective frustum, add code to do desktop stereo
    void PerspectivePane::setProjectionAndViewMatrix(Pane* np) 
    { 
	PerspectivePane* npp = np->asPerspective() ;
	osg::Vec3 offset ;
    
	double fov = npp->getFov() ;
	double aspect = npp->getAspect() ;
	double near = npp->getNear() ;
	double far = npp->getFar() ;

	osg::Vec3 position = SceneGraph::instance()->getViewerPosition() ;
	osg::Quat attitude = SceneGraph::instance()->getViewerAttitude() ;

	for (unsigned int i=0; i<np->getCameraList().size(); i++)
	{
	    // get eye offset if stereo, set iod to zero if mono
	    double iod = SceneGraph::instance()->getInterocular()/(2*SceneGraph::instance()->getMeters()) ;

	    osg::Camera* c = np->getCameraList()[i].get() ;

	    GLenum buffer =  c->getDrawBuffer() ;
	
	    if (buffer == GL_BACK_LEFT || buffer == GL_FRONT_LEFT)
	    {
		iod *= -1.f ;
	    }
	    else if (buffer == GL_BACK_RIGHT || buffer == GL_FRONT_RIGHT)
	    {
	    }
	    else if (buffer == GL_BACK || buffer == GL_FRONT)
	    {
		iod = 0.f ;
	    }
	    else
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::PerspectivePane::setImmersiveProjectionAndViewMatrix: unknown OpenGL buffer value\n") ;
		return ;
	    }
	    offset = osg::Vec3(iod, 0.f, 0.f) ;
	

	    // get viewport from camera
	    const osg::Viewport* v = c->getViewport() ;
	    //fprintf(stderr,"    camera viewport x, y, width, height = %f %f %f %f\n",v->x(), v->y(), v->width(), v->height()) ;
	    
	    // get generic perspective frustum
	    c->setProjectionMatrixAsPerspective(fov, aspect, near, far) ;
	    
	    // scale it if the window is resized and reset frustum paramweters
	    if (resizePane(c))
	    {
		double fovy, aspectRatio, zNear, zFar ;
		c->getProjectionMatrixAsPerspective (fov, aspect, near, far) ;
	    	_fov = fov ; 
		_aspect = aspect ; 
		setNear(near) ; 
		setFar(far) ;
#ifdef IRIS_VERSION_1_3
		fprintf(stderr,"iris::PerspectivePane: fov = %f, aspect = %f, near = %f, far =%f\n", fov, aspect, getNear(), getFar()) ;
#endif
	    }
	    
	    // pane distance
	    // wild guess- make it a meter()? make it a user setable method?
	    double sd = 1.f ;  
	
	    if (iod != 0.f)
	    {
		// get the projection matrix
		osg::Matrix pm =  c->getProjectionMatrix() ;
	    
		// shear it to get eye offset
	    
		// in osgUtil::SceneView.cpp look for "shjear" in computeRightEyeProjectionImplementation()
		//  uses "iod/(2.0*sd)" (iod = interocular, sd = pane distance) for shear value
		//    double iod = _displaySettings->getEyeSeparation();
		//    double sd = _displaySettings->getPaneDistance();
		//    return osg::Matrixd(1.0,           0.0,0.0,0.0,
		//                        0.0,           1.0,0.0,0.0,
		//                        -iod/(2.0*sd), 0.0,1.0,0.0,
		//                        0.0,           0.0,0.0,1.0) *
		//                        projection;
	    
		// set for testing, left eye- right eye is just the negative value of this shear
		//iod = SceneGraph::instance()->getInterocular()/(SceneGraph::instance()->getMeters()) ;
	    
		osg::Matrix spm(1.0,    0.0, 0.0, 0.0,
				0.0,    1.0, 0.0, 0.0,
				iod/sd, 0.0, 1.0, 0.0,
				0.0,    0.0, 0.0, 1.0) ;
	    
		pm.preMult(spm) ;
		//pm = spm * pm ;
		c->setProjectionMatrix(pm) ;
	    }
	
	    osg::Matrix mu = osg::Matrix::rotate(attitude) ;
	    osg::Matrix mc = mu * osg::Matrix::translate(position) ;
	
	    osg::Vec3 center = osg::Vec3(0,1,0)*mc ;
	    osg::Vec3 up = osg::Vec3(0,0,1)*mu ;
	    c->setViewMatrixAsLookAt(position, center, up) ;
	
	    if (iod != 0.0)
	    {
		// get the view matrix
		osg::Matrix vm =  c->getViewMatrix() ;
	    
		// shear it to get eye offset
	    
		// in osgUtil::SceneView.cpp, computeRightEyeViewImplementation(), they set the view matrix as
		//   double iod = _displaySettings->getEyeSeparation();
		//   double sd = _displaySettings->getPaneDistance();
		//   double es = 0.5*iod*(fusionDistance/(sd);
		//
		//   return view *
		//       osg::Matrixd(1.0,0.0,0.0,0.0,
		//                   0.0,1.0,0.0,0.0,
		//                   0.0,0.0,1.0,0.0,
		//                   -es,0.0,0.0,1.0);
	    
	    
		double es = iod*(SceneGraph::instance()->getFusion()/sd);
		osg::Matrix svm(1.0, 0.0, 0.0, 0.0,
				0.0, 1.0, 0.0, 0.0,
				0.0, 0.0, 1.0, 0.0,
				es,  0.0, 0.0, 1.0) ;
	    
		vm.preMult(svm) ;
		//vm = svm * vm ;
		c->setViewMatrix(vm) ;
	    }
	}

	return ; 
    } ;

    ////////////////////////////////////////////////////////////////////////
    bool PerspectivePane::control(const std::string& line)
    {
	return control(line, ParseString(line)) ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool PerspectivePane::control(const std::string& line, const std::vector<std::string>& vec)
    {
	dtkMsg.add(DTKMSG_INFO, "iris::PerspectivePane::control passed: \"%s\"\n",line.c_str()) ;

	float v ;
	PerspectivePane* perspectivePane ;

	// fov
	if (vec.size()==2 && IsSubstring("fov",vec[0],3) && StringToFloat(vec[1],&v))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::PerspectivePane::control fov set to %f for all perspective panes\n",v) ;
	    for (unsigned int i=0 ; i<_paneList.size() ; i++)
	    {
		perspectivePane = _paneList[i]->asPerspective() ;
		if (perspectivePane) perspectivePane->setFov(v) ;	
	    }
	    return true ;
	}

	if (vec.size()==3 && (perspectivePane = findPerspectivePane(vec[0].c_str())) && IsSubstring("fov",vec[1],3) && StringToFloat(vec[2],&v))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::PerspectivePane::control fov set to %f for perspective pane named %s\n",v,vec[0].c_str()) ;
	    perspectivePane->setFov(v) ;
	    return true ;
	}

	// aspect
	else if (vec.size()==2 && IsSubstring("aspect",vec[0],3) && StringToFloat(vec[1],&v))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::PerspectivePane::control aspect set to %f for all perspective panes\n",v) ;
	    for (unsigned int i=0 ; i<_paneList.size() ; i++)
	    {
		perspectivePane = _paneList[i]->asPerspective() ;
		if (perspectivePane) perspectivePane->setAspect(v) ;	
	    }
	    return true ;
	}

	if (vec.size()==3 && (perspectivePane = findPerspectivePane(vec[0].c_str())) && IsSubstring("aspect",vec[1],3) && StringToFloat(vec[2],&v))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::PerspectivePane::control aspect set to %f for perspective pane named %s\n",v,vec[0].c_str()) ;
	    perspectivePane->setAspect(v) ;
	    return true ;
	}

	// see if the Pane class has any control commands to process
	else return Pane::control(line,vec) ;

    }

    ////////////////////////////////////////////////////////////////////////
    PerspectivePane* const PerspectivePane::findPerspectivePane(const char* name)
    {
	Pane* pane = Pane::findPane(name) ;
	PerspectivePane *perspectivePane = NULL ;
	if (pane) perspectivePane = pane->asPerspective() ;
	if (perspectivePane) return perspectivePane ;
	dtkMsg.add(DTKMSG_ERROR, "iris::PerspectivPane::getPerspectivePane: no perspective pane named \"%s\" found.\n",name) ;
	return NULL ;
    }

}
