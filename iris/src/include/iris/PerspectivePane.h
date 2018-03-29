#ifndef __IRIS_PERSPECTIVE_PANE__
#define __IRIS_PERSPECTIVE_PANE__

#include <iris/Pane.h>
#include <iris/Window.h>

namespace iris
{

    /**
       \brief The %PerspectivePane class defines a Pane object and a symmetric
       perspective frustum callback that is designed for use on desktop systems.
       \n \n The frustum is calculated using the head position and orientation, and a
       field of view angle and aspect ratio.  
       \n \n Stereo is supported by using the
       interocular distance and a fusion distance.

     */

    class PerspectivePane : public Pane
    {
    public:
	PerspectivePane(Window* window, const char* name = NULL) : Pane(window, name), _fov(60), _aspect(1.f) {} ;
	
	virtual PerspectivePane* asPerspective() { return this ; } ;

	// calculates the projection and view matrices for an immersive pane
	virtual void setProjectionAndViewMatrix(Pane* fs) ;

	virtual bool control(const std::string& line) ;
	virtual bool control(const std::string& line, const std::vector<std::string>&) ;

	float getFov() { return _fov ; } ;
	void setFov(float fov) { _fov = fov ; } ;

	float getAspect() { return _aspect ; } ;
	void setAspect(float aspect) { _aspect = aspect ; } ;

	// why isn't this a std::string?
	static PerspectivePane* const findPerspectivePane(const char* name) ;

    private:
	float _fov ;
	float _aspect ;
    } ;
}
#endif
