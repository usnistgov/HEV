/*

  a DSO which grabs the contents of the OpenGL buffer, either once or every
  frame.

  the OpenGL buffer is tied to a iris::Pane object. Multiple Pane objects
  can be grabbed per frame.

  the DSO reads control commands, all of which begin with "DSO frameGrabber"

   - FRAMERATE t

     how many frames to grab per second- default is 30. Zero means every
     frame regardless of the frame rate of the graphics

  -  SNAP pane ...

     grab the buffer once for every pane listed

  -  START pane ...

     grab the buffer every frame for every pane listed

  -  STOP [ pane ... ]

     stop grabbing frames for every pane listed, or all panes if none listed

  -  PREFIX string (default "/tmp/frameGrabber") 

  When you snap a pane, the default file name is:
    /tmp/frameGrabber-${USER}-snap-pane-N.pam

  When you make a movie, the default file name is:
    /tmp/frameGrabber-${USER}-movie-pane-N.pam

  where N is an integer that increments every frame.

  To get a list of the pane names, use the "QUERY fifo PANES" control command

  To quickly view a movie, use the ImageMagick animate command- 
  for example, using the defaults and the perspective pane:
    animate /tmp/frameGrabber-${USER}-movie-perspective-*.pam

  To make an animated GIF file for a web page, use the ImageMagick 
  convert command, for example:

  convert -delay 20 -loop 0 /tmp/frameGrabber-kelso-movie-perspective-*.pam /tmp/foo.gif

  To see what it looks like, use the URL file:///tmp/foo.gif
 */
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

#include <osg/BufferObject>
#include <osg/GLExtensions>
#include <osgDB/WriteFile>
#include <OpenThreads/Thread>
#include <OpenThreads/ReadWriteMutex>

#include <iris/SceneGraph.h>
#include <iris/Augment.h>
#include <iris/Utils.h>
#include <iris/Pane.h>
#include <iris/Window.h>
#include <iris/Utils.h>

// evil global variable 
static bool active = true ;

// tell the pane callback if grabbing the image or not, and how often
enum GrabType { NONE, SNAP, MOVIE } ;
enum GrabMode { MONO, STEREO };

// indicates whether to assign numbers to grabbed data based on
// the osg frame number or based on a sequential scheme
enum NumberingType { FRAMENUMBER, SEQUENTIAL } ;

// dynamic data for every pane to be grabbed- control sets this
// constant data is passed in via the WindowCaptureCallback constructor
struct CallbackData
{
    // what to do - control sets this
    GrabType grabType ;
    GrabMode grabMode ;
    // for a movie, how often to grab
    double frameTime ;
    NumberingType numberingMode;
    unsigned int nextSeqNum;
    // prefix and pane name- control sets these
    std::string prefix ;
    std::string paneName ;
    bool stereo;  // the associated window is stereo
    // the callback creates one of these and adds to the vector
    std::string filesnapName ;
    // reference time of last movie frame grabbed
    double lastRefTime;
} ;

// do this so this DSO's callbacks don't interfere with other DSO's
namespace frameGrabber
{

    void writePAM(unsigned char* pixels, int width, int height,
                  std::string const& FN)
    {
        FILE* fh = fopen(FN.c_str(), "w") ;
        if (!fh) return ;

        fprintf(fh, "P7\nWIDTH %d\nHEIGHT %d\nDEPTH 4\nMAXVAL 255\n"
                "TUPLTYPE RGB_ALPHA\nENDHDR\n", width, height) ;

        // we assume that the grabbed framebuffer is bottom-up and needs
        // to be vertically flipped for the PAM format.
        size_t const stride=width*4 ;
        for (int i=height-1; i>=0; --i) {
            fwrite(pixels + i*stride, sizeof(unsigned char), stride, fh) ;
        }

        fclose(fh) ;
    }

