// HEV Things in Motion
//
// reads object and flip position and orientation files
// object position written to <id>-posn.mha
// object quaternion written to <id>-quat.mha
// composite file containing all objects written to <id>.osgt
//
// Marc Olano, olano@nist.gov, 2/12

#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osg/GraphicsContext>
#include <osg/NodeVisitor>
#include <osg/Texture2D>

#include <string>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>

using namespace std;

//////////////////////////////////////////////////////////////////////
// command line arguments
struct Args {
    string error;       // if not empty, an error to report
    string prog;        // program name
    string vert;        // vertex shader
    string frag;        // fragment shader
    string id;          // output ID
    int objectAttrib;       // attribute slot for object number
    int posnSlot, quatSlot; // position and quaternion texture numbers
    ifstream objFile, flipbookFile; // input files

    ostringstream command;  // command line to reproduce

    // intitialize command line arguments
    Args(int argc, char **argv);
};

// parse command line arguments
Args::Args(int argc, char **argv) 
    :  vert("tim.vert"), frag("tim.frag"), objectAttrib(4),
       posnSlot(4), quatSlot(5)
{
    prog = *argv++; --argc;
    command << prog;
    ostringstream errstr;
    errstr 
    << "Usage: " << prog << " [options] objects.timo frames.timf id\n"
    << "Things in Motion. Create id.osg animation\n\n"
    << "Options:\n"
    << "  -v, --vert <shader.vert> (default: " << vert << ")\n"
    << "  -f, --frag <shader.frag> (default: " << frag << ")\n"
    << "  -a, --attrib <objectNumber> (default: " << objectAttrib << ")\n"
    << "  -p, --posn <textureNumber> (default: " << posnSlot << ")\n"
    << "  -q, --quat <textureNumber> (default: " << quatSlot << ")\n";
    error = errstr.str();

    // parse options
    while(argc) {
    // vertex shader option
    if (argc>1 && strcmp(*argv,"-v")==0||strcmp(*argv,"--vert")==0) {
        vert = argv[1];
        argv +=2; argc -=2;
    }

    // fragment shader option
    else if (argc>1 && strcmp(*argv,"-f")==0||strcmp(*argv,"--frag")==0) {
        frag = argv[1];
        argv +=2; argc -=2;
    }

    // change attribute slot
    else if (argc>1 && strcmp(*argv,"-a")==0||strcmp(*argv,"--attrib")==0) {
        sscanf(argv[1],"%i",&objectAttrib);
        argv +=2; argc -=2;
    }

    // change position texture unit
    else if (argc>1 && strcmp(*argv,"-p")==0||strcmp(*argv,"--posn")==0) {
        sscanf(argv[1],"%i",&posnSlot);
        argv +=2; argc -=2;
    }

    // change quaternion texture unit
    else if (argc>1 && strcmp(*argv,"-q")==0||strcmp(*argv,"--quat")==0) {
        sscanf(argv[1],"%i",&quatSlot);
        argv +=2; argc -=2;
    }

    // ask for help
    else if (strcmp(*argv,"-h")==0||strcmp(*argv,"--help")==0)
        return;

    else
        break;
    }
    // add arguments to recorded command
    command << " --vert " << vert;
    command << " --frag " << frag;
    command << " --attrib " << objectAttrib;
    command << " --posn " << posnSlot;
    command << " --quat " << quatSlot;

    // required command arguments
    if (argc != 3) 
    return;

    objFile.open(argv[0]);
    if (! objFile) {
    error = string("Error opening ")+argv[0];
    return;
    }
    command << ' ' << argv[0];

    flipbookFile.open(argv[1]);
    if (! flipbookFile) {
    error = string("Error opening ")+ argv[1];
    return;
    }
    command << ' ' << argv[1];

    id = argv[2];
    command << ' ' << argv[2];

    // no error
    error.clear();
}

