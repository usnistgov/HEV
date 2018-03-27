// Simple MHA/MHD Reader -- Written by Marc Olano 
// Reads a subset of the MHA and MHD file format
// handles 2D and 3D images with byte, short and float data types
// Understands the following tags:
//   NDims = <1-3>
//   DimSize = <int> [<int> [<int>]]
//   Levels = <MIP levels>
//      Non-standard header for MIP pyramid. Should store largest
//      level first, followed by 1/2 size in all dimensions,
//      then 1/4 size in all dimensions, etc. This allows a reader
//      that doesn't understand Levels to read just the top level
//   ElementNumberOfChannels = <1-4>
//   HeaderSize = <int>
//      -1 = compute size and start that far from the end of the file
//   BinaryDataByteOrderMSB = (true|false)
//   ElementByteOrderMSB = (true|false)
//   ElementType = MET_(UCHAR|USHORT|FLOAT)
//   ElementDataFile = (<file>|LOCAL)
//      LOCAL starts on the *next* line of the file, and must be the
//      last header line to appear. Other files are relative to the
//      directory containing the mha file

#include <osg/Image>
#include <osg/Notify>
#include <osg/Endian>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <fstream>
#include <string>

#include <string.h>

using namespace osg;
using namespace std;

// info about element types
struct typeData {
    const char *name;		// mha string name to match
    int size;			// data size in bytes
    GLint ogltype;		// OpenGL pixel type enum
    GLint oglInternal[4];	// OpenGL internal format enum
};
static typeData elementInfo[] = {
    {"MET_UCHAR",  1, GL_UNSIGNED_BYTE,
     {GL_LUMINANCE,GL_LUMINANCE_ALPHA,GL_RGB,GL_RGBA}},
    {"MET_USHORT", 2, GL_UNSIGNED_SHORT,
     {GL_LUMINANCE16, GL_LUMINANCE16_ALPHA16, GL_RGB16, GL_RGBA16}},
    {"MET_FLOAT",  4, GL_FLOAT,
     {GL_LUMINANCE32F_ARB, GL_LUMINANCE_ALPHA32F_ARB, 
      GL_RGB32F_ARB, GL_RGBA32F_ARB}}
};

// OpenGL pixel formats for differing numbers of channels
static GLint oglPixeltype[] = {
    GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA
};

class ReaderWriterMHA : public osgDB::ReaderWriter
{
public:
    virtual const char* className() const { return "MHA Image Reader/Writer"; }
    virtual bool acceptsExtension(const string &extension) const
    {
	return (osgDB::equalCaseInsensitive(extension, "mha") ||
		osgDB::equalCaseInsensitive(extension, "mhd"));
    }


    virtual ReadResult readImage(
	const string &file, const osgDB::ReaderWriter::Options *options) const;

    virtual WriteResult writeImage(
        const Image &image, const string &file, 
        const osgDB::ReaderWriter::Options *) const;
};

