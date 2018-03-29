#ifndef __IRIS_LIGHT_NODE
#define __IRIS_LIGHT_NODE
#include <osg/LightSource>

#define IRIS_LIGHT_NODE_MAX_NUMBER 8
namespace iris
{	  
    /**

       \brief The %LightNode class is subclassed from the osg::LightSource
       class.  
       \n \n Each object of the %LightNode class has a osg::Light attached
       to it, and checks are made to ensure that the maximum number of lights
       is not exceeded.

       By default the light shines in the +Y direction
    */

    class LightNode : public osg::LightSource
    {	    
    public: 
	LightNode() ;
	virtual ~LightNode();
	virtual void enable() ;
	virtual void disable() ;
	virtual bool isEnabled() { return _enabled ; } ; 
	static int maxNodes() { return IRIS_LIGHT_NODE_MAX_NUMBER ; } ;
	bool isValid() { return _valid ; } ;
	bool isInvalid() { return !_valid ; } ;
	static std::vector<LightNode*> getLightNodes() ;
    private:
	bool _enabled ;
	bool _valid ;
	int _num ;
 	static LightNode* _lightNodes[IRIS_LIGHT_NODE_MAX_NUMBER] ;
	static osg::Uniform* _lightEnableUniform ;
    } ;
}
#endif
