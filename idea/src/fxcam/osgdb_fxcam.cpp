// OSG pseudo-loader for an "effects camera" for full-screen post-processing
// Given a file name test.fxcam the loader creates:
// - A parent node named "test" (or whatever name you gave for the .fxcam)
//     Add or remove this from the scene graph
// - A node named "pre_test"
//     Anything attached to this node is rendered into color & depth textures
// - A node named "post_test"
//     Anything attached to this node can use the generated color & depth
//     Color is a TextureRectangle on texture unit 0, shader name fxcamColor
//     Depth Is A TextureRectangle On Texture Unit 1, shader name fxcamDepth
//     In addition, a boolean shader variable named fxcam can be used in 
//       shaders to control use of the fxcamColor and fxcamDepth textures
//
// Normally, anything in pre_test is rendered into the color and
// depth textures, and will not visible unless something in post_test
// copys values out of the textures. To make visible, attach the
// fxFrameCam.osg to post_test
//
// -- Written by Marc Olano 

#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <osg/TextureRectangle>
#include <osg/Texture2D>
	
#include <osgDB/ReaderWriter>
#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>

using namespace osg;

class ReaderWriterFxCam : public osgDB::ReaderWriter
{
public:
    virtual const char* className() const { return "fxcam Reader/Writer"; }
    virtual bool acceptsExtension(const std::string& extension) const
    {
	return osgDB::equalCaseInsensitive(extension, "fxcam");
    }


    // loads filenames <nodename>.fxcam
    virtual ReadResult readNode(
	const std::string& file, 
	const osgDB::ReaderWriter::Options* options) const
    {
	// do we handle this file?
	std::string ext = osgDB::getLowerCaseFileExtension(file);
	if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;
	osg::notify(osg::INFO) << "ReaderWriterFxCam( \"" << file 
			       << "\" )\n";

	// strip pseudo-loader extension
	std::string nodeName = file.substr(0,file.rfind('.'));
	osg::notify(osg::INFO) << " nodeName = \"" << nodeName << "\" )\n";
	if (nodeName.empty()) return ReadResult::FILE_NOT_FOUND;

	// create three named nodes
	osg::Group *parentNode = new osg::Group;
	parentNode->setName(nodeName);
	osg::Group *postNode = new osg::Group;
	postNode->setName("post_"+nodeName);

	// Note on texture sizes below: If set the texture size to
	// (0,0), OSG will size the texture to the window size on
	// first render. HOWEVER, the RAVE appears to get the texture
	// size of the console (700 x 700) (the first window) rather
	// than the walls (1024 x 1024). I've decided to just make
	// each texture (1920 x 1200). That's big enough for an HD
	// image (1920x1080), and fullscreen on one window of my
	// desktop (1600x1200), though not both displays of my desktop
	// (3200x1200).

	// create color textures
	osg::TextureRectangle* colorTex = new osg::TextureRectangle;
	colorTex->setName("colorTex");
	colorTex->setTextureSize(1920,1200);
	colorTex->setResizeNonPowerOfTwoHint(false);
	colorTex->setFilter(osg::TextureRectangle::MIN_FILTER,
			    osg::TextureRectangle::NEAREST);
	colorTex->setFilter(osg::TextureRectangle::MAG_FILTER,
			    osg::TextureRectangle::NEAREST);
	colorTex->setInternalFormat(GL_RGBA);

	// create depth texture
	osg::TextureRectangle* depthTex = new osg::TextureRectangle;
	depthTex->setName("depthTex");
	depthTex->setTextureSize(1920,1200);
	depthTex->setResizeNonPowerOfTwoHint(false);
	depthTex->setFilter(osg::TextureRectangle::MIN_FILTER,
			    osg::TextureRectangle::NEAREST);
	depthTex->setFilter(osg::TextureRectangle::MAG_FILTER,
			    osg::TextureRectangle::NEAREST);
	depthTex->setInternalFormat(GL_DEPTH_COMPONENT);

	// use these as textures in postNode
	osg::StateSet *ss = postNode->getOrCreateStateSet();
	ss->setTextureAttribute(0,colorTex);
	ss->setTextureAttribute(1,depthTex);
	ss->addUniform(new osg::Uniform("fxcamColor",0));
	ss->addUniform(new osg::Uniform("fxcamDepth",1));
	ss->addUniform(new osg::Uniform("fxcam",true));

	// create camera using these textures
	osg::Camera *cam = new osg::Camera();
	cam->setName("pre_"+nodeName);
	cam->setClearColor(osg::Vec4(.5,.5,.5,0));
	cam->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cam->setRenderOrder(osg::Camera::PRE_RENDER);
	cam->setReferenceFrame(osg::Camera::RELATIVE_RF);
	cam->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

	cam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	cam->attach(osg::Camera::COLOR_BUFFER, colorTex);
	cam->attach(osg::Camera::DEPTH_BUFFER, depthTex);

	// construct structure	       // Node
	parentNode->addChild(cam);     //  +- cam
	parentNode->addChild(postNode);//  \- post_Node

	return parentNode;
    }
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterFxCam> g_readerWriter_FxCam_Proxy;
