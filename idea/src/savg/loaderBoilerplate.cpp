static osgDB::ReaderWriter::ReadResult parseStream(std::istream& fin) ;

// probably hopefully don't need to mess with what's below
class OSGLOADERFILETYPE: public osgDB::ReaderWriter
{
public:
  OSGLOADERFILETYPE() {}
  
  virtual const char* className() { return OSGLOADERSTRING" Pseudo Loader"; }
  
  virtual bool acceptsExtension(const std::string& extension) const
  {
    return osgDB::equalCaseInsensitive(extension,OSGLOADERSTRING);
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
    
    osg::notify( osg::INFO ) << OSGLOADERSTRING" file loader::readNode, compiled on "<< 
      __DATE__  " at "  __TIME__ << std::endl << 
      "   loading file: " << fileName << "foundFile=" << foundFile << std::endl ;
    
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

osgDB::RegisterReaderWriterProxy<OSGLOADERFILETYPE> OSGLOADERFILETYPE_Proxy;
