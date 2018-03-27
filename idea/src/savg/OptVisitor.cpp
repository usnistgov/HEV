////////////////////////////////////////////////////////////////////////
// the class below was written by Paul Martz of Skew Matrix Software LLC.
// Thanks, Paul!
//
// hacked up and debugged a good bit by kelso@nist.gov in 2010
//

#include "OptVisitor.h"
#include <osg/Geode>
#include <osg/Geometry>

#include <osg/io_utils>
#include <iostream>
#include <set>


OptVisitor::OptVisitor()
    : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
      changeDLtoVBO_( true ),
      changeDynamicToStatic_( true ),
      changeDAtoDEUI_( true )
{
}
OptVisitor::~OptVisitor()
{
}

void
OptVisitor::apply( osg::Node& node )
{
    if( changeDynamicToStatic_ )
        node.setDataVariance( osg::Object::STATIC );
    traverse( node );
}

void
OptVisitor::apply( osg::Geode& geode )
{
    for(unsigned int i=0;i<geode.getNumDrawables();++i)
    {
        osg::Drawable* draw = geode.getDrawable(i);

	if( changeDLtoVBO_ )
	{
	    draw->setUseDisplayList( false );
	    draw->setUseVertexBufferObjects( true );
	}
	if( changeDynamicToStatic_ )
	{
	    draw->setDataVariance( osg::Object::STATIC );
	}

        osg::Geometry* geom = draw->asGeometry() ;

        if( ( geom != NULL ) && changeDAtoDEUI_ )
        {

            osg::ref_ptr< osg::DrawElementsUInt > tdeui = new osg::DrawElementsUInt( GL_TRIANGLES );
            osg::ref_ptr< osg::DrawElementsUInt > ldeui = new osg::DrawElementsUInt( GL_LINES );
            osg::ref_ptr< osg::DrawElementsUInt > pdeui = new osg::DrawElementsUInt( GL_POINTS );

            unsigned int numPS( geom->getNumPrimitiveSets() );
            while( numPS > 0 )
            {
                numPS--;
                osg::PrimitiveSet* ps( geom->getPrimitiveSet( numPS ) );
                if( ps->getType() == osg::PrimitiveSet::DrawArraysPrimitiveType )
                {
                    osg::DrawArrays* da = dynamic_cast< osg::DrawArrays* >( ps );
                    if( ps->getMode() == osg::PrimitiveSet::TRIANGLES )
                    {
                        processTriangles( *da, *tdeui );
                        geom->removePrimitiveSet( numPS );
                    }
                    else if( ps->getMode() == osg::PrimitiveSet::TRIANGLE_FAN )
                    {
                        processTriFan( *da, *tdeui );
                        geom->removePrimitiveSet( numPS );
                    }
                    else if( ps->getMode() == osg::PrimitiveSet::TRIANGLE_STRIP )
                    {
                        processTriStrip( *da, *tdeui );
                        geom->removePrimitiveSet( numPS );
                    }
                    else if( ps->getMode() == osg::PrimitiveSet::LINES )
                    {
                        processLines( *da, *ldeui );
                        geom->removePrimitiveSet( numPS );
                    }
                    else if( ps->getMode() == osg::PrimitiveSet::LINE_STRIP )
                    {
                        processLineStrip( *da, *ldeui );
                        geom->removePrimitiveSet( numPS );
                    }
                    else if( ps->getMode() == osg::PrimitiveSet::LINE_LOOP )
                    {
                        processLineLoop( *da, *ldeui );
                        geom->removePrimitiveSet( numPS );
                    }
                    else if( ps->getMode() == osg::PrimitiveSet::POINTS )
                    {
                        processLines( *da, *pdeui );
                        geom->removePrimitiveSet( numPS );
                    }
                }
            }

            // Create the new DEUIs.
            if( tdeui->size() > 0 )
            {
                geom->addPrimitiveSet( tdeui.get() );
            }
            if( ldeui->size() > 0 )
            {
                geom->addPrimitiveSet( ldeui.get() );
            }
            if( pdeui->size() > 0 )
            {
                geom->addPrimitiveSet( pdeui.get() );
            }
        }
    }
}

void
OptVisitor::processTriangles( const osg::DrawArrays& da, osg::DrawElementsUInt& indices )
{

    GLint first = da.getFirst();
    GLsizei count = da.getCount();

    unsigned int index( first );
    GLsizei processed( 0 );
    while( processed + 3 <= count )
    {
        indices.push_back( index++ );
        indices.push_back( index++ );
        indices.push_back( index++ );
        processed += 3;
    }
}

void
OptVisitor::processTriFan( const osg::DrawArrays& da, osg::DrawElementsUInt& indices )
{

    GLint first = da.getFirst();
    GLsizei count = da.getCount();

    unsigned int index( first+1 );
    GLsizei processed( 0 );
    while( processed + 3 <= count )
    {
        indices.push_back( first );
        indices.push_back( index++ );
        indices.push_back( index );
        processed++;
    }
}

void
OptVisitor::processTriStrip( const osg::DrawArrays& da, osg::DrawElementsUInt& indices )
{

    GLint first = da.getFirst();
    GLsizei count = da.getCount();

    unsigned int index( first );
    GLsizei processed( 0 );
    while( processed + 3 <= count )
    {
	if ( processed%2 == 0 )
	{
	    indices.push_back( index );
	    indices.push_back( index+1 );
	    indices.push_back( index+2 );
	}
	else
	{
	    indices.push_back( index );
	    indices.push_back( index+2 );
	    indices.push_back( index+1 );
	}
        index++;
        processed++;
    }
}

void
OptVisitor::processLines( const osg::DrawArrays& da, osg::DrawElementsUInt& indices )
{

    GLint first = da.getFirst();
    GLsizei count = da.getCount();

    unsigned int index( first );
    GLsizei processed( 0 );
    while( processed + 2 <= count )
    {
        indices.push_back( index++ );
        indices.push_back( index++ );
        processed += 2;
    }
}

void
OptVisitor::processLineStrip( const osg::DrawArrays& da, osg::DrawElementsUInt& indices )
{

    GLint first = da.getFirst();
    GLsizei count = da.getCount();

    unsigned int index( first );
    GLsizei processed( 0 );
    while( processed + 1 <= count )
    {
        indices.push_back( index );
        indices.push_back( index+1 );
        index++;
        processed++;
    }
}

void
OptVisitor::processLineLoop( const osg::DrawArrays& da, osg::DrawElementsUInt& indices )
{

    GLint first = da.getFirst();
    GLsizei count = da.getCount();

    unsigned int index( first );
    GLsizei processed( 0 );
    while( processed + 1 <= count )
    {
        indices.push_back( index );
        indices.push_back( index+1 );
        index++;
        processed++;
    }
    indices.push_back( count-1 ); 
    indices.push_back( first ); 
    processed++;
}

void
OptVisitor::processPoints( const osg::DrawArrays& da, osg::DrawElementsUInt& indices )
{

    GLint first = da.getFirst();
    GLsizei count = da.getCount();

    unsigned int index( first );
    GLsizei processed( 0 );
    while( processed + 1 <= count )
    {
        indices.push_back( index++ );
        processed++;
    }
}
