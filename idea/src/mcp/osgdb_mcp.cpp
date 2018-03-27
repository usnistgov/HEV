#include <osg/Notify>
#include <osg/Matrix>
#include <osg/MatrixTransform>

#include <osgDB/ReaderWriter>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <stdio.h>

#include <iris.h>

class ReaderWriterMCP : public osgDB::ReaderWriter
{
public:
    ReaderWriterMCP()
    {
        supportsExtension("mcp","hev-masterControlPanel file loader");
    }
    
    virtual const char* className() const { return "hev-masterControlPanel file loader" ; }

    virtual bool acceptsExtension(const std::string& extension) const
    {
	return osgDB::equalCaseInsensitive(extension,"mcp");
    }
  
    // this gets called directly if the file ends in mcp
    virtual ReadResult readNode(const std::string& filename, const osgDB::ReaderWriter::Options* options) const
    {

	//fprintf(stderr,"\n\n  in mcp read node for a file: %s\n\n\n",filename.c_str()) ; fflush(stderr) ;

        if (!acceptsExtension(osgDB::getLowerCaseFileExtension(filename))) return ReadResult::FILE_NOT_HANDLED ;

        OSG_INFO << "ReaderWriterMCP( \"" << filename << "\" )" << std::endl;

	// wait around for the MCP fifo to get created
	char* env = getenv("IRISFLY_MCP_CONTROL_FIFO") ;
	if (!env)
	{
	    dtkMsg.add(DTKMSG_ERROR, "MCP file loader: IRISFLY_MCP_CONTROL_FIFO not defined\n") ;
	    return ReadResult::FILE_NOT_HANDLED;
	}
	iris::SceneGraph* isg = iris::SceneGraph::instance() ;
	std::string command = "after realize wait echo " + filename + " > $IRISFLY_MCP_CONTROL_FIFO" ;
	isg->control(command) ;

	//fprintf(stderr,"\n\n  normal exit from read node for a file: %s\n\n\n",filename.c_str()) ; fflush(stderr) ;

	// don't return a NULL pointer, as this usually indicates a problem, and
	// an iris file can run successfully and not return anything
	return new osg::Node ;
    }

    virtual ReadResult readObject(const std::string& fileName, const Options* opt) const 
    { 
	return readNode(fileName, opt);
    }
    
};

// Add to the Registry
REGISTER_OSGPLUGIN(mcp, ReaderWriterMCP)

