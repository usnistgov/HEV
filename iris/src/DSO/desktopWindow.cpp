#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

#include <osgDB/WriteFile>

#include <iris/SceneGraph.h>
#include <iris/Augment.h>
#include <iris/Utils.h>
#include <iris/PerspectivePane.h>
#include <iris/ImmersivePane.h>
#include <iris/OrthoPane.h>

#define MULTIPLE_FRUSTA 1
// stinky global variable so we only take one snapshot
bool snap = true ;

// do this so this DSO's callbacks don't interfere with other DSO's
namespace desktopWindow
{
    ////////////////////////////////////////////////////////////////////////
    // lifted from the osgautocapture example
    class WindowCaptureCallback : public osg::Camera::DrawCallback
    {
    public:    
	WindowCaptureCallback(GLenum readBuffer, int x, int y, int width, int height, const std::string& name):
	    _readBuffer(readBuffer),
	    _fileName(name),
	    _x(x), _y(y), _width(width), _height(height)
        {
            _image = new osg::Image;
        }

	virtual void operator () (osg::RenderInfo& renderInfo) const
        {

	    if (!snap) return ;
	    snap = false ;

#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE)
            glReadBuffer(_readBuffer);
#else
	    dtkMsg.add(DTKMSG_ERROR, "iris::%s: GLES unable to do glReadBuffer\n") ;
	    return ;
#endif

            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
            osg::GraphicsContext* gc = renderInfo.getState()->getGraphicsContext();
            if (gc->getTraits())
            {
                GLenum pixelFormat;

                if (gc->getTraits()->alpha)
                    pixelFormat = GL_RGBA;
                else 
                    pixelFormat = GL_RGB;
                
#if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
		if (pixelFormat == GL_RGB)
		{
                    GLint value = 0;
#ifndef GL_IMPLEMENTATION_COLOR_READ_FORMAT
#define GL_IMPLEMENTATION_COLOR_READ_FORMAT 0x8B9B
#endif
                    glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &value);
                    if ( value != GL_RGB ||
                         value != GL_UNSIGNED_BYTE )
                    {
                        pixelFormat = GL_RGBA;//always supported
                    }
		}
#endif
		dtkMsg.add(DTKMSG_INFO, "iris::%s: image size: %d x %d, format: %s\n",getName().c_str(), _width, _height, (pixelFormat == GL_RGBA ? "GL_RGBA":"GL_RGB") ) ;
		osg::Timer_t beforeReadTick = osg::Timer::instance()->tick();
                _image->readPixels(_x, _y, _width, _height, pixelFormat, GL_UNSIGNED_BYTE);
		osg::Timer_t afterReadTick = osg::Timer::instance()->tick();
		dtkMsg.add(DTKMSG_INFO, "iris::%s: image read took %f seconds\n",getName().c_str(),osg::Timer::instance()->delta_s(beforeReadTick, afterReadTick)) ;
            }
                
            if (!_fileName.empty())
            {
		dtkMsg.add(DTKMSG_NOTICE, "iris::%s: writing image file %s\n",getName().c_str(),_fileName.c_str()) ;
		osg::Timer_t beforeWriteTick = osg::Timer::instance()->tick();
                osgDB::writeImageFile(*_image, _fileName);
		osg::Timer_t afterWriteTick = osg::Timer::instance()->tick();
		dtkMsg.add(DTKMSG_INFO, "iris::%s: image write took %f seconds\n",getName().c_str(),osg::Timer::instance()->delta_s(beforeWriteTick, afterWriteTick)) ;
            }
        }