    ////////////////////////////////////////////////////////////////////////
    // generate a file name from a GrabData structure
    std::string getFilename(CallbackData const& cbd, int count, const char *fnTag)
    {
        //
        // If the the filesnapName and the fnTag are both non-empty,
        // then we're going to try to insert the fnTag right before
        // the extension.
        //
        // This whole section seems overly complicated, but it doesn't
        // need to be elegant or fast.
        //
        if (cbd.filesnapName != "")
        {
            if (fnTag != NULL)
            {
                if (fnTag[0] != 0)
                {
                    int extPos = cbd.filesnapName.find_last_of ('.');
                    if (extPos == 0)
                    {
                        if (fnTag[0] == '.')
                        {
                            // remove the leading . if it starts the name
                            fnTag++;
                        }
                        // filesnap name is nothing but extension
                        return fnTag + cbd.filesnapName;
                    }
                    else if (extPos >= cbd.filesnapName.length())
                    {
                        // there's no dot in the filesnapName; assume it's 
                        // an extension
                        if (fnTag[0] == '.')
                        {
                            // remove the leading . if it starts the name
                            fnTag++;
                        }
                        // filesnap name is nothing but extension with no dot
                        return fnTag + ("." + cbd.filesnapName);
                    }

                    // we found the position of the extension; insert fnTag
                    return cbd.filesnapName.substr (0, extPos-1) + 
                                fnTag + 
                                cbd.filesnapName.substr (extPos);

                }
            }

            // we get here when the filesnapName is non-empty and 
            // the tag is empty
            return cbd.filesnapName ;
        }


        // We get here when filesnapName is empty, so we construct a name
        // from scratch.
        char fn[PATH_MAX];
        snprintf(fn, PATH_MAX, "%s-%s-%s-%s-%6.6d%s%s.pam", cbd.prefix.c_str(),
            getenv("USER"), (cbd.grabType==MOVIE)?"movie":"snap",
            cbd.paneName.c_str(), count, (fnTag==NULL)?"":"-",
            (fnTag==NULL)?"":fnTag) ;
        return fn;
    }  // end of getFilename


    ////////////////////////////////////////////////////////////////////////
    // lifted from the osgautocapture example
    class WindowCaptureCallback : public osg::Camera::DrawCallback
    {
    public:
        WindowCaptureCallback (int x, int y,
                               int width, int height,
                               osg::GLBufferObject::Extensions* ext,
                               CallbackData& callbackData ):
            _isg(iris::SceneGraph::instance()), _callbackData(callbackData),
            _x(x), _y(y), _width(width), _height(height),
            _framebufferData (new unsigned char[_width*_height*4]),
            _ext(ext), _currPbo(1) {
#if 0
            if (_ext->isPBOSupported())
            {
                dtkMsg.add (DTKMSG_NOTICE, "iris::frameGrabber using PBOs\n") ;

                _ext->glGenBuffers (2, &_pboIds[0]) ;
                GLenum err = glGetError ();
                if (err != GL_NO_ERROR || !_ext->glIsBuffer(_pboIds[0]) ||
                    !_ext->glIsBuffer(_pboIds[1]))
                {
                    dtkMsg.add (DTKMSG_ERROR,
                                "iris::frameGrabber: unable to create PBOs\n");
                }

                _ext->glBindBuffer (GL_PIXEL_PACK_BUFFER, _pboIds[0]) ;
                _ext->glBufferData (GL_PIXEL_PACK_BUFFER,
                                    _width*_height*4*sizeof(GLubyte), 0,
                                    GL_STREAM_READ) ;
                err = glGetError ();
                if (err != GL_NO_ERROR)
                {
                    dtkMsg.add (DTKMSG_ERROR,
                                "iris::frameGrabber: unable to allocate PBO\n");
                }

                _ext->glBindBuffer (GL_PIXEL_PACK_BUFFER, _pboIds[1]) ;
                _ext->glBufferData (GL_PIXEL_PACK_BUFFER,
                                    _width*_height*4*sizeof(GLubyte), 0,
                                    GL_STREAM_READ) ;
                err = glGetError ();
                if (err != GL_NO_ERROR)
                {
                    dtkMsg.add (DTKMSG_ERROR,
                                "iris::frameGrabber: unable to allocate PBO\n");
                }

                _ext->glBindBuffer (GL_PIXEL_PACK_BUFFER, 0) ;
            }
            else
            {
                dtkMsg.add (DTKMSG_WARNING,
                            "iris::frameGrabber NOT using PBOs\n") ;
            }
#endif
        }