//////////////////////////////////////////////////////////////////////
// graphics context setup for command-line OSG app
// modified from osgconv source
class DummyGraphicsContext {
public:
    DummyGraphicsContext() {
    // try to create tiny-pbuffer context
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = 
        new osg::GraphicsContext::Traits;
    traits->x = traits->y = 0;
    traits->width = traits->height = 1;
    traits->windowDecoration = false;
    traits->doubleBuffer = false;
    traits->sharedContext = 0;
    traits->pbuffer = true;
    _gc = osg::GraphicsContext::createGraphicsContext(traits.get());

    // if not successful, try tiny window instead
    if (!_gc) {
        traits->pbuffer=false;
        _gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    }

    // realize context
    if (_gc.valid()) {
        _gc->realize();
        _gc->makeCurrent();
    }
    }

    // report if valid context
    bool valid() const { return _gc.valid() && _gc->isRealized(); }
    
private:
    osg::ref_ptr<osg::GraphicsContext> _gc;
};


//////////////////////////////////////////////////////////////////////
// add object number attribute to existing geometry
class ObjectNumberVisitor : public osg::NodeVisitor {
public:
    ObjectNumberVisitor(unsigned int objNum, unsigned int objectAttrib)
    : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
      _objNum(objNum), _objectAttrib(objectAttrib)
    {}

    virtual void apply(osg::Geode &geode) {
    for(unsigned int i=0; i<geode.getNumDrawables(); ++i) {
        osg::Geometry *geom = 
        dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
        if (geom) {
        osg::FloatArray *a = new osg::FloatArray(1);
        (*a)[0] = _objNum;
        geom->setVertexAttribArray(_objectAttrib, a);
        geom->setVertexAttribBinding(_objectAttrib, 
                         osg::Geometry::BIND_OVERALL);
        }
    }
    };

    virtual void apply(osg::Node &node) { traverse(node); }

private:
    unsigned int _objNum;   // object number to use
    unsigned int _objectAttrib; // vertex attribute index
};

//////////////////////////////////////////////////////////////////////
// object & flipbook data
struct TIMData {
    // public data
    osg::ref_ptr<osg::Group> objects;  // group containing objects
    osg::ref_ptr<osg::Image> posnImage;  // image with position/object/frame
    osg::ref_ptr<osg::Image> quatImage;  // image with quaternion/obj/frame

    // construct data given object and flipbook files
    TIMData(ifstream &objFile, ifstream &flipFile,
        string vertShader, string fragShader,
        unsigned int objectAttrib,
        int posnSlot, int quatSlot);

private:            // internal helpers
    // read object file and create objects group 
    void readObjects(ifstream &objFile, 
             string vertShader, string fragShader,
             unsigned int objectAttrib);

    // read flipFile
    void readFlipbook(ifstream &flipFile);

    // read line, with comments and white space trimmed
    string readTrimmed(ifstream &file);

    // create posnImage and quatImage
    void createTextures(int posnSlot, int quatSlot);

private:
    // flipbook frame data
    struct FlipData {
    float px, py, pz, pw;
    float qx, qy, qz, qw;
    };

    vector<FlipData> flipData;
};

TIMData::TIMData(ifstream &objFile, ifstream &flipFile,
         string vertShader, string fragShader,
         unsigned int objectAttrib,
         int posnSlot, int quatSlot)
{
    readObjects(objFile, vertShader, fragShader, objectAttrib);
    readFlipbook(flipFile);
    createTextures(posnSlot, quatSlot);
}

// integer log base 2, from graphics.stanford.edu/~seander/bithacks.html
unsigned lg2(unsigned v) {
    // one-byte log table
#define LT(n) n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n
    static const char LogTable[256] = {
    -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
    LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
    LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)
    };
#undef LT

    unsigned t, tt;
    if (tt = v>>16)
    return (t = tt>>8) ? 24 + LogTable[t] : 16 + LogTable[tt];
    else
    return (t = v>>8) ? 8 + LogTable[t] : LogTable[v];
}

