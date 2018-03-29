#ifndef __IRIS_CONVERT_NODE_VISITOR__
#define __IRIS_CONVERT_NODE_VISITOR__

#include <osg/Node>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/NodeVisitor>

namespace iris
{
    /**
       \brief The %ConvertNodeVisitor is a osg::NodeVisitor that modifies the nodes it
       visits to do any or all of the following:
       - set datavariance to static
       - remove the stateset
       - remove lighting
       - copy (deinstance) primitive data
       - make culling active or inactive
    */
    
    
    class ConvertNodeVisitor : public osg::NodeVisitor
    {
    public:
    ConvertNodeVisitor() : NodeVisitor(UPDATE_VISITOR,TRAVERSE_ALL_CHILDREN), _culling(true), _static(false), _noStateSets(false), _noLighting(false), _copyPrimitives(false) { ; } ;

	void apply(osg::Node& node) ;
	void apply(osg::Geode& geode) ;
	void apply(osg::Geometry* geom) ;
	void setStatic(bool b) { _static = b ; } ;
	void setNoStateSets(bool b) { _noStateSets = b ; } ;
	void setNoLighting(bool b) { _noLighting = b ; } ;
	void setCulling(bool b, bool on) { _culling = b ; _cullingValue = on ; } ;
	void setCopyPrimitives(bool b) { _copyPrimitives = b ; } ;
	bool getStatic() { return _static ; } ;
	bool getNoStateSets() { return _noStateSets ; } ;
	bool getNoLighting() { return _noLighting ; } ;
	bool getCopyPrimitives() { return _copyPrimitives ; } ;
    private:
	bool _culling ;
	bool _cullingValue ;
	bool _static ;
	bool _noStateSets ;
	bool _noLighting ;
	bool _copyPrimitives ;
    } ;

}
#endif