        ~WindowCaptureCallback () {
            _ext->glDeleteBuffers(2, &_pboIds[0]);
            delete[] _framebufferData;
        }


        ////////////////////////////////////////////////////////////////////////
        // what gets called every frame
        virtual void operator () (osg::RenderInfo& renderInfo) const
        {
            if (!active) return ;
            if (_callbackData.grabType == NONE) return ;

            osg::FrameStamp const* fs = _isg->getViewer()->getFrameStamp() ;
            int const fn = fs->getFrameNumber() ;
            double const rt = fs->getReferenceTime() ;
            double const d = rt - _callbackData.lastRefTime ;

            if (_callbackData.grabType == SNAP) _callbackData.grabType = NONE ;
            else if (_callbackData.frameTime <= d) _callbackData.lastRefTime = rt ;
            else return ; // not a snap and movie framerate not yet met

            int count = fn ;
            if (_callbackData.numberingMode == SEQUENTIAL)
            {
                count = (_callbackData.nextSeqNum)++ ;
            }

            // Note that we had problems with the grab of the
            // GL_BACK_RIGHT buffer; it always came out black.
            // Note sure why, but the result is that we use 
            // the FRONT buffers for stereo grabs.
            // To keep parity, mono grabs use GL_FRONT.
            if (_callbackData.grabMode == MONO)
            {
#if 0
                if (_ext->isPBOSupported())
                {
                    const_cast<int&>(_currPbo) = (_currPbo + 1) % 2 ; // sigh
                    int const nextPbo = (_currPbo + 1) % 2 ;

                    glReadBuffer (GL_FRONT) ;

                    _ext->glBindBuffer (GL_PIXEL_PACK_BUFFER, _pboIds[_currPbo]) ;
                    glReadPixels (_x, _y, _width, _height, GL_RGBA,
                                  GL_UNSIGNED_BYTE, 0) ;

                    _ext->glBindBuffer (GL_PIXEL_PACK_BUFFER, _pboIds[nextPbo]) ;
                    GLubyte* ptr =
                        (GLubyte*) _ext->glMapBuffer (GL_PIXEL_PACK_BUFFER,
                                                      GL_READ_ONLY) ;

                    if (ptr)
                    {
                        dtkMsg.add( DTKMSG_ERROR,
                                    "iris::frameGrabber: writing PAM\n") ;
                        writePAM (ptr, _width, _height,
                                  getFilename(_callbackData, count, NULL)) ;
                        _ext->glUnmapBuffer (GL_PIXEL_PACK_BUFFER);
                    }
                    else
                    {
                        dtkMsg.add( DTKMSG_ERROR,
                                    "iris::frameGrabber: unable to map PBO\n") ;
                    }

                    _ext->glBindBuffer (GL_PIXEL_PACK_BUFFER, 0) ;
                }
                else
                {
#endif
                    glReadBuffer (GL_FRONT) ;
                    glReadPixels (_x, _y, _width, _height, GL_RGBA,
                                  GL_UNSIGNED_BYTE, _framebufferData) ;
                    writePAM (_framebufferData, _width, _height,
                              getFilename(_callbackData, count, NULL)) ;
#if 0
                }
#endif
            }
            else
            {
                glReadBuffer (GL_FRONT_LEFT) ;
                glReadPixels (_x, _y, _width, _height, GL_RGBA,
                              GL_UNSIGNED_BYTE, _framebufferData) ;
                writePAM(_framebufferData, _width, _height,
                         getFilename(_callbackData, count, ".left")) ;

                glReadBuffer (GL_FRONT_RIGHT) ;
                glReadPixels (_x, _y, _width, _height, GL_RGBA,
                              GL_UNSIGNED_BYTE, _framebufferData) ;
                writePAM(_framebufferData, _width, _height,
                         getFilename(_callbackData, count, ".right")) ;
            }
        }   // end of virtual void operator () (osg::RenderInfo& ) const

    protected:
        iris::SceneGraph*           _isg ;
        CallbackData&               _callbackData ;
        mutable OpenThreads::Mutex  _mutex ;

