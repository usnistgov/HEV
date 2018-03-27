#include <strings.h>

#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/TexGen>
#include <osg/TexMat>
#include <osg/Texture2D>
#include <osg/Vec3>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgUtil/Optimizer>
#include <osg/BlendFunc>
#include <osg/AlphaFunc>

char *textureFile ;
char *outFile ;
float tmin[2] = { 0, 0 } ;
float tmax[2] = { 1, 1 } ;
float min[2] = { -1, -1 } ;
float max[2] = { 1, 1 } ;
bool normals = true ;

class Square : public osg::Geode
{
    public:
    Square()
    {
	////////////////////////  Make Geometry ///////////////////////////////
	// vertices of the square
	
	osg::Vec3Array *coords = new osg::Vec3Array;
	osg::Vec2Array *texcoords = new osg::Vec2Array;
	osg::Vec4Array *color   = new osg::Vec4Array;
	
	coords->push_back( osg::Vec3(min[0], 0, min[1])) ;
	coords->push_back( osg::Vec3(max[0], 0, min[1])) ;
	coords->push_back( osg::Vec3(max[0], 0, max[1])) ;
	coords->push_back( osg::Vec3(min[0], 0, max[1])) ;
	
	texcoords->push_back( osg::Vec2(tmin[0], tmin[1])) ;
	texcoords->push_back( osg::Vec2(tmax[0], tmin[1])) ;
	texcoords->push_back( osg::Vec2(tmax[0], tmax[1])) ;
	texcoords->push_back( osg::Vec2(tmin[0], tmax[1])) ;
	
	color->push_back( osg::Vec4(1,1,1,1));
	
	osg::Geometry *geometry = new osg::Geometry;
	geometry->setVertexArray(coords);
	if (normals)
	{
	    osg::Vec3Array *normals = new osg::Vec3Array;
	    normals->push_back( osg::Vec3(0.f, -1.f, 0.f)) ;
	    normals->push_back( osg::Vec3(0.f, -1.f, 0.f)) ;
	    normals->push_back( osg::Vec3(0.f, -1.f, 0.f)) ;
	    normals->push_back( osg::Vec3(0.f, -1.f, 0.f)) ;
	    geometry->setNormalArray(normals);
	}
	geometry->setTexCoordArray( 0, texcoords );
	geometry->setColorArray(color);
	geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
	geometry->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, coords->size()));
	
	addDrawable( geometry );
	
	///////////////////// Make and Popluate State Set ////////////////////////////
	osg::StateSet *sset = new osg::StateSet;
	
	geometry->setStateSet( sset );
	
	// Add a texture
	osg::Texture2D *tex = new osg::Texture2D;
	tex->setImage( osgDB::readImageFile( textureFile ));
	tex->setWrap(osg::Texture2D::WRAP_S, osg::Texture::REPEAT);
	tex->setWrap(osg::Texture2D::WRAP_T, osg::Texture::REPEAT);
	sset->setTextureAttributeAndModes( 0, tex, osg::StateAttribute::ON );
	
	osg::BlendFunc *transp = new osg::BlendFunc;
	transp->setDestination( osg::BlendFunc::ONE_MINUS_SRC_ALPHA );
	sset->setAttributeAndModes( transp, osg::StateAttribute::ON );
	
	osg::AlphaFunc *alphafunc = new osg::AlphaFunc;
	alphafunc->setFunction(osg::AlphaFunc::GREATER,0.0f);
	sset->setAttributeAndModes(alphafunc, osg::StateAttribute::ON );


    }
};

void usage()
{
    fprintf(stderr,"Usage: hev-texturedRectangle [--nonormals] [--tmin x z] [--tmax x z] [--min x z] [--max x z] texFile outFile\n") ;
}