// read object file list
void TIMData::readObjects(ifstream &objFile, string vert, string frag,
              unsigned int objectAttrib) {
    osg::Program *prog = new osg::Program;
    osg::Shader *vs = new osg::Shader(osg::Shader::VERTEX);

    if (! vert.empty()) {
    string vfile = osgDB::findDataFile(vert);
    if (vfile.empty())
        fprintf(stderr, 
            "cannot find vertex shader \"%s\"\n", vert.c_str());
    else {
        vs->loadShaderSourceFromFile(vfile);
        prog->addShader(vs);
        prog->addBindAttribLocation("object",objectAttrib);
    }
    }
    if (! frag.empty()) {
    string ffile = osgDB::findDataFile(frag);
    if (ffile.empty())
        fprintf(stderr, 
            "cannot find fragment shader \"%s\"\n", frag.c_str());
    else {
            osg::Shader *fs = new osg::Shader(osg::Shader::FRAGMENT);
        fs->loadShaderSourceFromFile(ffile);
        prog->addShader(fs);
    }
    }

    objects = new osg::Group;

    while(objFile) {
    // read line
    string str = readTrimmed(objFile);
    if (str.empty()) continue;

    // add to file list
    osg::ref_ptr<osg::Node> n = osgDB::readNodeFile(str);
    if (! n.get()) {
        fprintf(stderr,"error reading object \"%s\"\n",str.c_str());
        continue;
    }

    // add object number
        // getNumChildren will provide numberer with an object id that n will
        // get when it is added with addChild() below.
    ObjectNumberVisitor numberer(objects->getNumChildren(), objectAttrib);
    n->accept(numberer);

#if 0
        // add vertex shader to existing program on node with specified tim shader.
        osg::StateSet *ss = n->getStateSet();
        if (ss) {
            osg::Program* np = static_cast<osg::Program*>(ss->getAttribute(osg::StateAttribute::PROGRAM));
            if (np) {
                // only replace the vertex shader if one does not already exist
                bool exists = false;
                for (int i = 0; i < np->getNumShaders(); ++i) {
                    if (np->getShader(i)->getType() == osg::Shader::VERTEX) exists = true;
                }
                if (!exists) {
                    np->addShader(vs);
                    np->addBindAttribLocation("object",objectAttrib);
                }
            }
        }
#endif

    // add to collection
    objects->addChild(n);
    }

    // optimize objects
    osgUtil::Optimizer opt;
    opt.optimize(objects);

    // attach shaders
    osg::StateSet *ss = objects->getOrCreateStateSet();
    ss->setAttribute(prog);

    printf("%d objects\n", objects->getNumChildren());
}

void TIMData::readFlipbook(ifstream &flipFile) {

    while (flipFile) {
        // read line
        string str = readTrimmed(flipFile);
        if (str.empty()) continue;

        // parse with defaults
        FlipData d = {0,0,0,1, 0,0,0,1};
        float s=1;
        sscanf(str.c_str(), "%f %f %f %f %f %f %f %f %f",
               &d.px, &d.py, &d.pz, &d.qx, &d.qy, &d.qz, &d.qw, &s, &d.pw);

        // normalize quaternion and fold in scale
        s = sqrtf(fabs(s) / (d.qx*d.qx + d.qy*d.qy + d.qz*d.qz + d.qw*d.qw));
        d.qx *= s; d.qy *= s; d.qz *= s; d.qw *= s;

        // add to frame data
        flipData.push_back(d);
    }
}


// read line, trimming white space and comments
string TIMData::readTrimmed(ifstream &file) {
    // read line
    string str;
    getline(file, str);

    // trim leading spaces
    size_t pos = str.find_first_not_of(" \t");
    if (pos != string::npos) str.erase(0, pos);

    // trim comment
    pos = str.find_first_of('#');
    if (pos != string::npos) str.erase(pos);

    // trim trailing spaces
    pos = str.find_last_not_of(" \t");
    if (pos != string::npos) str.erase(pos+1);

    // nothing left
    if (pos == string::npos) str.erase();
    
    return str;
}