        int _x ;
        int _y ;
        int _width ;
        int _height ;
        unsigned char* _framebufferData;

        osg::GLBufferObject::Extensions* _ext;
        GLuint _pboIds[2];
        int _currPbo;

    }; // end of class WindowCaptureCallback

    ////////////////////////////////////////////////////////////////////////
    class frameGrabber : public iris::Augment
    {
    public:
        frameGrabber():iris::Augment("frameGrabber") 
        {

            setDescription (
                "frameGrabber- grabs and stores OpenGL read buffers") ;
 
            // default prefix and extension, without grabType, 
            // pane name, frame rate and frame number
            _prefix = "/tmp/frameGrabber" ;
            _frameTime = 1.0/30.0 ;
            _numberingMode = SEQUENTIAL;

            // dtkAugment::dtkAugment() will not validate the object
            validate() ;
        } ;



        ////////////////////////////////////////////////////////////////////////
        CallbackData* findCallbackData(std::string paneName, bool spew = false)
        {
            for (unsigned int i=0; i<_callbackData.size(); i++)
            {
                if (_callbackData[i]->paneName == paneName) return 
                                                             _callbackData[i] ;
            }
            if (spew) dtkMsg.add(DTKMSG_ERROR, 
                        "iris::frameGrabber: no data associated with pane %s\n",
                        paneName.c_str()) ;
            return NULL ;
        } ;

        ////////////////////////////////////////////////////////////////////////
        // return a vector of calbackData pointers, either for the panes
        // given by name, or for all panes with callbacks if none given
        // parameter vec comes from the control command, so ignore vec[0]
        std::vector<CallbackData*> 
        getCallbackDataVector (std::vector<std::string> vec)
        {
            if (vec.size() == 1) return _callbackData ; // return all
            else
            {
                std::vector<CallbackData*> cbds ;
                for (unsigned int i=1; i<vec.size(); i++)
                {
                    CallbackData* cbd = findCallbackData(vec[i],true) ;
                    if (cbd) cbds.push_back(cbd) ;
                }
                return cbds ;
            }
        }


        ///////////////////////////////////////////////////////////////////////
        // The setNumber function handles the SEQNUM and FRAMENUM command
        bool setNumbering ( NumberingType nt, 
                            const std::vector<std::string>& vec)
        {
            _numberingMode = nt;

            std::vector<CallbackData*> cbds = getCallbackDataVector(vec) ;
            std::string panes ;
            for (unsigned int i=0; i<cbds.size(); i++)
            {
                panes += cbds[i]->paneName + " " ;
                // If we're currently grabbing a movie, don't do it.
                if (cbds[i]->grabType == MOVIE)
                {
                    dtkMsg.add(DTKMSG_WARNING, 
                      "iris::frameGrabber: "
                      "pane %s is currently grabbing a movie sequence; "
                      "command %s is ignored.\n",
                      cbds[i]->paneName.c_str(), vec[0].c_str()) ;
                }
                else
                {
                    cbds[i]->numberingMode = _numberingMode;
                }

            }

            return true ;
        } // end of setNumbering


        bool setSeqStartNum ( const std::vector<std::string>& vec )
        {
            int startNum;
            if ( ! iris::StringToInt (vec[1], &startNum) )
            {
                dtkMsg.add (DTKMSG_ERROR, 
                  "iris::frameGrabber: invalid frame number: %s\n", 
                  vec[1].c_str() ) ;
                return false;
            }

            std::vector<std::string> pVec = vec;
            pVec.erase (pVec.begin());

            std::vector<CallbackData*> cbds = getCallbackDataVector(pVec) ;
            std::string panes ;
            for (unsigned int i=0; i<cbds.size(); i++)
            {
                panes += cbds[i]->paneName + " " ;
                // If we're currently grabbing a movie, don't do it.
                if (cbds[i]->grabType == MOVIE)
                {
                    dtkMsg.add(DTKMSG_WARNING, 
                      "iris::frameGrabber: "
                      "pane %s is currently grabbing a movie sequence; "
                      "command %s is ignored.\n",
                      cbds[i]->paneName.c_str(), vec[0].c_str()) ;
                }
                else
                {
                    cbds[i]->nextSeqNum = startNum;
                    if (cbds[i]->numberingMode != SEQUENTIAL)
                    {
                        dtkMsg.add(DTKMSG_WARNING, 
                          "iris::frameGrabber: "
                          "Setting next sequential number  of pane %s to %d, "
                          "but pane is not in sequential numbering mode.\n",
                          cbds[i]->paneName.c_str(), startNum);
                    }
                }

            }

            dtkMsg.add(DTKMSG_NOTICE, "iris::frameGrabber: %s panes: %s\n",
                vec[0].c_str(), panes.c_str()) ;

            return true ;
        } // end of setSeqStartNum



