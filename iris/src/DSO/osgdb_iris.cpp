#include <osgDB/ReadFile>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>
#include <osgDB/Export>
#include <osgDB/WriteFile>

#include <dtk.h>
#include <iris/SceneGraph.h>
#include <iris/Utils.h>

///////////////////////////////////////////////////////////////////////////
// return a pointer to a node if all's well, or return NULL ;
static osgDB::ReaderWriter::ReadResult parseStream(std::istream& fin) {


    unsigned int lineCount ;
    osg::Node* node ;
    std::string line ;

    // init data- might have been changed from the last iris file loaded
    lineCount = 0 ;

    node = NULL ;

    // create a SceneGraph object if none already exists
    iris::SceneGraph* isg = iris::SceneGraph::instance() ;

    isg->resetStartModel() ;

    bool ret ;
    while (getline(fin, line)) 
    {
	// reading model file data via STARTMODEL/ENDMODEL ?
	if (isg->getStartModelDataFileStream())
	{
	    // make sure it's not an ENDMODEL command
	    std::vector<std::string> vec = iris::ParseString(line) ;  
	    if (vec.size()>0 && !iris::IsSubstring("endmodeldata", vec[0]))
	    {
		*(isg->getStartModelDataFileStream()) << line << std::endl ;
		continue ;
	    }
	}

	// do variable substitution
	line = isg->substituteVariable(line) ;
	
	//fprintf(stderr,"iris file loader: \"%s\"\n",line.c_str()) ;
	lineCount++ ;
	std::vector<std::string> vec = iris::ParseString(line) ;
	if (vec.size() == 0) continue ;
	// the RETURN command only is used in the iris file loader
	if (vec.size() == 2 && iris::IsSubstring("return", vec[0],3))
	{
	    if (isg->getEcho()) fprintf(stderr,"ECHO: %s\n",line.c_str()) ;
	    node = (osg::Node*) isg->findNode(vec[1]) ;
	    if (!node)
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris file loader: line number %d, can not find node named \"%s\"\n", lineCount, vec[1].c_str()) ;
		ret = false ;
	    }
	    else ret = true ;
	}
	else ret = isg->control(line, vec) ;
	if (!ret) dtkMsg.add(DTKMSG_ERROR, "iris file loader: ignoring invalid command \"%s\" at line number %d\n", line.c_str(), lineCount) ;
    }

    // don't return a NULL pointer, as this usually indicates a problem, and
    // an iris file can run successfully and not return anything
    {
	if (!node) node = new osg::Node ;
    }
    return node ;
}

class irisLoader: public osgDB::ReaderWriter
{
public:
    irisLoader() { }
  
    virtual const char* className() { return "iris File Loader"; }
  
    virtual bool acceptsExtension(const std::string& extension) const
    {
	return osgDB::equalCaseInsensitive(extension,"iris");
    }
  
    // this is called once for each file being loaded
    virtual ReadResult readNode(const std::string& fileName, const Options* options) const
    {
	std::string ext = osgDB::getFileExtension(fileName);
	// return if the file isn't of a type we load
	if (!acceptsExtension(ext))
	    return ReadResult::FILE_NOT_HANDLED;
    
	std::string foundFile = osgDB::findDataFile( fileName, options );
	if (foundFile.empty()) 
	    return ReadResult::FILE_NOT_FOUND;
    
	dtkMsg.add(DTKMSG_INFO, "iris file loader:readNode, compiled on %s at %s. loading file %s\n",__DATE__, __TIME__, fileName.c_str(), foundFile.c_str()) ;
    
	std::ifstream fin(foundFile.c_str());
	if (fin)
	{
	    return readNode(fin, options);
	}

	return ReadResult::FILE_NOT_FOUND;
    }
  
    // read file data from stream- just pass it to user defined function
    virtual ReadResult readNode(std::istream& fin, const Options* options) const
    {
	return parseStream(fin) ;
    }
  
    virtual ReadResult readObject(const std::string& fileName, const Options* opt) const 
    { 
	return readNode(fileName, opt);
    }
  
    virtual ReadResult readObject(std::istream& fin, const Options* opt) const 
    { 
	return readNode(fin, opt); 
    }

};

osgDB::RegisterReaderWriterProxy<irisLoader> iris_Proxy;
