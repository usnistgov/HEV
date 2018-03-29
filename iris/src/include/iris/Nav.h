#ifndef __IRIS_NAV__
#define __IRIS_NAV__

#include <osg/Node>
#include <osg/Referenced>
#include <osg/Vec3>
#include <osg/Quat>
#include <osg/ref_ptr>

#include <iris/MatrixTransform.h>

namespace iris
{

    /**
       \brief The %Nav class has methods to manage the transformation in the scenegraph's
       \c nav node, and methods to switch between multiple %Nav objects.  
       \n \n In IRIS, navigation is implemented by changes to the \c nav node, as
       opposed to moving a osg::Camera through the scene.  This allows the
       IRIS scenegraph to easily support models that navigate (under the \c nav
       or \c world nodes) and models that don't (under the \c scene or \c ether nodes).
       \n \n Navigations are implemented in Augment DSOs that create and modify a
       %Nav object as appropriate the interface it is implementing.

       The "set" commands set the navigation node directly. These can be
       useful for setting the navigation to a specific value

       The pivot command apply relative motion commands to
       the navigation node's transformation.


    */

    class Nav: public osg::Referenced
    {
    public:
    
	Nav(const char* name, bool addToList = true) ;

	// this is for an object that just wants to mess with the class, like
	// the one in the SceneGraph class
	Nav(bool addToList = true) ;

	/** \brief set the translation components of the navigation node */
	static void setPosition(const osg::Vec3 pos) { _nav->setPosition(pos) ; }

	/** \brief set the rotation components of the navigation node. */
        static void setAttitude(const osg::Quat& quat) { _nav->setAttitude(quat) ; }

	/** \brief set the scale components of the navigation node */
        static void setScale(const osg::Vec3d& scale) ;
        
	/** \brief set the matrix of the navigation node */
	static void setMatrix(const osg::Matrix& matrix) { _nav->setMatrix(matrix) ; } ;
	
	/** \brief rotate around the pivot point in the pivot node by piv. This can change both rotation and translation. */
        static void pivot(const osg::Quat& piv) ;

	/** \brief scale the navigation, centered around the pivot point in the pivot node. This can change both scale and translation. */
        static void pivotScale(const osg::Vec3d& scale) ;
        
	/** get the values out of the navigation node */
        static osg::Vec3d getPosition() { return _nav->getPosition() ; }
        static osg::Quat getAttitude() { return _nav->getAttitude() ; }
        static osg::Vec3d getScale() { return _nav->getScale() ; } ;
	static osg::Matrix getMatrix() { return _nav->getMatrix() ; }

	/**
	   \brief sets the position in a node to pivot around when rotating

	   The default pivot node is iris::Scenegraph::getWorldNode().
	*/
	static void setPivotPoint(const osg::Vec3d& pivot) { _pivotPoint = pivot ; }
	static osg::Vec3d getPivotPoint() { return _pivotPoint ; }

	/**
	   \brief sets the node whose coordinate system the pivot point uses
	   if \param node is NULL, then navigation will do use a pivot point
	 */
	static bool setPivotNode(osg::Node* node) ;
	static osg::Node* getPivotNode() { return _pivotNode ; }

	// anybody can mess with the following
	static std::vector<osg::ref_ptr<Nav> > getNavList() { return _navList ; } ;
      
	// the current navigation DSO
	static Nav* const getCurrentNav() { return _currentNav ; } ;
      
	// get/set response
	static double getResponse() { return _response ; } ;
	static void setResponse(double response) { _response = response ; } ;
      
	// get/set active
	bool getActive() { return _active ; } ;
	void setActive(bool active) { _active = active ; } ;
      
	// reset navigation
	static void reset() ;

	// get pivot point in normalized coordinates, given settings of node and pivot point
	static osg::Vec3 getNormalizedPivotPoint() ;

	// get pivot transformation in normalized coordinates, given settings of node and pivot point
	static osg::Matrix getNormalizedPivotTransformation() ;

	// get/set reset position, attitude, scale
	static osg::Vec3 getResetPosition() { return _resetPosition ; } ;
	static void setResetPosition(osg::Vec3 resetPosition) { _resetPosition = resetPosition ; } ;
      
	static osg::Quat getResetAttitude() { return _resetAttitude ; } ;
	static void setResetAttitude(osg::Quat resetAttitude) { _resetAttitude = resetAttitude ; } ;
      
	static osg::Vec3 getResetScale() { return _resetScale ; } ;
	static void setResetScale(osg::Vec3 resetScale) { _resetScale = resetScale ; } ;
      
	static osg::Vec3 getResetPivotPoint() { return _resetPivotPoint ; } ;
	static void setResetPivotPoint(osg::Vec3 resetPivot) { _resetPivotPoint = resetPivot ; } ;
      
	static osg::Node* getResetPivotNode() { return _resetPivotNode ; } ;
	static void setResetPivotNode(osg::Node* const resetNode) { _resetPivotNode = resetNode ; } ;
      
	static double getResetResponse() { return _resetResponse ; } ;
	static void setResetResponse(double const resetResponse) { _resetResponse = resetResponse ; } ;
      
	// next navigation
	static void next() ;
      
	// use specific navigation
	static void use(const std::string& navName) ;
      
	virtual bool control(const std::string& line, const std::vector<std::string>& vec) ;
	virtual bool control(const std::string& line) ;
      
	// why isn't this a string?
	const char* getName() { return _name ; } ;
      
    protected:
#if 0
	virtual ~Nav() dtkMsg.add(DTKMSG_INFO,"iris::~Nav() called\n") ; } ;
#endif
 private:
    static MatrixTransform* _nav ;
    static Nav* _currentNav ;
    static std::vector<osg::ref_ptr<Nav> > _navList ;
    static double _response ;
    static osg::Vec3 _pivotPoint;
    static osg::Node* _pivotNode;
    static osg::Vec3 _resetPosition ;
    static osg::Quat _resetAttitude ;
    static osg::Vec3 _resetScale ;
    static osg::Vec3 _resetPivotPoint ;
    static osg::Node* _resetPivotNode ;
    static double _resetResponse ;
    const char* _name ;
    bool _active ;
} ;
}
#endif
