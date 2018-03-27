#include <osgDB/ReadFile>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>
#include <osgDB/Export>
#include <osgDB/WriteFile>

#include <dtk.h>

class sgeLoader: public osgDB::ReaderWriter
{
public:
    sgeLoader() { }
  
    virtual const char* className() { return "sge File Loader"; }
  
    virtual bool acceptsExtension(const std::string& extension) const
    {
	return osgDB::equalCaseInsensitive(extension,"sge");
    }
  
    // this is called once for each file being loaded
    virtual ReadResult readNode(const std::string& fileName, const Options* options) const
    {
	static bool first = true ;
	if (first)
	{
	    dtkMsg.add(DTKMSG_WARNING, "%s not loaded: the sge file loader has been replaced by the IRIS file loader\n",fileName.c_str()) ;
	    first = false ;
	}
	return ReadResult::FILE_NOT_HANDLED ;	
    }
};

osgDB::RegisterReaderWriterProxy<sgeLoader> sge_Proxy;
