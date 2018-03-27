// just does a 90 degree pitch to transform Inventor coordinates to OSG
// coordinates.

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

class ReaderWriterIVU : public osgDB::ReaderWriter
{
public:
    ReaderWriterIVU()
    {
        supportsExtension("ivu","Inventor Up pseudo loader");
    }
    
    virtual const char* className() const { return "Inventor Up pseudo-loader" ; }

    virtual bool acceptsExtension(const std::string& extension) const
    {
	return osgDB::equalCaseInsensitive(extension,"ivu");
    }
  
    // this seems to get called if theres's a pseudo loader in the file name, such as foo.ivu.gz
    // if we don't return a valid node then the other readNode is called with the file name
    virtual ReadResult readNode(std::istream& fin, const Options* options) const
    {
	//fprintf(stderr,"\n\n  in ivu read node for a stream\n\n\n") ; fflush(stderr) ;
	osg::Node* node(NULL) ;
	osgDB::ReaderWriter* ivrw = osgDB::Registry::instance()->getReaderWriterForExtension("iv") ;
	if (ivrw == NULL) return ReadResult::FILE_NOT_HANDLED ; 
	ReadResult rr = ivrw->readNode(fin, options) ;
	if (rr.validNode()) node = rr.getNode() ;
	else return ReadResult::FILE_NOT_HANDLED ;

	// pitch it up by 90 degrees
        osg::MatrixTransform *xform = new osg::MatrixTransform(osg::Matrix(osg::Quat(-sqrt(2.0)/2.0, 0.0, 0.0,sqrt(2.0)/2.0))) ;
        xform->setDataVariance(osg::Object::STATIC) ;
        xform->addChild(node) ;
	//fprintf(stderr,"\n\n  normal exit from read node for a stream\n\n\n") ; fflush(stderr) ;
        return xform ;

    }
    
    // this gets called directly if the file ends in ivu
    virtual ReadResult readNode(const std::string& filename, const osgDB::ReaderWriter::Options* options) const
    {

	//fprintf(stderr,"\n\n  in ivu read node for a file: %s\n\n\n",filename.c_str()) ; fflush(stderr) ;

        if (!acceptsExtension(osgDB::getLowerCaseFileExtension(filename))) return ReadResult::FILE_NOT_HANDLED ;

        OSG_INFO << "ReaderWriterROT( \"" << filename << "\" )" << std::endl;

	std::string fn = osgDB::findDataFile(filename, options);
	//printf("  fn = %s\n",fn.c_str()) ;
	if (fn.empty()) return ReadResult::FILE_NOT_FOUND;

	osg::Node* node(NULL);
	osgDB::ifstream fin(fn.c_str()) ;
	ReadResult rr = readNode(fin, options) ;
	if (!rr.validNode()) return ReadResult::FILE_NOT_HANDLED ;
	//fprintf(stderr,"\n\n  normal exit from read node for a file: %s\n\n\n",filename.c_str()) ; fflush(stderr) ;
	return rr.getNode() ;
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

// Add to the Registry
REGISTER_OSGPLUGIN(ivu, ReaderWriterIVU)

