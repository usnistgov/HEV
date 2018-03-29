#ifndef __IRIS_ORTHO_PANE__
#define __IRIS_ORTHO_PANE__

#include <iris/Pane.h>
#include <iris/Window.h>

namespace iris
{

    /**
       \brief The %OrthoPane class defines a Pane object and a symmetric
       orthographic frustum callback that is designed for use on desktop systems.
       \n \n The sides of an orthographic projection are parallel, so objects do
       not get smaller as they get further away.  %OrtoPane objects are
       handy when trying to see if objects align, as there is no perspective
       distortion.  
       \n \n The frustum is defined by the head position and
       orientation and the sides of the frustum

    */

    class OrthoPane : public Pane
    {
    public:
	OrthoPane(Window* window, const char* name = NULL) : Pane(window, name), _left(-1.f), _right(1.f), _bottom(-1.f), _top(1.f) {} ;
	
	virtual OrthoPane* asOrtho() { return this ; } ;

	// calculates the projection and view matrices for an immersive pane
	virtual void setProjectionAndViewMatrix(Pane* fs) ;

	virtual bool control(const std::string& line) ;
	virtual bool control(const std::string& line, const std::vector<std::string>&) ;

	float getLeft() { return _left ; } ;
	void setLeft(float left) { _left = left ; } ;

	float getRight() { return _right ; } ;
	void setRight(float right) { _right = right ; } ;

	float getBottom() { return _bottom ; } ;
	void setBottom(float bottom) { _bottom = bottom ; } ;

	float getTop() { return _top ; } ;
	void setTop(float top) { _top = top ; } ;

	// why isn't this a std::string?
	static OrthoPane* const findOrthoPane(const char* name) ;

    private:
	float _left ;
	float _right ;
	float _bottom ;
	float _top ;
    } ;
}
#endif