void TIMData::createTextures(int posnSlot, int quatSlot) {
    // Allocate images based on        0-1   4-5
    // power-of-two Morton (or Z)       /  -  / 
    // Morton order is a nested        2-3   6-7
    // order that keeps nearby             /      
    // values nearby in the 2D cache   8-9   C-D
    // For example, here is the         /  -  / 
    // Morton order for 0-15           A-B   E-F
    unsigned bits = lg2(flipData.size())+1;
    unsigned w = 1<<((bits+1)/2), h = 1<<(bits/2);
    posnImage = new osg::Image;
    quatImage = new osg::Image;
    posnImage->allocateImage(w,h,1, GL_RGBA, GL_FLOAT);
    quatImage->allocateImage(w,h,1, GL_RGBA, GL_FLOAT);

    // fill with data in Morton order
    // loop code based on 
    //   http://fgiesen.wordpress.com/2011/01/17/texture-tiling-and-swizzling/
    int ym = 0;
    for(int y=0, ym=0; y<h; ++y, ym = (ym - 0xAAAAAAAA) & 0xAAAAAAAA) {
        for(int x=0, xm=0; x<w; ++x, xm = (xm - 0x55555555) & 0x55555555) {

            int i;
            i = xm+ym;

            FlipData d = {0,0,0,0, 0,0,0,0};
            if (i <= flipData.size())
            d = flipData[i];

            float *posn = (float*)(posnImage->data(x,y));
            posn[0] = d.px; posn[1] = d.py; posn[2] = d.pz; posn[3] = d.pw; 

            float *quat = (float*)(quatImage->data(x,y));
            quat[0] = d.qx; quat[1] = d.qy; quat[2] = d.qz; quat[3] = d.qw; 
        }
    }

    // attach textures to objects node
    osg::StateSet *ss = objects->getOrCreateStateSet();
    osg::Texture2D *posnTex = new osg::Texture2D(posnImage);
    posnTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
    posnTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
    posnTex->setUseHardwareMipMapGeneration(false);
    posnTex->setUnRefImageDataAfterApply(true);
    float *d = (float*)(posnImage->data(0,0));
    printf("%d x %d; [0,0] = (%g,%g,%g,%g)\n", 
       posnImage->s(), posnImage->t(), 
       d[0], d[1], d[2], d[3]);
    ss->setTextureAttributeAndModes(posnSlot,posnTex);
    ss->addUniform(new osg::Uniform("timPosn",posnSlot));

    osg::Texture2D *quatTex = new osg::Texture2D(quatImage);
    quatTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
    quatTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
    quatTex->setUseHardwareMipMapGeneration(false);
    quatTex->setUnRefImageDataAfterApply(true);
    ss->setTextureAttributeAndModes(quatSlot,quatTex);
    ss->addUniform(new osg::Uniform("timQuat",quatSlot));

    // total number of objects and frames
    unsigned int nObjects = objects->getNumChildren();
    unsigned int nFrames = flipData.size()/nObjects;
    ss->addUniform(new osg::Uniform("timFrames", nFrames));
    ss->addUniform(new osg::Uniform("timObjects", nObjects));
    ss->addUniform(new osg::Uniform("frame", 0u));

    printf("%d frames\n", nFrames);
}


// parse arguments and run
int main(int argc, char *argv[]) {
    //////////////////////////////
    // parse command line arguments
    Args args(argc, argv);
    if (! args.error.empty()) {
    fprintf(stderr, "%s\n", args.error.c_str());
    return 1;
    }

    //////////////////////////////
    // create graphics context
    DummyGraphicsContext gc;
    if (! gc.valid()) {
    fprintf(stderr, "Error creating graphics context\n");
    return 1;
    }

    //////////////////////////////
    // read files
    TIMData timData(args.objFile, args.flipbookFile, args.vert, args.frag,
            args.objectAttrib, args.posnSlot, args.quatSlot);

    //////////////////////////////
    // write image files
    timData.posnImage->setFileName(args.id+"-posn.mha");
    timData.quatImage->setFileName(args.id+"-quat.mha");
    osgDB::writeImageFile(*timData.posnImage, args.id+"-posn.mha");
    osgDB::writeImageFile(*timData.quatImage, args.id+"-quat.mha");

    // write out final file
    timData.objects->setName(args.id);
    timData.objects->addDescription(args.command.str());
    osgDB::writeNodeFile(*timData.objects, args.id + ".osg");

    return 0;
}
