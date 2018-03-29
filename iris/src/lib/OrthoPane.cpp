#include <string.h>
#include <iris/OrthoPane.h>
#include <iris/SceneGraph.h>

//#define IRIS_VERSION_1_3

namespace iris
{

    // calculate ortho frustum, add code to do desktop stereo
    void OrthoPane::setProjectionAndViewMatrix(Pane* np) 
    { 
	osg::Vec3 position = SceneGraph::instance()->getViewerPosition() ;
	osg::Quat attitude = SceneGraph::instance()->getViewerAttitude() ;

	OrthoPane* nop = np->asOrtho() ;
	osg::Vec3 offset ;
    
	double left = nop->getLeft() ;
	double right = nop->getRight() ;
	double top = nop->getTop() ;
	double bottom = nop->getBottom() ;
	double near = nop->getNear() ;
	double far = nop->getFar() ;

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
		dtkMsg.add(DTKMSG_ERROR, "iris::OrthoPane::setImmersiveProjectionAndViewMatrix: unknown OpenGL buffer value\n") ;
		return ;
	    }
	
	    offset = osg::Vec3(iod, 0.f, 0.f) ;
	
	    // get viewport from camera
	    const osg::Viewport* v = c->getViewport() ;

	    if (dtkMsg.isSeverity(DTKMSG_DEBUG))
	    {
		static bool first = true ;
		if (first) 
		{
		    fprintf(stderr,"%s: before scaling, left = %f right = %f bottom = %f top = %f\n",getName(),left, right, bottom, top) ; 
		    first = false ;
		}
	    }
	    
	    // get generic ortho frustum
	    c->setProjectionMatrixAsOrtho(left, right, bottom, top, near, far) ;

	    // scale it if the window is resized and reset frustum parameters
	    if (resizePane(c))
	    {
		c->getProjectionMatrixAsOrtho(left, right, bottom, top, near, far) ;
	    	_left = left ; 
	    	_right = right ; 
	    	_bottom = bottom ; 
	    	_top = top ; 
		setNear(near) ; 
		setFar(far) ;
#ifdef IRIS_VERSION_1_3
		fprintf(stderr,"iris::OrthoPane: left = %f, right = %f, bottom = %f, top = %f, near = %f, far =%f\n", left, right, bottom, top, getNear(), getFar()) ;
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
	
	    if (iod != 0.f)
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
    bool OrthoPane::control(const std::string& line)
    {
	return control(line, ParseString(line)) ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool OrthoPane::control(const std::string& line, const std::vector<std::string>& vec)
    {
	dtkMsg.add(DTKMSG_INFO, "iris::OrthoPane::control passed: \"%s\"\n",line.c_str()) ;

	float v ;
	OrthoPane* orthoPane ;

	// left
	if (vec.size()==2 && IsSubstring("left",vec[0],3) && StringToFloat(vec[1],&v))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::OrthoPane::control left set to %f for all ortho panes\n",v) ;
	    for (unsigned int i=0 ; i<_paneList.size() ; i++)
	    {
		orthoPane = _paneList[i]->asOrtho() ;
		if (orthoPane) orthoPane->setLeft(v) ;	
	    }
	    return true ;
	}

	if (vec.size()==3 && (orthoPane = findOrthoPane(vec[0].c_str())) && IsSubstring("left",vec[1],3) && StringToFloat(vec[2],&v))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::OrthoPane::control left set to %f for ortho pane named %s\n",v,vec[0].c_str()) ;
	    orthoPane->setLeft(v) ;
	    return true ;
	}

	// right
	else if (vec.size()==2 && IsSubstring("right",vec[0],3) && StringToFloat(vec[1],&v))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::OrthoPane::control right set to %f for all ortho panes\n",v) ;
	    for (unsigned int i=0 ; i<_paneList.size() ; i++)
	    {
		orthoPane = _paneList[i]->asOrtho() ;
		if (orthoPane) orthoPane->setRight(v) ;	
	    }
	    return true ;
	}

	if (vec.size()==3 && (orthoPane = findOrthoPane(vec[0].c_str())) && IsSubstring("right",vec[1],3) && StringToFloat(vec[2],&v))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::OrthoPane::control right set to %f for ortho pane named %s\n",v,vec[0].c_str()) ;
	    orthoPane->setRight(v) ;
	    return true ;
	}

	// bottom
	else if (vec.size()==2 && IsSubstring("bottom",vec[0],3) && StringToFloat(vec[1],&v))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::OrthoPane::control bottom set to %f for all ortho panes\n",v) ;
	    for (unsigned int i=0 ; i<_paneList.size() ; i++)
	    {
		orthoPane = _paneList[i]->asOrtho() ;
		if (orthoPane) orthoPane->setBottom(v) ;	
	    }
	    return true ;
	}

	if (vec.size()==3 && (orthoPane = findOrthoPane(vec[0].c_str())) && IsSubstring("bottom",vec[1],3) && StringToFloat(vec[2],&v))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::OrthoPane::control bottom set to %f for ortho pane named %s\n",v,vec[0].c_str()) ;
	    orthoPane->setBottom(v) ;
	    return true ;
	}

	// top
	else if (vec.size()==2 && IsSubstring("top",vec[0],3) && StringToFloat(vec[1],&v))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::OrthoPane::control top set to %f for all ortho panes\n",v) ;
	    for (unsigned int i=0 ; i<_paneList.size() ; i++)
	    {
		orthoPane = _paneList[i]->asOrtho() ;
		if (orthoPane) orthoPane->setTop(v) ;	
	    }
	    return true ;
	}

	if (vec.size()==3 && (orthoPane = findOrthoPane(vec[0].c_str())) && IsSubstring("top",vec[1],3) && StringToFloat(vec[2],&v))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::OrthoPane::control top set to %f for ortho pane named %s\n",v,vec[0].c_str()) ;
	    orthoPane->setTop(v) ;
	    return true ;
	}


	// see if the Pane class has any control commands to process
	else return Pane::control(line,vec) ;

    }

    ////////////////////////////////////////////////////////////////////////
    OrthoPane* const OrthoPane::findOrthoPane(const char* name)
    {
	Pane* pane = Pane::findPane(name) ;
	OrthoPane *orthoPane = NULL ;
	if (pane) orthoPane = pane->asOrtho() ;
	if (orthoPane) return orthoPane ;
	dtkMsg.add(DTKMSG_ERROR, "iris::OrthoPane::getOrthoPane: no ortho pane named \"%s\" found.\n",name) ;
	return NULL ;
    }
}