    protected:    
	GLenum                      _readBuffer;
	std::string                 _fileName;
	osg::ref_ptr<osg::Image>    _image;
	mutable OpenThreads::Mutex  _mutex;
	int _x ;
	int _y ;
	int _width ;
	int _height ;
	
    };

    ////////////////////////////////////////////////////////////////////////
    class desktopWindow : public iris::Augment
    {
    public:
	desktopWindow():iris::Augment("desktopWindow") 
	{

	    setDescription("%s- a simple desktop window",getName()) ;

	    _window = new iris::Window("desktop") ;

	    // traits define the window
	    osg::GraphicsContext::Traits* const traits = _window->getTraits() ;
	    traits->x = 320 ;
	    traits->y = 320 ;
	    traits->width = 720 ;
	    traits->height = 720 ;
	    traits->windowDecoration = true;
	    traits->doubleBuffer = true;

#if 0
	    // use whatever DISPLAY is set to if ssh'ing in, otherwise use 0
	    // this is so in the RAVE and PowerWall the console doesn't pop
	    // up on the front wall
	    if (getenv("SSH_CONNECTION")) traits->readDISPLAY() ;
	    else traits->screenNum = 0 ;
#else
	    // use an osg displayName if specified in $IRIS_DISPLAY_NAME,
	    // otherwise use $DISPLAY
	    //fprintf(stderr, "displayName=%s\n",traits->displayName().c_str()) ;
	    if (getenv("IRIS_DISPLAY_NAME")) traits->setScreenIdentifier(getenv("IRIS_DISPLAY_NAME")) ;
	    else traits->readDISPLAY() ;
#endif

#ifdef MULTIPLE_FRUSTA 
	    _orthoPane = new iris::OrthoPane(_window.get(),"ortho") ;
	    _window->addPane(_orthoPane.get()) ;
	    //_orthoPane->setViewport(100, 100, traits->width - 200, traits->height - 200) ;
	    // make (right-left)/(top-bottom) = viewport width/height
	    _orthoPane->setLeft(-1.f) ;
	    _orthoPane->setRight(1.f) ;
	    _orthoPane->setBottom(-1.f) ;
	    _orthoPane->setTop(1.f) ;
	    _orthoPane->setNear(.1f) ;
	    _orthoPane->setFar(1000.f) ;
	    //_orthoPane->setProjectionResizePolicy(osg::Camera::HORIZONTAL) ;
	    //_orthoPane->setProjectionResizePolicy(osg::Camera::VERTICAL) ;
	    //_orthoPane->setProjectionResizePolicy(osg::Camera::FIXED) ;
	    

	    _immersivePane = new iris::ImmersivePane(_window.get(),"immersive") ;
	    _window->addPane(_immersivePane.get()) ;
	    //_immersivePane->setViewport(100, 100, traits->width - 200, traits->height - 200) ;
	    // extent is in dgl coordinates relative to a physical cube
	    // make xextent/yextent = viewport width/height
	    float xextent = 2.f ;
	    float yextent = 2.f ;
	    _immersivePane->setExtent(osg::Vec2(xextent, yextent)) ;
	    _immersivePane->setCenter(osg::Vec3(0.f, 1.f, 0.f)) ;
	    _immersivePane->setOrientation(iris::EulerToQuat(0.f, 0.f, 0.f)) ;
	    _immersivePane->setNear(.1f) ;
	    _immersivePane->setFar(1000.f) ;
	    //_immersivePane->setProjectionResizePolicy(osg::Camera::HORIZONTAL) ;
	    //_immersivePane->setProjectionResizePolicy(osg::Camera::VERTICAL) ;
	    //_immersivePane->setProjectionResizePolicy(osg::Camera::FIXED) ;
#endif
	    _perspectivePane = new iris::PerspectivePane(_window.get(),"perspective") ;
	    _window->addPane(_perspectivePane.get()) ;
	    //_perspectivePane->setViewport(100, 100, traits->width - 200, traits->height - 200) ;
	    // set this to 90 and the edges of frustum should meet the dgl cube
	    _perspectivePane->setFov(60.f) ;
	    // make aspect the same as the viewport width/height
	    _perspectivePane->setAspect(1.f) ;
	    _perspectivePane->setNear(.1f) ;
	    _perspectivePane->setFar(1000.f) ;
	    //_perspectivePane->setProjectionResizePolicy(osg::Camera::HORIZONTAL) ;
	    //_perspectivePane->setProjectionResizePolicy(osg::Camera::VERTICAL) ;
	    //_perspectivePane->setProjectionResizePolicy(osg::Camera::FIXED) ;
#ifdef MULTIPLE_FRUSTA 
	    // initially only use perspective
	    _immersivePane->setActive(false) ;
	    _orthoPane->setActive(false) ;
#endif

	    // this might be the biggest kludge I've ever written

	    // for some reason if you don't do this, then the frame rate is
	    // reduced until you cycle through all the panes. All this does
	    // is set the nodemask for each camera in each pane. It needs to
	    // be called "after realize" because realize() creates the camera nodes.
	    iris::SceneGraph::instance()->control("AFTER REALIZE DSO desktopWindow cycle") ;

	    // dtkAugment::dtkAugment() will not validate the object
	    validate() ;
	} ;

	bool control(const std::string& line, const std::vector<std::string>& vec)
	{
	    if (vec.size() == 2 && iris::IsSubstring("snap",vec[0],3)) 
	    {
		// lifted from osgautocapture example
		// Add the WindowCaptureCallback
		GLenum buffer = _window->getTraits()->doubleBuffer ? GL_BACK : GL_FRONT ;
		// NEED TO ADD CHECKS TO SEE IF ITS ACTIVE AND IF IT HAS A CAMERA
		int x ;
		int y ;
		int width ;
		int height ;
		_perspectivePane->getViewport(&x, &y, &width, &height) ;
		(_perspectivePane->getCameraList())[0]->setFinalDrawCallback(new WindowCaptureCallback(buffer, x, y, width, height, vec[1]));
		snap = true;
		return true ;
	    }
#ifdef MULTIPLE_FRUSTA 
	    else if (iris::IsSubstring("perspective",vec[0],3))
	    {
		dtkMsg.add(DTKMSG_NOTICE, "iris::%s::control switching to perspective window\n",getName()) ;
		_perspectivePane->setActive(true) ;
		_orthoPane->setActive(false) ;
		_immersivePane->setActive(false) ;
		return true ;
	    }
	    else if (iris::IsSubstring("immersive",vec[0],3)) 
	    {
		dtkMsg.add(DTKMSG_NOTICE, "iris::%s::control switching to immersive window\n",getName()) ;
		_immersivePane->setActive(true) ;
		_perspectivePane->setActive(false) ;
		_orthoPane->setActive(false) ;
		return true ;
	    }
	    else if (iris::IsSubstring("ortho",vec[0],3)) 
	    {
		dtkMsg.add(DTKMSG_NOTICE, "iris::%s::control switching to ortho window\n",getName()) ;
		_orthoPane->setActive(true) ;
		_perspectivePane->setActive(false) ;
		_immersivePane->setActive(false) ;
		return true ;
	    }
	    else if (iris::IsSubstring("cycle",vec[0],3)) 
	    {
		dtkMsg.add(DTKMSG_INFO, "iris::%s::control cycling windows\n",getName()) ;
#if 0		
		// why doesn't this do the same thing as calling iris::Pane::setActive()?
		std::vector< osg::ref_ptr <osg::Camera > > cl ;
		iris::Pane* panes[3] = { _perspectivePane, _orthoPane, _immersivePane } ;
		for (int p = 0; p<3; p++)
		{
		    cl = panes[p]->getCameraList() ;
		    fprintf(stderr,"pane %s: cl.size() = %d\n",panes[p]->getName(), cl.size()) ;
		    for (unsigned int i=0; i<cl.size(); i++)
		    {
			osg::Node::NodeMask m = cl[i]->getNodeMask() ;
			fprintf(stderr,"pane %s: nodemask is %d\n", panes[p]->getName(),m) ;	
			cl[i]->setNodeMask(~m) ;
			cl[i]->setNodeMask(m) ;
		    }
		}
#else
		_perspectivePane->setActive(true) ;
		_orthoPane->setActive(false) ;
		_immersivePane->setActive(false) ;
#endif
		return true ;
	    }
#endif
	    else
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::%s::control called with unknown command, %s\n",getName(),line.c_str()) ;
		// if the DSO defines a control method you have to call the base class's control method too
		return iris::Augment::control(line, vec) ;
	    }
	}

    private:
#ifdef MULTIPLE_FRUSTA 
	osg::ref_ptr<iris::OrthoPane> _orthoPane ;
	osg::ref_ptr<iris::ImmersivePane> _immersivePane ;
#endif
	osg::ref_ptr<iris::PerspectivePane> _perspectivePane ;
	osg::ref_ptr<iris::Window> _window ;
    } ;

} ;

/************ DTK C++ dtkAugment loader/unloader functions ***************
 *
 * All DTK dtkAugment DSO files are required to declare these two
 * functions.  These function are called by the loading program to get
 * your C++ objects loaded.
 *
 *************************************************************************/

static dtkAugment* dtkDSO_loader(dtkManager* manager, void* p)
{
    return new desktopWindow::desktopWindow ;
}

static int dtkDSO_unloader(dtkAugment* augment)
{
#if 0
    delete augment;
#endif
    return DTKDSO_UNLOAD_CONTINUE;
}

