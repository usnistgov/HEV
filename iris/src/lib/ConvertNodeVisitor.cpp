#include <iris/ConvertNodeVisitor.h>

namespace iris
{
    ////////////////////////////////////////////////////////////////////////
    void ConvertNodeVisitor::apply(osg::Node& node)
    {
	if (_culling) node.setCullingActive(_cullingValue) ;
	if (_static) node.setDataVariance(osg::Object::STATIC) ;
	if (_noStateSets) node.setStateSet(NULL) ;
	if (!_noStateSets && _noLighting && node.getStateSet()) node.getStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF) ;
	traverse(node) ;
    }
    
    void ConvertNodeVisitor::apply(osg::Geode& geode)
    {
	if (_culling) geode.setCullingActive(_cullingValue) ;
	if (_static) geode.setDataVariance(osg::Object::STATIC) ;
	if (_noStateSets) geode.setStateSet(NULL) ;
	if (!_noStateSets && _noLighting && geode.getStateSet()) geode.getStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF) ;
	osg::Geode::DrawableList dl = geode.getDrawableList() ;
	for (unsigned int i=0; i<dl.size(); i++)
	{
	    if (_static) dl[i]->setDataVariance(osg::Object::STATIC) ;
	    if (_noStateSets) dl[i]->setStateSet(NULL) ;
	    if (!_noStateSets && _noLighting && dl[i]->getStateSet()) dl[i]->getStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF) ;	    
	    if (_copyPrimitives)
	    {
		osg::Geometry* geom = dl[i]->asGeometry() ;
		if (geom != NULL) apply(geom) ;
	    }
	}
    }

    ////////////////////////////////////////////////////////////////////////
    // the code below was based on code by Paul Martz of Skew Matrix Software LLC.
    // Thanks, Paul!
    void ConvertNodeVisitor::apply(osg::Geometry* geom)
    {
	geom->setDataVariance(osg::Object::STATIC );
	
	for( unsigned int idx = 0 ; idx < geom->getNumPrimitiveSets() ; idx++)
	{
	    osg::PrimitiveSet* ps = geom->getPrimitiveSet(idx );
	    if (ps->referenceCount() > 1)
	    {
		osg::PrimitiveSet* newPs = static_cast< osg::PrimitiveSet* >(ps->clone(osg::CopyOp::DEEP_COPY_ALL)) ;
		geom->setPrimitiveSet(idx, newPs) ;
	    }
	}
    }
}