int main(int argc, char **argv)
{
 
    if (argc<3)
    {
	usage() ;
	return 1 ;
    }

    int i = 1 ;
    while (i<argc-2)
    {
	if (!strcasecmp("--nonormals", argv[i]))
	{
	    normals = false ;
	}

	else if (!strcasecmp("--tmin", argv[i]))
	{
	    i++ ;
	    if (i<argc)
	    {
		if (sscanf(argv[i],"%f",tmin) != 1)
		{
		    fprintf(stderr, "hev-texturedRectangle invalid tmin x\n") ;
		    return 1 ;
		}
	    }
	    else
	    {
		fprintf(stderr, "hev-texturedRectangle missing tmin x\n") ;
		return 1 ;
	    }
	    i++ ;
	    if (i<argc)
	    {
		if (sscanf(argv[i],"%f",tmin+1) != 1)
		{
		    fprintf(stderr, "hev-texturedRectangle invalid tmin z\n") ;
		    return 1 ;
		}
	    }
	    else
	    {
		fprintf(stderr, "hev-texturedRectangle missing tmin z\n") ;
		return 1 ;
	    }
	}

	else if (!strcasecmp("--tmax", argv[i]))
	{
	    i++ ;
	    if (i<argc)
	    {
		if (sscanf(argv[i],"%f",tmax) != 1)
		{
		    fprintf(stderr, "hev-texturedRectangle invalid tmax x\n") ;
		    return 1 ;
		}
	    }
	    else
	    {
		fprintf(stderr, "hev-texturedRectangle missing tmax x\n") ;
		return 1 ;
	    }
	    i++ ;
	    if (i<argc)
	    {
		if (sscanf(argv[i],"%f",tmax+1) != 1)
		{
		    fprintf(stderr, "hev-texturedRectangle invalid tmax z\n") ;
		    return 1 ;
		}
	    }
	    else
	    {
		fprintf(stderr, "hev-texturedRectangle missing tmax z\n") ;
		return 1 ;
	    }
	}

	else if (!strcasecmp("--min", argv[i]))
	{
	    i++ ;
	    if (i<argc)
	    {
		if (sscanf(argv[i],"%f",min) != 1)
		{
		    fprintf(stderr, "hev-texturedRectangle invalid min x\n") ;
		    return 1 ;
		}
	    }
	    else
	    {
		fprintf(stderr, "hev-texturedRectangle missing min x\n") ;
		return 1 ;
	    }
	    i++ ;
	    if (i<argc)
	    {
		if (sscanf(argv[i],"%f",min+1) != 1)
		{
		    fprintf(stderr, "hev-texturedRectangle invalid min z\n") ;
		    return 1 ;
		}
	    }
	    else
	    {
		fprintf(stderr, "hev-texturedRectangle missing min z\n") ;
		return 1 ;
	    }
	}

	else if (!strcasecmp("--max", argv[i]))
	{
	    i++ ;
	    if (i<argc)
	    {
		if (sscanf(argv[i],"%f",max) != 1)
		{
		    fprintf(stderr, "hev-texturedRectangle invalid max x\n") ;
		    return 1 ;
		}
	    }
	    else
	    {
		fprintf(stderr, "hev-texturedRectangle missing max x\n") ;
		return 1 ;
	    }
	    i++ ;
	    if (i<argc)
	    {
		if (sscanf(argv[i],"%f",max+1) != 1)
		{
		    fprintf(stderr, "hev-texturedRectangle invalid max z\n") ;
		    return 1 ;
		}
	    }
	    else
	    {
		fprintf(stderr, "hev-texturedRectangle missing max z\n") ;
		return 1 ;
	    }
	}
	i++ ;
    }

    //printf("min = %f %f\n",min[0], min[1]) ;
    //printf("max = %f %f\n",max[0], max[1]) ;

    if (i<argc) 
    {
	textureFile = argv[i] ;
	i++ ;
    }
    else
    {
	fprintf(stderr, "hev-texturedRectangle missing texture file name\n") ;
	return 1 ;
    }

    //printf("textureFile = %s\n",textureFile) ;

    if (i<argc) 
    {
	outFile = argv[i] ;
    }
    else
    {
	fprintf(stderr, "hev-texturedRectangle missing output file name\n") ;
	return 1 ;
    }

    //printf("outFile = %s\n",outFile) ;

    osg::Node *square = new Square;
    
    // Optimize the scene
    osgUtil::Optimizer optimizer;
    optimizer.optimize(square);
    
    osgDB::writeNodeFile(*square, outFile) ;
    
    return 0;
}