        ////////////////////////////////////////////////////////////////////////

        std::vector<std::string> getPaneNameList ()
        {

            std::vector<std::string> paneNames;
            std::vector<osg::ref_ptr<iris::Pane> > panes = 
                                        iris::Pane::getPaneList() ;

            for (unsigned int i=0; i<panes.size(); i++)
            {
                if (panes[i]->getName())
                {
                    if (panes[i]->getActive()) 
                    {
                        paneNames.push_back (std::string(panes[i]->getName()));
                    }
                    else
                    {
                        dtkMsg.add (DTKMSG_INFO, 
                          "iris::frameGrabber: skipping inactive pane \"%s\"\n",
                          panes[i]->getName()) ;

                    }
                }
                else
                {
                    dtkMsg.add (DTKMSG_WARNING, 
                      "iris::frameGrabber: Pane with no name has been "
                      "encounter,  Pane will be ignored.\n");
                }
            } // end of loop of panes

        return paneNames;
        }  // end of getPaneNameList





        ////////////////////////////////////////////////////////////////////////
        bool control ( const std::string& line, 
                       const std::vector<std::string>& vec )
        {
            ////////////////
            // set filenane prefix
            if (vec.size() == 2 && iris::IsSubstring("prefix",vec[0],3)) 
            {
                if (!active) return true ;
                _prefix = vec[1] ;
                dtkMsg.add(DTKMSG_NOTICE, 
                    "iris::frameGrabber: prefix %s\n",_prefix.c_str()) ;
                return true ;
            }
            ////////////////
            // set frame rate
            else if (vec.size() == 2 && iris::IsSubstring("framerate",vec[0],3))
            {
                if (!active) return true ;
                double fr = -1 ;
                if (iris::StringToDouble(vec[1], &fr))
                {
                    if (fr > 0.f ) _frameTime = 1.0/fr ;
                    else if (fr == 0) _frameTime = 0.0 ;
                    else 
                    {
                        dtkMsg.add(DTKMSG_ERROR, 
                            "iris::frameGrabber: invalid frame rate\n") ;
                        return false ;
                    }
                    dtkMsg.add(DTKMSG_NOTICE, 
                        "iris::frameGrabber: frame rate %f\n",fr) ;
                }
                return true ;
            }
            else if (vec.size() > 0 && iris::IsSubstring("SEQNUM",vec[0],4) )
            {
                return setNumbering ( SEQUENTIAL, vec );
            }
            else if (vec.size() > 0 && iris::IsSubstring("FRAMENUM",vec[0],6) )
            {
                return setNumbering ( FRAMENUMBER, vec );
            }
            else if (vec.size() > 0 && iris::IsSubstring("SETSEQNUM",vec[0],7) )
            {
                return setSeqStartNum ( vec );
            }
            ////////////////
            // snap/start pane filename
            else if (vec.size() > 0 && 
                      ( iris::IsSubstring("snap",vec[0],3) || 
                        iris::IsSubstring("filesnap",vec[0],3) ||
                        iris::IsSubstring("start",vec[0],3) ||
                        iris::IsSubstring("snapstereo",vec[0],3) || 
                        iris::IsSubstring("filesnapstereo",vec[0],3) ||
                        iris::IsSubstring("startstereo",vec[0],3)
                      ) ) 
            {
                // we know it's one of the six valid snap/start commands
                int iPane;
                if (!active) return true ;

                std::string panes ;
                GrabType gt ;
                GrabMode grabMode;
                std::string filesnapName ;

                if ( iris::IsSubstring("snap",vec[0],3) )
                {
                    gt = SNAP ;
                    grabMode = MONO;
                    filesnapName  = "";
                    iPane = 1;
                }
                else if ( iris::IsSubstring("snapstereo",vec[0],3) ) 
                {
                    gt = SNAP ;
                    grabMode = STEREO;
                    filesnapName  = "";
                    iPane = 1;
                }
                else if ( iris::IsSubstring("filesnap",vec[0],3) ) 
                {
                    if (vec.size() < 2)
                    {
                        dtkMsg.add(DTKMSG_WARNING, 
                            "iris::frameGrabber: FILESNAP command issued "
                            "with no file name specified; command ignored.\n");
                        return false;
                    }
                    gt = SNAP ;
                    grabMode = MONO;
                    filesnapName  = vec[1];
                    iPane = 2;
                }
                else if ( iris::IsSubstring("filesnapstereo",vec[0],3) )
                {
                    if (vec.size() < 2)
                    {
                        dtkMsg.add(DTKMSG_WARNING, 
                            "iris::frameGrabber: FILESNAPSTEREO command issued "
                            "with no file name specified; command ignored.\n");
                        return false;
                    }
                    gt = SNAP ;
                    grabMode = STEREO ;
                    filesnapName  = vec[1];
                    iPane = 2;
                }
                else if ( iris::IsSubstring("start",vec[0],3) )
                {
                    gt = MOVIE ;
                    grabMode = MONO;
                    filesnapName  = "";
                    iPane = 1;
                }
                else // startstereo
                {
                    gt = MOVIE ;
                    grabMode = STEREO;
                    filesnapName  = "";
                    iPane = 1;
                }


                std::vector<std::string> paneList;
                std::vector<std::string> const * pList;

                if ( iPane < vec.size () )
                {
                    // the command specifies pane(s)
                    pList = &vec;
                }
                else
                {
                    // the command does not specify pane(s)
                    // so get the full list of pane names
                    paneList = getPaneNameList ();
                    pList = &paneList;
                    iPane = 0;
                }



                for (unsigned int i=iPane; i<pList->size(); i++)
                {

                    std::string paneName = (*pList)[i] ;


                    // does the pane already have a callback?
                    CallbackData* cbd = findCallbackData(paneName) ;

                    if (!cbd) //nope- need to create one
                    {
                        iris::Pane* pane = 
                                iris::Pane::findPane(paneName.c_str()) ;

                        if (!pane) return false ;
                        if (!(pane->getActive())) 
                        {
                            dtkMsg.add(DTKMSG_WARNING, 
                                "iris::frameGrabber: pane %s is not active; "
                                "you will probably get a black image.\n",
                                paneName.c_str()) ;
                        }
                        iris::Window* window = pane->getWindow() ;
                        if (!window) return false ;

                        // get a new one of these for every new pane
                        cbd = new CallbackData ;
                        _callbackData.push_back(cbd) ;

                        bool inStereo = window->getTraits()->quadBufferStereo;

                        osg::GLBufferObject::Extensions* ext =
                            osg::GLBufferObject::getExtensions(
                                window->getGraphicsContext()->getState()->getContextID(),
                                true);
                        if (ext == NULL) {
                          dtkMsg.add(
                              DTKMSG_ERROR,
                              "iris::frameGrabber: extension loader is null");
                        }

                        int x ;
                        int y ;
                        int width ;
                        int height ;
                        pane->getViewport(&x, &y, &width, &height) ;
                        if ( (pane->getCameraList()).size() > 0 )
                        {
                            (pane->getCameraList())[0]->setFinalDrawCallback (
                              new WindowCaptureCallback (x, y, width, height,
                                                         ext, *cbd ) );
                        }
                        else
                        {
                            // no camera!
                            dtkMsg.add(DTKMSG_ERROR, 
                                "iris::frameGrabber: pane %s has no camera; "
                                "no image will be produced.\n",
                                paneName.c_str()) ;
                           // do we need to do something about cbd?
                        }

                        cbd->stereo = inStereo;

                        cbd->nextSeqNum = 0;
                        cbd->numberingMode = _numberingMode;
                        cbd->prefix = _prefix ;
                        cbd->paneName = paneName ;

                    }  // end of clause for creating new cbd
                    else
                    {
                        // There is an existing cbd


                        // If we're currently grabbing a movie, don't do it.
                        if (cbd->grabType == MOVIE)
                        {
                            dtkMsg.add(DTKMSG_WARNING, 
                              "iris::frameGrabber: "
                              "pane %s is currently grabbing a movie sequence; "
                              "command %s is ignored.\n",
                              paneName.c_str(), vec[0].c_str() ) ;
                            return false;
                        }

                        // has prefix changed ?
                        if ( (cbd->prefix != _prefix) )
                        {
                            cbd->nextSeqNum = 0;
                            cbd->prefix = _prefix ;
                        }
                    }

                    cbd->grabType = gt ;

                    // set grab mode to stereo only if the window is stereo
                    if ( (grabMode == STEREO) && ( ! cbd->stereo ) )
                    {
                        dtkMsg.add ( DTKMSG_WARNING, 
                            "iris::frameGrabber: "
                            "Requested a stereo frame grab for pane %s; "
                            "reverting to mono.\n",
                            cbd->paneName.c_str() );
                        cbd->grabMode = MONO;
                    }
                    else
                    {
                        cbd->grabMode = grabMode ;
                    }

                    cbd->frameTime = _frameTime ;
                    cbd->filesnapName  = filesnapName;
                    cbd->lastRefTime = 0.0;

                    panes += paneName + ", " ;
                }  // end of clause for existing cbd

                int fn = iris::SceneGraph::instance()->getViewer()->
                                             getFrameStamp()->getFrameNumber() ;
                if (gt == SNAP) 
                {
                    dtkMsg.add(DTKMSG_NOTICE, 
                      "   iris::frameGrabber: snapping panes: %sat or one "
                      "after  frame number %d\n", panes.c_str(), fn) ;
                }
                else if (gt == MOVIE) 
                {
                    dtkMsg.add (DTKMSG_NOTICE, 
                      "iris::frameGrabber: starting panes: %sat or one "
                      "after frame number %d\n", panes.c_str(), fn) ;
                }

                return true ;
            }
            ////////////////
            // stop a movie
            else if (vec.size() >0 && iris::IsSubstring("stop",vec[0],3)) 
            {
                if (!active) return true ;
                std::vector<CallbackData*> cbds = getCallbackDataVector(vec) ;
                std::string panes ;
                for (unsigned int i=0; i<cbds.size(); i++)
                {
                    panes += cbds[i]->paneName + " " ;
                    cbds[i]->grabType = NONE ;
                }
                int fn = iris::SceneGraph::instance()->
                                            getViewer()->
                                            getFrameStamp()->
                                            getFrameNumber() ;
                dtkMsg.add(DTKMSG_NOTICE, 
                  "iris::frameGrabber: stopping panes: %s at frame number %d\n",
                  panes.c_str(),fn) ;

                return true ;
            }
            ////////////////
            // handle active/inactive in base class
            else
            {
                // if the DSO defines a control method you have to call 
                // the base class's control method too
                bool ret = iris::Augment::control(line, vec) ;
                if (ret)
                {
                    active = getActive() ;
                }
                else dtkMsg.add(DTKMSG_ERROR, 
                       "iris::frameGrabber::control called with "
                       "unknown command, %s\n", line.c_str()) ;

                return ret ;
            }
        } ;

    private:
        std::vector<CallbackData*> _callbackData ;
        std::string  _prefix ;

        NumberingType _numberingMode;

        // inverse of frame rate
        double      _frameTime ;

    } ; // end of class frameGrabber 

} ;  // end of namespace frameGrabber

/************ DTK C++ dtkAugment loader/unloader functions ***************
 *
 * All DTK dtkAugment DSO files are required to declare these two
 * functions.  These function are called by the loading program to get
 * your C++ objects loaded.
 *
 *************************************************************************/

static dtkAugment* dtkDSO_loader(dtkManager* manager, void* p)
{
    return new frameGrabber::frameGrabber ;
}

static int dtkDSO_unloader(dtkAugment* augment)
{
    return DTKDSO_UNLOAD_CONTINUE;
}

