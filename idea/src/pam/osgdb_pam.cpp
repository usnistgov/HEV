// Simple OSG PAM Reader
// see pam(5) for format

#include <osg/Image>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <fstream>
#include <string>

#include <string.h>

// OpenGL pixel formats for differing numbers of channels
static GLint oglPixeltype[] = {
    GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA
};

class ReaderWriterPAM : public osgDB::ReaderWriter {
public:
    virtual const char* className() const { return "PAM Image Reader/Writer"; }
    virtual bool acceptsExtension(const std::string& extension) const {
        return osgDB::equalCaseInsensitive(extension, "pam");
    }

    virtual ReadResult
    readImage(const std::string& file,
              const osgDB::ReaderWriter::Options* options) const;

    virtual WriteResult writeImage(const osg::Image& image,
                                   const std::string& file,
                                   const osgDB::ReaderWriter::Options*) const;
};

ReaderWriterPAM::ReadResult
ReaderWriterPAM::readImage(const std::string& file,
                           const osgDB::ReaderWriter::Options* options) const {
    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    std::string fileName = osgDB::findDataFile(file, options);
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

    std::ifstream fs(fileName.c_str(), std::ios::in | std::ios::binary);
    if (!fs) return ReadResult::ERROR_IN_READING_FILE;

    int width;
    int height;
    int depth;
    int maxval;

    std::string line;
    while (fs) {
        std::getline(fs, line);

        int pos = 0; // use with %n to recognize line match
        if (sscanf(line.c_str(), "P7%n", &pos), pos) {
            continue;
        } else if (sscanf(line.c_str(), "#%n", &pos), pos) {
            continue;
        } else if (sscanf(line.c_str(), "WIDTH %d%n", &width, &pos), pos) {
            continue;
        } else if (sscanf(line.c_str(), "HEIGHT %d%n", &height, &pos), pos) {
            continue;
        } else if (sscanf(line.c_str(), "DEPTH %d%n", &depth, &pos), pos) {
            continue;
        } else if (sscanf(line.c_str(), "MAXVAL %d%n", &maxval, &pos), pos) {
            continue;
        } else if (sscanf(line.c_str(), "TUPLTYPE %n", &pos), pos) {
            // TODO: implement TUPLTYPE ??
            continue;
        } else if (sscanf(line.c_str(), "ENDHDR%n", &pos), pos) {
            break;
        }
    }

    unsigned int size = width * height * depth;
    unsigned char* data = new unsigned char[size];
    if (!data) return ReadResult("Out of memory");

    fs.read((char*)data, size);

    osg::Image* im = new osg::Image();
    im->setFileName(file.c_str());
    im->setImage(width, height, 1, oglPixeltype[depth - 1],
                 oglPixeltype[depth - 1], GL_UNSIGNED_BYTE, data,
                 osg::Image::USE_NEW_DELETE);
    return im;
}

ReaderWriterPAM::WriteResult
ReaderWriterPAM::writeImage(const osg::Image& image, const std::string& file,
                            const osgDB::ReaderWriter::Options*) const {
    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

    if (image.r() > 1 || !image.t() || image.isMipmap()) {
        return WriteResult("Unsupported image dimensions");
    }

    std::ofstream fs(file.c_str(), std::ios::out | std::ios::binary);
    if (!fs) return WriteResult::ERROR_IN_WRITING_FILE;

    fs  << "P7\n"
        << "WIDTH " << image.s() << "\n"
        << "HEIGHT " << image.t() << "\n";

    switch (image.getPixelFormat()) {
    case GL_DEPTH_COMPONENT:
    case GL_LUMINANCE:
    case GL_ALPHA: fs << "DEPTH 1\n"; break;
    case GL_LUMINANCE_ALPHA: fs << "DEPTH 2\n"; break;
    case GL_RGB: fs << "DEPTH 3\n"; break;
    case GL_RGBA: fs << "DEPTH 4\n"; break;
    default: return WriteResult("Unsupported pixel format"); break;
    }

    switch (image.getDataType()) {
    case GL_FLOAT:
    case GL_SHORT: return WriteResult("Unsupported data type"); break;
    default: fs << "MAXVAL 255\n"; break;
    }

    switch (image.getPixelFormat()) {
    case GL_DEPTH_COMPONENT: fs << "TUPLTYPE DEPTH\n"; break;
    case GL_LUMINANCE: fs << "TUPLTYPE LUMINANCE\n"; break;
    case GL_ALPHA: fs << "TUPLTYPE ALPHA\n"; break;
    case GL_LUMINANCE_ALPHA: fs << "TUPLTYPE LUMINANCE_ALPHA\n"; break;
    case GL_RGB: fs << "TUPLTYPE RGB\n"; break;
    case GL_RGBA: fs << "TUPLTYPE RGB_ALPHA\n"; break;
    default: return WriteResult("Unsupported pixel format"); break;
    }

    fs.write((const char*)image.getDataPointer(),
             image.getTotalSizeInBytesIncludingMipmaps());
    fs.close();

    return WriteResult::FILE_SAVED;
}

// now register this plugin
REGISTER_OSGPLUGIN(png, ReaderWriterPAM);
