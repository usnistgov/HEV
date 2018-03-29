#ifndef __IRIS_CLIP_NODE
#define __IRIS_CLIP_NODE
#include <osg/ClipNode>

#define IRIS_CLIP_NODE_MAX_NUMBER 6
namespace iris
{	  
    /**

       \brief The %ClipNode class is subclassed from the osg::ClipNode
       class.  
       \n \n Each object of the %ClipNode class has a osg::ClipPlane attached
       to it, and checks are made to ensure that the maximum number of clipping
       planes is not exceeded.

    */

    class ClipNode : public osg::ClipNode
    {	    
    public: 
	ClipNode() ;
	virtual ~ClipNode();
	virtual void enable() ;
	virtual void disable() ;
	virtual bool isEnabled() { return _enabled ; } ; 
	static int maxNodes() { return IRIS_CLIP_NODE_MAX_NUMBER ; } ;
	bool isValid() { return _valid ; } ;
	bool isInvalid() { return !_valid ; } ;
	static std::vector<ClipNode*> getClipNodes() ;
    private:
	bool _enabled ;
	bool _valid ;
	int _num ;
 	static ClipNode* _clipNodes[IRIS_CLIP_NODE_MAX_NUMBER] ;
	static osg::Uniform* _clipEnableUniform ;
    } ;
}
#endif
