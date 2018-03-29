#ifndef __IRIS_IMMERSIVE_PANE__
#define __IRIS_IMMERSIVE_PANE__

#include <iris/Pane.h>
#include <iris/Window.h>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Quat>

namespace iris
{

    /**
       \brief The %ImmersivePane class defines a Pane object and an
       asymmetric frustum callback that is designed for use in a
       head-tracked immersive environment.
       \n \n %ImmersivePane objectss are defined by their physical position in the
       virtual environment, and the frustum is calculated using this
       position and the head position.  
       \n \n Stereo is supported by calculating
       tho eye position based on the head position and orientation and
       interocular distance.
     */

    class ImmersivePane : public Pane
    {
    public:
	ImmersivePane(Window* window, const char* name = NULL) : Pane(window, name), _extent(osg::Vec2(2.f, 2.f)), _center(osg::Vec3(0.f, 1.f, 0.f)), _orientation(osg::Quat(0.f,0.f,0.f,1.f)) {} ;

	virtual ImmersivePane* asImmersive() { return this ; } ;

	// calculates the projection and view matrices for an immersive pane
	virtual void setProjectionAndViewMatrix(Pane* fs) ;

	virtual bool control(const std::string& line) ;
	virtual bool control(const std::string& line, const std::vector<std::string>&) ;

	// all data are in normalized units, NOT METERS
	// your display DSO should convert them

	// this is the data for an immersive pane, such as a RAVE wall.  think
	// of this as a window attached to the wall - as the user moves the
	// pane stays in place.  this data might also be useful for tiled
	// displays, but you'll need to write a new callback function to pass to
	// calculateProjectionAndViewMatrix()

	osg::Vec2 getExtent() { return _extent ; } ;
	void setExtent(osg::Vec2 extent) { _extent = extent ; } ;

	osg::Vec3 getCenter() { return _center ; } ;
	void setCenter(osg::Vec3 center) { _center = center ; } ;

	osg::Quat getOrientation() { return _orientation ; } ;
	void setOrientation(osg::Quat orientation) { _orientation = orientation ; } ;
	
	// why isn't this a std::string?
	static ImmersivePane* const findImmersivePane(const char* name) ;

    private:
	// width and height of pane in window in normalized units
	osg::Vec2 _extent ;
	// center of the pane relative to the origin
	osg::Vec3 _center ;
	// the quat that will orient the pane relative to the front pane,
	// for example, the left wall of the RAVE has a rotation of 90 around
	// the Z axis, so the quat is 0.000 0.000 0.707 0.707
	osg::Quat _orientation ;
    } ;
}
#endif
