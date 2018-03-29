#ifndef __IRIS_PANE__
#define __IRIS_PANE__

#include <osg/Referenced>
#include <osg/Camera>

namespace iris
{
    class Window ;
    class ImmersivePane ;
    class PerspectivePane ;
    class OrthoPane ;

    /**

       \brief The %Pane class is a base class for the ImmersivePane, OrthoPane and
       PerspectivePane classes.  
       A %Pane object is attached to a Window object, and creates a viewport
       in the Window, and defines near and far clipping distances.  %Pane
       has a virtual method that defines a callback which calculates the
       object's view frustum.  Each %Pane object creates a osg::Camera
       object which is a slave camera to the master camera created in the
       SceneGraph class' osgViewer::Viewer object.

     */

    ////////////////////////////////////////////////////////////////////////
    // defines the data needed to calculate the frusta
    // an object of this type is passed as user data to the callback

    class Pane : public osg::Referenced
    {
    public:

    Pane(Window* window, const char* name, bool addToList = true) : _window(window), _realized(false), _active(true), _near(.1f), _far(1000.f), _viewportSet(false), _projectionResizePolicy(osg::Camera::HORIZONTAL), _oldAspectRatio(1.0)
	{ 
	    if (window && addToList) 
	    {
		if (name) _name = strdup(name) ;
		else _name = NULL ;
		_paneList.push_back(this) ;
	    }
	} ;

    Pane(bool addToList = true) : _window(NULL), _name(NULL), _active(true), _near(.1f), _far(1000.f), _viewportSet(false), _projectionResizePolicy(osg::Camera::HORIZONTAL), _oldAspectRatio(1.0)
	{ 
	    if (addToList) 
	    {
		_paneList.push_back(this) ;
	    }
	} ;

	// quicker than casting
	virtual ImmersivePane* asImmersive() { return NULL ; } ;
	virtual PerspectivePane* asPerspective() { return NULL ; } ;
	virtual OrthoPane* asOrtho() { return NULL ; } ;

	// anybody can mess with the list
	static std::vector<osg::ref_ptr<Pane> > const getPaneList() { return _paneList ; } ;

	virtual bool control(const std::string& line) ;
	virtual bool control(const std::string& line, const std::vector<std::string>&) ;

	float getNear() { return _near ; } ;
	void setNear(float near) { _near = near ; } ;

	float getFar() { return _far ; } ;
	void setFar(float far) { _far = far ; } ;

	// used to set the nodemask and start/stop the frustum callback computations
	bool getActive() ;
	void setActive(bool active) ;

	// why isn't this a std::string?
	static Pane* const findPane(const char* name) ;
	const char* getName() { return _name ; } ;
	void setName(const char* name) { _name = name ; } ;
    
	Window* const getWindow() { return _window ; } ;

	/**
	   \brief sets the viewport for the %Pane. 

	 */
	bool setViewport(int x, int y, int width, int height) ;

	/**
	   \brief returns the viewport for the %Pane in the supplied parameters. 
	   
	   \note If the Window containing the %Pane has been resized,
	   %getViewport() will return viewport values scaled to fit the new
	   window.

	 */
	void getViewport(int* x, int* y, int* width, int* height) ;

	// this should also set the camera resize policy again if after realized
	void setProjectionResizePolicy(osg::Camera::ProjectionResizePolicy prp) { _projectionResizePolicy = prp ; } ;
	osg::Camera::ProjectionResizePolicy getProjectionResizePolicy() { return _projectionResizePolicy ; } ;
	
	// a dummy call- each derived class should create a function that actually does something
	// don't make it pure virtual since Viewer needs an object of this, not the derived, class
	virtual void setProjectionAndViewMatrix(Pane* fs) ;

	std::vector<osg::ref_ptr<osg::Camera> > const getCameraList() { return _cameraList ; } ;

	virtual void realize() ;

    protected:
	static std::vector<osg::ref_ptr<Pane> > _paneList ;
	bool resizePane(osg::Camera* c) ;
	void resizeImmersivePane(osg::Camera* c) ;

    private:
	Window* _window ;
	const char* _name ;
	bool _realized ;
	bool _active ;
	std::vector<osg::ref_ptr<osg::Camera> > _cameraList ;
	float _near ;
	float _far ;
	int _x, _y, _width, _height ;
	bool _viewportSet ;
	osg::Camera::ProjectionResizePolicy _projectionResizePolicy ;
	double _oldAspectRatio ;
    } ;
}
#endif
