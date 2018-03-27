/*
 * SAVG fileloader
 * John Kelso, NIST 1/2011
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <osg/Notify>
#include <osg/Node>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Point>
#include <osg/LineWidth>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>
#include <osgDB/Export>
#include <osgDB/WriteFile>
#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>
#include <osgUtil/TriStripVisitor>

#include <iris.h>

// to convert draw arrays to draw elements
#include "OptVisitor.cpp"

// needed for "loaderBoilerplate.cpp"
#define OSGLOADERFILETYPE savg
#define OSGLOADERSTRING "savg"
#include "loaderBoilerplate.cpp"

// uncomment this to get extra spewage in the trick spots
//#define SAVG_DEBUG

// sloppy global variables

// true if trying to be OpenGL3 compatible
bool gl3 ;

// true if we do spewage printfs
static bool spewage ;

// we create a small scenegraph.  Before returning it we trim off unneeded group nodes

// what is returned if using both opaque and transparent geometry
static osg::Group *group ;

// the opaque geometry is under this group
static osg::Group *opaqueGroup ;

// opaque points and lines under this geode
static osg::Geode *opaquePointsLinesGeode ;
// opaque polys under this geode
static osg::Geode *opaquePolysGeode ;

// the transparent geometry is under this group
static osg::Group *transparentGroup ;

// transparent points and lines under this geode
static osg::Geode *transparentPointsLinesGeode ;
// transparent polys under this geode
static osg::Geode *transparentPolysGeode ;

// text under this group
static osg::Group *textGroup ;

// used for error messages
static int lineCount = 0 ;
// how big does alpha get before we call it opaque?
static const float opacity  = .9f;
// true if color data includes transparency
static bool transparency ;
// vector of vertices
static osg::Vec3Array *coords ;
// vector of normals
static osg::Vec3Array *norms ;
// vector of colors
static osg::Vec4Array *colors ;
// type of primitive we're creating
static enum {END, LINE, POLY, POLYN, TRI, POINT} primitive ;
// color overall, or color per vertex
// BIND_PER_PRIMITIVE is slower than either BIND_OVERALL or BIND_PER_VERTEX!
static enum {OVERALL, VERTEX, DEFAULT} color ;
// number of sides in a polygon if parameter is supplied
static int sides ; 
// the single primitive color value- also default color if none specified
static osg::Vec4 primColor ;
// size of points and lines in pixels
static float pixelSize ;
// text values
struct textData {
    std::string font ;
    std::string justification ;
    std::string parts ;
} ;
static textData text ; // they retain their values so make them global
// true if the default optimizations are not applied- use NOOPTIMIZE command
static bool nooptimize ;

// search path for shader files
static std::vector<std::string> shaderpath ;

//std::string attributeTypes[] = { "b", "s", "i", "f", "d", "ub", "us" "ui" } ;

struct attribute {
    int size ;
    //std::string type ;
    std::vector<float> *data ;
} ;
static attribute attributes[16] ;

struct program {
    std::string name ;
    osg::Shader::Type type ;
    GLenum inputType ;
    GLenum outputType ;
    GLint maxVertices ;
    std::string attributes[16] ;
} ;
static std::vector<program> programs ;

// function templates
static bool parseLine(std::vector<std::string> vec) ;
static bool parseText(std::vector<std::string> vec) ;
static bool parseData(std::vector<std::string> vec) ;
static bool parsePixelSize(std::vector<std::string> vec) ;
static bool parsePrimitive(std::vector<std::string> vec) ;
static bool parseAttribute(std::vector<std::string> vec) ;
static bool parseProgram(std::vector<std::string> vec) ;
static bool processPrimitive(void) ;
static bool doPolyN(void) ;
static bool doTriOrPolyOrLineOrPoint(void) ;
static void resetState(void) ;
static bool getColor(std::vector<std::string> vec, unsigned int idx, osg::Vec4 *v) ;
static bool getVec4(std::vector<std::string> vec, unsigned int idx, osg::Vec4 *v) ;
static bool getVec3(std::vector<std::string> vec, unsigned int idx, osg::Vec3 *v) ;
static void ivOutput(std::ofstream &ivfile, std::string ivline) ;
static void polyToTristrip() ;
static void writePrograms(osg::Node *n) ;
static void setTransparency(osg::StateSet *ss) ;
static void printNode(unsigned int, osg::Node*) ;
static std::string indent(const std::string s, const unsigned int i, const unsigned int per=2) ;

///////////////////////////////////////////////////////////////////////////
// return a pointer to a node if all's well, or return NULL ;
static osgDB::ReaderWriter::ReadResult parseStream(std::istream& fin) {

    if (getenv("SAVG_VERBOSE")) spewage = true ;
    else spewage = false ;

    if (getenv("SAVG_OPENGL3"))
    {
	gl3 = true ;
    }
    else gl3 = false ;

    // for now just use OpenGL3
    gl3 = true ;
    if (spewage && gl3) fprintf(stderr,"savg loader: using OpenGL3 compatibility\n") ;

    if (spewage) fprintf(stderr, "using the OpenGL3 savg loader\n") ;
    std::string line; 

    // set the search path for shader files
    shaderpath.clear() ;
    std::string path ;
    path = "." ;
    char *osgfilepath = getenv("OSG_FILE_PATH") ;
    if (osgfilepath)
    {
	path += ":" ;
	path += osgfilepath ;
    }

    std::string::size_type idx ;
    while ((idx = path.find(":")) != std::string::npos)
    {
	std::string dir = path.substr(0,idx) ;
	if (dir != "") 
	{
	    shaderpath.push_back(dir) ;
	}
	path = path.substr(idx+1) ;
    }
    if (path != "")
    {
	shaderpath.push_back(path) ;
    }
    
    coords = new osg::Vec3Array ;
    norms = new osg::Vec3Array ;
    colors = new osg::Vec4Array ;

    transparency = false ;
    primColor = osg::Vec4( 1.f, 1.f, 1.f, 1.f) ;
    pixelSize = 1.f ;
    nooptimize = false ;

    programs.clear() ;

    text.justification = "LEFT" ;
    text.font = "courier" ;
    text.parts = "ALL" ;

    group = new osg::Group;
    opaqueGroup = new osg::Group ;
    opaquePointsLinesGeode = new osg::Geode ;
    opaquePolysGeode = new osg::Geode ;
    transparentGroup = new osg::Group ;
    osg::StateSet *stateSet = transparentGroup->getOrCreateStateSet() ;
    setTransparency(stateSet) ;
    transparentPointsLinesGeode = new osg::Geode ;
    transparentPolysGeode = new osg::Geode ;
    textGroup = new osg::Group ;

#ifdef SAVG_DEBUG
    group->setName("savg loader: group") ;
    opaqueGroup->setName("savg loader: opaqueGroup") ;
    opaquePointsLinesGeode->setName("savg loader: opaquePointsLinesGeode") ;
    opaquePolysGeode->setName("savg loader: opaquePolysGeode") ;
    transparentGroup->setName("savg loader: transparentGroup") ;
    transparentPointsLinesGeode->setName("savg loader: transparentPointsLinesGeode") ;
    transparentPolysGeode->setName("savg loader: transparentPolysGeode") ;
    textGroup->setName("savg loader: textGroup") ;
#endif

    group->addChild(opaqueGroup) ;
    opaqueGroup->addChild(opaquePointsLinesGeode) ;
    opaqueGroup->addChild(opaquePolysGeode) ;
    group->addChild(transparentGroup) ;
    transparentGroup->addChild(transparentPointsLinesGeode) ;
    transparentGroup->addChild(transparentPolysGeode) ;
    group->addChild(textGroup) ;

    opaquePointsLinesGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED) ;
    transparentPointsLinesGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED) ;
    
    memset(attributes, 0, sizeof(attributes)) ;

    while (getline(fin, line)) 
    {
	lineCount++ ;
	if (osg::isNotifyEnabled(osg::INFO) && !(lineCount%1000))
	{
	    osg::notify( osg::INFO ) << OSGLOADERSTRING << ": " << lineCount << " lines read\n" ;
	}


	// chop off trailing spaces
	line = line.substr(0,line.find_last_not_of(" ")+1) ;

	osg::notify( osg::DEBUG_INFO ) << lineCount << ": line = " << line << std::endl ;
	std::vector<std::string> vec = iris::ParseString(line) ;

	if (vec.size() >0)
	{
	    if (!parseLine(vec)) 
	    {
		osg::notify( osg::FATAL ) << OSGLOADERSTRING" file loader:: line number " 
					  << lineCount << ", invalid line: " << line << std::endl ;
		return NULL ;
	    }
	}
    }
    
    // do whatever's piled up at the end of data
    std::vector<std::string> vec ;
    parseLine(vec) ;

    if (osg::isNotifyEnabled(osg::INFO))
	osg::notify( osg::INFO ) << OSGLOADERSTRING << ": " << lineCount << " total lines read\n" ;


#if 0
    fprintf(stderr,"group node has %d children\n", group->getNumChildren()) ;
    fprintf(stderr,"opaqueGroup node has %d children\n", opaqueGroup->getNumChildren()) ;
    fprintf(stderr,"opaquePointsLinesGeode has %d drawables\n", opaquePointsLinesGeode->getNumDrawables()) ;
    fprintf(stderr,"opaquePolysGeode has %d drawables\n", opaquePolysGeode->getNumDrawables()) ;
    fprintf(stderr,"transparentGroup node has %d children\n", transparentGroup->getNumChildren()) ;
    fprintf(stderr,"transparentPointsLinesGeode has %d drawables\n", transparentPointsLinesGeode->getNumDrawables()) ;
    fprintf(stderr,"transparentPolysGeode has %d drawables\n", transparentPolysGeode->getNumDrawables()) ;
    fprintf(stderr,"textGroup node has %d children\n", textGroup->getNumChildren()) ;
#endif


    // trim the tree
    if (opaquePointsLinesGeode->getNumDrawables() == 0) opaqueGroup->removeChild(opaquePointsLinesGeode) ;
    if (opaquePolysGeode->getNumDrawables() == 0) opaqueGroup->removeChild(opaquePolysGeode) ;
    if (transparentPointsLinesGeode->getNumDrawables() == 0) transparentGroup->removeChild(transparentPointsLinesGeode) ;
    if (transparentPolysGeode->getNumDrawables() == 0) transparentGroup->removeChild(transparentPolysGeode) ;

    if (opaqueGroup->getNumChildren() == 0) group->removeChild(opaqueGroup) ;
    if (transparentGroup->getNumChildren() == 0) group->removeChild(transparentGroup) ;
    if (textGroup->getNumChildren() == 0) group->removeChild(textGroup) ;

#ifdef SAVG_DEBUG
    fprintf(stderr,"savg loader: tree trimmed, here's what's left:\n") ;
    printNode(1,group) ;
#endif

    osg::ref_ptr<osg::Node> node ;

    if (group->getNumChildren() > 1) 
    {
#ifdef SAVG_DEBUG
	fprintf(stderr,"using group\n") ;
#endif
	node = group ;
    }
    else if (opaqueGroup->getNumChildren() > 0) 
    {
	if (opaqueGroup->getNumChildren() == 2) 
	{
#ifdef SAVG_DEBUG
	    fprintf(stderr,"using opaqueGroup\n") ;
#endif
	    node = opaqueGroup ;
	}
	else if (opaquePolysGeode->getNumDrawables() > 0) 
	{
#ifdef SAVG_DEBUG
	    fprintf(stderr,"using opaquePolysGeode\n") ;
#endif
	    node = opaquePolysGeode ;
	}
	else 
	{
#ifdef SAVG_DEBUG
	    fprintf(stderr,"using opaquePointsLinesGeode\n") ;
#endif
	    node = opaquePointsLinesGeode ;
	}
    }
    else if (transparentGroup->getNumChildren() > 0) 
    {
	if (transparentGroup->getNumChildren() == 2)
	{
#ifdef SAVG_DEBUG
	    fprintf(stderr,"using transparentGroup\n") ;
#endif
	    node = transparentGroup ;
	}
	else if (transparentPolysGeode->getNumDrawables() > 0)
	{
#ifdef SAVG_DEBUG
	    fprintf(stderr,"using transparentPolysGeode\n") ;
#endif
	    osg::StateSet *stateSet = transparentPolysGeode->getOrCreateStateSet() ;
	    setTransparency(stateSet) ;
	    node = transparentPolysGeode ;
	}
	else 
	{
#ifdef SAVG_DEBUG
	    fprintf(stderr,"using transparentPointsLinesGeode\n") ;
#endif
	    osg::StateSet *stateSet = transparentPointsLinesGeode->getOrCreateStateSet() ;
	    setTransparency(stateSet) ;
	    node = transparentPointsLinesGeode ;
	}
    }
    else if (textGroup->getNumChildren() > 0) 
    {
#ifdef SAVG_DEBUG
	fprintf(stderr,"using textGroup\n") ;
#endif
	node = textGroup ;
    }
    else 
    {
#ifdef SAVG_DEBUG
	fprintf(stderr,"using NULL\n") ;
#endif
	node = NULL ;
    }

    //fprintf(stderr, "returned pointer = %p\n",node.get()) ;
    if (!node) return new osg::Group ;
    
    time_t t1, t2 ;    

    // get a list of optimizations to apply one at a time
    std::vector<std::string> optimization ;
    std::string opts ;
    if (getenv("OSG_OPTIMIZER")) 
    {
	opts = getenv("OSG_OPTIMIZER") ;

	std::string::size_type idx ;
	while ((idx = opts.find(":")) != std::string::npos)
	{
	    std::string opt = opts.substr(0,idx) ;
	    if (opt != "") 
	    {
		optimization.push_back(opt) ;
	    }
	    opts = opts.substr(idx+1) ;
	}
	if (opts != "")
	{
	    optimization.push_back(opts) ;
	}
	
	for (unsigned int i=0; i<optimization.size(); i++)
	{
	    setenv("OSG_OPTIMIZER",optimization[i].c_str(),1) ;
	    osgUtil::Optimizer *o = new osgUtil::Optimizer() ;
	    if (spewage) fprintf(stderr,"savg file loader: applying optimization \"%s\"... ",optimization[i].c_str()) ;
	    t1 = time(NULL) ;
	    o->optimize(node) ;
	    t2 = time(NULL) ;
	    if (spewage) fprintf(stderr,"took %d seconds\n",t2-t1) ;
	} 
    }
    else if (!nooptimize)
    {
	if (spewage) fprintf(stderr,"savg file loader: stripifying... ") ;	
	osgUtil::TriStripVisitor *tsv = new osgUtil::TriStripVisitor ;
	t1 = time(NULL) ;
	node->accept(*tsv) ;
	tsv->setCacheSize(16) ;
	tsv->stripify();
	t2 = time(NULL) ;
	if (spewage) fprintf(stderr,"took %d seconds\n",t2-t1) ;
	
	if (spewage) fprintf(stderr,"savg file loader: merging geometry... ") ;	
	osgUtil::Optimizer::MergeGeometryVisitor *mgv = new osgUtil::Optimizer::MergeGeometryVisitor ;
	t1 = time(NULL) ;
	mgv->setTargetMaximumNumberOfVertices(65536);
	node->accept(*mgv) ;
	t2 = time(NULL) ;
	if (spewage) fprintf(stderr,"took %d seconds\n",t2-t1) ;
    }

    if (gl3)
    {
	// convert to DrawElements
	t1 = time(NULL) ;
	if (spewage) fprintf(stderr,"savg file loader: converting to draw elements... ") ;	
	OptVisitor *ov = new OptVisitor ;
	node->accept(*ov) ;
	t2 = time(NULL) ;
	if (spewage) fprintf(stderr,"took %d seconds\n",t2-t1) ;
    }

    writePrograms(node) ;

    // be sure the returned node has no parents!
    std::vector<osg::Group*> ps = node->getParents() ;
    //fprintf(stderr,"%s %d, node->getParents() returns %d nodes\n",__FILE__,__LINE__,ps.size()) ;
    for (unsigned int i=0; i<ps.size(); i++) ps[i]->removeChild(node) ;
    //fprintf(stderr,"%s %d, node->getParents() returns %d nodes\n",__FILE__,__LINE__,ps.size()) ;
    return node.get() ;


}

////////////////////////////////////////////////////////////////////////
static bool parseLine(std::vector<std::string> vec)
{

#if 0
    std::clog << vec.size() << " parameters\n" ;
    for (unsigned int i=0; i< vec.size(); i++)
	std::clog << i << ": " << vec[i] << std::endl ;
#endif

    if (vec.size()==0 || (vec.size() == 1 && iris::IsSubstring("end",vec[0],3)))
	return processPrimitive() ;
    else if (parseData(vec)) //parse data first, as other keywords can signal end of data
	return true ;
    else if (parsePrimitive(vec))
	return true ;
    else if (parseText(vec))
	return true ;
    else if (parsePixelSize(vec))
	return true ;
    else if (parseAttribute(vec))
	return true ;
    else if (parseProgram(vec))
	return true ;
    else if (iris::IsSubstring("NOOPTIMIZE",vec[0],3))
    {
	nooptimize = true ;
    }
    else if (iris::IsSubstring("ALPHA",vec[0],3) ||
	     iris::IsSubstring("AMBIENT",vec[0],3) ||
	     iris::IsSubstring("COLORMODE",vec[0],3) ||
	     iris::IsSubstring("DIFFUSE",vec[0],3) ||
	     iris::IsSubstring("EMISSION",vec[0],3) ||
	     iris::IsSubstring("INDEXED",vec[0],3) ||
	     iris::IsSubstring("LIGHTING",vec[0],3) ||
	     iris::IsSubstring("OCTREE",vec[0],3) ||
	     iris::IsSubstring("PRESERVEPLANARCHECKING",vec[0],15) ||
	     iris::IsSubstring("PRESERVEPLANARQUADS",vec[0],15) ||
	     iris::IsSubstring("SHININESS",vec[0],3) ||
	     iris::IsSubstring("SHRINKAGE",vec[0],3) ||
	     iris::IsSubstring("SPECULAR",vec[0],3) ||
	     iris::IsSubstring("STRIPLENGTH",vec[0],3) ||
	     iris::IsSubstring("STYLE",vec[0],3) ||
	     iris::IsSubstring("TEXTURE",vec[0],5) ||
	     iris::IsSubstring("TRANSPARENCY",vec[0],3) ||
	     iris::IsSubstring("VERBOSITY",vec[0],3) )
    {
	osg::notify( osg::WARN ) << OSGLOADERSTRING" file loader:: ignoring unimplemented command: " << vec[0] << ", line " << lineCount << "\n" ;
	return true ;
    }
    else
    {
	osg::notify( osg::FATAL ) << OSGLOADERSTRING" file loader:: unknown line syntax or line incompatible with earlier lines\n" ;
	return false ;
    }

    // return false earlier on error
    return true ;
}

////////////////////////////////////////////////////////////////////////
static bool processPrimitive(void)
{
    if (coords->size()>0)
    {
	if (norms->size()>0 && (norms->size() != coords->size()))
	{
	    osg::notify( osg::FATAL ) << OSGLOADERSTRING" file loader:: normals supplied, and number of vertices not equal to number of normals- norms->size()=" << norms->size() << ",coords->size()=" << coords->size() << "\n" ; 
	    return false ;
	}
      
	if (colors->size()>0 && (colors->size() != coords->size()))
	{
	    osg::notify( osg::FATAL ) << OSGLOADERSTRING" file loader:: colors supplied, and number of vertices not equal to number of colors- colors->size()=" << colors->size() << ",coords->size()=" << coords->size() << "\n" ; 
	    return false ;
	}
      
	if (primitive==POLYN)
	{
	    if (!doPolyN())
	    {
		return false ;
	    }
	}
	else if (!doTriOrPolyOrLineOrPoint())
	    return false ;

	resetState() ;
    }
    return true ;
}

////////////////////////////////////////////////////////////////////////
static bool parsePrimitive(std::vector<std::string> vec) 
{
    // process previous primitive, if any
    if (!processPrimitive()) return false ;
    else if (iris::IsSubstring("tristrips",vec[0],3))
    {
	primitive = TRI ;
    }
    else if (iris::IsSubstring("polygons",vec[0],3))
    {
	primitive = POLY ;
    }
    else if (iris::IsSubstring("lines",vec[0],3))
    {
	primitive = LINE ;
    }
    else if (iris::IsSubstring("points",vec[0],3))
    {
	primitive = POINT ;
    }
    else return false ;
  
    if (vec.size() == 1) //prim
    {
	color = DEFAULT ;  // might use default or might specify in data
	return true ;
    }
    else if (vec.size() == 5 && getColor(vec, 1, &primColor)) //prim r g b a
    {
	color = OVERALL ;
	if (primColor.a() <= opacity) 
	    transparency = true ;
	return true ;
    }
    else if (primitive == POLY && vec.size() == 2 && iris::StringToInt(vec[1], &sides)) //poly n
    {
	color = DEFAULT ; // might use default or might specify in data
	primitive = POLYN ;
	return true ;
    }
    else if (primitive == POLY && vec.size() == 6 && getColor(vec, 1, &primColor) && 
	     iris::StringToInt(vec[5], &sides)) //poly r g b a n
    {
	primitive = POLYN ;
	color = OVERALL ;
	if (primColor.a() <= opacity) 
	    transparency = true ;
	return true ;
    }
    else
    {
	return false ;
    }
}

////////////////////////////////////////////////////////////////////////
static bool parseData(std::vector<std::string> vec)
{
    static osg::Vec3 v ;
    static osg::Vec3 n ;
    static osg::Vec4 c ;

    // find and remove vertex attribute data
    {
	unsigned int firstAttributeIndex = 0 ;
	// need at least three floats before attribute keyword- keeps us from mistaking this line for an attribute command
	for (unsigned int i=3; i<vec.size(); i++)
	{
	    if (iris::IsSubstring("ATTRIBUTE",vec[i],3))
	    {
		if (firstAttributeIndex == 0) 
		{
		    firstAttributeIndex = i ;
		}
		if (vec.size() == i+1) return false ; // no data!
		i++ ;
		int index ;
		if (!iris::StringToInt(vec[i], &index) || index<0 || index>15) return false ;
		if (attributes[index].size == 0) return false ; // no attribute with this index declared earlier
		int size = attributes[index].size ;
		if (vec.size() < i+size+1) return false ; // not enough data!
		i++ ;
		for (unsigned int j=0; j<size; j++)
		{
		    float d ;
		    if (!iris::StringToFloat(vec[i+j],&d)) return false ;
		    attributes[index].data->push_back(d) ;
		}
	    }
	}
	// chop off vector starting at firstAttributeIndex
	if (firstAttributeIndex > 0) vec.resize(firstAttributeIndex) ;
    }


    // data without a primitive type
    if (primitive==END)
	return false ;
    else if (vec.size() == 3 && getVec3(vec, 0, &v)) //vertex
    {
	if (color == VERTEX)
	    return false ;
	coords->push_back(v) ;
	return true ;
    }
    else if (vec.size() == 6 && getVec3(vec, 0, &v) && getVec3(vec, 3, &n)) //vertex normal
    {
	if (color == VERTEX)
	    return false ;
	coords->push_back(v) ;
	norms->push_back(n) ;
	return true ;
    }
    else if (vec.size() == 7 && getVec3(vec, 0, &v) && getVec4(vec, 3, &c)) //vertex color
    {
	if (color == OVERALL)
	    return false ;
	coords->push_back(v) ;
	colors->push_back(c) ;
	primColor = c ;
	if (primColor.a() <= opacity) 
	    transparency = true ;
	color = VERTEX ;
	return true ;
    }
    else if (vec.size() == 10 && getVec3(vec, 0, &v) && getVec4(vec, 3, &c) 
	     && getVec3(vec, 7, &n)) //vertex color normal
    {
	if (color == OVERALL)
	    return false ;
	coords->push_back(v) ;
	norms->push_back(n) ;
	colors->push_back(c) ;
	primColor = c ;
	if (primColor.a() <= opacity) 
	    transparency = true ;
	color = VERTEX ;
	return true ;
    }
    else
	return false ;
}

////////////////////////////////////////////////////////////////////////
static bool parsePixelSize(std::vector<std::string> vec)
{
    // process previous primitive, if any
    if (!processPrimitive())
	return false ;
    else if (vec.size()==2 && iris::IsSubstring("pixelsize",vec[0],3) && iris::StringToFloat(vec[1], &pixelSize))
    {
	osg::StateSet *opaqueStateSet = opaquePointsLinesGeode->getOrCreateStateSet() ;
	osg::StateSet *transparentStateSet = transparentPointsLinesGeode->getOrCreateStateSet() ;

	opaqueStateSet->setMode(GL_POINT_SMOOTH, osg::StateAttribute::ON);
	transparentStateSet->setMode(GL_POINT_SMOOTH, osg::StateAttribute::ON);

	osg::Point *point = new osg::Point(pixelSize) ;
	opaqueStateSet->setAttribute(point, osg::StateAttribute::ON);
	transparentStateSet->setAttribute(point, osg::StateAttribute::ON);

	opaqueStateSet->setMode(GL_LINE_SMOOTH, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED) ;
	transparentStateSet->setMode(GL_LINE_SMOOTH, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED) ;

	opaqueStateSet->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED) ;
	transparentStateSet->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED) ;


	osg::LineWidth *linewidth = new osg::LineWidth;
	linewidth->setWidth(pixelSize);
	opaqueStateSet->setAttribute(linewidth, osg::StateAttribute::ON);
	transparentStateSet->setAttribute(linewidth, osg::StateAttribute::ON);

	return true ;
    }
    else
	return false ;	
}

////////////////////////////////////////////////////////////////////////
// this is one of the bigger kludges I've ever written
// text isn't supported, so we do it with Inventor, creating and loading an iv file
// we put it under a matrix transform node because the current (2.8.6) OSG Inventor loader doesn't support transformations
static bool parseText(std::vector<std::string> vec)
{
    // process previous primitive, if any
    if (!processPrimitive())
	return false ;
    else if (vec.size()>2 && iris::IsSubstring("text",vec[0],3))
    {

	std::string string ;
	bool colorSet = false ;
	osg::Vec3 position(0,0,0);
#define UNITCOURIERTEXT
#ifdef UNITCOURIERTEXT
	osg::Vec3 scale(1.f/6.f,1.f/6.f,1.f/6.f) ;
#else
	osg::Vec3 scale(1.f,1.f,1.f) ;
#endif
	osg::Quat attitude ;
	unsigned int i=1 ;
	while (i<vec.size())
	{
	    if (iris::IsSubstring("string",vec[i],3))
	    {
		i++ ;
		if (i<vec.size())
		{
		    string = vec[i] ;
		    i++ ;
		}
		else
		    return false ;
	    }
	    else if (iris::IsSubstring("font",vec[i],3))
	    {
		i++ ;
		if (i<vec.size())
		{
		    text.font = vec[i] ;
		    i++ ;
		}
		else
		    return false ;
	    }
	    else if (iris::IsSubstring("flatten",vec[i],3)|| iris::IsSubstring("style",vec[i],3))
	    {
		i++ ;
		if (i<vec.size())
		{
		    i++ ;
		}
		else
		    return false ;
	    }
	    else if (iris::IsSubstring("justification",vec[i],3))
	    {
		i++ ;
		if (i<vec.size())
		{
		    if (iris::IsSubstring("left",vec[i])) text.justification = "LEFT" ;
		    else if (iris::IsSubstring("center",vec[i])) text.justification = "CENTER" ;
		    else if (iris::IsSubstring("right",vec[i])) text.justification = "RIGHT" ;
		    i++ ;
		}
		else
		    return false ;
	    }
	    else if (iris::IsSubstring("parts",vec[i],3))
	    {
		i++ ;
		if (i<vec.size())
		{
		    if (iris::IsSubstring("front",vec[i])) text.parts = "FRONT" ;
		    else if (iris::IsSubstring("sides",vec[i])) text.parts = "SIDES" ;
		    else if (iris::IsSubstring("all",vec[i])) text.parts = "ALL" ;
		    else if (iris::IsSubstring("back",vec[i])) text.parts = "BACK" ;
		    i++ ;
		}
		else
		    return false ;
	    }
	    else if (iris::IsSubstring("xyz",vec[i],3))
	    {
		i++ ;
		float x, y, z ;
		if (i+2<vec.size() && 
		    iris::StringToFloat(vec[i], &x) &&
		    iris::StringToFloat(vec[i+1], &y) &&
		    iris::StringToFloat(vec[i+2], &z) )
		{
		    position = osg::Vec3(x,y,z) ;
 		    i+=3 ;
		}
		else
		    return false ;
	    }
	    else if (iris::IsSubstring("sxyz",vec[i],3))
	    {
		i++ ;
		float x, y, z ;
		if (i+2<vec.size() && 
		    iris::StringToFloat(vec[i], &x) &&
		    iris::StringToFloat(vec[i+1], &y) &&
		    iris::StringToFloat(vec[i+2], &z) )
		{
#ifdef  UNITCOURIERTEXT
		    scale = osg::Vec3(1.f/6.f*x, 1.f/6.f*y, 1.f/6.f*z) ;
#else
		    scale = osg::Vec3(x,y,z) ;
#endif
 		    i+=3 ;
		}
	    }
	    else if (iris::IsSubstring("s",vec[i]))
	    {
		i++ ;
		float s ;
		if (i<vec.size() && iris::StringToFloat(vec[i], &s))
		{
#ifdef  UNITCOURIERTEXT
		    scale = osg::Vec3(1.f/6.f*s, 1.f/6.f*s, 1.f/6.f*s) ;
#else
		    scale = osg::Vec3(s,s,s) ;
#endif
		    i++ ;
		}
		else
		    return false ;
	    }
	    else if (iris::IsSubstring("hpr",vec[i],3))
	    {
		i++ ;
		float h, p, r;
		if (i+2<vec.size() && 
		    iris::StringToFloat(vec[i], &h) &&
		    iris::StringToFloat(vec[i+1], &p) &&
		    iris::StringToFloat(vec[i+2], &r) )
		{
		    attitude = iris::EulerToQuat(h,p,r) ;
		    i+=3 ;
		}
		else
		{
		    return false ;
		}
	    }
	    else if (iris::IsSubstring("rgba",vec[i],3))
	    {
		i++ ;
		float r, g, b, a ;
		if (i<vec.size() && 
		    iris::StringToFloat(vec[i], &r) &&
		    iris::StringToFloat(vec[i+1], &g) &&
		    iris::StringToFloat(vec[i+2], &b) &&
		    iris::StringToFloat(vec[i+3], &a) )
		{
		    colorSet = true ;
		    primColor.set(r,g,b,a) ;
		    i+=4 ;
		}
		else
		    return false ;
	    }
	    else 
	    {
		return false ;
	    }
	}

	// now make a string using Inventor
	if (string != "")
	{

	    std::string ivFileName= iris::GetTempFileName("iv", "/tmp") ;

	    std::ofstream ivFile(ivFileName.c_str(), std::ios::out) ;

	    ivOutput(ivFile, "#Inventor V2.0 ascii") ;
	    ivOutput(ivFile, "Separator {") ;

	    if (colorSet)
	    {
		std::ostringstream r, g, b ;
		r << primColor.r() ;
		g << primColor.g() ;
		b << primColor.b() ;
		ivOutput(ivFile, "Material {") ;
		ivOutput(ivFile, "diffuseColor " + r.str() + " " + g.str() + " " + b.str()) ;
		if (primColor.a()<opacity)
		{
		    std::ostringstream t ;
		    t << 1.f-primColor.a() ;
		    ivOutput(ivFile, "transparency " + t.str()) ;
		}
		ivOutput(ivFile, "}") ;
	    }

	    ivOutput(ivFile, "Font {") ;
	    ivOutput(ivFile, "name \"" + text.font + "\"") ;
	    ivOutput(ivFile, "}") ;
	    
	    ivOutput(ivFile, "Text3 {") ;
	    ivOutput(ivFile, "string [\"" + string + "\"]") ;
	    ivOutput(ivFile, "parts " + text.parts ) ; 
	    ivOutput(ivFile, "justification " + text.justification ) ; 
	    ivOutput(ivFile, "}") ;
	    
	    ivOutput(ivFile, "}") ;
	    ivFile.close() ;
	    
	    osg::Node *node = osgDB::readNodeFile(ivFileName) ;
#ifdef SAVG_DEBUG
	    node->setName("savg loader: Inventor text node " + string) ;
#endif

#if 1
	    iris::MatrixTransform *mtn = new  iris::MatrixTransform ;
	    mtn->setDataVariance( osg::Object::STATIC );
	    mtn->setAttitude(attitude,false);
	    mtn->setPosition(position,false);
	    mtn->setScale(scale,true);
#ifdef SAVG_DEBUG
	    mtn->setName("savg loader: text node DCS " + string) ;
#endif
	    textGroup->addChild(mtn) ;
	    mtn->addChild(node) ;
#else
	    osg::Matrix mat ;
	    mat.makeTranslate(position) ;
	    mat.postMultRotate(attitude) ;
	    mat.preMultScale(scale) ;
	    osg::MatrixTransform *matt = new osg::MatrixTransform ;
	    matt->setDataVariance( osg::Object::STATIC );
	    matt->setMatrix(mat) ;
	    textGroup->addChild(matt) ;
	    matt->addChild(node) ;
#endif

	    
#ifdef SAVG_DEBUG
	    fprintf(stderr,"did not delete tmp file %s\n",ivFileName.c_str()) ;
#else
	    unlink(ivFileName.c_str()) ;
#endif

	}
	return true ;
	
    }
    else
	return false ;
}

////////////////////////////////////////////////////////////////////////
static void resetState(void)
{
    primitive = END ;
    coords->clear() ;
    norms->clear() ;
    color = DEFAULT ;
    colors->clear() ;
    sides = 0 ;
    transparency = false ;
    {
	for (unsigned int i=0; i<16; i++) if (attributes[i].size != 0) attributes[i].data->clear() ;
    }
}

////////////////////////////////////////////////////////////////////////
static bool getColor(std::vector<std::string> t, unsigned int i, osg::Vec4 *v)
{
    return (getVec4(t, i, v) && v->r()>=0.f && 
	    v->r()<=1.0 && v->g()>=0.f && v->g()<=1.0 &&  v->b()>=0.f && 
	    v->b()<=1.0 && v->a()>=0.f && v->a()<=1.0) ;
}

////////////////////////////////////////////////////////////////////////
static bool getVec3(std::vector<std::string> t, unsigned int i, osg::Vec3 *v)
{
    return iris::StringToFloat(t[i], &(v->x())) && iris::StringToFloat(t[i+1], &(v->y())) && 
	iris::StringToFloat(t[i+2], &(v->z())) ;
}

////////////////////////////////////////////////////////////////////////
static bool getVec4(std::vector<std::string> t, unsigned int i, osg::Vec4 *v)
{
    return iris::StringToFloat(t[i], &(v->x())) && iris::StringToFloat(t[i+1], &(v->y())) && 
	iris::StringToFloat(t[i+2], &(v->z())) && iris::StringToFloat(t[i+3], &(v->w())) ;
}

////////////////////////////////////////////////////////////////////////
// output a line of inventor text with automatic indentation
static void ivOutput(std::ofstream &ivfile, std::string ivline)
{
    static unsigned int ivlevel = 0 ;

    bool obrace = !(ivline.find("{") ==  std::string::npos);
    bool cbrace = !(ivline.find("}") ==  std::string::npos);


    if (cbrace && !obrace)
    {
	ivlevel-- ;
	ivfile << indent(ivline+"\n", ivlevel) ;
    }
    else if (!cbrace && obrace)
    {
	ivfile << indent(ivline+"\n", ivlevel) ;
	ivlevel++ ;
    }
    else
    {
	ivfile << indent(ivline+"\n", ivlevel) ;	
    }
}

////////////////////////////////////////////////////////////////////////
static bool doPolyN(void)
{
    if (coords->size()>0)
    {
	if (coords->size()%sides != 0) 
	{
	    osg::notify( osg::FATAL ) << OSGLOADERSTRING" file loader:: number of vertices not divisbible by number of sides" ;
	    return false ;
	}
	// bust up our collection of coords/normals/colors into seperate polygons
	osg::Vec3Array *copyCoords = new osg::Vec3Array(*coords) ;
	osg::Vec3Array *copyNorms = new osg::Vec3Array ;
	static bool doNorms ;
	if (norms->size() == coords->size())
	{
	    doNorms=true ;
	    copyNorms = new osg::Vec3Array(*norms) ;
	}
	osg::Vec4Array *copyColors = new osg::Vec4Array ;
	if (color==VERTEX)
	{
	    copyColors = new osg::Vec4Array(*colors) ;
	}
	primitive=POLY ;
	for (unsigned int i=0; i<copyCoords->size(); i+=sides)
	{
	    coords->clear() ;
	    if (doNorms) 
		norms->clear() ;
	    if (color==VERTEX)
	    {
		colors->clear() ;
	    }
	    for (unsigned int j=0; j<sides; j++)
	    {
		coords->push_back((*copyCoords)[i+j]) ;
		if (doNorms)
		    norms->push_back((*copyNorms)[i+j]) ;
		if (color==VERTEX)
		    colors->push_back((*copyColors)[i+j]) ;
	    }
	    doTriOrPolyOrLineOrPoint() ;
	}
    }
    return true ;
}

////////////////////////////////////////////////////////////////////////
static bool doTriOrPolyOrLineOrPoint(void)
{

    // set color if none specified
    if (color == DEFAULT)
	color = OVERALL ;

    //start new geometry
    osg::Geometry *geometry = new osg::Geometry ;

    if (gl3)
    {
	// use VBOs if possible
	geometry->setUseDisplayList(false) ;
	geometry->setUseVertexBufferObjects(true) ;
    }

    // turn polygons into tri-strips
    if (primitive == POLY)
    {
	// check for vertices in polygon being coplanar and polygon being convex
	if (coords->size()>3)
	{

	    bool done = false ;

	    // use cross products to see if convex
	    // get cross product using first three points
	    osg::Vec3 x0 = ((*coords)[2] - (*coords)[0])^((*coords)[1] - (*coords)[0]) ;
	    // the cross priducts of the remaining angles should point in the same direction
	    for (unsigned int i=1; i<coords->size() && !done; i++)
	    {
		unsigned int p1 = (i+1)%coords->size() ;
		unsigned int p2 = (i+2)%coords->size() ;
		osg::Vec3 x = ((*coords)[p2] - (*coords)[i])^((*coords)[p1] - (*coords)[i]) ;
		if (x0*x <0)
		{
		    dtkMsg.add(DTKMSG_WARNING,"savg file loader: polygon defined before line %d is not convex\n",lineCount) ;
		    done = true ;
		}
	    }

	    // use the first three points to define a plane
	    osg::Plane p((*coords)[0],(*coords)[1],(*coords)[2]) ;
	    // and see if the rest of the points are on it
	    for (unsigned int i=3; i<coords->size() && !done; i++)
	    {
		if (p.distance((*coords)[i])>FLT_EPSILON)
		{
		    dtkMsg.add(DTKMSG_WARNING,"savg file loader: polygon defined before line %d is not planer\n",lineCount) ;
		    done = true ;
		}
	    }
	    
	}


	polyToTristrip() ;
    }

    osg::Vec3Array *copyCoords = new osg::Vec3Array(*coords) ;
    geometry->setVertexArray(copyCoords) ;

    if (norms->size() >0) 
    {
	osg::Vec3Array *copyNorms = new osg::Vec3Array(*norms) ;
	geometry->setNormalArray(copyNorms) ;
	geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX) ;
    }
    if (color == OVERALL)
    {
	osg::Vec4Array *singlecolor = new osg::Vec4Array;
	singlecolor->push_back(primColor) ;
	geometry->setColorArray(singlecolor) ;
	geometry->setColorBinding(osg::Geometry::BIND_OVERALL) ;
    }
    else if (color == VERTEX)
    {
	osg::Vec4Array *copyColors = new osg::Vec4Array(*colors) ;
	geometry->setColorArray(copyColors) ;
	geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX) ;
    }

    // vertex attributes
    {
	for (unsigned int i=0; i<16; i++)
	{
	    if (attributes[i].data && attributes[i].data->size()>0)
	    {
#if 0
		fprintf(stderr,"attribute %d has %d values, size = %d\n",i,attributes[i].data->size(),attributes[i].size) ;
#endif
		if (attributes[i].data->size()%attributes[i].size != 0) return false ;
		if (attributes[i].size == 1)
		{
		    osg::FloatArray *v = new osg::FloatArray ;
		    for (unsigned int j=0; j<attributes[i].data->size(); j++)
		    {
			v->push_back((*attributes[i].data)[j]) ;
		    }
		    geometry->setVertexAttribArray(i, v);
		    if (attributes[i].data->size() > 1) geometry->setVertexAttribBinding(i, osg::Geometry::BIND_PER_VERTEX);
		    else geometry->setVertexAttribBinding(i, osg::Geometry::BIND_OVERALL);
		}
		else if (attributes[i].size == 2)
		{
		    osg::Vec2Array *v = new osg::Vec2Array ;
		    for (unsigned int j=0; j<attributes[i].data->size(); j+=2)
		    {
			v->push_back(osg::Vec2((*attributes[i].data)[j],(*attributes[i].data)[j+1])) ;
		    }
		    geometry->setVertexAttribArray(i, v);
		    if (attributes[i].data->size() > 2) geometry->setVertexAttribBinding(i, osg::Geometry::BIND_PER_VERTEX);
		    else geometry->setVertexAttribBinding(i, osg::Geometry::BIND_OVERALL);
		}
		else if (attributes[i].size == 3)
		{
		    osg::Vec3Array *v = new osg::Vec3Array ;
		    for (unsigned int j=0; j<attributes[i].data->size(); j+=3)
		    {
			v->push_back(osg::Vec3((*attributes[i].data)[j],(*attributes[i].data)[j+1],(*attributes[i].data)[j+2])) ;
		    }
		    geometry->setVertexAttribArray(i, v);
		    if (attributes[i].data->size() > 3) geometry->setVertexAttribBinding(i, osg::Geometry::BIND_PER_VERTEX);
		    else geometry->setVertexAttribBinding(i, osg::Geometry::BIND_OVERALL);
		}
		else if (attributes[i].size == 4)
		{
		    osg::Vec4Array *v = new osg::Vec4Array ;
		    for (unsigned int j=0; j<attributes[i].data->size(); j+=4)
		    {
			v->push_back(osg::Vec4((*attributes[i].data)[j],(*attributes[i].data)[j+1],(*attributes[i].data)[j+2],(*attributes[i].data)[j+3])) ;
		    }
		    geometry->setVertexAttribArray(i, v);
		    if (attributes[i].data->size() > 4) geometry->setVertexAttribBinding(i, osg::Geometry::BIND_PER_VERTEX);
		    else geometry->setVertexAttribBinding(i, osg::Geometry::BIND_OVERALL);
		}
	    }
	}
    }

    if (gl3)
    {
	unsigned int indices[coords->size()] ;
	for (unsigned int i=0; i<coords->size(); i++) indices[i] = i ;
	
	if (primitive == POLY || primitive == TRI)
	    geometry->addPrimitiveSet(new osg::DrawElementsUInt(GL_TRIANGLE_STRIP, coords->size(), indices)) ;
	else if(primitive == LINE)
	    geometry->addPrimitiveSet(new osg::DrawElementsUInt(GL_LINE_STRIP, coords->size(), indices)) ;
	else if(primitive == POINT)
	    geometry->addPrimitiveSet(new osg::DrawElementsUInt(GL_POINTS, coords->size(), indices)) ;
    }
    else
    {
	if (primitive == POLY || primitive == TRI)
	    geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, coords->size())) ;
	else if(primitive == LINE)
	    geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, coords->size())) ;
	else if(primitive == POINT)
	    geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, coords->size())) ;
    }

    osg::Geode *geode ;
    if (transparency && (primitive == POLY || primitive == TRI)) geode = transparentPolysGeode ;
    else if (transparency && (primitive == POINT || primitive == LINE)) geode = transparentPointsLinesGeode ;
    else if (!transparency && (primitive == POLY || primitive == TRI)) geode = opaquePolysGeode ;
    else if (!transparency && (primitive == POINT || primitive == LINE)) geode = opaquePointsLinesGeode ;
    geode->addDrawable(geometry) ;

    return true ;
}

////////////////////////////////////////////////////////////////////////
// rearrange points so a polygon becomes a tri-strip
void polyToTristrip()
{
    // make a copy of the vertices
    osg::Vec3Array *copyCoords = new osg::Vec3Array(*coords) ;
    osg::Vec3Array *copyNorms = new osg::Vec3Array(*norms) ;
    osg::Vec4Array *copyColors = new osg::Vec4Array(*colors) ;
    attribute copyAttributes[16] ;
    for (unsigned int j=0; j<16; j++)
    {
	if (attributes[j].size != 0)
	{
#if 0
	    fprintf(stderr,"copying attribute %d\n",j) ;
#endif
#if 0
	    fprintf(stderr,"attribute %d size = %d\n",j,attributes[j].size) ;
#endif
#if 0
	    fprintf(stderr,"attribute %d data size = %d\n",j,attributes[j].data->size()) ;
#endif
	    copyAttributes[j].data = new std::vector<float> ;
	    *(copyAttributes[j].data) = *(attributes[j].data) ;
	}
    }

    unsigned int n = coords->size() ;
    for (unsigned int i=1; i<n; i++)
    {
	if (i%2)
	{
#if 0
	    fprintf(stderr,"odd  coord %d gets value of coord %d\n",i, (i+1)/2) ;
#endif
	    (*coords)[i] = (*copyCoords)[(i+1)/2] ;
	    if (norms->size()>0) (*norms)[i] = (*copyNorms)[(i+1)/2] ;
	    if (colors->size()>0) (*colors)[i] = (*copyColors)[(i+1)/2] ;
	    for (unsigned int j=0; j<16; j++)
	    {
		if (attributes[j].size != 0)
		{
		    for (unsigned int k=0; k<attributes[j].size; k++)
		    {
			unsigned int size = attributes[j].size ;
			(*attributes[j].data)[i*size + k] = 
			    (*copyAttributes[j].data)[((i+1)*size)/2 + k] ;
		    }
		}
	    }
	}
	else
	{
#if 0
	    fprintf(stderr,"even coord %d gets value of coord %d\n",i, n-(i/2)) ;
#endif
	    (*coords)[i] = (*copyCoords)[n-(i/2)] ;
	    if (norms->size()>0) (*norms)[i] = (*copyNorms)[n-(i/2)] ;
	    if (colors->size()>0) (*colors)[i] = (*copyColors)[n-(i/2)] ;
	    for (unsigned int j=0; j<16; j++)
	    {
		if (attributes[j].data != 0) 
		{
		    for (unsigned int k=0; k<attributes[j].size; k++)
		    {
			unsigned int size = attributes[j].size ;
			(*attributes[j].data)[i*size + k] = 
			    (*copyAttributes[j].data)[(n*size)-((i*size)/2)+k] ;
		    }
		}
	    }
	}
    }

    for (unsigned int j=0; j<16; j++)
    {
	if (attributes[j].size != 0)
	{
	    delete copyAttributes[j].data ;
	}
    }
}

////////////////////////////////////////////////////////////////////////
bool parseAttribute(std::vector<std::string> vec)
{
    // process previous primitive, if any
    if (!processPrimitive()) return false ;

    if (!iris::IsSubstring("ATTRIBUTE",vec[0],3)) return false ;

    if (vec.size() != 3) return false ;

    int index ;
    if (!iris::StringToInt(vec[1], &index)) return false ;

    int size ;
    if (!iris::StringToInt(vec[2], &size)) return false ;

    if (index<0 || index>15 || size <1 || size>4) return false ;

    if (attributes[index].data != NULL) return false ;  // already have an attribute for this index 

    attributes[index].size = size ;
    attributes[index].data = new std::vector<float> ;

    if (index==0) fprintf(stderr,"savg loader: Warning, using vertex attribute 0, which can conflict with vertex data\n") ;
    else if (index==2) fprintf(stderr,"savg loader: Warning, using vertex attribute 2, which can conflict with normal data\n") ;
    else if (index==3) fprintf(stderr,"savg loader: Warning, using vertex attribute 3, which can conflict with color data\n") ;
    else if (index==8) fprintf(stderr,"savg loader: Warning, using vertex attribute 3, which can conflict with texture data\n") ;

    return true ;
}

////////////////////////////////////////////////////////////////////////
bool parseProgram(std::vector<std::string> vec)
{
    // process previous primitive, if any
    if (!processPrimitive()) return false ;

    if (!iris::IsSubstring("PROGRAM",vec[0],3)) return false ;

    if (vec.size() < 3) return false ;

    program p ;

    if (iris::IsSubstring("VERTEX",vec[1],3)) p.type = osg::Shader::VERTEX ;
    else if (iris::IsSubstring("GEOMETRY",vec[1],3)) p.type = osg::Shader::GEOMETRY ;
    else if (iris::IsSubstring("FRAGMENT",vec[1],3)) p.type = osg::Shader::FRAGMENT ;
    else return false ;

    // look for geomSpec
    std::string::size_type idx = vec[1].find(":") ;
    if (p.type == osg::Shader::GEOMETRY && idx != std::string::npos)
    {
	std::string geomSpec = vec[1].substr(idx+1) ;
	if (geomSpec.size()<3) return false ;

	if (geomSpec[0] == 't' || geomSpec[0] == 'T') p.inputType = GL_TRIANGLES ;
	else if (geomSpec[0] == 'p' || geomSpec[0] == 'P') p.inputType = GL_POINTS ;
	else if (geomSpec[0] == 'l' || geomSpec[0] == 'L') p.inputType = GL_LINES ;
	else return false ;

	if (geomSpec[1] == 't' || geomSpec[1] == 'T') p.outputType = GL_TRIANGLE_STRIP ;
	else if (geomSpec[1] == 'p' || geomSpec[1] == 'P') p.outputType = GL_POINTS ;
	else if (geomSpec[1] == 'l' || geomSpec[1] == 'L') p.outputType = GL_LINE_STRIP ;
	else return false ;
	
	if (!iris::StringToInt(geomSpec.substr(2), &(p.maxVertices))) return false ;
    }
    else
    {
	p.inputType = GL_TRIANGLES ;
	p.outputType = GL_TRIANGLE_STRIP ;
	p.maxVertices = 3 ;
    }

    p.name = vec[2] ;

    unsigned int i = 3 ;
    while (i<vec.size())
    {
	if (i+1>=vec.size()) return false ;
	int index ;
	if (!iris::StringToInt(vec[i], &index)) return false ;
	if (index<0 || index>15) return false ;
	if (p.attributes[index] != "") return false ; 
	p.attributes[index] = vec[i+1] ; 
	i+=2 ;
    }

    programs.push_back(p) ;

    return true ;
}

////////////////////////////////////////////////////////////////////////
void writePrograms(osg::Node *n)
{
    if (programs.size() == 0) return ;

    osg::StateSet *ss = n->getOrCreateStateSet() ;
    osg::Program *p ;
    for (unsigned int i=0; i<programs.size(); i++)
    {
	//fprintf(stderr,"program %d: %s\n",i,programs[i].name.c_str()) ;
	if (i==0)
	{
	    p = new osg::Program ;
	    ss->setAttribute(p) ;
	}
	else
	{
	    p = dynamic_cast<osg::Program*>(ss->getAttribute(osg::StateAttribute::PROGRAM)) ;
	}
	osg::Shader *s = new osg::Shader(programs[i].type) ;

	// geomSpec
	if (programs[i].type == osg::Shader::GEOMETRY)
	{
	    p->setParameter( GL_GEOMETRY_VERTICES_OUT_EXT, programs[i].maxVertices );
	    p->setParameter( GL_GEOMETRY_INPUT_TYPE_EXT, programs[i].inputType);
	    p->setParameter( GL_GEOMETRY_OUTPUT_TYPE_EXT, programs[i].outputType);
	}

	struct stat buf ;
	std::string fullpath ;
	if (stat(programs[i].name.c_str(),&buf) == 0) fullpath = programs[i].name ;
	else
	{
	    // need to get path so can load from data dirs
	    // there's got to be an easier way
	    for (unsigned int j=0; j<shaderpath.size(); j++)
	    {
		fullpath = shaderpath[j] + std::string("/") + programs[i].name ;
		if (stat(fullpath.c_str(),&buf) == 0)
		{
		    break ;
		}
	    }
	}

#if 0
	fprintf(stderr,"fullpath = %s\n",fullpath.c_str()) ;
#endif
	
	s->loadShaderSourceFromFile(fullpath) ;
	p->addShader(s) ;
	for (unsigned int j=0; j<16; j++)
	{
	    if (programs[i].attributes[j] != "") p->addBindAttribLocation(programs[i].attributes[j], j) ;
	}
    }
}

////////////////////////////////////////////////////////////////////////
void setTransparency(osg::StateSet *ss)
{
	ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	ss->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
}
////////////////////////////////////////////////////////////////////////
void printNode(unsigned int level, osg::Node *node)
{
    osg::Group *group = node->asGroup() ;
    osg::Geode *geode = node->asGeode() ;

    if (group && geode) 
    {
	fprintf(stderr,"a node named \"%s\" is both a group and a geode?\n",node->getName().c_str()) ;
	exit(1) ;
    }

    for(unsigned int i=0; i<level; i++) fprintf(stderr,"  ") ;

    if (!group && !geode) 
    {
	fprintf(stderr,"a node named \"%s\" is neither a group and a geode?\n",node->getName().c_str()) ;
    }

    if (group)
    {
	fprintf(stderr,"group node \"%s\" has %d children\n",group->getName().c_str(),group->getNumChildren()) ;
	for (unsigned int i=0; i<group->getNumChildren(); i++)
	{
	    printNode(level+1,group->getChild(i)) ;
	}
    }

    if (geode)
    {
	fprintf(stderr,"geode \"%s\" has %d drawables\n",geode->getName().c_str(),geode->getNumDrawables()) ;
    }
}

////////////////////////////////////////////////////////////////////////
/*
  returns s, but each line of s is indented i "levels"
  puts i*per spaces at beginning of s, then after every \n except the
  one that might be at the end of s
*/
std::string indent(const std::string s, const unsigned int i, 
			     const unsigned int per)
{
  const std::string indent(i*per,' ') ;
  std::string r = indent ;
  for (int i=0; i<s.size()-1; i++)
    {
      r += s[i] ;
      if (s[i] == '\n')
	{
	  r += indent ;
	}
    }
  r += s[s.size()-1] ;

  return r ;
}