ReaderWriterMHA::ReadResult ReaderWriterMHA::readImage(
    const string &file, const osgDB::ReaderWriter::Options *options) const
{
    string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    string fileName = osgDB::findDataFile( file, options );
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

    // Open file.
    ifstream fs(fileName.c_str(), ios::in | ios::binary);
    if (!fs) return ReadResult::ERROR_IN_READING_FILE;


    // read tags
    int nDims = 1;		// number of dimensions
    int dims[4] = {1,1,1,1};// size of each dimension
    int levels = 1;         // number of MIP levels
    int channels = 1;	// channels per element
    int header = 0;         // bytes of header to skip
    bool littleEndian = true; // byte order
    int elementType = 0;	// data type
    string rawfile;         // name of raw data file

    string line;
    while (fs) {
        // read line
        getline(fs, line);

        int pos=0;		// use with %n to recognize line match
        if (sscanf(line.c_str(), "NDims = %d%n", &nDims, &pos), pos)
            continue;

        else if (sscanf(line.c_str(), "DimSize = %n%d %d %d %d", 
                        &pos, &dims[0], &dims[1], &dims[2], &dims[3]), pos)
            continue;

        else if (sscanf(line.c_str(), "Levels = %d%n", &levels, &pos), pos)
            continue;

        else if (sscanf(line.c_str(), "ElementNumberOfChannels = %d%n", 
                        &channels, &pos), pos)
            continue;

        else if (sscanf(line.c_str(), "HeaderSize = %d%n", 
                        &header, &pos), pos)
            continue;

        else if ((sscanf(line.c_str(), "BinaryDataByteOrderMSB = %n", &pos), pos)||
                 (sscanf(line.c_str(), "ElementByteOrderMSB = %n", &pos), pos)) {
            littleEndian = (strncmp(line.c_str() + pos, "false", 4)==0 ||
                            strncmp(line.c_str() + pos, "False", 4)==0);
            continue;
        }

        else if (sscanf(line.c_str(), "ElementType = %n", &pos), pos) {
            elementType = -1;
            for(int i=0; i<sizeof(elementInfo)/sizeof(*elementInfo); ++i) {
                if (strncmp(line.c_str() + pos, elementInfo[i].name,
                            strlen(elementInfo[i].name))==0) {
                    elementType = i;
                    break;
                }
            }

            if (elementType == -1) 
                return ReadResult("ElementType not handled");
        }

        else if (sscanf(line.c_str(), "ElementDataFile = %n", 
                        &pos), pos) {
            rawfile = line.substr(pos, string::npos);
            if (rawfile == "LOCAL")
                break;
            else
                continue;
        }
    }

    // allocate data for all MIP levels
    Image::MipmapDataType mipOffsets(levels-1);
    int x=dims[0], y=dims[1], z=dims[2], w=dims[3];
    unsigned int size = x*y*z*w*channels*elementInfo[elementType].size;
    for(int i=1; i<levels; ++i) {
        x=((x-1)>>1)+1, y=((y-1)>>1)+1, z=((z-1)>>1)+1, w=((w-1)>>1)+1;
        mipOffsets[i-1] = size;
        size += x*y*z*w*channels*elementInfo[elementType].size;
    }

    unsigned char *data = new unsigned char[size];
    if (!data) return ReadResult("Out of memory");

    if (rawfile != "LOCAL") {
        // find raw file in .mha file directory

        string dir = osgDB::getFilePath(fileName);
        if (dir.empty()) dir = string(".");
        string rawpath = dir + '/' + rawfile;

        fs.close();
        fs.open(rawpath.c_str(), ios::in | ios::binary);
        if (! fs)
            return ReadResult(string("Could not open raw file ") + rawpath);
    }

    // header to skip?
    if (header > 0)
        fs.seekg(header, ios::beg);
    else if (header < 0) {	// -1 = *last* size bytes of file
        fs.seekg(0, ios::end); // windows offset is unsigned :(
        header = int(fs.tellg()) - size;
        fs.seekg(header, ios::beg);
    }

    // read data
    fs.read((char*)data, size);

    // swap byte order if necessary
    if ((getCpuByteOrder()==LittleEndian) ^ littleEndian) {
        unsigned int i;
        switch(elementInfo[elementType].size) {
        case 2:
            for(i=0; i<size/2; ++i)
                swapBytes2((char*)data+2*i);
            break;
        case 4:
            for(i=0; i<size/4; ++i)
                swapBytes4((char*)data+4*i);
            break;
        case 8:
            for(i=0; i<size/8; ++i)
                swapBytes8((char*)data+8*i);
            break;
        default:
            break;
        }
    }

    // stuff into an image
    Image* im = new Image();
    im->setFileName(file.c_str());
    im->setImage(dims[0], dims[1], dims[2],
                 elementInfo[elementType].oglInternal[channels-1], 
                 oglPixeltype[channels-1],
                 elementInfo[elementType].ogltype,
                 data,
                 Image::USE_NEW_DELETE);
    if (levels>1) im->setMipmapLevels(mipOffsets);
    return im;
}

ReaderWriterMHA::WriteResult 
ReaderWriterMHA::writeImage(
    const Image &image,
    const string &file, 
    const osgDB::ReaderWriter::Options *) const
{
    string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

    ofstream fs(file.c_str(), ios::out | ios::binary);
    if (!fs) return WriteResult::ERROR_IN_WRITING_FILE;

    fs << "ObjectType = Image\n";

    if (image.r() > 1)
        fs << "NDims = 3\n"
           << "DimSize = "<<image.s()<<' '<<image.t()<<' '<<image.r()<<'\n';
    else if (image.t())
        fs << "NDims = 2\n"
           << "DimSize = "<<image.s()<<' '<<image.t()<<'\n';
    else
        fs << "NDims = 1\n"
           << "DimSize = "<<image.s()<<'\n';

    if (image.isMipmap())
        fs << "Levels = " << image.getNumMipmapLevels() << '\n';

    switch (image.getPixelFormat()) {
    case GL_DEPTH_COMPONENT: 
    case GL_LUMINANCE:
    case GL_ALPHA:
        fs << "ElementNumberOfChannels = 1\n";
        break;
    case GL_LUMINANCE_ALPHA:
        fs << "ElementNumberOfChannels = 2\n";
        break;
    case GL_RGB:
        fs << "ElementNumberOfChannels = 3\n";
        break;
    case GL_RGBA:
        fs << "ElementNumberOfChannels = 4\n";
        break;
    default:
        return WriteResult("Unsupported pixel format");
        break;
    }

    const char *byteOrder = (getCpuByteOrder() == LittleEndian)
        ? "False" : "True";
    switch(image.getDataType()) {
    case GL_FLOAT:
        fs << "ElementType = MET_FLOAT\n"
           << "ElementByteOrderMSB = " << byteOrder << '\n';
        break;
    case GL_SHORT:
        fs << "ElementType = MET_USHORT\n"
           << "ElementByteOrderMSB = " << byteOrder << '\n';
        break;
    default:
        fs << "ElementType = MET_UCHAR\n";
        break;
    }

    // write data
    fs << "ElementDataFile = LOCAL\n";
    fs.write((const char *)image.getDataPointer(), 
             image.getTotalSizeInBytesIncludingMipmaps());

    fs.close();

    return WriteResult::FILE_SAVED;
}

// now register this plugin
REGISTER_OSGPLUGIN(mha,ReaderWriterMHA);
