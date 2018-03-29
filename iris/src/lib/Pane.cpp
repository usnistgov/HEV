#include <string.h>
#include <iris/Pane.h>
#include <iris/SceneGraph.h>
#include <iris/ImmersivePane.h>
#include <iris/OrthoPane.h>
#include <iris/PerspectivePane.h>

//#define IRIS_VERSION_1_3

namespace iris
{

    class PaneCB : public osg::NodeCallback
    {
    public:
	
	PaneCB(Pane* pane) : _pane(pane) 
	{ 
#ifdef IRIS_VERSION_1_3
	    _vp = new osg::Viewport(0,0,0,0) ;
#endif
	    _isg = SceneGraph::instance() ;
	} ;
	
	virtual void operator()(osg::Node* node,
				osg::NodeVisitor* nv)
	{

	    if (_pane==NULL) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::Pane: can't get Pane object pointer!\n") ;
		return ;
	    }

	    if (_pane->getActive())
	    {
	
		_isg->logLine("iris::Pane \"%s\" callback entered",_pane->getName()) ;
		double paneStart = osg::Timer::instance()->time_s();
		std::vector<osg::ref_ptr<osg::Camera> > cl = _pane->getCameraList() ;

		// we add this uniform for our shaders
		// OK to use just camera 0 as the setViewport uses the same value for both when stereo
		osg::Uniform *uniform = cl[0]->getOrCreateStateSet()->getOrCreateUniform( "iris_Viewport", osg::Uniform::FLOAT_VEC4) ;
		const osg::Viewport* vp = cl[0]->getViewport() ;
		uniform->set(osg::Vec4f(vp->x(), vp->y(), vp->width(), vp->height())) ;
		//fprintf(stderr,"iris_Viewport set to %f %f %f %f\n",vp->x(), vp->y(), vp->width(), vp->height()) ;
	    

#ifdef IRIS_VERSION_1_3
		if (*_vp != *vp)
		{
		    _vp->setViewport(vp->x(), vp->y(), vp->width(), vp->height()) ;
		    uniform->set(osg::Vec4f(vp->x(), vp->y(), vp->width(), vp->height())) ;
		    fprintf(stderr,"iris::Pane: osg::Camera viewport %f %f %f %f\n",_vp->x(), _vp->y(), _vp->width(), _vp->height()) ;	
		}
#endif

		ImmersivePane* is = _pane->asImmersive() ;
		PerspectivePane* ps = _pane->asPerspective() ;
		OrthoPane* os = _pane->asOrtho() ;
	    
		// choose the right callback
		if (is) is->setProjectionAndViewMatrix(_pane) ;
		else if (ps) ps->setProjectionAndViewMatrix(_pane) ;
		else if (os) os->setProjectionAndViewMatrix(_pane) ;
		else _pane->setProjectionAndViewMatrix(_pane) ;
		double paneStop = osg::Timer::instance()->time_s();
		_isg->logLine("iris::Pane \"%s\" callback finished, elapsed time %18.17f",_pane->getName(),paneStop-paneStart) ;
	    }

	    // Continue traversing so that OSG can process
	    //   any other nodes with callbacks.
	    traverse(node, nv);
	}
	
