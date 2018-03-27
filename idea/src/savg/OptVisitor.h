////////////////////////////////////////////////////////////////////////
// the class below was written by Paul Martz of Skew Matrix Software LLC.
// Thanks, Paul!

#ifndef __OPT_VISITOR_H__
#define __OPT_VISITOR_H__


#include <osg/NodeVisitor>
#include <osg/Geometry>
#include <iostream>


class OptVisitor : public osg::NodeVisitor
{
public:
    OptVisitor();
    ~OptVisitor();

    META_NodeVisitor(osgBullet,OptVisitor)

    virtual void apply( osg::Node& node );
    virtual void apply( osg::Geode& geode );

    bool changeDLtoVBO_;
    bool changeDynamicToStatic_;
    bool changeDAtoDEUI_;

protected:
    void processTriangles( const osg::DrawArrays& da, osg::DrawElementsUInt& indices );
    void processTriFan( const osg::DrawArrays& da, osg::DrawElementsUInt& indices );
    void processTriStrip( const osg::DrawArrays& da, osg::DrawElementsUInt& indices );
    void processLines( const osg::DrawArrays& da, osg::DrawElementsUInt& indices );
    void processLineStrip( const osg::DrawArrays& da, osg::DrawElementsUInt& indices );
    void processLineLoop( const osg::DrawArrays& da, osg::DrawElementsUInt& indices );
    void processPoints( const osg::DrawArrays& da, osg::DrawElementsUInt& indices );

};


#endif