    private:
	Pane* _pane ;
	SceneGraph* _isg ;
#ifdef IRIS_VERSION_1_3
	osg::Viewport* _vp ;
#endif
    } ;

    ////////////////////////////////////////////////////////////////////////
    bool Pane::getActive() 
    { 
	return _active ; 
    } 


    void Pane::setActive(bool active) 
    { 
	_active = active ; 
#if 1
	unsigned int mask ;
	if (_active) mask = ~0 ;
	else mask = 0 ;
	for (unsigned int i=0; i<_cameraList.size(); i++)
	{
	    _cameraList[i]->setNodeMask(mask) ;
	}
#else
	// this didn't improve the frame rate
	for (unsigned int i=0; i<_cameraList.size(); i++)
	{
	    if (_active)
	    {
		_cameraList[i]->setNodeMask(~0) ;
		SceneGraph::instance()->getViewer()->addSlave(_cameraList[i]) ;
	    }
	    else
	    {
		_cameraList[i]->setNodeMask(0) ;
		unsigned int sn = SceneGraph::instance()->getViewer()->findSlaveIndexForCamera(_cameraList[i]) ;
		SceneGraph::instance()->getViewer()->removeSlave(sn) ;
	    }
	}
#endif
    } 


    ////////////////////////////////////////////////////////////////////////
    void Pane::setProjectionAndViewMatrix(Pane* ns) 
    {
	dtkMsg.add(DTKMSG_WARNING, "iris::Pane::setProjectionAndViewMatrix called\n") ;
    } ;

    ////////////////////////////////////////////////////////////////////////
    bool Pane::control(const std::string& line)
    {
	return control(line, ParseString(line)) ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool Pane::control(const std::string& line, const std::vector<std::string>& vec)
    {
	dtkMsg.add(DTKMSG_INFO, "iris::Pane::control passed: \"%s\"\n",line.c_str()) ;

	float v ;
	int x, y, width, height ;
	bool onOff ;
	Pane* pane ;

	// near
	if (vec.size()==2 && IsSubstring("near",vec[0],3) && StringToFloat(vec[1],&v))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::Pane::control near set to %f for all panes\n",v) ;
	    for (unsigned int i=0 ; i<_paneList.size() ; i++)
	    {
		(_paneList[i])->setNear(v) ;	
	    }
	    return true ;
	}

	if (vec.size()==3 && (pane = findPane(vec[0].c_str())) && IsSubstring("near",vec[1],3) && StringToFloat(vec[2],&v))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::Pane::control near set to %f for pane named %s\n",v,vec[0].c_str()) ;
	    pane->setNear(v) ;
	    return true ;
	}

	// far
	if (vec.size()==2 && IsSubstring("far",vec[0],3) && StringToFloat(vec[1],&v))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::Pane::control far set to %f for all panes\n",v) ;
	    for (unsigned int i=0 ; i<_paneList.size() ; i++)
	    {
		(_paneList[i])->setFar(v) ;	
	    }
	    return true ;
	}

	if (vec.size()==3 && (pane = findPane(vec[0].c_str())) && IsSubstring("far",vec[1],3) && StringToFloat(vec[2],&v))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::Pane::control far set to %f for pane named %s\n",v,vec[0].c_str()) ;
	    pane->setFar(v) ;
	    return true ;
	}

	// active
	if (vec.size()==2 && IsSubstring("active",vec[0],3) && OnOff(vec[1],&onOff))
	{
	    if (onOff) dtkMsg.add(DTKMSG_INFO, "iris::Pane::control active set to ON for all panes\n") ;
	    else dtkMsg.add(DTKMSG_INFO, "iris::Pane::control active set to OFF for all panes\n") ;
	    for (unsigned int i=0 ; i<_paneList.size() ; i++)
	    {
		(_paneList[i])->setActive(onOff) ;	
	    }
	    return true ;
	}

	if (vec.size()==3 && (pane = findPane(vec[0].c_str())) && IsSubstring("active",vec[1],3) && OnOff(vec[2],&onOff))
	{
	    if (onOff) dtkMsg.add(DTKMSG_INFO, "iris::Pane::control active set to ON for pane named %s\n",vec[0].c_str()) ;
	    else dtkMsg.add(DTKMSG_INFO, "iris::Pane::control active set to OFF for pane named %s\n",vec[0].c_str()) ;
	    pane->setActive(onOff) ;
	    return true ;
	}

#if 1
	// viewport
	if (vec.size()==5 && IsSubstring("viewport",vec[0],3) && StringToInt(vec[1],&x) && StringToInt(vec[2],&y)  && StringToInt(vec[3],&width) && StringToInt(vec[4],&height))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::Pane::control viewport set to %d %d, %d %d for all panes\n", x, y, width, height) ;
	    for (unsigned int i=0 ; i<_paneList.size() ; i++)
	    {
		(_paneList[i])->setViewport(x, y, width, height) ;	
	    }
	    return true ;
	}

	if (vec.size()==6 && (pane = findPane(vec[0].c_str())) && IsSubstring("viewport",vec[1],3) && StringToInt(vec[2],&x) && StringToInt(vec[3],&y) && StringToInt(vec[4],&width) && StringToInt(vec[5],&height))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::Pane::control viewport set to %d %d, %d %d for pane named %s\n", x, y, width, height, vec[0].c_str()) ;
	    pane->setViewport(x, y, width, height) ;
	    return true ;
	}
#endif

	return false ;
    }

    ////////////////////////////////////////////////////////////////////////
    // scale a pane if the viewport has been resized, but always compare it to the original viewport
    // the immersive pane doesn't need to reextract the immersive parameters from the matrix
    void Pane::resizeImmersivePane(osg::Camera* c)
    {
	// if you don't scale it the object's aspect ratio isn't correct if the window changes size

	// get viewport from camera
	const osg::Viewport* vp = c->getViewport() ;

	// this is the new ratio
	double immersiveAspectRatio = (double(vp->width())*_height) / (double(vp->height()*_width)) ;
	
	// scale by window resizing policy
	//fprintf(stderr,"%s:   immersiveAspectRatio = %f\n",getName(),immersiveAspectRatio) ;
	if (c->getProjectionResizePolicy() == osg::Camera::HORIZONTAL) c->getProjectionMatrix() *= osg::Matrix::scale(1.0/immersiveAspectRatio, 1.0, 1.0);
	else if (c->getProjectionResizePolicy() == osg::Camera::VERTICAL) c->getProjectionMatrix() *= osg::Matrix::scale(1.0, immersiveAspectRatio, 1.0); 
    }

    ////////////////////////////////////////////////////////////////////////
    bool Pane::resizePane(osg::Camera* c)
    {
	// if you don't scale it the object's aspect ratio isn't correct if the window changes size

	// get viewport from camera
	const osg::Viewport* vp = c->getViewport() ;

	// this is the new ratio
	double aspectRatio = double(vp->width())/double(vp->height()) ;

	// for testing without changing Pane.h
	//c->setProjectionResizePolicy(osg::Camera::HORIZONTAL) ; //default in header
	//c->setProjectionResizePolicy(osg::Camera::VERTICAL) ;
	//c->setProjectionResizePolicy(osg::Camera::FIXED) ;

	if (_oldAspectRatio != aspectRatio)
	{
	    double deltaAspectRatio = aspectRatio/_oldAspectRatio ;
	    //fprintf(stderr,"%s:   aspectRatio = %f\n",getName(),aspectRatio) ;
	    // scale by window resizing policy
	    if (c->getProjectionResizePolicy() == osg::Camera::HORIZONTAL) c->getProjectionMatrix() *= osg::Matrix::scale(1.0/deltaAspectRatio, 1.0, 1.0);
	    else if (c->getProjectionResizePolicy() == osg::Camera::VERTICAL) c->getProjectionMatrix() *= osg::Matrix::scale(1.0, deltaAspectRatio, 1.0); 
	    _oldAspectRatio = aspectRatio ;
	    return true ;
	}
	else return false ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool Pane::setViewport(int x, int y, int width, int height)
    {
	_x = x ; _y = y ;  _width = width ; _height = height ; _viewportSet = true ; 
	if (_realized) // set in camera
	{
	    for (unsigned int i=0; i<getCameraList().size(); i++)
	    {
		(getCameraList())[i]->setViewport(x, y, width, height) ;
	    }
	}
	return true ;
    } ;

    ////////////////////////////////////////////////////////////////////////
    void Pane::getViewport(int* x, int* y, int* width, int* height) 
    { 
	if (!_viewportSet)
	{
	    // use the default, the whole window
	    osg::ref_ptr<osg::GraphicsContext::Traits> traits = _window->getTraits() ;
	    *x = 0; *y = 0; *width = traits->width; *height = traits->height ;
	}
	else
	{
	    if (_realized) // get from the camera
	    {
		const osg::Viewport* vp = (getCameraList())[0]->getViewport() ;
		*x = int(vp->x()); *y = int(vp->y()) ; *width = int(vp->width()); *height = int(vp->height()) ;
	    }
	    else
	    {
		*x = _x; *y = _y ;  *width = _width ; *height = _height; 
	    }
	}
    } ;

    ////////////////////////////////////////////////////////////////////////
    void Pane::realize()
    {

	if (_realized) return ;

	osg::ref_ptr<osg::GraphicsContext::Traits> traits = _window->getTraits() ;

	if (!_viewportSet)
	{
	    setViewport(0, 0, traits->width, traits->height) ;
	}

	// sorta kludgey way to set up stereo screens
	// you define two Camera objects if doing stereo
	// if doing stereo you'd use GL_BACK_LEFT or GL_BACK_RIGHT if double buffer, or
	// GL_FRONT_LEFT or GL_FRONT_RIGHT if single buffer
	std::vector<GLenum> buffers ;
	osg::ref_ptr<osg::Camera> camera1 = new osg::Camera ;
	camera1->setClearColor(SceneGraph::instance()->getClearColor()) ;
	//camera1->setClearColor(osg::Vec4(1.,0.,0.,1.)) ;
	// this'll make the area outside of the viewport match the gc's clear color
	//camera1->setClearMask( GL_DEPTH_BUFFER_BIT );
	_cameraList.push_back(camera1.get()) ;
	if (traits->quadBufferStereo)
	{
	    osg::ref_ptr<osg::Camera> camera2 = new osg::Camera ;
	    camera2->setClearColor(SceneGraph::instance()->getClearColor()) ;
	    //camera2->setClearMask( GL_DEPTH_BUFFER_BIT );
	    _cameraList.push_back(camera2.get()) ;
	    if (traits->doubleBuffer)
	    {
		buffers.push_back(GL_BACK_LEFT) ;
		buffers.push_back(GL_BACK_RIGHT) ;
	    }
	    else
	    {
		buffers.push_back(GL_FRONT_LEFT) ; 
		buffers.push_back(GL_FRONT_RIGHT) ; 
	    }
	}
	else
	{
	    buffers.push_back(traits->doubleBuffer ? GL_BACK : GL_FRONT);
	}

	for (unsigned int b=0; b<_cameraList.size(); b++)
	{
	    _cameraList[b]->setViewport(_x, _y, _width, _height) ;
	    _cameraList[b]->setGraphicsContext(_window->getGraphicsContext()) ;
	    _cameraList[b]->setProjectionResizePolicy(_projectionResizePolicy) ;

#if 0
	    fprintf(stderr,"iris::Pane::realize, gc = %p\n", _window->getGraphicsContext()) ;
	    osg::Vec4 clearColor = _window->getGraphicsContext()->getClearColor() ;
	    fprintf(stderr,"iris::Pane::realize, gc->getClearColor = %f %f %f %f\n",clearColor.r(), clearColor.g(), clearColor.b(), clearColor.a()) ;
	    GLbitfield mask = _window->getGraphicsContext()->getClearMask() ;
	    fprintf(stderr,"iris::Pane::realize, gc->mask = %d\n",mask) ;
#endif
	
	    // each camera is independantly calculated
	    _cameraList[b]->setReferenceFrame(osg::Transform::ABSOLUTE_RF) ;
	    // this keeps the mouse data from getting messed up
	    _cameraList[b]->setAllowEventFocus(false) ;
	
	    _cameraList[b]->setDrawBuffer(buffers[b]);
	
	    // only need if you're reading back the buffer
	    _cameraList[b]->setReadBuffer(buffers[b]);

	    // if you don't do this the near and far will be ignored
	    // if using multiple cameras into the same viewport, near and far have to have the same value
	    _cameraList[b]->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR) ;

	    // add camera to the viewer
	    _cameraList[b]->setView(SceneGraph::instance()->getViewer()) ;
	    SceneGraph::instance()->getViewer()->addSlave(_cameraList[b]) ;

	    osg::ref_ptr<PaneCB> cb = new PaneCB(this) ;
	    SceneGraph::instance()->getSceneNode()->addUpdateCallback(cb) ;
	}

	_realized = true ;
    }

    ////////////////////////////////////////////////////////////////////////
    Pane* const Pane::findPane(const char* name)
    {
	for (unsigned int i=0 ; i<_paneList.size() ; i++)
	{
	    if (!strcmp(_paneList[i].get()->getName(),name)) return _paneList[i].get() ;
	}
	dtkMsg.add(DTKMSG_ERROR, "iris::Pane::getPane: no pane named \"%s\" found.\n",name) ;
	return NULL ;
    }

    std::vector<osg::ref_ptr<Pane> > Pane::_paneList ;
    bool _realized ;

}
