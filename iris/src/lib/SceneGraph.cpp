#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#include <dtk.h>

// uncomment to call dtkAugment callbacks
//#define DTK_CALLBACKS

//here because of DTK magic numbers
#include <dtk/__manager_magicNums.h>

#include <osg/LightModel>
#include <osg/NodeCallback>
#include <osg/NodeVisitor>
#include <osg/Sequence>
#include <osg/View>
#include <osg/Uniform>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/StateAttribute>
#include <osg/PrimitiveSet>
#include <osg/Shader>
#include <osg/Program>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/Registry>
#include <osgDB/WriteFile>
#include <osgGA/GUIEventAdapter>
#include <osgText/Text>
#include <osgUtil/SmoothingVisitor>
#include <osg/PolygonMode>
#if 0
#include <osgUtil/PrintVisitor>
#endif
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <iris/Utils.h>
#include <iris/SceneGraph.h>
#include <iris/Pane.h>
#include <iris/PerspectivePane.h>
#include <iris/OrthoPane.h>
#include <iris/ImmersivePane.h>
#include <iris/Nav.h>
#include <iris/Window.h>
#include <iris/ClipNode.h>
#include <iris/LightNode.h>
#include <iris/ShmMatrixTransform.h>
#include <iris/ConvertNodeVisitor.h>
#include <iris/MatrixTransform.h>
#include <iris/BillboardTransform.h>
#include <iris/ShaderStorageBuffer.h>

////////////////////////////////////////////////////////////////////////
namespace iris
{

    // evil static global variables- I couldn't figure out how to make them static class variables
    // they are used by the control facility to send a command to a class instead of a specific DSO
    static Augment _augment(false) ;
    static Nav _navigation(false) ;
    static Window _window(false) ;
    static Pane _pane(false) ;
    static ImmersivePane _immersivePane(false) ;
    static PerspectivePane _perspectivePane(false) ;
    static OrthoPane _orthoPane(false) ;



    ////////////////////////////////////////////////////////////////////////
    static void signal_catcher(int sig)
    {
        dtkMsg.add(DTKMSG_ERROR,"iris::SceneGraph:: PID %d, caught signal %d, starting exit sequence ...\n", getpid(), sig);
        SceneGraph::instance()->logLine("signal %d caught",sig) ;
        SceneGraph::instance()->setDone(true) ;
        DumpBacktrace() ;
        // reset the signals
        Signal(SIG_DFL); 
    }


    ////////////////////////////////////////////////////////////////////////
    // _GraphQueryVisitor- used to traverse SceneGraph for GRAPH query
    class _GraphQueryVisitor : public osg::NodeVisitor
    {
    public:
        enum OutputType { JSON };

        _GraphQueryVisitor(OutputType outputType)
            : NodeVisitor(), m_outputType(outputType), m_first(true)
        {
            setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
        }

        virtual ~_GraphQueryVisitor() {}

        virtual void apply(osg::Group& group)
        {
            bool old_first;

            switch(m_outputType)
            {
            case JSON:
                if (m_first) m_first = false;
                else m_response += ", ";

                m_response += "{ \"name\": \"" + group.getName() + "\"";

                if (group.getNumChildren() > 0)
                {
                    m_response += ", ";
                    m_response += "\"children\": [ ";
                    old_first = m_first;
                    m_first = true;
                }

                break;
            }

            traverse(group);

            switch(m_outputType)
            {
            case JSON:
                if (group.getNumChildren() > 0)
                {
                    m_response += "] ";
                    m_first = old_first;
                }

                m_response += "} ";
                break;
            }
        }

        virtual void apply(osg::Node& node)
        {
            switch(m_outputType)
            {
            case JSON:
                if (m_first) m_first = false;
                else m_response += ", ";

                m_response += "{ \"name\": \"" + node.getName() + "\"";
                break;
            }

            traverse(node);

            switch(m_outputType)
            {
            case JSON:
                m_response += "} ";
                break;
            }
        }

        std::string response() const { return m_response+"\n"; }

    private:
        OutputType m_outputType;
        int m_indent;
        bool m_first;
        std::string m_response;
    };

    ////////////////////////////////////////////////////////////////////////
    // _ViewerKeyboardMousePollingAndScenegraphUpdate- used to keep a map of event states
    class _ViewerKeyboardMousePollingAndScenegraphUpdate : public osgGA::GUIEventHandler  
    {
        friend class SceneGraph ;    
    public: 
        _ViewerKeyboardMousePollingAndScenegraphUpdate() { } ;

        bool handle( const osgGA::GUIEventAdapter& ea,
                     osgGA::GUIActionAdapter& aa)
        {
            // create a SceneGraph object if none already exists
            SceneGraph* isg = SceneGraph::instance() ;

            switch(ea.getEventType())
            {
            case osgGA::GUIEventAdapter::FRAME:
                {

                    SceneGraph::instance()->logLine("SceneGraph::GUIEventHandler::FRAME entered") ;
                    double frameStart = osg::Timer::instance()->time_s();

                    //copy the world node's matrix to the ether node when it
                    //changes.  Document this- any changes to the matrix in the
                    //ether node wll be clobberd by the matrix in the world node
                    //every frame.  compare them and issue a warning if the
                    //ether node changes from the previous frame
                
                    MatrixTransform* wn = isg->getWorldNode() ;
                    MatrixTransform* en = isg->getEtherNode() ;

                    static osg::Matrix oldWorldMatrix ;
                    static osg::Matrix worldMatrix ;
                    static osg::Matrix oldEtherMatrix;
                    static osg::Matrix etherMatrix ;
                
                    etherMatrix = en->getMatrix() ;
                
                    static bool first = true ;
                    if (first)
                    {
                        oldEtherMatrix = etherMatrix ;
                        first = false ;
                    }
                    else if (oldEtherMatrix != etherMatrix)
                    {
                        dtkMsg.add(DTKMSG_WARNING, "iris::SceneGraph:: the ether node's matrix was changed.  It will be overwritten by the world node's world\n") ;
                        en->setMatrix(worldMatrix) ;
                        oldEtherMatrix = worldMatrix ;
                    }

                    worldMatrix = wn->getMatrix() ;
                
                    if (oldWorldMatrix != worldMatrix)
                    {
                        en->setMatrix(worldMatrix) ;
                        oldWorldMatrix = worldMatrix ;
                        oldEtherMatrix = worldMatrix ;
                        dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph:: the world node's matrix was copied to the ether node\n") ;
                    }

                    // set the matrix in the pivot node
                    iris::SceneGraph::instance()->getPivotNode()->setMatrix(Nav::getNormalizedPivotTransformation()) ;
                    
                    SceneGraph::instance()->logLine("SceneGraph::GUIEventHandler::FRAME reading fifo") ;
                    double fifoStart = osg::Timer::instance()->time_s();
                    double fifoStop ;
                    std::string command ;
                    while (isg->getFifo()->readLine(&command))
                    {
                        
                        // reading model file data via STARTMODEL/ENDMODEL ?
                        if (isg->getStartModelDataFileStream())
                        {
                            // make sure it's not an ENDMODEL command
                            std::vector<std::string> vec = ParseString(command) ;  
                            if (vec.size()>0 && !IsSubstring("endmodeldata", vec[0]))
                            {
                                *(isg->getStartModelDataFileStream()) << command << std::endl ;
                                continue ;
                            }
                        }
                                        
                        // do variable substitution
                        command = isg->substituteVariable(command) ;
                        
                        std::vector<std::string> vec = ParseString(command) ;
                        if (vec.size() == 1 && IsSubstring("frame", vec[0],3)) 
                        {
                            fifoStop = osg::Timer::instance()->time_s() ;
                            SceneGraph::instance()->logLine("SceneGraph::GUIEventHandler::FRAME FRAME control command, exitign FRAME, time to process fifo commands = %18.17f",fifoStop-fifoStart) ;
                            return false ;
                        }
                        else if (vec.size()>0)
                        {
                            if (!isg->control(command, vec))
                            {
                                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph:: unrecognized control command: %s\n",command.c_str()) ;
                            }
                        }
                    }
                    fifoStop = osg::Timer::instance()->time_s() ;
                    SceneGraph::instance()->logLine("SceneGraph::GUIEventHandler::FRAME fifo drained, time to process fifo commands = %18.17f",fifoStop-fifoStart) ;
                    
                    double frameStop = osg::Timer::instance()->time_s();
                    SceneGraph::instance()->logLine("SceneGraph::GUIEventHandler::FRAME exiting, time to process frame = %18.17f",frameStop-frameStart) ;
                    return false;
                }
            
            case osgGA::GUIEventAdapter::PUSH:
                {
                    SceneGraph::instance()->logLine("SceneGraph::GUIEventHandler::PUSH") ;
                    int b = ea.getButton() ;
                    _modSet(_buttonState, b, true) ;
                    _buttonMask |= (unsigned int)b ;
                    return false;
                }
            
            case osgGA::GUIEventAdapter::RELEASE:
                {
                    SceneGraph::instance()->logLine("SceneGraph::GUIEventHandler::RELEASE") ;
                    int b = ea.getButton() ;
                    _modSet(_buttonState, b, false) ;
                    _buttonMask &= ~((unsigned int)b) ;
                    return false;
                }    
            case osgGA::GUIEventAdapter::KEYDOWN:
                {
                    SceneGraph::instance()->logLine("SceneGraph::GUIEventHandler::KEYDOWN") ;
                    int c = ea.getKey() ;
                    _modSet(_keyState, c, true) ;
#if 0
                    //fprintf(stderr,"    c = %d\n",c) ;
                    dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph:: _keyState has %d element(s)\n",_keyState.size()) ;
                    std::set<int>::iterator pos ;
                    unsigned int i = 0 ;
                    for (pos=_ViewerKeyboardMousePollingAndScenegraphUpdate::_keyState.begin(); 
                         pos!=_ViewerKeyboardMousePollingAndScenegraphUpdate::_keyState.end(); 
                         pos++)
                    {
                        if (*pos>=' ' && *pos<='~') dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::   keys[%d] = x%X, \"%c\"\n",i,*pos,*pos) ;
                        else dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::   keys[%d] = x%X\n",i,*pos) ;
                        i++ ;
                    }
#endif
                    // hopefully you won't get this, so warn if you do
                    if (c == isg->getViewer()->getKeyEventSetsDone())
                    {
                        dtkMsg.add(DTKMSG_WARNING, "iris::SceneGraph:: KeyEventSetsDone starting exit sequence ...\n") ;
                        isg->setDone(true) ;
                    }
                    // the usual way to exit
                    else if (c == osgGA::GUIEventAdapter::KEY_Escape)
                    {
                        dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph:: ESCAPE pressed, starting exit sequence ...\n") ;
                        isg->setDone(true) ;
                    }
                    // undocumented exit key for debugging
                    else if (c == osgGA::GUIEventAdapter::KEY_Scroll_Lock)
                    {
                        dtkMsg.add(DTKMSG_WARNING, "iris::SceneGraph:: Scroll_Lock pressed, starting exit sequence ...\n") ;
                        isg->setDone(true) ;
                    }

                    return false;


                }
            case osgGA::GUIEventAdapter::KEYUP:
                {
                    SceneGraph::instance()->logLine("SceneGraph::GUIEventHandler::KEYUP") ;
                    int c = ea.getKey() ;
                    _modSet(_keyState, c, false) ;
                    return false;
                }
            case osgGA::GUIEventAdapter::QUIT_APPLICATION:
                {
                    SceneGraph::instance()->logLine("SceneGraph::GUIEventHandler::QUIT") ;
                    dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph: osgGA::GUIEventAdapter::QUIT_APPLICATION:\n") ;
                    isg->setDone(true) ;
                    return false;
                }
            case osgGA::GUIEventAdapter::CLOSE_WINDOW:
                {
                    SceneGraph::instance()->logLine("SceneGraph::GUIEventHandler::CLOSE_WINDOW") ;
                    dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph: osgGA::GUIEventAdapter::CLOSE_WINDOW:\n") ;
                    isg->setDone(true) ;
                    return false;
                }
            default:
                return false;
            }
        }
    private:
        static std::set<int> _keyState ;
        static std::set<int> _buttonState ;
        static unsigned int _buttonMask ;

        // v == true, add to set, v == false, remove from set
        void _modSet(std::set<int> &set, int k, bool v)
        {
            std::set<int>::iterator pos ;
            pos = set.find(k) ;
            if (pos != set.end() && !v) set.erase(pos) ; //found it, erase it?
            else if (pos == set.end() && v) set.insert(k) ; // didn't find it, add it?
    
#if 0
            // debug- print out the set
            fprintf(stderr,"set.size = %d\n",set.size()) ;
            if (set.size()>0) 
            {
                for (pos=set.begin(); pos!=set.end(); pos++) fprintf(stderr,"x%X ",*pos) ;
                fprintf(stderr,"\n") ;
            }
#endif

        }
    };

    std::map<std::string, osg::ref_ptr<osg::Texture> > SceneGraph::_textures;    

    ////////////////////////////////////////////////////////////////////////
    SceneGraph* const SceneGraph::instance(int i)
    {

        //dtkMsg.add(DTKMSG_NOTICE, "iris::SceneGraph::instance.size() = %d\n",_instances.size()) ;

        if (_instances.size()>1) dtkMsg.add(DTKMSG_WARNING,"iris::SceneGraph: %d SceneGraph objects exists.  The current version of iris only supports SceneGraph as a singleton.\n",_instances.size()) ;        

        if (!_instanced) 
        {
#if 0            
            dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::instance() no SceneGraph object\n") ;        
            return NULL ;
#else
            dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::instance() creating SceneGraph object\n") ;        
            return new SceneGraph ;
#endif
        }
        else if (i<0 || i>=_instances.size()) 
        {
            dtkMsg.add(DTKMSG_NOTICE, "iris::SceneGraph::instance() index %d out of range\n",i) ;
            return NULL ;
        }
        return _instances[i] ;
    } ;

    ////////////////////////////////////////////////////////////////////////
    void SceneGraph::_init(int argc, char **argv, bool createViewer)
    {
        _instanced = true ;
        SceneGraph* isg = this ;
        _instances.push_back(isg) ;
        _index = _instances.size()-1 ;
        _createViewer = createViewer ;
        _inModelDataFileStream = NULL ;

        // set up logging variables
        _logFileName = "/tmp/iris-" + std::string(getenv("USER")) + ".log" ;
        _logFILE = NULL ;
        _logOn = false ;
        _logFirstTry = true ;
        // seed the variable map with the current environment variables
        initEnvVariables() ;

        if (_instances.size()!=1) dtkMsg.add(DTKMSG_WARNING,"iris::SceneGraph: %d SceneGraph objects exists.  The current version of iris only supports SceneGraph as a singleton.\n",_instances.size()) ;        

        _realized = false ;
        _done = false ;

        // defaults: this stuff will be overridden by command line options to n-view at some point
        const osg::Vec4 pos = osg::Vec4(0.0f, -sqrt(2.0f), sqrt(2.0f), 0.f) ;
        const double ambient = 0.f ;
        const double diffuse = 0.8f ;
        const double specular = 1.0f ;
        const bool backfacecull = false ;

        _meters = 1.f ;
        _interocular = .07f ;
        _fusion = 1.f ;
        _jump = false ;
        _clearColor = osg::Vec4(0.,0.,0.,1.) ; 
        
        // get a fifo to read control commands
        if (getenv("IRIS_CONTROL_FIFO")) _fifo.setName(getenv("IRIS_CONTROL_FIFO")) ;
        else 
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph:: IRIS_CONTROL_FIFO not defined\n") ;
            return ;
        }
        if (!_fifo.open())
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph:: can't open Control fifo\n") ;
            return ;
        }
                        

        // create the "scene" node, the root of the scenegraph
        _scene = new MatrixTransform ;
        _scene->setName("scene") ;
        osg::ref_ptr<osg::StateSet> sceneStateSet = _scene->getOrCreateStateSet() ;

#if 1
        // create one default light
        LightNode* light = new LightNode ;
        light->enable() ;
        light->setName("light") ;
        light->getLight()->setPosition(osg::Vec4(0.0f, -sqrt(2.0f), sqrt(2.0f), 0.f)) ;
        _nodeMap.insert(std::make_pair("light",light)) ;
#ifdef SCENEGRAPH_NODE_PTR_SET
        _nodePtrSet.insert(light) ;
#endif
        _scene->addChild(light) ;
#else
        // create all lights and set defaults for first one
        {
            std::string base = "light" ;
            std::string name ;
            for (int i=0; i<LightNode::maxNodes(); i++)
            {
                name = base + IntToString(i) ;
                LightNode* light = new LightNode ;
                light->setName(name) ;
                _nodeMap.insert(std::make_pair(name,light)) ;
#ifdef SCENEGRAPH_NODE_PTR_SET
                _nodePtrSet.insert(light) ;
#endif
                if (i == 0)
                {
                    // set defaults for first one
                    _scene->addChild(light) ;
                }
            }
        }
#endif
        // get correct specular lighting across pipes
        // see http://www.ds.arch.tue.nl/General/Staff/Joran/osg/osg_specular_problem.htm
        osg::ref_ptr<osg::LightModel> lightModel = new osg::LightModel ;
        lightModel->setLocalViewer(true);
        lightModel->setDataVariance(osg::Object::DYNAMIC) ;

        // turn on two sided lighting if not backface culling
        if (!backfacecull)
        {
            sceneStateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF) ;
            lightModel->setTwoSided(true) ;
        }
        else
        {
            sceneStateSet->setMode(GL_CULL_FACE, osg::StateAttribute::ON) ;
            lightModel->setTwoSided(false) ;
        }
        sceneStateSet->setAttribute(lightModel.get());

        // resize normals when resizing geometry
        sceneStateSet->setMode((osg::StateAttribute::GLMode)GL_NORMALIZE, osg::StateAttribute::ON);

        // create the "ether" node
        _ether = new MatrixTransform ;
        _ether->setName("ether") ;

        // create the "world" node
        _world = new MatrixTransform ;
        _world->setName("world") ;

        // create the "nav" node
        _nav = new MatrixTransform ;
        _nav->setName("nav") ;

        // create the "pivot" node
        _pivot = new MatrixTransform ;
        _pivot->setName("pivot") ;

        // create the "head" node
        _head = new MatrixTransform ;
        _head->setName("head") ;

        // create the "wand" node
        _wand = new MatrixTransform ;
        _wand->setName("wand") ;

        // create the "clear" node
        _clear = new MatrixTransform ;
        _clear->setName("clear") ;

        // hook 'em up
        _scene->addChild(_ether.get()) ;
        _scene->addChild(_nav.get()) ;
        _scene->addChild(_pivot.get()) ;
        _scene->addChild(_head.get()) ;
        _scene->addChild(_wand.get()) ;
        _scene->addChild(_clear.get()) ;
        _nav->addChild(_world.get()) ;

#if 0
        // create six osg::ClipNodes.  They aren't part of the scenegraph, so manually add to the node map so they can be found
        {
            std::string base = "clip" ;
            std::string name ;
            for (int i=0; i<ClipNode::maxNodes(); i++)
            {
                name = base + IntToString(i) ;
                ClipNode* clip = new ClipNode ;
                clip->setName(name) ;
                _nodeMap.insert(std::make_pair(name,clip)) ;
#ifdef SCENEGRAPH_NODE_PTR_SET
                _nodePtrSet.insert(clip) ;
#endif
            }
        }
#endif

        std::string str ;
#if 0
        // set the search path for model files
        str = std::string(".:") + std::string(IRIS_PREFIX) + std::string("/data:") ;
        char* osgfilepath = getenv("OSG_FILE_PATH") ;
        if (osgfilepath)
        {
            str += osgfilepath ;
        }
        osgDB::Registry::instance()->setDataFilePathList(str) ;
#endif
        // set the search path for DSO files
        str = std::string(".:") + std::string(IRIS_PREFIX) + std::string("/lib:") ;
        str += dtkConfig.getString(dtkConfigure::AUGMENT_DSO_DIR) ;
        char* irisdsopath = getenv("IRIS_FILE_PATH") ;
        if (irisdsopath)
        {
            str += irisdsopath ;
        }
        path(str.c_str()) ;

        // create default viewer
        if (createViewer) addNewDefaultViewer() ;

        // load the specified files
        char* irisfiles = getenv("IRIS_FILES") ;
        if (irisfiles)
        {
#if 0
            str = irisfiles + std::string(":") ;
            //fprintf(stderr,"IRIS_FILES = %s\n",str.c_str()) ;
            std::string::size_type idx ;
            while ((idx=str.find(":"))!=std::string::npos)
            {
                LoadFile(str.substr(0,idx), _world.get()) ;
                str = str.substr(idx+1) ;
            }
#else
            osgDB::FilePathList fpl ;
            osgDB::convertStringPathIntoFilePathList(irisfiles, fpl) ;
            while(fpl.size()>0)
            {
                LoadFile(fpl.front(), _world.get()) ;
                fpl.pop_front() ;
            }
#endif
        }
#if 0
        else
        {
            dtkMsg.add(DTKMSG_NOTICE, "iris::SceneGraph::SceneGraph no DSOs specified\n") ;
        }
#endif

        if (isInvalid())
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::SceneGraph failed.\n");
            return ;
        }

#if 0
        // This doesn't seem to be needed if not doing the usual callbacks
        setType(TYPE_IRIS);
#endif

        // validate before loading DSOs
        validate();

    }

    ////////////////////////////////////////////////////////////////////////
    SceneGraph::~SceneGraph() 
    { 
        dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::~SceneGraph() called\n") ; 
        logLine("iris::SceneGraph::~SceneGraph() called") ; 
        _deleteFifos() ;
        if (!_done) setDone(true) ; 
        std::vector<Augment*> augments = Augment::getAugments() ;
        for (unsigned int i=0; i<augments.size(); i++)
        {
            dtkMsg.add(DTKMSG_DEBUG,"iris::SceneGraph::~SceneGraph(): DSO %d: calling %s::cleanup\n",i,augments[i]->getName()) ;
            augments[i]->cleanup();
        }
        if (_logData.size()>0 && _logFILE)
        {
            dtkMsg.add(DTKMSG_NOTICE,"iris::SceneGraph::~SceneGraph(): writing %d lines to log file %s\n",_logData.size(),_logFileName.c_str()) ;
            // start time
            double s = _logData[0]->time ;
            for (unsigned int i=0; i<_logData.size(); i++)
            {
                double t, d ;
                if (_logData[i]->frameNumber < 2)
                {
                    t = d = 0.0 ;
                }
                else
                {
                    // ellapsed time
                    t = _logData[i]->time - s ;
                    // delta time from last frame
                    if (i==0 || _logData[i-1]->frameNumber < 2) d = 0.0 ;
                    else d = _logData[i]->time - _logData[i-1]->time ;
                }
                fprintf(_logFILE,"%28.17f %18.17f %12d %s\n",t, d, _logData[i]->frameNumber,  _logData[i]->line.c_str()) ;
            }
            fclose(_logFILE) ;
        }

        char ccStatsFileName[PATH_MAX];
        snprintf(ccStatsFileName, PATH_MAX,
                 "/tmp/iris-%s-control-command-stats.txt", getenv("USER"));
        FILE* fh = fopen(ccStatsFileName, "w") ;
        if (fh) {
            std::map<std::string, size_t>::const_iterator ccStat,
                ccEnd = _ccStats.end();
            for (ccStat=_ccStats.begin(); ccStat!=ccEnd; ++ccStat) {
                fprintf(fh, "%s,%zu\n", ccStat->first.c_str(), ccStat->second);
            }
            fclose(fh);
        }

    } 

    ////////////////////////////////////////////////////////////////////////
    void SceneGraph::frame(double time) 
    { 
        if (!_realized) realize() ;

#ifdef DTK_CALLBACKS
        preFrame(__DTKMANAGER_PREFRAME_MAGIC_);
#endif

        //fprintf(stderr,"before frame %d, time = %f \n",getViewer()->getFrameStamp()->getFrameNumber(),(float)getViewer()->getFrameStamp()->getReferenceTime()) ;
        processBeforeFrameWhenCommands() ;

        for (unsigned int i=0; i<_viewers.size(); i++)
        {
            double viewerStart = osg::Timer::instance()->time_s();
            logLine("SceneGraph::frame: calling osg::Viewer::frame()") ;
            _viewers[i]->frame(time) ; 
            double viewerStop = osg::Timer::instance()->time_s();
            logLine("SceneGraph::frame: osg::Viewer::frame() finished, elapsed time %18.17f",viewerStop-viewerStart) ;
        }

        //fprintf(stderr,"after frame %d, time = %f \n",getViewer()->getFrameStamp()->getFrameNumber(),(float)getViewer()->getFrameStamp()->getReferenceTime()) ;

#ifdef DTK_CALLBACKS
        dtkManager::frame() ; 
#endif
        processAfterRealizeWhenCommands() ;
        processAfterFrameWhenCommands() ;
        processAfterTimeWhenCommands() ;

#ifdef DTK_CALLBACKS
        postFrame(__DTKMANAGER_POSTFRAME_MAGIC_) ;
#endif
        {
            // informational
            static bool first = true ;
            if (first)
            {
                first = false ;
                std::string modelString ;
                osgViewer::ViewerBase::ThreadingModel model = getViewer()->getThreadingModel() ;
                if (model == osgViewer::ViewerBase::CullDrawThreadPerContext) modelString = "CullDrawThreadPerContext" ;
                else if (model == osgViewer::ViewerBase::SingleThreaded) modelString = "SingleThreaded" ;
                else if (model == osgViewer::ViewerBase::DrawThreadPerContext) modelString = "DrawThreadPerContext" ;
                else if (model == osgViewer::ViewerBase::CullThreadPerCameraDrawThreadPerContext) modelString = " CullThreadPerCameraDrawThreadPerContext" ;
                else if (model == osgViewer::ViewerBase::AutomaticSelection) modelString = "AutomaticSelection" ;
                dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::SceneGraph threading model is \"%s\"\n",modelString.c_str()) ;
            }
        }

        return ; 
    } ;

    ////////////////////////////////////////////////////////////////////////
    void SceneGraph::setDone(bool done)
    {
        logLine("SceneGraph::setDone()") ;
        if (!done) return ;

        _done = done ;

        dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::setDone(%d)\n",done) ;
        if (done) processBeforeCleanupWhenCommands() ;

        for (unsigned int i=0; i<_childPids.size(); i++)
        {
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph: killing pid %d\n",_childPids[i]) ;
            ::kill(_childPids[i],SIGTERM) ;
        }

        dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph: turning off viewers\n") ;
        for (unsigned int i=0; i<_viewers.size(); i++)
        {
            _viewers[i]->setDone(done) ; 
        }

        dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::setDone deleting nodes in control list\n",done) ;
        if (done)
        {  
            // delete nodes in control list
            _nodeMap.clear() ;
#ifdef SCENEGRAPH_NODE_PTR_SET
            _nodePtrSet.clear() ;
#endif
            processAfterCleanupWhenCommands() ;
        }

        dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::setDone finished\n",done) ;
    }

    ////////////////////////////////////////////////////////////////////////
    osgViewer::Viewer* const SceneGraph::getViewer(int i)
    {
        if (i<0 || i>=_viewers.size())
        {
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::getViewer: index %d out of range\n",i) ;
            return NULL ;
        }
        return _viewers[i] ;
    }

    ////////////////////////////////////////////////////////////////////////
    int SceneGraph::addNewDefaultViewer()
    {
        osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer ;
        viewer->setEndBarrierPosition(osgViewer::ViewerBase::BeforeSwapBuffers) ;
        viewer->setEndBarrierOperation(osg::BarrierOperation::GL_FINISH) ;

        // if you don't use CullDrawThreadPerContext buffers don't swap at the same time between windows
#if 1        
        viewer->setThreadingModel(osgViewer::ViewerBase::CullDrawThreadPerContext) ;
#else
        viewer->setThreadingModel(osgViewer::ViewerBase::AutomaticSelection) ;
#endif
        viewer->setDataVariance(osg::Object::DYNAMIC) ;

        // switch off small feature culling to prevent points from being culled.
        // this is for the "master camera"- hopefully slaves will inherit it
        viewer->getCamera()->setCullingMode(viewer->getCamera()->getCullingMode() & ~osg::CullStack::SMALL_FEATURE_CULLING);

        // set the scene to render
        viewer->setSceneData(_scene);

        // keep track of key states and copy world matrix to the ether matrix
        osg::ref_ptr<_ViewerKeyboardMousePollingAndScenegraphUpdate> eventPolling = new _ViewerKeyboardMousePollingAndScenegraphUpdate ;
        viewer->addEventHandler(eventPolling.get());

        // jtk 8/14/12 trying to avoid using the OSG exit routines as they seem to hang the graphics loop
        // pressing ^C will usually get you out of the hang
        // the docs say this will disable it, but you can get it to be called by pressin control-@
        //viewer->setKeyEventSetsDone(0) ;
        // this will hopefully never get called
        viewer->setKeyEventSetsDone(-1) ;

        // add a couple of viewer handlers
        // uses 's'
        viewer->addEventHandler(new osgViewer::StatsHandler) ;
        // uses 'm'
        viewer->addEventHandler(new osgViewer::ThreadingHandler) ;

        return addViewer(viewer) ;
    }

    ////////////////////////////////////////////////////////////////////////
    int SceneGraph::addViewer(osgViewer::Viewer* viewer)
    {
        osg::ref_ptr<osgViewer::Viewer> vptr = viewer ;
        // if you supply your own viewer we don't mess with it
        _viewers.push_back(vptr) ;

        return _viewers.size()-1 ;
    }

    ////////////////////////////////////////////////////////////////////////
    void SceneGraph::realize()
    {
        logLine("SceneGraph::realize()") ;
        if (!getViewer()) 
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::realize no viewer object; ignoring\n");
            return ;
        }

        if (_realized) return ;

        // catch signals that kill us off
        Signal(signal_catcher) ;

#ifdef DTK_CALLBACKS
        preConfig() ;
#endif

#ifdef DTK_CALLBACKS
        config() ;
#endif

        processBeforeRealizeWhenCommands() ;

#if 1
        // is this a bad assumption?  What if the .iris file loads a window and doesn't want the default?
        // how to get the viewer to not supply a default if we don't?
        // load window and navigation DSOs if none already loaded
        if (Window::getWindowList().size()==0) 
        {
            dtkMsg.add(DTKMSG_NOTICE, "iris::SceneGraph::SceneGraph loading default window\n") ;
            Augment::getOrLoadDSO("desktopWindow") ;
        }

        if (Nav::getNavList().size()==0) 
        {
            dtkMsg.add(DTKMSG_NOTICE, "iris::SceneGraph::SceneGraph loading default navigation\n") ;
            Augment::getOrLoadDSO("trackballNav") ;
        }
#endif

        // call realize for every Window object in the list
        for (unsigned int i=0; i<(Window::getWindowList()).size(); i++) 
        {
            (Window::getWindowList())[i]->realize() ;
        }
#if 0
        // can this safely be skipped?
        for (unsigned int i=0; i<_viewers.size(); i++)
        {
            _viewers[i]->realize() ; 
        }
#endif


#ifdef DTK_CALLBACKS
        postConfig() ;
#endif

        processAfterRealizeWhenCommands() ;

#ifdef DTK_CALLBACKS
        // always want to do this just to have less to do each loop
        setPublicPrePostFrames(__DTKMANAGER_PREPOSTFRAME_MAGIC_);
#endif

        // set up shared memory for writing state

        _realized = true ;

        return ;
    }

    ////////////////////////////////////////////////////////////////////////
    void SceneGraph::addPID(pid_t pid)
    {
        _childPids.push_back(pid) ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::control(const std::string& line)
    {
        return control(line, ParseString(line)) ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::control(const std::string& line, const std::vector<std::string>& vec)
    {

        // commented out because it seems to cause before/after cleanup commands to be ignored
#if 0
        if (_done) 
        {
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph:control late command %s\n",line.c_str()) ;
            return true ;
        }
#endif


        logLine("SceneGraph::control: "+line) ;

        // track the use of this control command
        std::string ccUC = vec[0];
        std::transform(ccUC.begin(), ccUC.end(), ccUC.begin(), toupper);

        if (ccUC=="QUERY") {
            ccUC += " " + vec[1] + " " + vec[2];
        } else if (ccUC=="DSO") {
            ccUC += " " + vec[1];
        }

        _ccStats[ccUC]++;


        if (_echo)
        {
            fprintf(stderr,"ECHO: %s\n",line.c_str()) ;
        }
        
        if (0) {  } 

        else if (vec.size() == 1 && IsSubstring("set", vec[0], 3))
        {
            listVariables() ;
            return true ;
        }
        else if (vec.size() == 2 && IsSubstring("set", vec[0], 3))
        {
            addVariable(vec[1]) ;
            return true ;
        }
        else if (vec.size() == 3 && IsSubstring("set", vec[0], 3))
        {
            addVariable(vec[1], vec[2]) ;
            return true ;
        }
        else if (vec.size() == 2 && IsSubstring("unset", vec[0], 3))
        {
            delVariable(vec[1]) ;
            //_dumpVariables() ;
            //std::string foo = _substituteVariable(std::string("123${foo}456")) ;
            //fprintf(stderr,"123${foo}456 = %s\n",foo.c_str()) ;
            return true ;
        }

        else if ( IsSubstring("noop", vec[0], 4) )
        {
            return true;
        }

        else if (vec.size() == 3 && IsSubstring("addchild", vec[0], 3))
        {
            return _addChild(vec[1], vec[2]) ;
        }
        
        else if (vec.size() == 4 && IsSubstring("addchild", vec[0], 3))
        {
            return _addChild(vec[1], vec[2], vec[3]) ;
        }

        else if (IsSubstring("dcs", vec[0], 3))
        {
            return _dcs(true, false, vec) ;
        }

        else if (vec.size() == 4 && IsSubstring("background",vec[0],3))
        {
            float r, g, b ;
            if (!StringToFloat(vec[1],&r) || !StringToFloat(vec[2],&g) || !StringToFloat(vec[3],&b))
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::control: invalid background paramaters: %s\n",vec[1].c_str()) ;  
                return false ;
            }
            return background(r, g, b) ;
        }

        else if (IsSubstring("bound", vec[0], 3))
        {
            if (vec.size() == 1) return bound() ; 
            if (vec.size() == 2) return bound(vec[1]) ; 
            else if (vec.size() == 3)
            {
                // get the radius
                double n ;
                if (!StringToDouble(vec[2], &n))
                {
                    dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::bound: unable to convert radius parameter to a double\n") ;
                    return false ;
                }
                return bound(vec[1],n) ;
            }
            else return false ;
        }

        else if (IsSubstring("center", vec[0], 3))
        {
            if (vec.size() == 1) return center() ; 
            if (vec.size() == 2) return center(vec[1]) ; 
            else if (vec.size() == 5)
            {
                // get the center
                double x, y, z ;
                if ((!StringToDouble(vec[2], &x)) || (!StringToDouble(vec[3], &y)) || (!StringToDouble(vec[4], &z)))
                {
                    dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::control: unable to convert center parameter to doubles\n") ;
                    return false ;
                }
                return center(vec[1],osg::Vec3(x,y,z)) ;
            }
            else return false ;
        }

        else if (IsSubstring("clipnode",vec[0],3) && vec.size() >=2)
        {
            return clipnode(vec) ;
        }

        else if (IsSubstring("lightnode",vec[0],6) && vec.size() >= 2)
        {
            return lightnode(vec) ;
        }

        else if (IsSubstring("lightmodel",vec[0],6) && vec.size() >= 3)
        {
            return lightmodel(vec) ;
        }

        else if (IsSubstring("billboard",vec[0],5)) 
        {
            return billboard (vec) ;
        }

        else if (IsSubstring("clonenode",vec[0],3) && vec.size() == 3)
        {
            dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::control: using \"ALL\" node copy operator\n") ;
            return _cloneNode(vec[1], vec[2]) ;
        }

        // check copy operator
        else if (IsSubstring("clonenode",vec[0],3) && vec.size() > 3)
        {
            osg::CopyOp::CopyFlags op = 0 ;
            for (unsigned int i=3; i<vec.size(); i++)
            {
                if (IsSubstring("shallow",vec[i],3)) op |= osg::CopyOp::SHALLOW_COPY ;
                else if (IsSubstring("objects",vec[i],3)) op |= osg::CopyOp::DEEP_COPY_OBJECTS ;
                else if (IsSubstring("nodes",vec[i],3)) op |= osg::CopyOp::DEEP_COPY_NODES ;
                else if (IsSubstring("drawables",vec[i],3)) op |= osg::CopyOp::DEEP_COPY_DRAWABLES ;
                else if (IsSubstring("statesets",vec[i],6)) op |= osg::CopyOp::DEEP_COPY_STATESETS ;
                else if (IsSubstring("stateattributes",vec[i],6)) op |= osg::CopyOp::DEEP_COPY_STATEATTRIBUTES ;
                else if (IsSubstring("textures",vec[i],3)) op |= osg::CopyOp::DEEP_COPY_TEXTURES ;
                else if (IsSubstring("images",vec[i],3)) op |= osg::CopyOp::DEEP_COPY_IMAGES ;
                else if (IsSubstring("arrays",vec[i],3)) op |= osg::CopyOp::DEEP_COPY_ARRAYS ;
                else if (IsSubstring("primitives",vec[i],3)) op |= osg::CopyOp::DEEP_COPY_PRIMITIVES ;
                else if (IsSubstring("shapes",vec[i],3)) op |= osg::CopyOp::DEEP_COPY_SHAPES ;
                else if (IsSubstring("uniforms",vec[i],3)) op |= osg::CopyOp::DEEP_COPY_UNIFORMS ;
                else if (IsSubstring("all",vec[i],3)) op |= osg::CopyOp::DEEP_COPY_ALL ;
                else
                {
                    dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::control: unknown copy operator \"%s\"\n",vec[i].c_str()) ;
                    return false ;
                }
                dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::control: adding \"%s\" to list of node copy operations\n",vec[i].c_str()) ;
            }
            return _cloneNode(vec[1], vec[2], op) ;
        }

        else if (IsSubstring("copyprimitives", vec[0], 5))
        {
            if (vec.size()==1) return copyPrimitives() ;
            else if (vec.size()==2) return copyPrimitives(vec[1]) ;
            else return false ;
        }

        else if (IsSubstring("copystate",vec[0],5) && vec.size() == 3)
        {
            return _copyState(vec[1], vec[2]) ;
        }

        else if (vec.size() == 3 && IsSubstring("dump",vec[0],3))
        {
            return dump(vec[1], vec[2]) ;
        }

        else if (IsSubstring("shaderprog",vec[0],7))
        {
            return _shaderProg(vec) ;
        }

        else if (IsSubstring("uniform",vec[0],3))
        {
            return _uniform(vec) ;
        }

        else if (IsSubstring("shaderstoragebuffer",vec[0],7))
        {
            return _shaderstoragebuffer(vec) ;
        }

        else if (IsSubstring("txtr",vec[0],3))
        {
            return _txtr (vec) ;
        }

        else if (IsSubstring("txtrc",vec[0],3))
        {
            return _txtrc(vec);
        }

        else if (IsSubstring("txtradd",vec[0],3))
        {
            return _txtradd(vec);
        }
        else if (IsSubstring("text",vec[0],3))
        {
            return _text(vec) ;
        }

        else if (IsSubstring("culling",vec[0],3))
        {
            return _cullingOnOff(vec) ;
        }

#if 0
        else if (IsSubstring("print",vec[0],3))
        {
            if (vec.size() == 2) return print(vec[1],"") ;
            else if (vec.size() ==3) return print(vec[1],vec[2]) ;
            else return false ;
        }
#endif

        else if (vec.size() > 1 && (IsSubstring("exec",vec[0],3)))
        {
            return exec(line) ;
        }

        else if (vec.size() > 1 && (IsSubstring("wait",vec[0],3)))
        {
            return exec(line,true) ;
        }

        else if (vec.size() == 2 && (IsSubstring("kill",vec[0],3)))
        {
            return kill(vec[1]) ;
        }

        else if (vec.size() == 2 && IsSubstring("echo", vec[0], 3))
        {
            return _setEcho(vec[1]) ;
        }

        else if (vec.size() == 2 && IsSubstring("log", vec[0], 3))
        {
            return _setLog(vec[1]) ;
        }

        else if (IsSubstring("examine", vec[0], 3))
        {
            if (vec.size()==2) return examine(vec[1]) ;
            else if (vec.size()==1) return examine() ;
            else return false ;
        }

        else if (IsSubstring("polygonmode", vec[0], 3))
        {
            if (vec.size()==1) return polygonMode() ;
            else if (vec.size()==2) return polygonMode(vec[1]) ;
            else if (vec.size()==3) return polygonMode(vec[1], vec[2]) ;
            else if (vec.size()==4) return polygonMode(vec[1], vec[2], vec[3]) ;
            else return false ;
        }

        else if (vec.size() == 2 && IsSubstring("fusion",vec[0],3))
        {
            double fd ;
            if (!StringToDouble(vec[1],&fd))
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::control: invalid fusion paramaters: %s\n",vec[1].c_str()) ;  
                return false ;
            };
            setFusion(fd) ;
            dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::control: fusion: %f\n",fd) ;
            return true ;
        }

        else if (vec.size() == 2 && IsSubstring("map", vec[0], 3))
        {      
            return _map(vec[1]) ;
        }

        else if (vec.size() == 2 && IsSubstring("group", vec[0], 3))
        {      
            return _group(vec[1]) ;
        }

        else if (vec.size() >= 2 && IsSubstring("switch", vec[0], 3))
        {      
            return _switch(vec) ;
        }

        else if (vec.size() == 2 && IsSubstring("noclipnode", vec[0], 3))
        {      
            return _noclipnode(vec[1]) ;
        }

        else if (vec.size() == 2 && IsSubstring("nolightnode", vec[0], 3))
        {      
            return _nolightnode(vec[1]) ;
        }

        else if (vec.size() == 2 && IsSubstring("interocular",vec[0],3))
        {
            double iod ;
            if (!StringToDouble(vec[1],&iod))
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::control: invalid interocular paramaters: %s\n",vec[1].c_str()) ;  
                return false ;
            };
            setInterocular(iod) ;
            dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::control: interocular: %f\n",iod) ;
            return true ;
        }

        else if (vec.size()>0 && IsSubstring("jump",vec[0],3))
        {
            if (vec.size() == 2)
            {
                bool onOff ;
                if (OnOff(vec[1],&onOff))
                {
                    setJump(onOff) ;
                    if (onOff) dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::control: jump: on\n") ;
                    else dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::control: jump: off\n") ;
                    return true ;
                }
                else return false ;
            }
            else if (vec.size() == 5 && IsSubstring("position",vec[1],3))
            {
                double x, y, z ;
                if (!StringToDouble(vec[2],&x) ||
                    !StringToDouble(vec[3],&y) ||
                    !StringToDouble(vec[4],&z))
                {
                    dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::control: invalid jump position paramaters: %s %s %s\n",vec[2].c_str(),vec[3].c_str(),vec[4].c_str()) ;  
                    return false ;
                }
                else
                {
                    dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::control: jump position: %f %f %f\n",x,y,z) ;
                    setJumpPosition(osg::Vec3(x,y,z)) ;
                    return true ;
                }
            }
            else if (vec.size() == 6 && IsSubstring("attitude",vec[1],3))
            {
                double x, y, z, w ;
                if (!StringToDouble(vec[2],&x) ||
                    !StringToDouble(vec[3],&y) ||
                    !StringToDouble(vec[4],&z) ||
                    !StringToDouble(vec[5],&w))
                {
                    dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::control: invalid jump attitude paramaters: %s %s %s\n",vec[2].c_str(),vec[3].c_str(),vec[4].c_str(),vec[5].c_str()) ;  
                    return false ;
                }
                else
                {
                    dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::control: jump Euler attitude: %f %f %f %f\n",x,y,z,w) ;
                    setJumpAttitude(osg::Quat(x,y,z,w)) ;
                    return true ;
                }
            }
            else if (vec.size() == 5 && IsSubstring("orientation",vec[1],3))
            {
                double h, p, r;
                if (!StringToDouble(vec[2],&h) ||
                    !StringToDouble(vec[3],&p) ||
                    !StringToDouble(vec[4],&r))
                {
                    dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::control: invalid jump attitude paramaters: %s %s %s\n",vec[2].c_str(),vec[3].c_str(),vec[4].c_str(),vec[5].c_str()) ;  
                    return false ;
                }
                else
                {
                    dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::control: jump quaternion orientation: %f %f %f\n",h,p,r) ;
                    setJumpAttitude(EulerToQuat(h,p,r)) ;
                    return true ;
                }
            }
        }

        else if (vec.size() == 3 && IsSubstring("load", vec[0], 3))
        {      
            return _load(vec[1], vec[2]) ;
        }

        else if (vec.size() == 2 && IsSubstring("load", vec[0], 3))
        {      
            return _load(vec[1]) ;
        }

        else if (vec.size() == 3 && IsSubstring("startmodeldata", vec[0]))
        {      
            return _startModelData(vec[1], vec[2]) ;
        }
        
        else if (vec.size() == 1 && IsSubstring("endmodeldata", vec[0]))
        {      
            return _endModelData() ;
        }

        else if (IsSubstring("meters", vec[0], 3) && vec.size()==2)
        {
            double f ;
            if (StringToDouble(vec[1],&f))
            {
                dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::control: setting meters to %f\n",f) ;
                _meters = f ;
                return true ;
            }
            else return false ;
        }

        else if (IsSubstring("matrix", vec[0], 3))
        {
            if (vec.size()>=2) return _matrix(vec) ;
        }

        else if (IsSubstring("mergestate",vec[0],3) && vec.size() == 3)
        {
            return _mergeState(vec[1], vec[2]) ;
        }

        else if (IsSubstring("nolighting", vec[0], 3))
        {
            if (vec.size()==1) return noLighting() ;
            else if (vec.size()==2) return noLighting(vec[1]) ;
            else return false ;
        }

        else if (IsSubstring("nodemask", vec[0], 3) && vec.size() == 3)
        {
            return nodemask(vec[1],vec[2]) ;
        }

        else if (IsSubstring("normals", vec[0], 3))
        {
            if (vec.size()==1) return normals() ;
            else if (vec.size()==2) return normals(vec[1]) ;
            else return false ;
        }

        else if (IsSubstring("nostatesets", vec[0], 3))
        {
            if (vec.size()==1) return noStateSets() ;
            else if (vec.size()==2) return noStateSets(vec[1]) ;
            else return false ;
        }

        else if (IsSubstring("optimize", vec[0], 3))
        {
            if (vec.size()==1) return optimize() ;
            else if (vec.size()==2) return optimize(vec[1]) ;
            else
            {
                std::string opts = vec[2] ;
                for (unsigned int i=3; i<vec.size(); i++) opts += ":" + vec[i] ;
                return optimize(vec[1],opts) ;
            }
        }

        else if (IsSubstring("qdcs", vec[0], 3))
        {
            return _dcs(true, true, vec) ;
        }

        else if (IsSubstring("query", vec[0], 3) && vec.size()>2)
        {
            return _query(line, vec) ;
        }

        else if (IsSubstring("qscs", vec[0], 3))
        {
            return _dcs(false, true, vec) ;
        }

        else if (vec.size() == 3 && IsSubstring("removechild", vec[0], 3))
        {
            return _removeChild(vec[1], vec[2]) ;
        }

        // unadvertised special
        else if (vec.size() == 2 && IsSubstring("refcount", vec[0], 3))
        {      
            return getReferenceCount(vec[1]) ;
        }

        else if (IsSubstring("scs", vec[0], 3))
        {
            return _dcs(false, false, vec) ;
        }

        else if (IsSubstring("shmmatrixtransform", vec[0], 3))
        {
            return _shmMatrix(vec) ;
        }

        else if (IsSubstring("static", vec[0], 3))
        {
            if (vec.size()==1) return staticDataVariance() ;
            else if (vec.size()==2) return staticDataVariance(vec[1]) ;
            else return false ;
        }

        else if (vec.size() == 1 && IsSubstring("terminate",vec[0]))
        {
            dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::control: TERMINATE command received\n") ;
            setDone(true) ;
            return true  ;
        }

        else if (vec.size() == 2 && IsSubstring("unload", vec[0]))
        {
            return _unload(vec[1]) ;
        }

        ////////////////////////////////////////////////////////////////////////
        // see if it's a control method in another class
        else if (vec.size() > 1 && IsSubstring("augment", vec[0], 3))
        {      
            std::vector<std::string> vec2 = vec ;
            vec2.erase(vec2.begin()) ;
            return _augment.control(line, vec2) ; ;
        }

        else if (vec.size() > 1 && IsSubstring("navigation", vec[0], 3))
        {      
            std::vector<std::string> vec2 = vec ;
            vec2.erase(vec2.begin()) ;
            return _navigation.control(line, vec2) ; ;
        }

        else if (vec.size() > 1 && IsSubstring("window", vec[0], 3))
        {      
            std::vector<std::string> vec2 = vec ;
            vec2.erase(vec2.begin()) ;
            return _window.control(line, vec2) ; ;
        }

        else if (vec.size() > 1 && IsSubstring("pane", vec[0], 3))
        {      
            std::vector<std::string> vec2 = vec ;
            vec2.erase(vec2.begin()) ;
            return _pane.control(line, vec2) ; ;
        }

        else if (vec.size() > 1 && IsSubstring("immersivepane", vec[0], 3))
        {      
            std::vector<std::string> vec2 = vec ;
            vec2.erase(vec2.begin()) ;
            return _immersivePane.control(line, vec2) ; ;
        }

        else if (vec.size() > 1 && IsSubstring("perspectivepane", vec[0], 3))
        {      
            std::vector<std::string> vec2 = vec ;
            vec2.erase(vec2.begin()) ;
            return _perspectivePane.control(line, vec2) ; ;
        }

        else if (vec.size() > 1 && IsSubstring("orthopane", vec[0], 3))
        {      
            std::vector<std::string> vec2 = vec ;
            vec2.erase(vec2.begin()) ;
            return _orthoPane.control(line, vec2) ; ;
        }

        ////////////////////////////////////////////////////////////////////////
        // see if it's a DSO
        else if (vec.size() > 1 && IsSubstring("dso",vec[0],3))
        {
            std::vector<std::string> vec2 = vec ;
            vec2.erase(vec2.begin()) ;
            // these are for DSO-specific control commands- either handled by the DSO itself or the Augment class
            Augment* dso = Augment::getOrLoadDSO(vec2[0].c_str()) ;
            if (!dso || dso->isInvalid())
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::control: couldn't load DSO %s\n",vec2[0].c_str()) ;  
                return false ;
            }
            else
            {
                vec2.erase(vec2.begin()) ;
                if (vec2.size()>0)
                {
                    dtkMsg.add(DTKMSG_DEBUG, "iris::SceneGraph::control: calling method in DSO %s, line = %s\n",dso->getName(), line.c_str()) ;
                    return dso->control(line, vec2) ;
                }
                else return true ;
            }
        }

        ////////////////////////////////////////////////////////////////////////        
        // record when commands
        else if (vec.size()>1)
        {
            std::vector<std::string> vec2 = vec ;
            vec2.erase(vec2.begin()) ;
            // try when commands
            if (vec.size() > 3 && (IsSubstring("before", vec[0], 3) || IsSubstring("after", vec[0], 3)))
            {
                return recordWhenCommand(line, vec) ;
            }
        }

        ////////////////////////////////////////////////////////////////////////
        // you get here if all else failed
        dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::control passed unknown command: \"%s\"\n",line.c_str()) ;
        return false ;

    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::pollKey(int k)
    {
        std::set<int>::iterator pos ;
        pos = _ViewerKeyboardMousePollingAndScenegraphUpdate::_keyState.find(k) ;
        if (pos != _ViewerKeyboardMousePollingAndScenegraphUpdate::_keyState.end()) return true ;
        return false ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::pollButton(int b)
    {
        std::set<int>::iterator pos ;
        pos = _ViewerKeyboardMousePollingAndScenegraphUpdate::_buttonState.find(b) ;
        if (pos != _ViewerKeyboardMousePollingAndScenegraphUpdate::_buttonState.end()) return true ;
        return false ;
    }

    ////////////////////////////////////////////////////////////////////////
    std::vector<int> SceneGraph::getPressedKeys()
    {
        std::vector<int> v ;
        std::set<int>::iterator pos ;
        for (pos=_ViewerKeyboardMousePollingAndScenegraphUpdate::_keyState.begin(); pos!=_ViewerKeyboardMousePollingAndScenegraphUpdate::_keyState.end(); pos++) v.push_back(*pos) ;
        return v ;
    }

    ////////////////////////////////////////////////////////////////////////
    std::vector<int> SceneGraph::getPressedButtons()
    {
        std::vector<int> v ;
        std::set<int>::iterator pos ;
        for (pos=_ViewerKeyboardMousePollingAndScenegraphUpdate::_buttonState.begin(); pos!=_ViewerKeyboardMousePollingAndScenegraphUpdate::_buttonState.end(); pos++) v.push_back(*pos) ;
        return v ;
    }

    ////////////////////////////////////////////////////////////////////////
    unsigned int SceneGraph::getButtonMask()
    {
        return _ViewerKeyboardMousePollingAndScenegraphUpdate::_buttonMask ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::recordWhenCommand(const std::string& line, const std::vector<std::string>& vec)
    {
    
        dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::recordWhenCommand: recording \"%s\"\n",line.c_str()) ;
        bool before ;
        if (IsSubstring("before",vec[0],3)) before = true ;
        else if (IsSubstring("after",vec[0],3)) before = false ;
        else return false ;

        // get local copies, toss before/after
        std::vector<std::string> cmdVec ;
        cmdVec = vec ;
        cmdVec.erase(cmdVec.begin()) ;
        std::string cmdLine ;
        cmdLine = line ;
        ChopFirstWord(&cmdLine) ;

        if (cmdVec.size()<2) 
        {
            dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::recordWhenCommand: not enough parameters\n") ;
            return false ;
        }

        enum When { REALIZE, FRAME, TIME, ABSTIME, CLEANUP } ;
        When when ;
        if (IsSubstring("realize",cmdVec[0],3)) when = REALIZE ;
        else if (IsSubstring("frame",cmdVec[0],3)) when = FRAME ;
        else if (IsSubstring("time",cmdVec[0],3)) when = TIME ;
        else if (IsSubstring("abstime",cmdVec[0],3)) when = ABSTIME ;
        else if (IsSubstring("cleanup",cmdVec[0],3)) when = CLEANUP ;
        else 
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::recordWhenCommand: unknown event keyword \"%s\"\n",cmdVec[0].c_str()) ;
            return false ;
        }

        // toss when it's supposed to happen
        cmdVec.erase(cmdVec.begin()) ;
        ChopFirstWord(&cmdLine) ;

        int frameNumber ;
        double refTime ;
        if (when == FRAME)
        {
            if (cmdVec.size()<2)
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::recordWhenCommand: not enough parameters\n") ;
                return false ;
            }
            if (!StringToInt(cmdVec[0], &frameNumber) || frameNumber<0)
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::recordWhenCommand: invalid frame number %s\n",cmdVec[0].c_str()) ;
                return false ;
            }
            // toss frame number
            cmdVec.erase(cmdVec.begin()) ;
            ChopFirstWord(&cmdLine) ;
        }

        if (when == TIME || when == ABSTIME)
        {
            if (cmdVec.size()<2)
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::recordWhenCommand: not enough parameters\n") ;
                return false ;
            }
            if (!StringToDouble(cmdVec[0], &refTime) || refTime<0)
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::recordWhenCommand: invalid time %s\n",cmdVec[0].c_str()) ;
                return false ;
            }
            // convert relative time to absolute time
            if (when == TIME) refTime += getViewer()->getFrameStamp()->getReferenceTime() ;
            else if (when == ABSTIME) when = TIME ;

            // toss time
            cmdVec.erase(cmdVec.begin()) ;
            ChopFirstWord(&cmdLine) ;
        }

        if (before) // just before
        {
            if (when == REALIZE) 
            {
                if (_realized)
                {
                    dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::recordWhenCommand: BEFORE REALIZE command received after Viewer was realized\n") ;
                    return false ;
                }
                _beforeRealize.push_back(cmdLine) ;
            }
            else if (when == FRAME) 
            {
                if (frameNumber < getViewer()->getFrameStamp()->getFrameNumber())
                {
                    dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::recordWhenCommand: BEFORE FRAME %d command received when frame number is %d\n",frameNumber, getViewer()->getFrameStamp()->getFrameNumber()) ;
                    return false ;
                }
                _beforeFrameN.push_back(std::make_pair(cmdLine,frameNumber)) ;
            }
            else if (when == TIME) 
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::recordWhenCommand: BEFORE TIME command received\n") ;
                return false ;
            }
            else if (when == CLEANUP) 
            {
                // can't be an error as we don't record new commands after cleanup
                _beforeCleanup.push_back(cmdLine) ;
            }
            else return false ;
        }
        else // any time after
        {
            if (when == REALIZE) _afterRealize.push_back(cmdLine) ;
            else if (when == FRAME) _afterFrameN.push_back(std::make_pair(cmdLine,frameNumber)) ;
            else if (when == TIME) _afterTimeN.push_back(std::make_pair(cmdLine,refTime)) ;
            else if (when == CLEANUP) _afterCleanup.push_back(cmdLine) ;
            else return false ;
        }

        return true ;
    }

    void SceneGraph::processBeforeRealizeWhenCommands()
    {
        std::list<std::string>::iterator pos = _beforeRealize.begin() ;
        while (pos!=_beforeRealize.end())
        {
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::processBeforeRealizeWhenCommands: \"%s\"\n",(*pos).c_str()) ;
            control(*pos) ;
            pos=_beforeRealize.erase(pos) ;
            //pos++ ;
        }
    }

    void SceneGraph::processAfterRealizeWhenCommands()
    {
        std::list<std::string>::iterator pos = _afterRealize.begin() ;
        while (pos!=_afterRealize.end())
        {
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::processAfterRealizeWhenCommands: \"%s\"\n",(*pos).c_str()) ;
            control(*pos) ;
            pos=_afterRealize.erase(pos) ;
            //pos++ ;
        }
    }

    void SceneGraph::processBeforeFrameWhenCommands()
    {
        std::list<std::pair<std::string,int> >::iterator pos = _beforeFrameN.begin() ;
        int frameNumber = getViewer()->getFrameStamp()->getFrameNumber() ;
        while (pos!=_beforeFrameN.end())
        {
            int n = pos->second ;
            if (n == frameNumber)
            {
                dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::processBeforeFrameWhenCommands: frame %d, before frame %d: \"%s\"\n",frameNumber,n,(pos->first).c_str()) ;
                control(pos->first) ;
                pos=_beforeFrameN.erase(pos) ;
            }
            else pos++ ;
        }
    }

    void SceneGraph::processAfterFrameWhenCommands()
    {
        std::list<std::pair<std::string,int> >::iterator pos = _afterFrameN.begin() ;
        int frameNumber = getViewer()->getFrameStamp()->getFrameNumber() ;
        while (pos!=_afterFrameN.end())
        {
            int n = pos->second ;
            if (n < frameNumber)
            {
                dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::processAfterFrameWhenCommands: frame %d, after frame %d: \"%s\"\n",frameNumber,n,(pos->first).c_str()) ;
                control(pos->first) ;
                pos=_afterFrameN.erase(pos) ;
            }
            else pos++ ;
        }
    }

    void SceneGraph::processAfterTimeWhenCommands()
    {
        std::list<std::pair<std::string,double> >::iterator pos = _afterTimeN.begin() ;
        double refTime = getViewer()->getFrameStamp()->getReferenceTime() ;
        while (pos!=_afterTimeN.end())
        {
            double t = pos->second ;
            if (t < refTime)
            {
                dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::processAfterTimeWhenCommands: time %f, after %f: \"%s\"\n",(float)refTime,(float)t,(pos->first).c_str()) ;
                control(pos->first) ;
                pos=_afterTimeN.erase(pos) ;
            }
            else pos++ ;
        }
    }

    void SceneGraph::processBeforeCleanupWhenCommands()
    {
        std::list<std::string>::iterator pos = _beforeCleanup.begin() ;
        while (pos!=_beforeCleanup.end())
        {
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::processBeforeCleanupWhenCommands: \"%s\"\n",(*pos).c_str()) ;
            control(*pos) ;
            //pos=_beforeCleanup.erase(pos) ;
            pos++ ;
        }
    }

    void SceneGraph::processAfterCleanupWhenCommands()
    {
        std::list<std::string>::iterator pos = _afterCleanup.begin() ;
        while (pos!=_afterCleanup.end())
        {
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::processAfterCleanupWhenCommands: \"%s\"\n",(*pos).c_str()) ;
            control(*pos) ;
            //pos=_afterCleanup.erase(pos) ;
            pos++ ;
        }
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::setUseSceneGraphIndex(int i)
    {
        if (i<0 || i>=_instances.size())
        {
            dtkMsg.add(DTKMSG_ERROR,"iris::SceneGraph::setUseSceneGraphIndex: index out of range\n") ;
            return false ;
        }
        _useSceneGraphIndex = i ;
        return true ;
    }

    bool SceneGraph::setUseViewerIndex(int i) 
    {
        if (i<0 || i>=_viewers.size())
        {
            dtkMsg.add(DTKMSG_ERROR,"iris::SceneGraph::setUseViewerIndex: index out of range\n") ;
            return false ;
        }
        _useViewerIndex = i ;
        return true ;
    }
    
    ////////////////////////////////////////////////////////////////////////
    bool  SceneGraph::examine(const std::string& nodeName)
    {
        dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::examine: using node %s\n",nodeName.c_str()) ;
        osg::Node* node = findNode(nodeName) ;
        MatrixTransform* matNode ;
        if (node)
        {
            // node exists - is an MatrixTransform?
            matNode = dynamic_cast<MatrixTransform*>(node) ;
            if (!matNode)
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::examine: node named %s exists but is not a MatrixTransform\n",nodeName.c_str()) ;            
                return false ;
            }
            // do it
            matNode->setScale(osg::Vec3(1.f, 1.f, 1.f),false) ;
            matNode->setPosition(osg::Vec3(0.f, 0.f, 0.f)) ;
            
#if 1
            osg::BoundingBox bb = GetBoundingBox(matNode);
            osg::BoundingSphere s(bb.center(),bb.radius()) ; ;
#else
            osg::BoundingSphere s = matNode->getBound() ;
#endif
            
            if (s.radius() <= 0.0)
            {
                dtkMsg.add(DTKMSG_WARNING, "iris::SceneGraph::examine: node named %s has bounding sphere radius of zero, setting radius to 1.0\n",nodeName.c_str()) ;            
                s.radius() = 1.0f ;
            }
            // where its center will initially be located, in normalized units
            const osg::Vec3 examineCenter(0.f,2.f,0.f) ;
            
            // the radius of a cube -1 to 1 in normalized units
            // will be co-planar with toggleObjects cube
            //const double examineBound = sqrt(3.f) ;
            // just give it a bounding sphere of 1
            const double examineBound = 1.f ;
            
            // the units in a matrix node are in the cordinate system of the parent node, so the
            // arguments are in normalized coordinates
            
            // make big objects smaller and larger objects smaller so they will fit in the window
            double examineScale = examineBound/s.radius() ;
            matNode->setScale(osg::Vec3(examineScale,examineScale,examineScale),false) ;
            
            // need to take into account the size/center of the object and where you want it to go
            // examineOffset is in normalized coordinates, so scale s.center() to make it smaller
            // if object is big, bigger if object is small
            osg::Vec3 examineOffset = examineCenter - s.center()*examineScale ;
            matNode->setPosition(examineOffset) ;
            
            // this is what you want to rotate around, in coordinates of the node itself, 
            // the center of the object's b-sphere
            Nav::setPivotNode(matNode) ;
            Nav::setResetPivotNode(matNode) ;
            Nav::setPivotPoint(s.center()) ;
            Nav::setResetPivotPoint(s.center()) ;
            
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::examine: setting position to %f %f %f, scale to %f\n", examineOffset.x(), examineOffset.y(), examineOffset.z(), examineScale) ;
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::examine: setting pivot point to %f %f %f, pivot node to world\n",s.center().x(), s.center().y(),s.center().z()) ;
        }
        else
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::examine: node named %s does not exist.\n",nodeName.c_str()) ;            
            return false ;
        }
        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool  SceneGraph::polygonMode(const std::string& nodeName, const std::string& modeStr, const std::string& faceStr)
    {
        dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::polygonMode: using node %s\n",nodeName.c_str()) ;
        osg::Node* node = findNode(nodeName) ;
        if (node)
        {
            osg::ref_ptr<osg::PolygonMode> polygonMode = new osg::PolygonMode ;
            osg::PolygonMode::Mode mode ;
            if (IsSubstring("point", modeStr)) mode = osg::PolygonMode::POINT ;
            else if (IsSubstring("line", modeStr)) mode = osg::PolygonMode::LINE ;
            else if (IsSubstring("fill", modeStr)) mode = osg::PolygonMode::FILL ;
            else
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::polygonMode: unknown osg::PolygonMode::Mode %s.\n",modeStr.c_str()) ;
                return false ;
            }

            osg::PolygonMode::Face face ;
            if (IsSubstring("front", faceStr)) face = osg::PolygonMode::FRONT ;
            else if (IsSubstring("back", faceStr)) face = osg::PolygonMode::BACK ;
            else if (IsSubstring("front_and_back", faceStr)) face = osg::PolygonMode::FRONT_AND_BACK ;
            else
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::polygonMode: unknown osg::PolygonMode::Face %s.\n",faceStr.c_str()) ;
                return false ;
            }

            dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::polygonMode: setting node \"%s\": face: %s, mode: %s\n",nodeName.c_str(),faceStr.c_str(),modeStr.c_str()) ;            
            polygonMode->setMode(face,mode);

            osg::StateSet* stateset = node->getOrCreateStateSet() ;
            stateset->setAttributeAndModes(polygonMode,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
        }
        else
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::polygonMode: node named %s does not exist.\n",nodeName.c_str()) ;                        return false ;
        }
        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool  SceneGraph::bound(const std::string& nodeName, double radius)
    {
        // find the node and see if it's a matrix
        osg::Node* node = findNode(nodeName) ;
        MatrixTransform* matNode ;
        if (node)
        {
            // node exists - is a matrix?
            matNode = dynamic_cast<MatrixTransform*>(node) ;
            if (!matNode)
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::bound: node named %s exists but is not a iris::MatrixTransform\n",nodeName.c_str()) ;            
                return false ;
            }
            // do it
            matNode->setScale(osg::Vec3(1.f, 1.f, 1.f)) ;
            osg::BoundingSphere s = matNode->getBound() ;
            osg::Vec3 sc = matNode->getScale() ;
            
            osg::Vec3 sc2 = sc*(radius/s.radius()) ;
            
            matNode->setScale(sc2) ;
            return true ;
        }
        else
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::bound: node named %s does not exist.\n",nodeName.c_str()) ;            
            return false ;
        }
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::center(const std::string& nodeName, const osg::Vec3& center)
    {
        // find the node and see if it's a matrix
        osg::Node* node = findNode(nodeName) ;
        MatrixTransform* matNode ;
        if (node)
        {
            // node exists - is a matrix?
            matNode = dynamic_cast<MatrixTransform*>(node) ;
            if (!matNode)
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::center: node named %s exists but is not a iris::MatrixTransform\n",nodeName.c_str()) ;            
                return false ;
            }
            // do it
            matNode->setPosition(osg::Vec3(0.f, 0.f, 0.f)) ;
            osg::BoundingSphere s = matNode->getBound() ;
            matNode->setPosition(center - s.center()/s.radius()) ;
            return true ;
        }
        else
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::center: node named %s does not exist.\n",nodeName.c_str()) ;            
            return false ;
        }
        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::normals(const std::string& nodeName)
    {
        osg::Node* node = findNode(nodeName) ;
        if (node)
        {
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::normals: generating smooth normals for node %s\n",nodeName.c_str()) ;
            osgUtil::SmoothingVisitor sv ;
            node->accept(sv) ;
            return true ;
        }
        else
        {
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::normals: can't find node named %s\n",nodeName.c_str()) ;
            return false ;
        }
        return true ;
    }
 
    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::optimize(const std::string& nodeName, const std::string& optimizations)
    {
        osg::Node* node = findNode(nodeName) ;
        if (node)
        {
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::optimize: optimizing node %s\n",nodeName.c_str()) ;
            if (optimizations != "") return iris::Optimize(node, optimizations) ;
            else return iris::Optimize(node) ;
        }
        else
        {
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::optimize: can't find node named %s\n",nodeName.c_str()) ;
            return false ;
        }
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::staticDataVariance(const std::string& nodeName)
    {
        osg::Node* node = findNode(nodeName) ;
        if (node)
        {
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::staticDataVariance: setting node %s\n",nodeName.c_str()) ;
            ConvertNodeVisitor nv ;
            nv.setStatic(true) ;
            nv.apply(*node) ;
            return true ;
        }
        else
        {
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::staticDataVariance: can't find node named %s\n",nodeName.c_str()) ;
            return false ;
        }
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::copyPrimitives(const std::string& nodeName)
    {
        osg::Node* node = findNode(nodeName) ;
        if (node)
        {
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::copyPrimitives: setting node %s\n",nodeName.c_str()) ;
            ConvertNodeVisitor nv ;
            nv.setCopyPrimitives(true) ;
            nv.apply(*node) ;
            return true ;
        }
        else
        {
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::copyPrimitives: can't find node named %s\n",nodeName.c_str()) ;
            return false ;
        }
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::noStateSets(const std::string& nodeName)
    {
        osg::Node* node = findNode(nodeName) ;
        if (node)
        {
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::noStateSets: setting node %s\n",nodeName.c_str()) ;
            ConvertNodeVisitor nv ;
            nv.setNoStateSets(true) ;
            nv.apply(*node) ;
            return true ;
        }
        else
        {
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::noStateSets: can't find node named %s\n",nodeName.c_str()) ;
            return false ;
        }
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::noLighting(const std::string& nodeName)
    {
        osg::Node* node = findNode(nodeName) ;
        if (node)
        {
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::noLighting: setting node %s\n",nodeName.c_str()) ;
            ConvertNodeVisitor nv ;
            nv.setNoLighting(true) ;
            nv.apply(*node) ;
            return true ;
        }
        else
        {
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::noLighting: can't find node named %s\n",nodeName.c_str()) ;
            return false ;
        }
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::kill(const std::string& pid)
    {
        int n ;
        if (StringToInt(pid, &n)) addPID(n) ;
        else return false ;
        dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::control: adding PID %d to list of pids to be killed at exit\n",n) ;
        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::exec(const std::string& command, bool waitChild)
    {
        std::string execCommand ;
        if (waitChild) execCommand = ChopFirstWord(command) ;
        else execCommand = "exec " + ChopFirstWord(command) ; 
        
        char* eargv[4] ;
        eargv[0] = const_cast<char*>("sh") ;
        eargv[1] = const_cast<char*>("-c") ;
        eargv[2] = const_cast<char*>(execCommand.c_str()) ;
        eargv[3] = NULL ;
        
        if (waitChild) dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::control: waiting for command \'%s\'\n",execCommand.c_str()) ;
        else dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::control: exec'ing command \'%s\'\n",execCommand.c_str()) ;

        // fork takes too damn long when a lot of model files are loaded
        pid_t childPid = vfork() ;
        if (childPid == -1)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::control: unable to vfork!\n") ;
            return false ;
        }
        else if (childPid == 0)
        {
            int ret = execvp("sh",eargv) ;
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::control: if you see this something went wrong with the exec, ret = %d\n",ret) ;
            return false ;
        }
        else
        {
            if (waitChild)
            {
                waitpid(childPid, NULL, 0) ;
                dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::control: finished command \'%s\'\n",execCommand.c_str()) ;
            }
            else
            {
                dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::control: adding PID %d to list of pids to be killed at exit\n",childPid) ;
                addPID(childPid) ;
            }
            return true ;
        }
        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::background(float r, float g, float b)
    {
        setClearColor(osg::Vec4(r,g,b,0.)) ;
        std::vector<osg::ref_ptr<Pane> > pl = Pane::getPaneList() ;
        for (unsigned int p=0; p<pl.size(); p++)
        {
            std::vector<osg::ref_ptr<osg::Camera> > cl = pl[p]->getCameraList() ;
            for (unsigned int c=0; c<cl.size(); c++)
            {
                cl[c]->setClearColor(osg::Vec4(r,g,b,0.)) ;
            }
        }
        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::dump(const std::string& nodeName, const std::string& fileName)
    {
        //~fprintf(stderr,"node name = %s\n",nodeName.c_str()) ;
        //~fprintf(stderr,"%s %d\n",__FILE__,__LINE__) ;
        SceneGraph* isg = SceneGraph::instance() ;
        //~fprintf(stderr,"isg = %p\n",isg) ;
        osg::Node* node = findNode(nodeName) ;

        if (!node)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::dump: Unable to find node named \"%s\"\n",nodeName.c_str()) ;
            return false ;
        }
        //~fprintf(stderr,"%s %d\n",__FILE__,__LINE__) ;
        isg = SceneGraph::instance() ;
        //~fprintf(stderr,"isg = %p\n",isg) ;
        //~fprintf(stderr,"%s %d\n",__FILE__,__LINE__) ;
        osg::ref_ptr<osg::Group> foo = new osg::Group ;
        foo->addChild(node) ;
        osg::ref_ptr<osg::Group> bar = new osg::Group ;
        bar->addChild(node) ;
        //if (!osgDB::writeNodeFile(*node, fileName))
        if (!osgDB::writeNodeFile(*foo, fileName))
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::dump: Unable to dump to file \"%s\"\n", fileName.c_str()) ;
        }
        else
        {
            dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::dump: Node \"%s\" dumped to file \"%s\"\n", nodeName.c_str(),fileName.c_str()) ;
        }
        //~fprintf(stderr,"%s %d\n",__FILE__,__LINE__) ;
        //~fprintf(stderr,"isg = %p\n",isg) ;
        isg = SceneGraph::instance() ;
        //~fprintf(stderr,"%s %d\n",__FILE__,__LINE__) ;
        return true ;
    }




    ///////////////////////////////////////////////////////////////////
    //
    // The following code implements the SHADERPROG command.
    //

    // Here is the syntax for the command:
    //
    // SHADERPROG nodeName  _REMOVE_      (remove all existing shaders)
    // or
    // SHADERPROG nodeName  [vertShader] [geomShader] [fragShader]
    //
    // where 
    //         vertShader is the specification of a vertex shader:
    //                 V _REMOVE_   (remove any existing vertex shader)
    //                 or
    //                 V fileName   
    // 
    //         geomShader is the specification of a geometry shader:
    //                 V _REMOVE_   (remove any existing geometry shader)
    //                 or
    //                 G inPrim outoutPrim maxV fileName
    // 
    //                 where:
    // 
    //                   inPrim is the input primitive type:
    //                         P       points
    //                         L       lines
    //                         LA      lines with adjacency
    //                         T       triangles
    //                         TA      triangles with adjacency
    // 
    //                   outPrim is the output primitive type:
    //                         P       points
    //                         LS      line strips
    //                         TS      triangle strips
    // 
    //                   maxV is the maximum number of output vertices
    // 
    //         fragShader is the specification of a fragmentshader:
    //                 F _REMOVE_   (remove any existing fragment shader)
    //                 or
    //                 F fileName   
    //
    // The vertex, geometry, or fragment shaders specifications can be
    // in any order.  One, two, or three shaders can be specified on
    // each command line.  
    //
    // Shaders that are not specified are not modified at the specified node.


    // This implementation consists of two helper routines (getShaderProgInfo 
    // and removeShader) and SceneGraph::_shaderProg.

    
    // Extract the shader program specifications from the tokenized
    // command line.
    static
    bool getShaderProgInfo (const std::vector<std::string> & vec, 
                            std::string & vertFN,
                            unsigned int & geomVertsOut,
                            GLint & geomInType,
                            GLint & geomOutType,
                            std::string & geomFN,
                            std::string & fragFN,
                            GLint & compNumGroupsX,
                            GLint & compNumGroupsY,
                            GLint & compNumGroupsZ,                           
                            std::string & compFN )
    {

        vertFN = "";
        geomVertsOut = 1;
        geomInType = osg::PrimitiveSet::TRIANGLES;
        geomOutType = osg::PrimitiveSet::TRIANGLE_STRIP;
        geomFN = "";
        fragFN = "";
        compFN = "";

        int currTok = 2;

        while (currTok < vec.size())
        {
            if ( (vec[currTok] == "v") || (vec[currTok] == "V") )
            {
                if (vertFN != "")
                {
                    dtkMsg.add (DTKMSG_ERROR, 
                    "iris::SceneGraph::shaderProg: "
                        "Vertex shader specified twice.\n");
                    return false;
                }

                if ( (currTok + 1) >= vec.size() )
                {
                    dtkMsg.add (DTKMSG_ERROR, 
                    "iris::SceneGraph::shaderProg: "
                        "Bad argument count.\n");
                    return false;
                }
        
                vertFN = vec[currTok+1];

                currTok += 2;
            }  // end of vertex

            else if ( (vec[currTok] == "g") || (vec[currTok] == "G") )
            {
                if (geomFN != "")
                {
                    dtkMsg.add (DTKMSG_ERROR, 
                    "iris::SceneGraph::shaderProg: "
                        "Fragment shader specified twice.\n");
                    return false;
                }

                // if there is not at least one more token, we have a problem.
                if ( (currTok + 1) >= vec.size() )
                {
                    dtkMsg.add (DTKMSG_ERROR, 
                    "iris::SceneGraph::shaderProg: "
                        "Bad argument count.\n");
                    return false;
                }
        

                // if the first token is _REMOVE_
                if (vec[currTok+1] == "_REMOVE_")
                {
                    // special case when geom is just specified as _REMOVE_
                    geomFN = vec[currTok+1];
                    currTok += 2;
                }  // end of section for G specified as _REMOVE_
                else
                {


                    // If first token is not _REMOVE_ then we should have
                    // four more tokens.

                    if ( (currTok + 4) >= vec.size() )
                    {
                        dtkMsg.add (DTKMSG_ERROR, 
                        "iris::SceneGraph::shaderProg: "
                            "Bad argument count.\n");
                        return false;
                    }
        

                    // interpret the input primitive type
                    if (strcasecmp (vec[currTok+1].c_str(), "P") == 0)
                    {
                        geomInType = osg::PrimitiveSet::POINTS;
                    }
                    else if (strcasecmp (vec[currTok+1].c_str(), "L") == 0)
                    {
                        geomInType = osg::PrimitiveSet::LINES;
                    }
                    else if (strcasecmp (vec[currTok+1].c_str(), "LA") == 0)
                    {
                        geomInType = osg::PrimitiveSet::LINES_ADJACENCY;
                    }
                    else if (strcasecmp (vec[currTok+1].c_str(), "T") == 0)
                    {
                        geomInType = osg::PrimitiveSet::TRIANGLES;
                    }
                    else if (strcasecmp (vec[currTok+1].c_str(), "TA") == 0)
                    {
                        geomInType = osg::PrimitiveSet::TRIANGLES_ADJACENCY;
                    }
                    else
                    {
                        dtkMsg.add (DTKMSG_ERROR, 
                             "iris::SceneGraph::shaderProg: "
                             "Bad input geometry type: %s.\n", 
                                    vec[currTok+1].c_str() );
                            return false;
                    }


                    // interpret the output primitive type
                    if (strcasecmp (vec[currTok+2].c_str(), "P") == 0)
                    {
                        geomOutType = osg::PrimitiveSet::POINTS;
                    }
                    else if (strcasecmp (vec[currTok+2].c_str(), "LS") == 0)
                    {
                        geomOutType = osg::PrimitiveSet::LINE_STRIP;
                    }
                    else if (strcasecmp (vec[currTok+2].c_str(), "TS") == 0)
                    {
                        geomOutType = osg::PrimitiveSet::TRIANGLE_STRIP;
                    }
                    else
                    {
                        dtkMsg.add (DTKMSG_ERROR, 
                             "iris::SceneGraph::shaderProg: "
                             "Bad output geometry type: %s.\n", 
                                vec[currTok+2].c_str() );
                            return false;
                    }



                    // interpret the max number of vertices
                    unsigned int iVerts;
                    if ( ! StringToUInt (vec[currTok+3], &iVerts) )
                    {
                        dtkMsg.add (DTKMSG_ERROR, 
                         "iris::SceneGraph::shaderProg: "
                         "Bad number of vertices in geometry spec: %s.\n", 
                                vec[currTok+3].c_str() );
                        return false;
                    }

                    geomVertsOut = iVerts;

                    // final token is the file name
                    geomFN = vec[currTok+4];
                    currTok += 5;
                }  // end of section for G NOT specified as _REMOVE_

            }  // end of geometry

            else if ( (vec[currTok] == "f") || (vec[currTok] == "F") )
            {
                if (fragFN != "")
                {
                    dtkMsg.add (DTKMSG_ERROR, 
                    "iris::SceneGraph::shaderProg: "
                        "Fragment shader specified twice.\n");
                    return false;
                }

                if ( (currTok + 1) >= vec.size() )
                {
                    dtkMsg.add (DTKMSG_ERROR, 
                    "iris::SceneGraph::shaderProg: "
                        "Bad argument count.\n");
                    return false;
                }
        
                fragFN = vec[currTok+1];
                currTok += 2;
            } // end of fragment

            else if ( (vec[currTok] == "c") || (vec[currTok] == "C") )
            {
                if (compFN != "")
                {
                    dtkMsg.add (DTKMSG_ERROR, 
                    "iris::SceneGraph::shaderProg: "
                        "Compute shader specified twice.\n");
                    return false;
                }

                if ( (currTok + 1) >= vec.size() )
                {
                    dtkMsg.add (DTKMSG_ERROR, 
                    "iris::SceneGraph::shaderProg: "
                        "Bad argument count.\n");
                    return false;
                }
        
                // number of work groups
                unsigned int cNumGroupsX;
                unsigned int cNumGroupsY;
                unsigned int cNumGroupsZ;
                if (!StringToUInt (vec[currTok+1], &cNumGroupsX) ||
                    !StringToUInt (vec[currTok+2], &cNumGroupsY) ||
                    !StringToUInt (vec[currTok+3], &cNumGroupsZ))
                {
                    dtkMsg.add (DTKMSG_ERROR, 
                    "iris::SceneGraph::shaderProg: "
                    "Bad number of work groups: %s %s %s\n", 
                        vec[currTok+1].c_str(), vec[currTok+2].c_str(), vec[currTok+3].c_str());
                    return false;
                }

                compNumGroupsX = cNumGroupsX;
                compNumGroupsY = cNumGroupsY;
                compNumGroupsZ = cNumGroupsZ;

                compFN = vec[currTok+4];
                currTok += 5;

            } // end of compute

            else
            {
                dtkMsg.add (DTKMSG_ERROR, 
                "iris::SceneGraph::shaderProg: Unrecognized shader type: %s.\n",
                        vec[currTok].c_str());
                return false;
            }

        }  // end of while

        if ( (vertFN == "") && (geomFN == "") && (fragFN == "") && (compFN == "") )
        {
            // not sure how this could happen
            dtkMsg.add (DTKMSG_ERROR, 
                "iris::SceneGraph::shaderProg: No shaders specified.\n" );
            return false;
        }
    

        return true;

    } // end of getShaderProgInfo
                                

    /////////////////


    // Remove Shaders of the indicated type from the Program.
    static void removeShader (osg::Program *prog, osg::Shader::Type shaderType)
    {
        // I don't know that it's possible for a Program to have more than
        // one shader of the same type.  It shouldn't be, but I don't see
        // anything in the OSG code to prevent it.
        // So I'll go through all the shaders and remove those of the
        // specified type.

        std::vector<osg::Shader *> sList;

        for (int i = 0; i < prog->getNumShaders(); i++)
        {
            osg::Shader *shader = prog->getShader(i);
            if (shader->getType() == shaderType)
                {
                sList.push_back (shader);
                }
        }

        for (int i = 0; i < sList.size(); i++)
                {
                prog->removeShader (sList[i]);
                }

    } // end of removeShader



    // This method interprets the SHADERPROG command line and modifies
    // the specified node as appropriate.
    bool SceneGraph::_shaderProg (const std::vector<std::string> & vec)
    {

        // check the number of tokens in line
        if ( (vec.size() < 3) || (vec.size() > 11) )
        {
            dtkMsg.add (DTKMSG_ERROR, 
                "iris::SceneGraph::shaderProg: bad argument count.\n");
            return false ;
        }
        
        // Get the node
        osg::Node* node = findNode(vec[1]) ;
        if (!node)
        {
            dtkMsg.add (DTKMSG_ERROR, 
                "iris::SceneGraph::shaderProg: unable to find node named %s.\n",
                vec[1].c_str()) ;
            return false ;
        }

        // Get the node's StateSet
        osg::StateSet * stateSet = node->getOrCreateStateSet() ;
        stateSet->setDataVariance (osg::Object::DYNAMIC);

        // first see if it's asking us to remove the program
        if (vec.size() == 3)
        {
            if (vec[2] == "_REMOVE_")
            {
                // remove any existing program
                stateSet->removeAttribute (osg::StateAttribute::PROGRAM);
                return true;
            }
            else
            {
                dtkMsg.add (DTKMSG_ERROR, 
                "iris::SceneGraph::shaderProg: Unable to parse argument %s.\n",
                vec[2].c_str()) ;
                return false ;
            }

        }

        // If we got here, we have to interpret the vert, geom, and/or frag
        // shader specs.

        // Parse the line
        std::string vertFN;
        unsigned int geomVertsOut;
        GLint geomInType;
        GLint geomOutType;
        std::string geomFN;
        std::string fragFN;
        std::string compFN;
        GLint compNumGroupsX;
        GLint compNumGroupsY;
        GLint compNumGroupsZ;        
        if ( ! getShaderProgInfo (vec, 
               vertFN, geomVertsOut, geomInType, geomOutType, geomFN, fragFN, 
               compNumGroupsX, compNumGroupsY, compNumGroupsZ, compFN) )
        {
            return false;
        }

        // Should there be one Program object to hold all of the
        // specified shaders?  
        // Answer: One Program object to hold all shaders.
        
        // See if there is an existing Program in the StateSet
        osg::Program* prog;
        prog = dynamic_cast<osg::Program*> 
                (stateSet->getAttribute (osg::StateAttribute::PROGRAM));

        // If there isn't a program, create one.
        if (prog == NULL)
        {
            prog = new osg::Program;
            prog->setDataVariance(osg::Object::DYNAMIC) ;
            stateSet->setAttributeAndModes (prog, osg::StateAttribute::ON);
        }

        
        if (vertFN == "_REMOVE_")
        {
            // remove vertex shader
            removeShader (prog, osg::Shader::VERTEX);
        }
        else if (vertFN != "")
        {
            // create a new vertex shader and replace old one.
            osg::Shader* shadr = new osg::Shader( osg::Shader::VERTEX );
            // shadr->setDataVariance(osg::Object::DYNAMIC) ;

            if (! shadr->loadShaderSourceFromFile
                            ( osgDB::findDataFile(vertFN).c_str() ) )
            {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph : shaderprog : "
                "Unable to load shader source file %s.\n", vertFN.c_str() );
            return false;
            }

            removeShader (prog, osg::Shader::VERTEX);
            prog->addShader( shadr );
        }  // end of vert

        
        if (geomFN == "_REMOVE_")
        {
            // remove geometry shader
            removeShader (prog, osg::Shader::GEOMETRY);
            // Set the parameters to defaults.
            prog->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 1);
            prog->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, 
                                            osg::PrimitiveSet::TRIANGLES);
            prog->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, 
                                            osg::PrimitiveSet::TRIANGLE_STRIP);
        }
        else if (geomFN != "")
        {

            // create a new geometry shader and replace old one.
            osg::Shader* shadr = new osg::Shader( osg::Shader::GEOMETRY);
            // shadr->setDataVariance(osg::Object::DYNAMIC) ;

            if (! shadr->loadShaderSourceFromFile
                            ( osgDB::findDataFile(geomFN).c_str() ) )
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph : shaderprog : "
                    "Unable to load shader source file %s.\n", geomFN.c_str() );
                return false;
            }

            removeShader (prog, osg::Shader::GEOMETRY);
            prog->addShader( shadr );
            // set the parameters
            prog->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, geomVertsOut);
            prog->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, geomInType);
            prog->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, geomOutType);
        }  // end of geom


        if (fragFN == "_REMOVE_")
        {
            // remove fragment shader
            removeShader (prog, osg::Shader::FRAGMENT);
        }
        else if (fragFN != "")
        {
            // create a new fragment shader and replace old one.
            osg::Shader* shadr = new osg::Shader( osg::Shader::FRAGMENT );
            // shadr->setDataVariance(osg::Object::DYNAMIC) ;


            if (! shadr->loadShaderSourceFromFile
                            ( osgDB::findDataFile(fragFN).c_str() ) )
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph : shaderprog : "
                    "Unable to load shader source file %s.\n", fragFN.c_str() );
                return false;
            }

            removeShader (prog, osg::Shader::FRAGMENT);
            prog->addShader( shadr );
        }  // end of frag

        if (compFN == "_REMOVE_")
        {
            // remove fragment shader
            removeShader (prog, osg::Shader::COMPUTE);
        }
        else if (compFN != "")
        {
            // create a new compute shader and replace old one
            osg::Shader* shadr = new osg::Shader( osg::Shader::COMPUTE );

            if (! shadr->loadShaderSourceFromFile
                            ( osgDB::findDataFile(compFN).c_str() ) )
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph : shaderprog : "
                    "Unable to load shader source file %s.\n", compFN.c_str() );
                return false;
            }

            removeShader(prog, osg::Shader::COMPUTE);

            prog->setComputeGroups(compNumGroupsX, compNumGroupsY, compNumGroupsZ);
            prog->addShader(shadr);
            stateSet->setAttributeAndModes(prog);
        }

        return true;

    } // end of SceneGraph::shaderProg

    // end of code for SHADERPROG command
    ///////////////////////////////////////////////////////////////////




    ////////////////////////////////////////////////////////////////////////

    // The following code implements the UNIFORM command.

    //
    // Uniform syntax A is the original syntax of the UNIFORM command:
    //  UNIFORM nodeName uniformName value
    //
    bool SceneGraph::_uniformSyntaxA (std::vector<std::string> vec)
    {
        // printf ("uniformSyntaxA called\n");

        // vec[1]    node name
        // vec[2]    uniform name
        // vec[3]    value

        //fprintf(stderr,"uniform called, node = %s, name = %s, value = %s\n", vec[1].c_str(), vec[2].c_str(), vec[3].c_str()) ;


        float value ;
        if ( ! StringToFloat(vec[3], &value))
        {
            dtkMsg.add(DTKMSG_ERROR, 
                       "iris::SceneGraph : uniform (A) : Bad value %s.\n", vec[3].c_str() );
            return false;  // bad
        }

        // node must exist
        osg::Node *node = findNode(vec[1]) ;
        if (!node) 
        {
            dtkMsg.add(DTKMSG_ERROR, 
                       "iris::SceneGraph : uniform (A) : Node  %s not found.\n", 
                       vec[1].c_str() );
            return false;  // bad
        }

         osg::StateSet *s = node->getOrCreateStateSet() ;
        s->setDataVariance(osg::Object::DYNAMIC) ;
        osg::Uniform *u = new osg::Uniform(vec[2].c_str(), value) ;
        u->setDataVariance(osg::Object::DYNAMIC) ;
        s->addUniform(u) ;
    
        return true ;
    }  // end of SceneGraph::_uniformSyntaxA



    //
    // Uniform syntax B is the extended syntax for setting uniforms:
    //   UNIFORM nodeName uniformName typeName count value0 value1 . . . .
    //
    bool SceneGraph::_uniformSyntaxB (std::vector<std::string> vec)
    {
        // printf ("\nuniformSyntaxB called\n");

        // vec[1]    node name
        // vec[2]    uniform name
        // vec[3]    typeName
        // vec[4]    count
        // vec[5]... values for uniform

        osg::Node *node = findNode(vec[1]) ;
        if (!node) 
        {
            dtkMsg.add(DTKMSG_ERROR, 
                       "iris::SceneGraph : uniform (B) : Node  %s not found.\n", 
                       vec[1].c_str() );
            return false;  // bad
        }

        int count;
        if ( ! StringToInt (vec[4], &count) )
        {
            dtkMsg.add(DTKMSG_ERROR, 
                       "iris::SceneGraph : uniform (B) : Bad count:  %s.\n", 
                       vec[4].c_str() );
            return false;  // bad
        }
        else if (count <= 0)
        {
            dtkMsg.add(DTKMSG_ERROR, 
                       "iris::SceneGraph : uniform (B) : Bad count:  %s.\n", 
                       vec[4].c_str() );
            return false;  // bad
        }

        // count is numElements in uniform

        // The Uniform class gives us nice tools for turning the type
        // string (like "vec4", "mat4", etc.) into the type specifier.
        // We can also get the underlying internal data representation.
        // This enables us to handle essentially the entire range of
        // uniform types.

        osg::Uniform::Type typeSpec = osg::Uniform::getTypeId (vec[3]);
        // FLOAT, FLOAT_VEC4, FLOAT_MAT4, etc.
        if (typeSpec == osg::Uniform::UNDEFINED)
        {
            if (vec[3] == "uint")
            {
                typeSpec = osg::Uniform::UNSIGNED_INT;
            }
            else
            {
                dtkMsg.add(DTKMSG_ERROR, 
                           "iris::SceneGraph : uniform (B) : Unrecognized type:  %s.\n", 
                           vec[3].c_str() );
                return false;  // bad
            }
        }

        GLenum internalType = osg::Uniform::getInternalArrayType (typeSpec);
        // internal type is GL_FLOAT, GL_INT, or GL_UNSIGNED_INT

        int typeLength = osg::Uniform::getTypeNumComponents (typeSpec);


        // get the state set
        osg::StateSet *s = node->getOrCreateStateSet() ;
        s->setDataVariance(osg::Object::DYNAMIC) ;

        // printf ("state set has %d uniforms\n", s->getUniformList().size());

        //
        // Get or create the uniform conforming to this type and count.
        //

        // we try to get a uniform of the specified name
        osg::Uniform *u = s->getUniform (vec[2]);

        bool mkNewUni = true;
        if (u != 0)
        {
            // If we found a uniform of the correct name, we check to see if the 
            //  type and numElements is what has been specified by the input.
            //  if not, then we put out a warning message and make a new uniform.
            mkNewUni = false;
            osg::Uniform::Type oldType = u->getType();
            int oldNumEl = u->getNumElements();
            if (oldType != typeSpec)
            {
                mkNewUni = true;
                dtkMsg.add
                    (
                     DTKMSG_WARNING, 
                     "iris::SceneGraph : uniform (B) : "
                     "Conflicting types found for uniform %s.\n"
                     "    New type (%s) will replace old type (%s).\n",
                     vec[2].c_str(),
                     osg::Uniform::getTypename(typeSpec),
                     osg::Uniform::getTypename(oldType)
                     );
            }

            if (oldNumEl != count)
            {
                mkNewUni = true;

                dtkMsg.add
                    (
                     DTKMSG_WARNING, 
                     "iris::SceneGraph : uniform (B) : "
                     "Conflicting counts found for uniform %s.\n"
                     "    New count (%d) will replace old count (%d).\n",
                     vec[2].c_str(),
                     osg::Uniform::getTypename(typeSpec),
                     count, oldNumEl
                     );
            }
        }
                

        if (mkNewUni)
        {
            u = new osg::Uniform (typeSpec, vec[2], count);
            u->setDataVariance(osg::Object::DYNAMIC) ;
            // it appears that if you add a uniform that has the same
            // name as an existing uniform, then the new one replaces
            // the old one.
            s->addUniform (u);
        }



        int numVals = u->getInternalArrayNumElements ();
        if (numVals != (count*typeLength))
        {
            dtkMsg.add
                (
                 DTKMSG_ERROR, 
                 "iris::SceneGraph : uniform (B) : "
                 "Number of internal array elements (%d) does not "
                 "match count times type length (%d).\n",
                 numVals,
                 count*typeLength
                 );
            return false;  // bad
        }
    

        // make sure we have the right number of elements
        if (vec.size() != (5+numVals) )
        {
            dtkMsg.add
                (
                 DTKMSG_ERROR, 
                 "iris::SceneGraph : uniform (B) : "
                 "Number of specified values (%d) does not "
                 "match count times type length (%d).\n",
                 vec.size()-5,
                 count*typeLength
                 );
            return false;  // bad
        }

        // Note that the method below of filling up an Array and calling
        // u->setArray was stolen from Array_readLocalData in Geometry.cpp
        // and Uniform.cpp (from osgPlugins).
        if (internalType == GL_FLOAT)
        {
            osg::FloatArray *valArray = new osg::FloatArray;
            valArray->setDataVariance(osg::Object::DYNAMIC) ;
            valArray->reserve (numVals);
            for (int i = 0; i < numVals; i++)
            {
                float value;
                if ( ! StringToFloat (vec[i+5], &value))
                {
                    dtkMsg.add ( DTKMSG_ERROR, 
                                 "iris::SceneGraph : uniform (B) : Bad float value: %s \n",
                                 vec[i+5].c_str());
                    return false;  // bad
                }
                valArray->push_back(value);
            }
            u->setArray (valArray);
        }  // end of float
        else if (internalType == GL_INT)
        {
            osg::IntArray *valArray = new osg::IntArray;
            valArray->setDataVariance(osg::Object::DYNAMIC) ;
            valArray->reserve (numVals);
            for (int i = 0; i < numVals; i++)
            {
                int value;
                if ( ! StringToInt(vec[i+5], &value) )
                {
                    dtkMsg.add ( DTKMSG_ERROR, 
                                 "iris::SceneGraph : uniform (B) : Bad int value: %s \n",
                                 vec[i+5].c_str());
                    return false;  // bad
                }
                valArray->push_back(value);
            }
            u->setArray (valArray);
        }  // end of int
        else if (internalType == GL_UNSIGNED_INT)
        {
            osg::UIntArray *valArray = new osg::UIntArray;
            valArray->setDataVariance(osg::Object::DYNAMIC) ;
            valArray->reserve (numVals);
            for (int i = 0; i < numVals; i++)
            {
                unsigned int value;
                if ( ! StringToUInt(vec[i+5], &value) )
                {
                    dtkMsg.add ( DTKMSG_ERROR, 
                                 "iris::SceneGraph : uniform (B) : "
                                 "Bad unsigned int value: %s \n",
                                 vec[i+5].c_str());
                    return false;  // bad
                }
                valArray->push_back(value);
            }
            u->setArray (valArray);
        }  // end of unisgned int
        else
        {
            // unrecognized type 
            dtkMsg.add ( DTKMSG_ERROR, 
                         "iris::SceneGraph : uniform (B) : Bad internal data type (%d).\n",
                         internalType);
            return false;  // bad
        }

        return true;
    } // end of SceneGraph::_uniformSyntaxB



    bool SceneGraph::_uniform (std::vector<std::string> vec)
    {
        if (vec.size() == 4)
        {
            return _uniformSyntaxA (vec);
        }
        else if (vec.size() >= 6)
        {
            return _uniformSyntaxB (vec);
        }

        dtkMsg.add ( DTKMSG_ERROR, 
                     "iris::SceneGraph : uniform : Bad argument count.\n");
        return false;  // bad
    }  // end of SceneGraph::_uniform (std::vector<std::string> vec)



    bool SceneGraph::_shaderstoragebuffer (std::vector<std::string> vec)
    {
        //
        // Current SHADERSTORAGEBUFFER command format:
        // SHADERSTORAGEBUFFER nodeName bufferName bufferBinding bufferSize
        //
        // These are required:
        //     vec[1]   node name
        //     vec[2]   buffer name
        //     vec[3]   buffer binding
        //     vec[4]   buffer size (in bytes of the entire buffer)

        if (vec.size() != 5)
        {
            dtkMsg.add ( DTKMSG_ERROR,
                         "iris::SceneGraph : shaderstoragebuffer : Bad number of parameters.\n" );
            return false; // bad
        }

        // node must exist
        osg::Node *node = findNode(vec[1]) ;
        if (!node)
        {
            dtkMsg.add(DTKMSG_ERROR,
                       "iris::SceneGraph : shaderstoragebuffer: Node  %s not found.\n",
                       vec[1].c_str() );
            return false;  // bad
        }

        // get the buffer binding number
        int bufferBinding;
        if ( ! StringToInt (vec[3], &bufferBinding))
        {
            dtkMsg.add(DTKMSG_ERROR,
                "iris::SceneGraph : shaderstoragebuffer: Bad buffer binding value %s.\n", 
                vec[2].c_str() );
            return false;  // bauniform sphere event int 1 2

        }

        // get the buffer size
        int bufferSize;
        if ( ! StringToInt (vec[4], &bufferSize))
        {
            dtkMsg.add(DTKMSG_ERROR,
                "iris::SceneGraph : shaderstoragebuffer: Bad buffer size value %s.\n", 
                vec[3].c_str() );
            return false;  // bad
        }


        // get the state set
        osg::StateSet *s = node->getStateSet() ;
        if ( ! s )
        {
            dtkMsg.add(DTKMSG_ERROR,
                       "iris::SceneGraph : shaderstoragebuffer: Node %s does not have a program attached.\n",
                       vec[1].c_str() );
            return false;  // bad
        }

        // get the program
        osg::StateAttribute *sa = s->getAttribute(osg::StateAttribute::PROGRAM) ;
        if ( ! sa )
        {
            dtkMsg.add(DTKMSG_ERROR,
                       "iris::SceneGraph : shaderstoragebuffer: Node %s does not have a program attached.\n",
                       vec[1].c_str() );
            return false;  // bad
        }

        s->setDataVariance(osg::Object::DYNAMIC) ;
        osg::Program *p = static_cast<osg::Program*>(sa) ;

        // create a dummy array with the requested size
        osg::ByteArray *dummyArray = new osg::ByteArray() ;
        dummyArray->resize(bufferSize) ;

        iris::ShaderStorageBufferObject *ssbo =
            new iris::ShaderStorageBufferObject ;
        dummyArray->setBufferObject(ssbo) ;

        iris::ShaderStorageBufferBinding *ssbb =
            new iris::ShaderStorageBufferBinding(bufferBinding, ssbo, 0,
                                                 bufferSize) ;
        s->setAttributeAndModes(ssbb, osg::StateAttribute::ON) ;

        return true;
    } // end of SceneGraph::_shaderstoragebuffer (std::vector<std::string> vec)



    //
    //
    // end of commands that relate to shaders
    //
    ////////////////////////////////////////////////////////////////////////

    // txtr related commands


    bool SceneGraph::_txtradd(const std::vector<std::string> & vec)
    {
        if ((vec.size() != 4))
        {
            dtkMsg.add ( DTKMSG_ERROR, 
                "iris::SceneGraph : txtradd:  Bad number of parameters.\n" );
            return false;  // bad
        }

        // node must exist
        osg::Node *node = findNode(vec[1]) ;
        if (!node)
        {
            dtkMsg.add(DTKMSG_ERROR,
                       "iris::SceneGraph : txtradd: Node  %s not found.\n",
                       vec[1].c_str() );
            return false;  // bad
        }

        // get the texture unit number
        int textureUnit;
        if ( ! StringToInt (vec[2], &textureUnit))
        {
            dtkMsg.add(DTKMSG_ERROR,
                "iris::SceneGraph : txtradd: Bad texture unit value %s.\n", 
                vec[2].c_str() );
            return false;  // bad
        }        

        // Get the node's StateSet
        osg::StateSet *stateSet = node->getOrCreateStateSet() ;

        if (_textures.count(vec[3])) 
        {
            stateSet->setTextureAttributeAndModes(textureUnit, _textures[vec[3]].get()); 
        }
        else
        {
            dtkMsg.add(DTKMSG_ERROR,
                "iris::SceneGraph : txtradd: Texture name not found %s.\n", 
                vec[3].c_str() );
            return false;
        }

        return true;
    }


    // txtrc command

    bool SceneGraph::_txtrc (const std::vector<std::string> & vec)
    {
        // Current TXTRC command format:
        //
        // These are required:
        //     vec[1]   texture name        
        //     vec[2]   texture binding unit number
        //     vec[3]   texture dimensionality (1D, 2D or 3D)
        //     vec[4]   texture size X
        //     vec[5]   texture size Y
        //     vec[6]   texture size Z 
        //     vec[7]   texture InternalFormat
        //     vec[8]   texture SourceFormat
        //     vec[9]   texture SourceType
        //
        // Example: TXTRC tomaszcompute 0 2D 512 512 1 GL_R32F GL_RED GL_FLOAT
        //

        if ((vec.size() != 10))
        {
            dtkMsg.add ( DTKMSG_ERROR, 
                "iris::SceneGraph : txtraddc:  Bad number of parameters.\n" );
            return false;  // bad
        }
 
        // Create the osg texture 
        osg::ref_ptr<osg::Texture1D> txtr1D;
        osg::ref_ptr<osg::Texture2D> txtr2D;
        osg::ref_ptr<osg::Texture3D> txtr3D;
        osg::ref_ptr<osg::Texture> txtr;

        std::map<std::string, int> formats;
        // store dimentionality
        formats["1D"] = 1;
        formats["2D"] = 2;
        formats["3D"] = 3;
        // formats, based on:
        // http://www.opengl.org/sdk/docs/man3/xhtml/glTexBuffer.xml
        formats["GL_R8"] = GL_R8;
        formats["GL_R16"] = GL_R16;
        formats["GL_R16F"] = GL_R16F;
        formats["GL_R32F"] = GL_R32F;
        formats["GL_R8I"] = GL_R8I;
        formats["GL_R16I"] = GL_R16I;
        formats["GL_R32I"] = GL_R32I;
        formats["GL_R8UI"] = GL_R8UI;
        formats["GL_R16UI"] = GL_R16UI;
        formats["GL_R32UI"] = GL_R32UI;
        formats["GL_RG8"] = GL_RG8;
        formats["GL_RG16"] = GL_RG16; 
        formats["GL_RG16F"] = GL_RG16F;
        formats["GL_RG32F"] = GL_RG32F;
        formats["GL_RG8I"] = GL_RG8I;
        formats["GL_RG16I"] = GL_RG16I;
        formats["GL_RG32I"] = GL_RG32I;
        formats["GL_RG8UI"] = GL_RG8UI;
        formats["GL_RG16UI"] = GL_RG16UI;
        formats["GL_RG32UI"] = GL_RG32UI;
        formats["GL_RGBA8"] = GL_RGBA8;
        formats["GL_RGBA16"] = GL_RGBA16;
        formats["GL_RGBA16F"] = GL_RGBA16F;
        formats["GL_RGBA32F"] = GL_RGBA32F;
        formats["GL_RGBA8I"] = GL_RGBA8I;
        formats["GL_RGBA16I"] = GL_RGBA16I;
        formats["GL_RGBA32I"] = GL_RGBA32I;
        formats["GL_RGBA8UI"] = GL_RGBA8UI;
        formats["GL_RGBA16UI"] = GL_RGBA16UI;
        formats["GL_RGBA32UI"] = GL_RGBA32UI;
        // source format
        formats["GL_RED"] = GL_RED;
        formats["GL_DEPTH_COMPONENT"] = GL_DEPTH_COMPONENT;
        formats["GL_RG"] = GL_RG;
        formats["GL_RGB"] = GL_RGB;
        formats["GL_RGBA"] = GL_RGBA;
        // format of the pixel data
        formats["GL_FLOAT"] = GL_FLOAT;
        formats["GL_INT"] = GL_INT;
        formats["GL_UNSIGNED_BYTE"] = GL_UNSIGNED_BYTE;
        formats["GL_BYTE"] = GL_BYTE;
        formats["GL_UNSIGNED_SHORT"] = GL_UNSIGNED_SHORT;
        formats["GL_SHORT"] = GL_SHORT;
        formats["GL_UNSIGNED_INT"] = GL_UNSIGNED_INT;
        formats["GL_UNSIGNED_BYTE_3_3_2"] = GL_UNSIGNED_BYTE_3_3_2;
        formats["GL_UNSIGNED_BYTE_2_3_3_REV"] = GL_UNSIGNED_BYTE_2_3_3_REV;
        formats["GL_UNSIGNED_SHORT_5_6_5"] = GL_UNSIGNED_SHORT_5_6_5;
        formats["GL_UNSIGNED_SHORT_5_6_5_REV"] = GL_UNSIGNED_SHORT_5_6_5_REV;
        formats["GL_UNSIGNED_SHORT_4_4_4_4"] = GL_UNSIGNED_SHORT_4_4_4_4;
        formats["GL_UNSIGNED_SHORT_4_4_4_4_REV"] = GL_UNSIGNED_SHORT_4_4_4_4_REV;
        formats["GL_UNSIGNED_SHORT_5_5_5_1"] = GL_UNSIGNED_SHORT_5_5_5_1;
        formats["GL_UNSIGNED_SHORT_1_5_5_5_REV"] = GL_UNSIGNED_SHORT_1_5_5_5_REV;
        formats["GL_UNSIGNED_INT_8_8_8_8"] = GL_UNSIGNED_INT_8_8_8_8;
        formats["GL_UNSIGNED_INT_8_8_8_8_REV"] = GL_UNSIGNED_INT_8_8_8_8_REV;
        formats["GL_UNSIGNED_INT_10_10_10_2"] = GL_UNSIGNED_INT_10_10_10_2;
        formats["GL_UNSIGNED_INT_2_10_10_10_REV"] = GL_UNSIGNED_INT_2_10_10_10_REV;

        int texBinding;
        if ( ! StringToInt (vec[2], &texBinding))
        {
            dtkMsg.add(DTKMSG_ERROR,
                "iris::SceneGraph : txtraddc: Bad texture's binding value %s.\n", 
                vec[2].c_str() );
            return false;  // bad
        }    

        int dim;
        if (formats.count(vec[3])) 
        {
            dim = formats[vec[3]];
        }
        else
        {
            dtkMsg.add(DTKMSG_ERROR,
                "iris::SceneGraph : txtraddc: Texture's dimensionality not found %s.\n", 
                vec[3].c_str() );
            return false;
        }

        int texSizeX;
        if ( ! StringToInt (vec[4], &texSizeX))
        {
            dtkMsg.add(DTKMSG_ERROR,
                "iris::SceneGraph : txtraddc: Bad texture's X size value %s.\n", 
                vec[4].c_str() );
            return false;  // bad
        }    
        int texSizeY;
        if ( ! StringToInt (vec[5], &texSizeY))
        {
            dtkMsg.add(DTKMSG_ERROR,
                "iris::SceneGraph : txtraddc: Bad texture's Y size value %s.\n", 
                vec[5].c_str() );
            return false;  // bad
        }    
        int texSizeZ;
        if ( ! StringToInt (vec[6], &texSizeZ))
        {
            dtkMsg.add(DTKMSG_ERROR,
                "iris::SceneGraph : txtraddc: Bad texture's Z size value %s.\n", 
                vec[6].c_str() );
            return false;  // bad
        }    

        int internalFormat;
        if (formats.count(vec[7])) 
        {
            internalFormat = formats[vec[7]];
        }
        else
        {
            dtkMsg.add(DTKMSG_ERROR,
                "iris::SceneGraph : txtraddc: Texture's internal format not defined %s.\n", 
                vec[7].c_str() );
            return false;
        }

        int sourceFormat;
        if (formats.count(vec[8])) 
        {
            sourceFormat = formats[vec[8]];
        }
        else
        {
            dtkMsg.add(DTKMSG_ERROR,
                "iris::SceneGraph : txtraddc: Texture's source format not defined %s.\n", 
                vec[8].c_str() );
            return false;
        }

        int sourceType;
        if (formats.count(vec[9])) 
        {
            sourceType = formats[vec[9]];
        }
        else
        {
            dtkMsg.add(DTKMSG_ERROR,
                "iris::SceneGraph : txtraddc: Texture's source type not defined %s.\n", 
                vec[9].c_str() );
            return false;
        }

        switch(dim)
        {
        case 1:
            txtr1D = new osg::Texture1D;
            txtr1D->setTextureWidth(texSizeX);
            txtr1D->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
            txtr1D->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
            txtr1D->setInternalFormat(internalFormat);
            txtr1D->setSourceFormat(sourceFormat);
            txtr1D->setSourceType(sourceType);
            txtr1D->bindToImageUnit(texBinding, osg::Texture::WRITE_ONLY);
            txtr = txtr1D;
            break;
        case 2:
            txtr2D = new osg::Texture2D;
            txtr2D->setTextureSize(texSizeX, texSizeY);
            txtr2D->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
            txtr2D->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
            txtr2D->setInternalFormat(internalFormat);
            txtr2D->setSourceFormat(sourceFormat);
            txtr2D->setSourceType(sourceType);
            txtr2D->bindToImageUnit(texBinding, osg::Texture::WRITE_ONLY);
            txtr = txtr2D;
            break;
        case 3:
            txtr3D = new osg::Texture3D;
            txtr3D->setTextureSize(texSizeX, texSizeY, texSizeZ);
            txtr3D->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
            txtr3D->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
            txtr3D->setInternalFormat(internalFormat);
            txtr3D->setSourceFormat(sourceFormat);
            txtr3D->setSourceType(sourceType);
            txtr3D->bindToImageUnit(texBinding, osg::Texture::WRITE_ONLY);
            txtr = txtr3D;
            break;
        }

        // and store texture name
        _textures[vec[1]] = txtr;

        return true;

    }  // end of _txtrc (const std::vector<std::string>& vec)




    // txtr command


    bool SceneGraph::_txtr (const std::vector<std::string> & vec)
    {
        // 
        // Current TXTR command format:
        // TXTR nodeName textureUnit textureFileName  \
        //   [ FILTER LINEAR | NEAREST ]                \
        //   [ WRAP   CLAMP | CLAMP_TO_EDGE | CLAMP_TO_BORDER | REPEAT | MIRROR]
        //
        // There are other parameters that we might want to include
        // in the TXTR command. It is likely that we will want to extend
        // this command in the future.
        // 
        // These are required:
        //     vec[1]   node name
        //     vec[2]   texture unit number
        //     vec[3]   image file name
        //

        if ((vec.size() != 4) && (vec.size() != 6) && (vec.size() != 8))
        {
            dtkMsg.add ( DTKMSG_ERROR, 
                "iris::SceneGraph : txtr:  Bad number of parameters.\n" );
            return false;  // bad
        }

        // node must exist
        osg::Node *node = findNode(vec[1]) ;
        if (!node)
        {
            dtkMsg.add(DTKMSG_ERROR,
                       "iris::SceneGraph : txtr: Node  %s not found.\n",
                       vec[1].c_str() );
            return false;  // bad
        }

        // get the texture unit number
        int textureUnit;
        if ( ! StringToInt (vec[2], &textureUnit))
        {
            dtkMsg.add(DTKMSG_ERROR,
                "iris::SceneGraph : txtr: Bad texture unit value %s.\n", 
                vec[2].c_str() );
            return false;  // bad
        }

        // Read the image file
        osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile (vec[3]);
        if ( ! image.valid() )
        {
            dtkMsg.add(DTKMSG_ERROR,
                 "iris::SceneGraph : txtr: Unable to open image file  %s.\n",
                 vec[3].c_str() );
            return false;  // bad
        }


        // figure out the dimensionality of the Image
        int dim = 3;
        if (image->r() == 1)
        {
            dim = 2;
            if (image->t() == 1)
            {
                dim = 1;
            }
        }

        // get the state set
        osg::StateSet *s = node->getOrCreateStateSet() ;
        s->setDataVariance(osg::Object::DYNAMIC) ;


        // Create the osg texture 

        osg::ref_ptr<osg::Texture1D> txtr1D;
        osg::ref_ptr<osg::Texture2D> txtr2D;
        osg::ref_ptr<osg::Texture3D> txtr3D;
        osg::ref_ptr<osg::Texture> txtr;

        switch (dim)
        {
        case 1:
                txtr1D = new osg::Texture1D;
                txtr = txtr1D;
                break;

        case 2:
                txtr2D = new osg::Texture2D;
                txtr = txtr2D;
                break;

        case 3:
                txtr3D = new osg::Texture3D;
                txtr = txtr3D;
                break;
        }

        std::string fn = vec[3];
        _textures[fn] = txtr;        

        // set default texture attributes
        osg::Texture::FilterMode filterMode = osg::Texture::LINEAR;
        osg::Texture::WrapMode wrapMode = osg::Texture::CLAMP_TO_EDGE;


        // Now see if command specifies non-defaults 
        int c = 4;
        while (vec.size() > c)
        {
            if (IsSubstring("filter",vec[c],4) && (vec.size() >= c+2))
            {
                if (IsSubstring("nearest", vec[c+1], 4)) 
                {
                    filterMode = osg::Texture::NEAREST;
                } 
                else if (IsSubstring("linear", vec[c+1], 3)) 
                {
                    filterMode = osg::Texture::LINEAR;
                } 
                else 
                {
                    dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph : "
                            "txtr: bad filter mode parameter  %s.\n",
                            vec[c+1].c_str() );
                    return false;
                }

                c += 2;
            }  // end of if "filter"
            else if (IsSubstring("wrap",vec[c],4) && (vec.size() >= c+2))
            {
                if (IsSubstring("CLAMP_TO_EDGE", vec[c+1], 10))
                {
                    wrapMode = osg::Texture::CLAMP_TO_EDGE;
                }
                else if (IsSubstring("CLAMP_TO_BORDER", vec[c+1], 10))
                {
                    wrapMode = osg::Texture::CLAMP_TO_BORDER;
                }
                else if (IsSubstring("CLAMP", vec[c+1], 0))
                {
                    wrapMode = osg::Texture::CLAMP;
                }
                else if (IsSubstring("REPEAT", vec[c+1], 3))
                {
                    wrapMode = osg::Texture::REPEAT;
                }
                else if (IsSubstring("MIRROR", vec[c+1], 3))
                {
                    wrapMode = osg::Texture::MIRROR;
                }
                else
                {
                    dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph : "
                            "txtr: bad wrap mode parameter  %s.\n",
                            vec[c+1].c_str() );
                    return false;
                }

                c += 2;
            }  // end of if "wrap"
        else
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph : "
                            "txtr: bad keyword  %s.\n",
                            vec[c].c_str() );
                return false;
            }

        }

        // Set the filter, wrap, and other texture characteristics
        txtr->setFilter (osg::Texture::MIN_FILTER, filterMode);
        txtr->setFilter (osg::Texture::MAG_FILTER, filterMode);

        txtr->setWrap (osg::Texture::WRAP_S, wrapMode);
        txtr->setWrap (osg::Texture::WRAP_T, wrapMode);
        txtr->setWrap (osg::Texture::WRAP_R, wrapMode);

        txtr->setUseHardwareMipMapGeneration (false);
        txtr->setResizeNonPowerOfTwoHint (false);
        txtr->setUnRefImageDataAfterApply (true);


        // set the image in the texture and the texture in the state set
        switch (dim)
        {
        case 1:
                txtr1D->setImage (image);
                s->setTextureAttribute (textureUnit, txtr1D);
                break;

        case 2:
                txtr2D->setImage (image);
                s->setTextureAttribute (textureUnit, txtr2D);
                break;

        case 3:
                txtr3D->setImage (image);
                s->setTextureAttribute (textureUnit, txtr3D);
                break;

        }


        return true;

    }  // end of _txtr (const std::vector<std::string>& vec)

    //    
    ////////////////////////////////////////////////////////////////////////

    // text

    // bool SceneGraph::_makeText
    static bool makeText
    (
     std::string nodeName, 
     std::string string, 
     std::string font,
     int *resolution, 
     float *size, 
     osg::Vec4 *color,
     osgText::Text::AlignmentType *alignment, 
     osgText::Text::AxisAlignment * axis
     )
    {

        dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::_makeText: Node %s.\n", 
                   nodeName.c_str()) ;

        //printf("nodeName = %s, text = %s, font = %s, resolution = %p, size = %p, color = %p\n" ,nodeName.c_str(), string.c_str(), font.c_str(), resolution, size, color) ;

        // see if the node already exists and if so, is it a Geode?
        osg::Node *node = SceneGraph::instance()->findNode(nodeName) ;

        osg::ref_ptr<osg::Geode> geode ;
        osg::ref_ptr<osgText::Text> text ;
        if (node)
        {
            // node exists already- is a Geode?
            geode = dynamic_cast<osg::Geode*>(SceneGraph::instance()->findNode(nodeName)) ;
            if (geode)
            {
                // find text in geode
                if (geode->getNumDrawables() != 1)
                {

                    dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::_makeText: "
                               "Node named %s is an osg::Geode but \n"
                               "       doesn't have a one and only one osg::Drawable.\n", 
                               nodeName.c_str()); 
                    return false ;
                }
                text = dynamic_cast<osgText::Text*>(geode->getDrawable(0)) ;
                if (!text)
                {
                    dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::_makeText: "
                               "Node named %s is an osg::Geode and \n"
                               "   contains a single osg::Drawable, but it isn't "
                               "an osgText::Text.\n",
                               nodeName.c_str()); 
                    return false ;
                }
                else 
                {
                    dtkMsg.add(DTKMSG_DEBUG, "iris::SceneGraph::_makeText: reusing geode and text\n") ;
                    if (text->getDataVariance() != osg::Object::DYNAMIC) dtkMsg.add(DTKMSG_WARNING, "iris::SceneGraph::_makeText: text object does not have dynamic DataVariance\n") ;
                }

            }
            else
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::_makeText: "
                           "Node named %s already exists but is not an osg::Geode.\n",
                           nodeName.c_str()); 
                return false ;
            }
        }
        else // create and add
        {
            dtkMsg.add(DTKMSG_DEBUG, "iris::SceneGraph::_makeText: creating new geode and text\n") ;
            geode = new osg::Geode ;

            SceneGraph::instance()->addNode(nodeName, geode) ;
            text = new osgText::Text;
            // need to do this since we'll be modifying it
            text->setDataVariance(osg::Object::DYNAMIC) ;
            // iris overrides some defaults
            text->setAxisAlignment(osgText::Text::XZ_PLANE) ;
            text->setFont("arial.ttf") ;
            text->setFontResolution(100,100) ;
            geode->addDrawable(text) ;
            geode->getOrCreateStateSet()->setMode ( GL_LIGHTING, 
                                                    osg::StateAttribute::PROTECTED | osg::StateAttribute::OFF) ;
        }

        if (string.size() > 0)
        {
            dtkMsg.add(DTKMSG_DEBUG, "text = \"%s\"\n",string.c_str()) ;
            text->setText(string) ;
        }

        if (font.size() > 0)
        {
            dtkMsg.add(DTKMSG_DEBUG, "font = \"%s\"\n",font.c_str()) ;
            text->setFont(font);
        }

        if (resolution!=NULL)
        {
            dtkMsg.add(DTKMSG_DEBUG, "resolution = %d\n",*resolution) ;
            text->setFontResolution(*resolution,*resolution);
        }

        if (size!=NULL)
        {
            dtkMsg.add(DTKMSG_DEBUG, "size = %f\n",*size) ;
            text->setCharacterSize(*size);
        }

        if (color!=NULL)
        {
            dtkMsg.add(DTKMSG_DEBUG, "color = %f %f %f %f\n",(*color).r(),(*color).g(),(*color).b(),(*color).a()) ;
            text->setColor(*color);
        }

        if (alignment!=NULL)
        {
            dtkMsg.add(DTKMSG_DEBUG, "alignment = \"%d\"\n",*alignment) ;
            text->setAlignment(*alignment);
        }

        if (axis!=NULL)
        {
            dtkMsg.add(DTKMSG_DEBUG, "axis = \"%d\"\n",*axis) ;
            text->setAxisAlignment(*axis);
        }

        return true ;
    }  // end of makeText


    bool SceneGraph::_text (std::vector<std::string> vec)
    {

        if (vec.size() < 2)
        {
            dtkMsg.add ( DTKMSG_ERROR, 
                         "iris::SceneGraph::_text : Bad argument count.\n");
            return false;
        }

        std::string text ;
        std::string font ;
        int *resolutionPtr = NULL ;
        float *sizePtr = NULL ;
        osg::Vec4 *colorPtr = NULL ;
        osgText::Text::AlignmentType *alignmentPtr = NULL ;
        osgText::Text::AxisAlignment *axisPtr = NULL ;
        int resolution ;
        float size ;
        osg::Vec4 color ;
        osgText::Text::AlignmentType alignment ;
        osgText::Text::AxisAlignment axis ;

        int i = 2 ;
        while (i<vec.size())
        {
            if (IsSubstring("string", vec[i]))
            {
                text=vec[i+1] ;
                i+=2 ;
            }
            else if (IsSubstring("font", vec[i]))
            {
                font=vec[i+1] ;
                i+=2 ;
            }
            else if (IsSubstring("resolution", vec[i]))
            {
                if (StringToInt(vec[i+1], &resolution))
                {
                    resolutionPtr = &resolution ;
                }
                else
                {
                    return false ;
                }

                i+=2 ;
            }
            else if (IsSubstring("size", vec[i]))
            {
                if (StringToFloat (vec[i+1], &size))
                {
                    sizePtr = &size ;
                }
                else
                {
                    return false ;
                }
                i+=2 ;
            }
            else if (IsSubstring("color", vec[i]))
            {
                float r, g, b, a ;
                if (StringToFloat (vec[i+1], &r) &&
                    StringToFloat (vec[i+2], &g) &&
                    StringToFloat (vec[i+3], &b) &&
                    StringToFloat (vec[i+4], &a))
                {
                    color = osg::Vec4(r,g,b,a) ;
                    colorPtr = &color ;
                }
                else
                {
                    return false ;
                }
                i+=5 ;
            }  // end of color section
            else if (IsSubstring("alignment", vec[i]))
            {
                if (IsSubstring("left_top", vec[i+1]))
                {
                    alignment = osgText::Text::LEFT_TOP ;
                }
                else if (IsSubstring("left_center", vec[i+1]))
                {
                    alignment = osgText::Text::LEFT_CENTER ;
                }
                else if (IsSubstring("left_bottom", vec[i+1]))
                {
                    alignment = osgText::Text::LEFT_BOTTOM ;
                }
                else if (IsSubstring("center_top", vec[i+1]))
                {
                    alignment = osgText::Text::CENTER_TOP ;
                }
                else if (IsSubstring("center_center", vec[i+1]))
                {
                    alignment = osgText::Text::CENTER_CENTER ;
                }
                else if (IsSubstring("center_bottom", vec[i+1]))
                {
                    alignment = osgText::Text::CENTER_BOTTOM ;
                }
                else if (IsSubstring("right_top", vec[i+1]))
                {
                    alignment = osgText::Text::RIGHT_TOP ;
                }
                else if (IsSubstring("right_center", vec[i+1]))
                {
                    alignment = osgText::Text::RIGHT_CENTER ;
                }
                else if (IsSubstring("right_bottom", vec[i+1]))
                {
                    alignment = osgText::Text::RIGHT_BOTTOM ;
                }
                else if (IsSubstring("left_base_line", vec[i+1]))
                {
                    alignment = osgText::Text::LEFT_BASE_LINE ;
                }
                else if (IsSubstring("center_base_line", vec[i+1]))
                {
                    alignment = osgText::Text::CENTER_BASE_LINE ;
                }
                else if (IsSubstring("right_base_line", vec[i+1]))
                {
                    alignment = osgText::Text::RIGHT_BASE_LINE ;
                }
                else if (IsSubstring("left_bottom_base_line", vec[i+1]))
                {
                    alignment = osgText::Text::LEFT_BOTTOM_BASE_LINE ;
                }
                else if (IsSubstring("center_bottom_base_line", vec[i+1]))
                {
                    alignment = osgText::Text::CENTER_BOTTOM_BASE_LINE ;
                }
                else if (IsSubstring("right_bottom_base_line", vec[i+1]))
                {
                    alignment = osgText::Text::RIGHT_BOTTOM_BASE_LINE ;
                }
                else
                {
                    return false ;
                }
                alignmentPtr = &alignment ;
                i+=2 ;
            }  // end of alignment section

            else if (IsSubstring("axis", vec[i]))
            {
                if (IsSubstring("xy_plane", vec[i+1]))
                {
                    axis = osgText::Text::XY_PLANE ;
                }
                else if (IsSubstring("reversed_xy_plane", vec[i+1]))
                {
                    axis = osgText::Text::REVERSED_XY_PLANE ;
                }
                else if (IsSubstring("xz_plane", vec[i+1]))
                {
                    axis = osgText::Text::XZ_PLANE ;
                }
                else if (IsSubstring("reversed_xz_plane", vec[i+1]))
                {
                    axis = osgText::Text::REVERSED_XZ_PLANE ;
                }
                else if (IsSubstring("yz_plane", vec[i+1]))
                {
                    axis = osgText::Text::YZ_PLANE ;
                }
                else if (IsSubstring("reversed_yz_plane", vec[i+1]))
                {
                    axis = osgText::Text::REVERSED_YZ_PLANE ;
                }
                else if (IsSubstring("screen", vec[i+1]))
                {
                    axis = osgText::Text::SCREEN ;
                }
                else
                {
                    return false ;
                }
                axisPtr = &axis ;
                i+=2 ;
            }  // end of axis section
            else
            {
                return false ;
            }
        } // end of while

        return makeText (vec[1], text, font, resolutionPtr, 
                         sizePtr, colorPtr, alignmentPtr, axisPtr) ;
    } // end of SceneGraph::_text








    ////////////////////////////////////////////////////////////////////////

    // Culling

    bool SceneGraph::_cullingOnOff (std::vector<std::string> vec)
    {

        if (vec.size() != 3)
        {
            dtkMsg.add ( DTKMSG_ERROR, 
                         "iris::SceneGraph::_cullingOnOff : Bad argument count.\n");
            return false;
        }

        osg::Node *node = findNode(vec[1]) ;
        if (!node)
        {
            dtkMsg.add ( DTKMSG_ERROR, 
                         "iris::SceneGraph::_cullingOnOff : "
                         "Unable to find node %s\n",
                         vec[1].c_str());
            return false ;
        }


        bool onOff ;
        if (!OnOff(vec[2],&onOff)) 
        {
            dtkMsg.add(DTKMSG_ERROR, 
                       "iris::SceneGraph::_cullingOnOff : Bad on/off argument: %s\n",
                       vec[2].c_str() );
            return false;
        }

#if 1        
        ConvertNodeVisitor nv ;
        nv.setCulling(true,onOff) ;
        nv.apply(*node) ;
#else        
        node->setCullingActive(onOff) ;
#endif
        if (onOff) dtkMsg.add(DTKMSG_INFO, 
                              "iris::SceneGraph::_cullingOnOff : "
                              "Node %s culling is being set to ON\n",
                              vec[1].c_str());
        else dtkMsg.add(DTKMSG_INFO, 
                        "iris::SceneGraph::_cullingOnOff : "
                        "Node %s culling is being set to OFF\n",
                        vec[1].c_str());

        return true;
    }  // end of SceneGraph::_cullingOnOff

#if 0
    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::print(const std::string& nodeName, const std::string& fileName)
    {
        osg::Node* node = findNode(nodeName) ;
        if (!node)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::dump: Unable to find node named \"%s\"\n",nodeName.c_str()) ;
            return false ;
        }

        dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::normals: printing node %s\n",nodeName.c_str()) ;
        osgUtil::PrintVisitor pv(std::cout) ;
        node->accept(pv) ;
        return true ;

    }
#endif

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::clipnode(const std::vector<std::string>& vec)
    {
        ClipNode* cn ;

        osg::Node* node = findNode(vec[1]) ;
        if (!node)
        {
            cn = new ClipNode() ;
            if (cn->isInvalid())
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::clipnode: Unable to create clip node\n",vec[1].c_str()) ;
                return false ;
            }
            addNode(vec[1], cn) ;
        }
        else
        {
            cn = dynamic_cast<ClipNode*>(node) ;
            if (!cn)
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::clipnode: node \"%s\" exists but is not a clip node\n",vec[1].c_str()) ;
                return false ;
            }
        }

        if (vec.size() >= 3)
        {
            // parse the arguments
            bool onOff ;
            if (vec.size() == 3 && OnOff(vec[2],&onOff))
            {
                if (onOff) cn->enable() ;
                else cn->disable() ;
            }
            else if (IsSubstring("on",vec[2]) && vec.size() == 4 && OnOff(vec[3],&onOff))
            {
                if (onOff) cn->enable() ;
                else cn->disable() ;
            }
            else return false ;
        }
        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::lightmodel(const std::vector<std::string>& vec)
    {

        osg::LightModel* lm = dynamic_cast<osg::LightModel*>(SceneGraph::instance()->getSceneNode()->getOrCreateStateSet()->getAttribute(osg::StateAttribute::LIGHTMODEL)) ;
        if (!lm)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::lightmodel: Unable to get light model from scene node\n",vec[1].c_str()) ;
            return false ;
        }
        lm->setDataVariance(osg::Object::DYNAMIC) ; 

        // parse the arguments
        bool onOff ;
        float r, g, b, a ;
        int c = 1 ;
        while (vec.size()>c) 
        {
            //fprintf(stderr, "c=%d, vec.size=%d\n",c,vec.size()) ;
            //for (unsigned int i=c; i<vec.size(); i++) fprintf(stderr, "  vec[%d] = %s\n",i,vec[i].c_str()) ;
            if (IsSubstring("ambient",vec[c],3) && vec.size() >= c+5)
            {
                if (!StringToFloat(vec[c+1],&r) || !StringToFloat(vec[c+2],&g) || !StringToFloat(vec[c+3],&b) || !StringToFloat(vec[c+4],&a)) return false ; 
                lm->setAmbientIntensity(osg::Vec4(r, g, b, a));
                c+=5 ;
            }        
            else if (IsSubstring("color_control",vec[c],3) && vec.size() >= c+2)
            {
                if (IsSubstring("single",vec[c+1],3)) lm->setColorControl(osg::LightModel::SINGLE_COLOR) ;
                else if (IsSubstring("separate_specular",vec[c+1],3)) lm->setColorControl(osg::LightModel::SEPARATE_SPECULAR_COLOR) ;
                else return false ;
                c+=2 ;
            }        
            else if (IsSubstring("local_viewer",vec[c],3) && vec.size() >= c+2 && OnOff(vec[c+1],&onOff))
            {
                lm->setLocalViewer(onOff) ;
                c+=2 ;
            }        
            else if (IsSubstring("two_sided",vec[c],3) && vec.size() >= c+2 && OnOff(vec[c+1],&onOff))
            {
                lm->setTwoSided(onOff) ;
                c+=2 ;
            }        
            else return false ;
        }
        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::lightnode(const std::vector<std::string>& vec)
    {
        LightNode* ln ;

        osg::Node* node = findNode(vec[1]) ;
        if (!node)
        {
            ln = new LightNode() ;
            if (ln->isInvalid())
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::lightnode: Unable to create light node\n",vec[1].c_str()) ;
                return false ;
            }
            addNode(vec[1], ln) ;
        }
        else
        {
            ln = dynamic_cast<LightNode*>(node) ;
            if (!ln)
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::lightnode: node \"%s\" exists but is not a light node\n",vec[1].c_str()) ;
                return false ;
            }
        }

        // parse the arguments
        bool onOff ;
        float r, g, b, a ;
        float x, y, z, w ;
        float attenuation, exponent, cutoff ;
        osg::Light* l = ln->getLight() ;

        int c = 2 ;
        while (vec.size()>c) 
        {
            //fprintf(stderr, "c=%d, vec.size=%d\n",c,vec.size()) ;
            //for (unsigned int i=c; i<vec.size(); i++) fprintf(stderr, "  vec[%d] = %s\n",i,vec[i].c_str()) ;
            if (IsSubstring("ambient",vec[c],3) && vec.size() >= c+5)
            {
                if (!StringToFloat(vec[c+1],&r) || !StringToFloat(vec[c+2],&g) || !StringToFloat(vec[c+3],&b) || !StringToFloat(vec[c+4],&a)) return false ; 
                l->setAmbient(osg::Vec4(r, g, b, a));
                c+=5 ;
            }        
            else if (IsSubstring("diffuse",vec[c],3) && vec.size() >= c+5)
            {
                if (!StringToFloat(vec[c+1],&r) || !StringToFloat(vec[c+2],&g) || !StringToFloat(vec[c+3],&b) || !StringToFloat(vec[c+4],&a)) return false ; 
                l->setDiffuse(osg::Vec4(r, g, b, a));
                c+=5 ;
            }        
            else if (IsSubstring("specular",vec[c],3) && vec.size() >= c+5)
            {
                if (!StringToFloat(vec[c+1],&r) || !StringToFloat(vec[c+2],&g) || !StringToFloat(vec[c+3],&b) || !StringToFloat(vec[c+4],&a)) return false ; 
                l->setSpecular(osg::Vec4(r, g, b, a));
                c+=5 ;
            }        
            else if (IsSubstring("position",vec[c],3) && vec.size() >= c+5)
            {
                if (!StringToFloat(vec[c+1],&x) || !StringToFloat(vec[c+2],&y) || !StringToFloat(vec[c+3],&z) || !StringToFloat(vec[c+4],&w)) return false ; 
                l->setPosition(osg::Vec4(x, y, z, w));
                c+=5 ;
            }        
            else if (IsSubstring("direction",vec[c],3) && vec.size() >= c+4)
            {
                if (!StringToFloat(vec[c+1],&x) || !StringToFloat(vec[c+2],&y) || !StringToFloat(vec[c+3],&z)) return false ; 
                l->setDirection(osg::Vec3(x, y, z));
                c+=4 ;
            }        
            else if (IsSubstring("attenuation",vec[c],3) && vec.size() >= c+3 && StringToFloat(vec[c+2],&attenuation))
            {
                if (IsSubstring("constant",vec[c+1],3)) l->setConstantAttenuation(attenuation) ;
                else if (IsSubstring("linear",vec[c+1],3)) l->setLinearAttenuation(attenuation) ;
                else if (IsSubstring("quadratic",vec[c+1],3)) l->setQuadraticAttenuation(attenuation) ;
                else return false ;
                c+=3 ;
            }
            else if (IsSubstring("spot",vec[c],3) && vec.size() >= c+3 && StringToFloat(vec[c+1],&exponent) && StringToFloat(vec[c+2],&cutoff))
            {
                l->setSpotExponent(exponent) ;
                l->setSpotCutoff(cutoff) ;
                c+=3 ;
            }
            else if (IsSubstring("ref_frame",vec[c],3) && vec.size() >= c+2)
            {
                if (IsSubstring("relative",vec[c+1],3)) ln->setReferenceFrame(osg::LightSource::RELATIVE_RF) ;
                else if (IsSubstring("absolute",vec[c+1],3)) ln->setReferenceFrame(osg::LightSource::ABSOLUTE_RF) ;
                else return false ;
                c+=2 ;
            }
            else if (IsSubstring("on",vec[c]) && vec.size() >= c+2 && OnOff(vec[c+1],&onOff))
            {
                if (onOff) ln->enable() ;
                else ln->disable() ;
                c+=1 ;
            }
            else if (OnOff(vec[c],&onOff) && vec.size() >= c+1)
            {
                if (onOff) ln->enable() ;
                else ln->disable() ;
                c+=1 ;
            }
            else return false ;

        }
        return true ;

    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::billboard (const std::vector<std::string> & vec)
    {
        // This creates a new billboard node.  Command should look like:
        // BILLBOARD nodeName [axis0  axis1  axis2  [axisMode]]

        if ( (vec.size() != 2) && (vec.size() != 5) && (vec.size() != 6) )
        {
            dtkMsg.add ( DTKMSG_ERROR, 
                         "iris::SceneGraph : billboard: Bad argument count.\n");
            return false;  // bad
        }
           

        // if the node exists, don't proceed
        if (findNode(vec[1]))
        {
            dtkMsg.add ( DTKMSG_ERROR, 
                     " iris::SceneGraph::billboard: node %s already exists.\n");
            return false;
        }
                     

        // Get the axis if it's there
        double a0, a1, a2;
        if (vec.size() >= 5)
        {
            if ( ! ( StringToDouble(vec[2], &a0) &&
                     StringToDouble(vec[3], &a1) &&
                     StringToDouble(vec[4], &a2)      ) )
            {
                dtkMsg.add ( DTKMSG_ERROR, 
                   " iris::SceneGraph::billboard: "
                   "Unable to convert axis arguments.\n");
                return false;
            }

            if ( (a0 == 0) && (a1 == 0) && (a2 == 0) )
            {
                dtkMsg.add ( DTKMSG_ERROR, 
                   " iris::SceneGraph::billboard: "
                   "Zero length axis specified.\n");
                return false;
            }
        }


        // get the axis mode if it's there
        bool axisMode = true;
        if (vec.size() == 6)
        {
            if ( (vec[5] == "T") || (vec[5] == "t") )
            {
                axisMode = true;
            }
            else if ( (vec[5] == "F") || (vec[5] == "f") )
            {
                axisMode = false;
            }
            else
            {
                dtkMsg.add ( DTKMSG_ERROR, 
                   " iris::SceneGraph::billboard: "
                   "Bad axis mode: %s .\n", vec[5].c_str());
                return false;
            }
            
        }

        // At this point we have enough to successfully complete.

        // create the node and add it to the scene graph
        BillboardTransform *billboard = new BillboardTransform;
        addNode (vec[1], billboard);

        // if the axis was specified in the command, set it in the node.
        if (vec.size() >= 5)
        {
            // if axis is specified, set axis and mode
            osg::Vec3 axis (a0, a1, a2);
            billboard->axisRotate(axisMode);
            billboard->axis (axis);
        }


        return true;
    }  // end of SceneGraph::billboard




    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::nodemask(const std::string& nodeName, const std::string& state)
    {
        osg::Node* node = findNode(nodeName) ;
        if (!node)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::nodemask: Unable to find node named \"%s\"\n",nodeName.c_str()) ;
            return false ;
        }

        ClipNode* cn = dynamic_cast<ClipNode*>(node) ;
        LightNode* ln = dynamic_cast<LightNode*>(node) ;

        bool onOff ;
        int mask ;

        if (OnOff(state,&onOff))
        {
            if (onOff)
            {
                mask = ~0 ;
                if (cn) cn->enable() ;
                if (ln) ln->enable() ;
            }
            else 
            {
                mask = 0 ;
                if (cn) cn->disable() ;
                if (ln) ln->disable() ;
            }
        }
        else if (!StringToInt(state,&mask)) return false ;

        node->setNodeMask(mask) ;

        return true ;

    }

    ////////////////////////////////////////////////////////////////////////
    class SceneGraph::mapNodeVisitor : public osg::NodeVisitor
    {
    public :
        mapNodeVisitor() :
            osg::NodeVisitor( TRAVERSE_ALL_CHILDREN )
        {
        }

        virtual void apply( osg::Node &node )
        {
            std::string name = node.getName() ;
            if (name != "") 
            {
                _nodeMap.insert(std::make_pair(name, &node)) ;
#ifdef SCENEGRAPH_NODE_PTR_SET
                _nodePtrSet.insert(&node) ;
#endif
                dtkMsg.add(DTKMSG_DEBUG, "iris::SceneGraph::mapNode: adding node %s to map\n",name.c_str()) ;
                //SceneGraph::instance()->addNode(node.getName(), &node) ;
            }
            traverse(node);
        }

    } ;  // end of mapNodeVisitor class

    bool SceneGraph::mapNode(const std::string& name)
    {
        osg::Node* node = findNode(name) ;
        if (!node)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::mapNode: node named %s does not exist.\n",name.c_str()) ;            
            return false ;
        }
        
        mapNodeVisitor nmv ;
        // visit them regardless of their nodemask
        nmv.setNodeMaskOverride(~0) ;
        node->accept(nmv) ;

        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    osg::Node* SceneGraph::findNode(const std::string& name)
    {
        dtkMsg.add(DTKMSG_DEBUG, "iris::SceneGraph::findNode: looking for node %s\n",name.c_str()) ;
        // look for special last loaded node name; "-"
        if (name == "-")
        {
            if (_lastNode) return _lastNode ;
            else
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::findNode: \"-\" passed and no \"last node\" defined yet\n") ;
                return NULL ;
            }
                
        }

        // look for the node name in the map
        std::map<std::string, osg::ref_ptr<osg::Node> >::iterator nodeMapPos = _nodeMap.find(name) ;
        osg::Node* node = NULL ;
        // assign it if it's in the map
        if (nodeMapPos != _nodeMap.end()) 
        {
            node = nodeMapPos->second.get() ;
        }
        // if not look in the scenegraph if we have one
        else if (_instanced)
        {
            dtkMsg.add(DTKMSG_DEBUG, "iris::SceneGraph::findNode: node %s not found in map, searching scenegraph\n",name.c_str()) ;
            std::set<osg::Node*> nodes = FindNode(name) ;
            if (nodes.size()==1)
            {
                node = *nodes.begin() ;
                _nodeMap.insert(std::make_pair(name, node)) ;
#ifdef SCENEGRAPH_NODE_PTR_SET
                _nodePtrSet.insert(node) ;
#endif
            }
            else if (nodes.size()>1)
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::findNode: %d nodes named %s found\n",nodes.size(),name.c_str()) ;
            }
#if 0
            else
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::findNode: no node named %s found\n",name.c_str()) ;
            }
#endif
        }
        return node ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::removeNode(const std::string& name)
    {

        std::map<std::string, osg::ref_ptr<osg::Node> >::iterator nodeMapPos = _nodeMap.find(name) ;
        // is it in the set?
        if (nodeMapPos != _nodeMap.end())
        {
            osg::Node* node = nodeMapPos->second.get() ;
            _nodeMap.erase(nodeMapPos) ;
#ifdef SCENEGRAPH_NODE_PTR_SET
            _nodePtrSet.erase(node) ;
#endif
            return true ;
        }
        else 
        {
            return false ;
        }
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::addNode(const std::string& name, osg::Node* const node)
    {

        dtkMsg.add(DTKMSG_DEBUG, "iris::SceneGraph::addNode: node %p\n",
                    name.c_str()) ;


        // Check if the same node with the same name is already in
        // the node map.  If so, we don't need to do anything.
        osg::Node* sameNameNode = findNode(name) ;
        if (sameNameNode != NULL)
        {
            if (sameNameNode != node)
            {
                dtkMsg.add(DTKMSG_ERROR, 
                  "iris::SceneGraph::addNode: node named %s already exists\n",
                  name.c_str()) ;
                return false ;
            }
            else
            {
            dtkMsg.add(DTKMSG_INFO, 
             "iris::SceneGraph::addNode: name and node pointer already in map; "
             "not adding\n") ;
            return true;
            }
        }

        dtkMsg.add(DTKMSG_DEBUG, 
              "iris::SceneGraph::addNode: node map has %d names\n",
              _nodeMap.size()) ;


#ifdef SCENEGRAPH_NODE_PTR_SET
        // is the same node pointer already in the map?
        osg::ref_ptr<osg::Node> holder = node ;
        std::set<osg::ref_ptr<osg::Node> >::iterator nodePtrSetPos = _nodePtrSet.find(holder) ;
        if (nodePtrSetPos != _nodePtrSet.end()) 
        {
#if 0
            dtkMsg.add(DTKMSG_INFO, 
              "iris::SceneGraph::addNode: node pointer already in map; "
              "not adding\n") ;
            return false ;
#else
            dtkMsg.add(DTKMSG_INFO, 
              "iris::SceneGraph::addNode: node already in map; replacing\n") ;
            removeNode(name) ;
#endif
        }
#endif

        // Check if the node is in the node map under its old name.
        // If it is, remove it from the node map.
        std::string oldName = node->getName();
        if (oldName != "")
        {
            osg::Node* altNode = findNode(oldName) ;
            if (altNode == node)
                {
                removeNode(oldName);
                }
        }

        // change the node's name and add it to the node map
        node->setName(name) ;

        osg::ref_ptr<osg::Node> nodeRefPtr = node ;
        _nodeMap.insert(std::make_pair(name, nodeRefPtr)) ;
#ifdef SCENEGRAPH_NODE_PTR_SET
        _nodePtrSet.insert(nodeRefPtr) ;
#endif

        return true ;
    }
    
    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::getReferenceCount(const std::string& name)
    {
        osg::Node* node = findNode(name) ;
        if (!node)
        {
            dtkMsg.add(DTKMSG_ERROR, 
                "iris::SceneGraph::getReferenceCount: "
                "unable to find node node named %s\n",
                name.c_str()) ;
            return false ;
        }
        fprintf(stderr,"node %s has ref count %d\n", name.c_str(), 
                    node->referenceCount()) ;
        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::_setEcho(const std::string& echo)
    {
        bool ret = OnOff(echo, &_echo) ;
        if (_echo) fprintf(stderr,"ECHO %s\n",echo.c_str()) ;
        return ret ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::_setLog(const std::string& log)
    {
        bool onOff ;
        bool ret = OnOff(log, &onOff) ;
        if (ret) setLog(onOff) ;
        return ret ;
    }

    ////////////////////////////////////////////////////////////////////////
    void SceneGraph::_listNodes()
    {
        fprintf(stderr,"%d nodes in list: ", _nodeMap.size()) ;
        std::map<std::string, osg::ref_ptr<osg::Node> >::iterator pos ;
        for (pos = _nodeMap.begin(); pos != _nodeMap.end(); pos++)
        {
            fprintf(stderr,"    %s  %p\n",pos->first.c_str(), pos->second.get()) ;
        }
        fprintf(stderr,"\n") ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::_addChild(const std::string& nodeName, const std::string& groupName, const std::string& index)
    {
        osg::Node* node = findNode(nodeName) ;
        if (!node)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::addChild: unable to find node named %s\n", nodeName.c_str()) ;
            return false ;
        }
        
        osg::Node* groupNode = findNode(groupName) ;
        if (!groupNode) 
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::addChild: unable to find node named %s\n", groupName.c_str()) ;
            return false ;
        }
        
        osg::Group* group = groupNode->asGroup() ;
        if (!group)
            
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::addChild: unable to use node named %s as a group node\n", groupName.c_str()) ;
            return false ;
        }
        
        // if the child node is a ClipNode, check to see how many parents it has, enable it
        ClipNode *cn = dynamic_cast<ClipNode*>(node) ;
        if (cn)
        {
            if (cn->getNumParents() != 0) dtkMsg.add(DTKMSG_WARNING, "iris::SceneGraph::addChild: node %s is a ClipNode with multiple parents.  Expect the worst\n",nodeName.c_str()) ;
            cn->enable() ;
        }

        // if the child node is a LightNode, check to see how many parents it has, enable it
        LightNode *ln = dynamic_cast<LightNode*>(node) ;
        if (ln)
        {
            if (ln->getNumParents() != 0) dtkMsg.add(DTKMSG_WARNING, "iris::SceneGraph::addChild: node %s is a LightNode with multiple parents.  Expect the worst\n",nodeName.c_str()) ;
            ln->enable() ;
        }

        // sequence nodes overload addChild and insertChild
        osg::Sequence* seq = dynamic_cast<osg::Sequence*>(group) ;
    
        if (index == "")
        {
            if (seq) seq->addChild(node) ;
            else group->addChild(node) ;
        }
        else
        {
            int foo ;
            if (StringToInt(index, &foo))
            {
                if (seq) seq->insertChild(foo, node) ;
                else group->insertChild(foo, node) ;
            }
            else
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::addChild: unable to convert \"%s\" to an integer\n",index.c_str()) ;
                return false ;
            }
        }
    
        if (seq)
        {
            // osg::Sequence node bug workaround!
            seq->setInterval(osg::Sequence::LOOP, 0, seq->getNumChildren()-1) ;
            seq->setMode(osg::Sequence::START);
        }
    
        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::_matrix(const std::vector<std::string>& vec)
    {
        osg::Node *node ;
        osg::MatrixTransform* omat ;
        MatrixTransform* imat ;
        // does the node already exist?
        if ((node=findNode(vec[1])))
        {
            // is it a MatrixTransform?
            // we can handle an existing node that's either a osg::MatrixTransform or iris::MatrixTransform
            omat = dynamic_cast<osg::MatrixTransform*>(node) ;
            if (!omat)
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::_matrix: \"%s\" is not a iris::MatrixTransform or osg::MatrixTransform node\n",vec[1].c_str()) ;
                return false ;
            }
        }
        else
        {
            imat = new MatrixTransform ;
            addNode(vec[1],imat) ;
            omat = dynamic_cast<osg::MatrixTransform*>(imat) ;
            //printf("omat = %p\n",omat) ;
            //printf("imat = %p\n",imat) ;
        }
        if (vec.size()==18)
        {
            double a00, a01, a02, a03,
                a10, a11, a12, a13,
                a20, a21, a22, a23,
                a30, a31, a32, a33 ;
            if (StringToDouble(vec[ 2], &a00) &&
                StringToDouble(vec[ 3], &a01) &&
                StringToDouble(vec[ 4], &a02) &&
                StringToDouble(vec[ 5], &a03) &&
                StringToDouble(vec[ 6], &a10) &&
                StringToDouble(vec[ 7], &a11) &&
                StringToDouble(vec[ 8], &a12) &&
                StringToDouble(vec[ 9], &a13) &&
                StringToDouble(vec[10], &a20) &&
                StringToDouble(vec[11], &a21) &&
                StringToDouble(vec[12], &a22) &&
                StringToDouble(vec[13], &a23) &&
                StringToDouble(vec[14], &a30) &&
                StringToDouble(vec[15], &a31) &&
                StringToDouble(vec[16], &a32) &&
                StringToDouble(vec[17], &a33))
                omat->setMatrix(osg::Matrix(a00, a01, a02, a03,
                                            a10, a11, a12, a13,
                                            a20, a21, a22, a23,
                                            a30, a31, a32, a33)) ;
        }
        else if (vec.size()!=2) return false ;


        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::_dcs(bool dynamic, bool quaternion, const std::vector<std::string>& vec)
    {
        osg::Vec3 pos ;
        osg::Quat attitude ;
        osg::Vec3 scale ;
        // get translation
        if (vec.size()>4)
        {
            double tx, ty, tz ;
            if (StringToDouble(vec[2], &tx) && StringToDouble(vec[3], &ty) && StringToDouble(vec[4], &tz))
            {
                pos = osg::Vec3(tx,ty,tz) ;
            }
            else
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::dcs: unable to convert translation parameter to a double\n") ;
                return false ;
            }
        }
        // get rotation
        if (quaternion && vec.size()>8)
        {
            double qx, qy, qz, qw ;
            if (StringToDouble(vec[5], &qx) && StringToDouble(vec[6], &qy) && StringToDouble(vec[7], &qz) && StringToDouble(vec[8], &qw))
            {
                attitude = osg::Quat(qx, qy, qz, qw) ;
            }
            else
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::qdcs/qscs: unable to convert quaternion parameter to a double\n") ;
                return false ;
            }
        }
        else if (!quaternion && vec.size()>7)
        {
            double h, p, r ;
            if (StringToDouble(vec[5], &h) && StringToDouble(vec[6], &p) && StringToDouble(vec[7], &r))
            {
                attitude = EulerToQuat(h,p,r) ;
            }
            else
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::dcs/scs: unable to convert rotation parameter to a double\n") ;
                return false ;
            }
        }
        
        // the quat vector has one more parameter, so add one to scale parameter position
        int qoffset ;
        if (quaternion) qoffset=1;
        else qoffset=0 ;

        // get uniform scale
        if (vec.size()==9+qoffset)
        {
            double s ;
            if (StringToDouble(vec[8+qoffset], &s))
            {
                scale = osg::Vec3(s,s,s) ;
            }
            else
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::qdcs/qscs: unable to convert scale parameter to a double\n") ;
                return false ;
            }
        }
        // get non-uniform scale
        else if (vec.size()==11+qoffset)
        {
            double sx, sy, sz ;
            if (StringToDouble(vec[8+qoffset], &sx) && StringToDouble(vec[9+qoffset], &sy) && StringToDouble(vec[10+qoffset], &sz))
            {
                scale = osg::Vec3(sx,sy,sz) ;
            }
            else
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::qdcs/qscs: unable to convert scale parameter to a double\n") ;
                return false ;
            }
        }
        if (vec.size()==2) return _mat(vec[1],dynamic) ;
        else if (vec.size()==5) return _mat(vec[1],dynamic,pos) ;
        else if (vec.size()==8+qoffset) return _mat(vec[1],dynamic,pos,attitude) ;
        else if (vec.size()==9+qoffset || vec.size()==11+qoffset) return _mat(vec[1],dynamic,pos,attitude,scale) ;
        else 
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::qdcs/qscs: invalid number of parameters, %d\n",vec.size()) ;
            return false ;
        }
        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::_group(const std::string& nodeName)
    {
        osg::Node* node = findNode(nodeName) ;
        if (node)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::group: node named %s already exists\n",nodeName.c_str()) ;            
            return false ;
        }
        else
        {
            osg::Group* group = new osg::Group ;
            group->setDataVariance(osg::Object::DYNAMIC) ;
            addNode(nodeName, group) ;
        }
            
        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::_switch (std::vector<std::string> vec)
    {
        // see if the node already exists and if so, is it a Switch?
        osg::Node* node = findNode(vec[1]) ;
        osg::Switch* sw ;
        if (!node)
        {
            sw = new osg::Switch ;
            addNode(vec[1], sw) ;
        }
        else
        {
            sw = node->asSwitch() ;
            if (!sw)
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::switch: node named \"%s\" exists but isn't a osg::Switch\n",vec[1].c_str()) ;
                return false ;
            }
        }

        if (vec.size()<3) return true ;
        
        enum { ON, OFF, ONLYON } state ;
        if (IsSubstring("on", vec[2], 2)) state = ON ;
        else if (IsSubstring("off", vec[2], 3)) state = OFF;
        else if (vec.size() == 4 && IsSubstring("onlyon", vec[2], 3)) state = ONLYON ;
        else return false ;
        
        if (IsSubstring("all", vec[3], 3)) 
        {
            if (state == ONLYON)
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::switch: can't use ALL with ONLYON\n") ;
                return false ;
            }
            for (unsigned int i=0; i<sw->getNumChildren(); i++)
            {
                sw->setValue(i,(state==ON)) ;        
            }
        }
        else
        {
            std::vector<unsigned int> n ;
            for (unsigned int i=3; i<vec.size(); i++)
            {
                int j ;
                if (!StringToInt(vec[i], &j) || j<0 || j>=sw->getNumChildren())
                {
                    dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::switch: child number %s invalid or out of range, max = %d\n",vec[i].c_str(), sw->getNumChildren()-1) ;
                    return false ;
                }
                n.push_back(j) ;
            }
            
            if (state == ONLYON)
            {
                sw->setSingleChildOn(n[0]) ;
            }
            else
            {
                for (unsigned int i=0; i<n.size(); i++)
                {
                    sw->setValue(n[i],(state==ON)) ;
                }
            }
        }
        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::_map(const std::string& nodeName)
    {
        return mapNode(nodeName) ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::_noclipnode(const std::string& nodeName)
    {

        // see if the node already exists and if so, is it a MatrixTransform?
        osg::Node* node = findNode(nodeName) ;

        if (!node)
        {
            osg::Group* group = new osg::Group ;
            group->setDataVariance(osg::Object::DYNAMIC) ;
            for (int i=0; i<ClipNode::maxNodes(); i++) group->getOrCreateStateSet()->setMode(GL_CLIP_PLANE0+i, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );
            return addNode(nodeName, group) ;
        }
        else
        {
            for (int i=0; i<ClipNode::maxNodes(); i++) node->getOrCreateStateSet()->setMode(GL_CLIP_PLANE0+i, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );
            node->getOrCreateStateSet()->setDataVariance(osg::Object::DYNAMIC) ;
            return true ;
        }
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::_nolightnode(const std::string& nodeName)
    {

        // see if the node already exists and if so, is it a MatrixTransform?
        osg::Node* node = findNode(nodeName) ;

        if (!node)
        {
            osg::Group* group = new osg::Group ;
            group->setDataVariance(osg::Object::DYNAMIC) ;
            group->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
            return addNode(nodeName, group) ;
        }
        else
        {
            node->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
            node->getOrCreateStateSet()->setDataVariance(osg::Object::DYNAMIC) ;
            return true ;
        }
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::_startModelData(std::string nodeName, std::string extension)
    {
        dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph: starting model data\n") ;
        if (findNode(nodeName))
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::startModelData: node %s already exists\n",nodeName.c_str()) ; 
            return false ;
        }

        if (_inModelDataFileStream)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::startModelData: STARTMODELDATA command found inside of startModelData\n") ;
            return false ;
        }

        _inModelDataNodeName = nodeName ; 
        _inModelDataFileName = GetTempFileName(extension.c_str(), "/tmp") ;
        
        dtkMsg.add(DTKMSG_DEBUG, "iris::SceneGraph::startModelData: tmp file is \"%s\"\n",_inModelDataFileName.c_str()) ;

        _inModelDataFileStream = new std::ofstream(_inModelDataFileName.c_str(), std::ios::out) ;
        
        if (_inModelDataFileStream) return true ;
        else
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::startModelData: couldn't open stream to file %s\n",_inModelDataFileName.c_str()) ;
            return false ;
        }
    }
    
    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::_endModelData()
    {
        dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph: ending model data\n") ;
        
        if (!_inModelDataFileStream)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::startModelData: ENDMODELDATA command with no corresponding STARTMODELDATA command\n") ;
            return false ;
        }
        
        osg::Node *node = osgDB::readNodeFile(_inModelDataFileName) ;
        if (!node)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::startModelData: couldn't load file %s\n",_inModelDataFileName.c_str()) ;
            return false ;
        }
        addNode(_inModelDataNodeName, node) ;
        
        delete _inModelDataFileStream ;
        _inModelDataFileStream = NULL ;
        
        unlink(_inModelDataFileName.c_str()) ;
        
        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::_load(const std::string& nodeName, const std::string& fileName)
    {
        dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::load: loading file \"%s\"\n",fileName.c_str()) ;
        osg::Node* node = osgDB::readNodeFile(fileName) ;
        if (!node)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::load: unable to load file named \"%s\"\n",fileName.c_str()) ;
            return false ;
        }
        else  dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::load: loaded file \"%s\"\n",fileName.c_str()) ;

        std::string newName = nodeName ;
        if (nodeName == "-" && node->getName() != "") newName = node->getName() ;
        if (addNode(newName, node))
        {
            _lastNode = node ;
            return true ;
        }
        else return false ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::_load(const std::string& fileName)
    {
#if 0
        // this doesn't seem to work when loading a file with an absolute path
        std::string ff = osgDB::findFileInPath(fileName,osgDB::getDataFilePathList()) ;
        if (ff == "")
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::load: unable to load file named \"%s\"\n",fileName.c_str()) ;
            return false ;
        }
#endif
        osg::Node* node = osgDB::readNodeFile(fileName) ;
        if (!node)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::load: unable to load file named \"%s\"\n",fileName.c_str()) ;
            return false ;
        }
        _lastNode = node ;

        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::_copyState(const std::string& fromNodeName, const std::string& toNodeName) 
    {
        osg::Node* fromNode = findNode(fromNodeName) ;
        if (!fromNode)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::copyState: unable to find node named %s\n", fromNodeName.c_str()) ;
            return false ;
        }

        osg::Node* toNode = findNode(toNodeName) ;
        if (!toNode)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::copyState: unable to find node named %s\n", toNodeName.c_str()) ;
            return false ;
        }

        toNode->setStateSet(fromNode->getOrCreateStateSet()) ;

        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::_cloneNode(const std::string& nodeName, const std::string& copyName, osg::CopyOp::CopyFlags op)
    {
        osg::Node* node = findNode(nodeName) ;
        if (!node)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::cloneNode: unable to find node named %s\n", nodeName.c_str()) ;
            return false ;
        }

        if (findNode(copyName))
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::cloneNode: node named %s already exists\n", copyName.c_str()) ;
            return false ;
        }

        // does this really work?!
        osg::ref_ptr<osg::Node> copy = dynamic_cast<osg::Node*>(node->clone(op)) ;
        addNode(copyName, copy) ;

        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::_mergeState(const std::string& fromNodeName, const std::string& toNodeName) 
    {
        osg::Node* fromNode = findNode(fromNodeName) ;
        if (!fromNode)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::mergeState: unable to find node named %s\n", fromNodeName.c_str()) ;
            return false ;
        }

        osg::Node* toNode = findNode(toNodeName) ;
        if (!toNode)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::mergeState: unable to find node named %s\n", toNodeName.c_str()) ;
            return false ;
        }

        toNode->getOrCreateStateSet()->merge(*(fromNode->getOrCreateStateSet())) ;

        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::_removeChild(const std::string& nodeName, const std::string& groupName)
    {
        
        osg::Node* node = findNode(nodeName) ;
        if (!node)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::removeChild: unable to find node named %s\n", nodeName.c_str()) ;
            return false ;
        }
        
        osg::Node* groupNode = findNode(groupName) ;
        if (!groupNode) 
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::removeChild: unable to find node named %s\n", groupName.c_str()) ;
            return false ;
        }
        
        osg::Group* group = groupNode->asGroup() ;
        if (!group)
            
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::removeChild: unable to use node named %s as a group node\n", groupName.c_str()) ;
            return false ;
        }

        if (!group->removeChild(node))
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::removeChild: unable to remove node %s as child of group node %s\n", nodeName.c_str(),groupName.c_str()) ;
            return false ;
        }

        // if the child node is a ClipNode, disable it
        ClipNode *cn = dynamic_cast<ClipNode*>(node) ;
        if (cn) cn->disable() ;
        
        // if the child node is a LightNode, disable it
        LightNode *ln = dynamic_cast<LightNode*>(node) ;
        if (ln) ln->disable() ;
        
        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::_unload(const std::string& nodeName) 
    {
        osg::Node* node = findNode(nodeName) ;
        if (!node)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::unload: unable to find node named %s\n", nodeName.c_str()) ;
            return false ;
        }
        
        // remove all links to parents
        std::vector<osg::Group*> ps = node->getParents() ;

        for (unsigned int i=0; i< ps.size() ; i++)
        {
            ps[i]->removeChild(node) ;
        }

        // is it in the set? if so, remove it
        removeNode(nodeName) ;
        return true ;
    }
    
    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::_mat(const std::string& nodeName, bool dynamic, osg::Vec3 pos, osg::Quat attitude, osg::Vec3 scale) 
    {
        // see if the node already exists and if so, is it a MatrixTransform?
        osg::Node* node = findNode(nodeName) ;
        osg::MatrixTransform* omat ;
        MatrixTransform* imat ;
        if (node)
        {
            // node exists already- is a MatrixTransform?
            // we can handle an existing node that's either a osg::MatrixTransform or iris::MatrixTransform
            omat = dynamic_cast<osg::MatrixTransform*>(node) ;
            if (!omat)
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::_mat: \"%s\" is not a iris::MatrixTransform or osg::MatrixTransform node\n",nodeName.c_str()) ;
                return false ;
            }
        }
        else // create and add
        {
            imat = new MatrixTransform ;
            addNode(nodeName,imat) ;
            omat = dynamic_cast<osg::MatrixTransform*>(imat) ;
            //printf("omat = %p\n",omat) ;
            //printf("imat = %p\n",imat) ;
        }

        osg::Matrix m ;
        m.preMultTranslate(pos);
        m.preMultRotate(attitude);
        m.preMultScale(scale);

        omat->setMatrix(m) ;

        if (dynamic) omat->setDataVariance(osg::Object::DYNAMIC);
        else omat->setDataVariance(osg::Object::STATIC);
        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::_shmMatrix(const std::vector<std::string>& vec)
    {
        if (vec.size()<3) return false ;

#if 1
        osg::Node* node = findNode(vec[1]) ;
        ShmMatrixTransform* smt ;
        int mask ;
        if (node)
        {
            smt = dynamic_cast<ShmMatrixTransform*>(node) ;
            if (!smt)
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::shmMatrix: node named %s already exists but is not a ShmMatrixTransform\n",vec[1].c_str()) ;            
                return false ;
            }
            else // node exists- just setting the mask?
                if (vec.size() == 4 && IsSubstring("mask",vec[2],3) && 
                    StringToInt(vec[3], &mask) && mask>=0 && mask<256)
                {
                    smt->setActiveMask((unsigned char)mask) ;
                    dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::shmMatrix: setting active mask for node %s to %d\n",vec[1].c_str(),mask) ;
                    return true ;
                }
        }
        else // create and add
        {
            if (vec.size() == 3) smt = new ShmMatrixTransform(vec[2]) ;
            else if (vec.size() == 4) smt = new ShmMatrixTransform(vec[2],vec[3]) ;
            else if (vec.size() == 5 && 
                     StringToInt(vec[4], &mask) && mask>=0 && mask<256) 
                smt = new ShmMatrixTransform(vec[2],vec[3],(unsigned char)mask) ;
            else
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::shmMatrix: invalid number of parameters\n") ;
                return false ;
            }
            addNode(vec[1], smt) ;
        }

#else
        osg::Node* node = findNode(vec[1]) ;
        ShmMatrixTransform* smt ;
        int mask ;
        if (node)
        {
            smt = dynamic_cast<ShmMatrixTransform*>(node) ;
            if (!smt)
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::shmMatrix: node named %s already exists but is not a ShmMatrixTransform\n",vec[1].c_str()) ;            
                return false ;
            }
            else // node exists- just setting the mask?
                if (vec.size() == 4 && IsSubstring("mask",vec[2],3) && 
                    StringToInt(vec[3], &mask) && mask>=0 && mask<256)
                {
                    //smt->setActiveMask((unsigned char)mask) ;
                    dtkMsg.add(DTKMSG_INFO, "iris::SceneGraph::shmMatrix: setting active mask for node %s to %d\n",vec[1].c_str(),mask) ;
                    return true ;
                }
        }
        else // create and add
        {
            if (vec.size() == 3) smt = new ShmMatrixTransform() ; //vec[2]) ;
            else if (vec.size() == 4) smt = new ShmMatrixTransform() ; //vec[2],vec[3]) ;
            else if (vec.size() == 5 && 
                     StringToInt(vec[4], &mask) && mask>=0 && mask<256) 
                smt = new ShmMatrixTransform() ; //vec[2],vec[3],(unsigned char)mask) ;
            else
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::shmMatrix: invalid number of parameters\n") ;
                return false ;
            }
            addNode(vec[1], smt) ;
        }

#endif
    

        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    // queries always have a one line response, ending in a \n, to a single query
    bool SceneGraph::_query(const std::string& line, const std::vector<std::string>& vec)
    {
        // assumes vec.size()>2
        // vec[1] is always the name of the fifo

        FifoReader* fifo = _getFifo(vec[1]) ;
        int fd = fifo->getFD() ;
        if (fd < 0)
        {
            dtkMsg.add(DTKMSG_ERROR,"iris::SceneGraph::query FIFO FD = %d\n",fd) ;
            return false ;
        }
        fifo->open(O_RDWR | O_NONBLOCK) ;

        if (IsSubstring("lightmodel",vec[2],6))
        {
            std::string response ;
            response = "LIGHTMODEL " ;
            osg::LightModel* lm = dynamic_cast<osg::LightModel*>(SceneGraph::instance()->getSceneNode()->getOrCreateStateSet()->getAttribute(osg::StateAttribute::LIGHTMODEL)) ;
            if (!lm)
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::lightmodel: Unable to get light model from scene node\n",vec[1].c_str()) ;
                response += "ERROR: LIGHTMODEL NOT FOUND\n" ;
                fifo->write(response) ;
                return false ;
            }
            else
            {
                osg::Vec4 v4 ;
                v4 = lm->getAmbientIntensity() ;
                response += "AMBIENT " + FloatToString(v4.r()) + " " + FloatToString(v4.g()) + " " + FloatToString(v4.b()) + " " + FloatToString(v4.a()) + " " ; 

                if (lm->getLocalViewer()) response += "LOCAL_VIEWER TRUE " ;
                else response += "LOCAL_VIEWER FALSE " ;

                if (lm->getTwoSided()) response += "TWO_SIDED TRUE " ;
                else response += "TWO_SIDED FALSE " ;

                response += "\n" ;
                return fifo->write(response) ;
            }
        }

        else if (IsSubstring("clipnode",vec[2],3))
        {
            if (vec.size() == 3)
            {
                std::string response ;
                response = "CLIPNODE " ;
                std::vector<ClipNode*> cl = ClipNode::getClipNodes() ;
                if (cl.size() == 0)
                {
                    response += "0\n" ;
                }
                else
                {
                    response += " " + IntToString(cl.size()) ;
                    for (int i=0; i<cl.size(); i++)
                    {
                        response += " \'" +  std::string(cl[i]->getName()) + "\'" ;
                    }
                    response += "\n" ;
                }

                //dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::query clipnode response = %s",response.c_str()) ;
                return fifo->write(response) ;
            }
            else if (vec.size() > 3) 
            {
                osg::Node* node = findNode(vec[3]) ;
                ClipNode* cn ;
                if (node) cn = dynamic_cast<ClipNode*>(node) ;
                std::string response ;
                response = "CLIPNODE \'" ;
                response += vec[3] ;
                if (!node || !cn)
                {
                    dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::query: unable to find node named %s\n", vec[3].c_str()) ;
                    response += "\' ERROR: NODE NOT FOUND\n" ;
                    fifo->write(response) ;
                    return false ;
                }
                else
                {
                    int ncp = cn->getNumClipPlanes() ;
                    if (cn->getNumClipPlanes() != 1)
                    {
                        dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::query: node %s has %d clip planes\n",vec[3].c_str(),ncp) ;
                        response += "\' ERROR: NODE HAS " + IntToString(ncp) + " CLIP NODES\n" ;
                        fifo->write(response) ;
                        return false ;
                    }
                    response += "\' " ;

                    response += "NUMBER " + IntToString(cn->getClipPlane(0)->getClipPlaneNum()) + " " ;

                    if (cn->isEnabled()) response += "ON TRUE " ;
                    else response += "ON FALSE " ;

                    response += "\n" ;
                    return fifo->write(response) ;
                }
            }
        }

    else if (IsSubstring("uniform",vec[2],3))
    {
        if ( (vec.size() < 4) )
        {
            dtkMsg.add ( DTKMSG_ERROR, 
                     "iris::SceneGraph : query uniform : Bad argument count.\n");
            return false;  // bad
        }

        // node must exist
        osg::Node *node = findNode(vec[3]) ;
        if (!node)
        {
            dtkMsg.add(DTKMSG_ERROR,
                       "iris::SceneGraph : query uniform : Node  %s not found.\n",
                       vec[3].c_str() );
            return false;  // bad
        }

        // get the state set
        osg::StateSet *ss = node->getStateSet() ;
        if (!ss)
        {
            dtkMsg.add(DTKMSG_ERROR,
                       "iris::SceneGraph : query uniform : Node  %s has no stateset.\n",
                       vec[3].c_str() );
            return false; // bad
        }

        std::string response = "UNIFORM";
        if (vec.size() == 4)
        {

            osg::StateSet::UniformList ul = ss->getUniformList();
            if (ul.size() == 0)
            {
                response += "0";
            }
            else
            {
                response += " " + IntToString(ul.size()) ;
                for (osg::StateSet::UniformList::const_iterator it = ul.begin();
                     it != ul.end(); ++it)
                {
                    response += " \'" + it->first + "\'" ;
                }
            }

            response += "\n";
            return fifo->write(response);
        }
        else if (vec.size() > 4)
        {
            response += " \'";
            response += vec[3] ;
            response += "\' \'";
            response += vec[4] ;

            osg::Uniform *u = ss->getUniform(vec[4]);
            if (!u)
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph : query uniform : Node  %s has no uniform  %s\n",
                           vec[3].c_str(), vec[4].c_str() );
                response += "\' ERROR: UNIFORM NOT FOUND\n";
                fifo->write(response);
                return false;
            }
            else
            {
                response += "\' ";
                response += osg::Uniform::getTypename(u->getType());
                response += " ";
                response += IntToString(u->getNumElements());
                response += " ";

                switch(u->getType())
                {
                case osg::Uniform::FLOAT:
                case osg::Uniform::FLOAT_VEC2:
                case osg::Uniform::FLOAT_VEC3:
                case osg::Uniform::FLOAT_VEC4:
                case osg::Uniform::FLOAT_MAT2:
                case osg::Uniform::FLOAT_MAT3:
                case osg::Uniform::FLOAT_MAT4:
                case osg::Uniform::FLOAT_MAT2x3:
                case osg::Uniform::FLOAT_MAT2x4:
                case osg::Uniform::FLOAT_MAT3x2:
                case osg::Uniform::FLOAT_MAT3x4:
                case osg::Uniform::FLOAT_MAT4x2:
                case osg::Uniform::FLOAT_MAT4x3:
                    {
                        osg::FloatArray const *a = u->getFloatArray();
                        if (!a)
                        {
                            dtkMsg.add(DTKMSG_ERROR,
                                       "iris::SceneGraph : query uniform : Cannot query uniform  %s\n",
                                       vec[4].c_str() );
                            return false;
                        }
                        else
                        {
                            float const *f = (float const*)a->getDataPointer();
                            for (int k = 0; k < a->getNumElements(); ++k, ++f)
                            {
                                response += FloatToString(*f);
                                if (k < a->getNumElements() - 1) response += " ";
                            }
                        }
                        break;
                    }
                case osg::Uniform::INT:
                case osg::Uniform::INT_VEC2:
                case osg::Uniform::INT_VEC3:
                case osg::Uniform::INT_VEC4:
                    {
                        osg::IntArray const* a = u->getIntArray();
                        if (!a)
                        {
                            dtkMsg.add(DTKMSG_ERROR,
                                       "iris::SceneGraph : query uniform : Cannot query uniform  %s\n",
                                       vec[4].c_str() );
                            return false;
                        }
                        else
                        {
                            int const* i = (int const*)a->getDataPointer();
                            for (int k = 0; k < a->getNumElements(); ++k, ++i)
                            {
                                response += IntToString(*i);
                                if (k < a->getNumElements() - 1) response += " ";
                            }
                        }
                        break;
                    }
                case osg::Uniform::UNSIGNED_INT:
                case osg::Uniform::UNSIGNED_INT_VEC2:
                case osg::Uniform::UNSIGNED_INT_VEC3:
                case osg::Uniform::UNSIGNED_INT_VEC4:
                    {
                        osg::UIntArray const* a = u->getUIntArray();
                        if (!a)
                        {
                            dtkMsg.add(DTKMSG_ERROR,
                                       "iris::SceneGraph : query uniform : Cannot query uniform  %s\n",
                                       vec[4].c_str() );
                            return false;
                        }
                        else
                        {
                            unsigned int const* u = (unsigned int const*)a->getDataPointer();
                            for (int k = 0; k < a->getNumElements(); ++k, ++u)
                            {
                                response += IntToString(*u);
                                if (k < a->getNumElements() - 1) response += " ";
                            }
                        }
                        break;
                    }
                case osg::Uniform::BOOL:
                case osg::Uniform::BOOL_VEC2:
                case osg::Uniform::BOOL_VEC3:
                case osg::Uniform::BOOL_VEC4:
                    {
                        for (int k = 0; k < u->getNumElements(); ++k)
                        {
                            bool b;
                            if (!u->getElement(k, b))
                            {
                                dtkMsg.add(DTKMSG_ERROR,
                                           "iris::SceneGraph : query uniform : Cannot query uniform  %s\n",
                                           vec[4].c_str() );
                                return false;
                            }
                            else
                            {
                                response += (b ? "true" : "false");
                                if (k < u->getNumElements() - 1) response += " ";
                            }
                        }
                        break;
                    }
                }
            }

            response += "\n";
            return fifo->write(response);
        }
    }

        else if (IsSubstring("lightnode",vec[2],6))
        {
            if (vec.size() == 3)
            {
                std::string response ;
                response = "LIGHTNODE " ;
                std::vector<LightNode*> nl = LightNode::getLightNodes() ;
                if (nl.size() == 0)
                {
                    response += "0\n" ;
                }
                else
                {
                    response += " " + IntToString(nl.size()) ;
                    for (int i=0; i<nl.size(); i++)
                    {
                        response += " \'" +  std::string(nl[i]->getName()) + "\'" ;
                    }
                    response += "\n" ;
                }

                //dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::query lightnode response = %s",response.c_str()) ;
                return fifo->write(response) ;
            }
            else if (vec.size() > 3) 
            {
                osg::Node* node = findNode(vec[3]) ;
                LightNode* ln ;
                if (node) ln = dynamic_cast<LightNode*>(node) ;
                std::string response ;
                response = "LIGHTNODE \'" ;
                response += vec[3] ;
                if (!node || !ln)
                {
                    dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::query: unable to find node named %s\n", vec[3].c_str()) ;
                    response += "\' ERROR: NODE NOT FOUND\n" ;
                    fifo->write(response) ;
                    return false ;
                }
                else
                {
                    osg::Light* l = ln->getLight() ;
                    response += "\' " ;

                    response += "NUMBER " + IntToString(l->getLightNum()) + " " ;

                    if (ln->isEnabled()) response += "ON TRUE " ;
                    else response += "ON FALSE " ;

                    osg::Vec4 v4 ;
                    v4 = l->getAmbient() ;
                    response += "AMBIENT " + FloatToString(v4.r()) + " " + FloatToString(v4.g()) + " " + FloatToString(v4.b()) + " " + FloatToString(v4.a()) + " " ; 
                
                    v4 = l->getDiffuse() ;
                    response += "DIFFUSE " + FloatToString(v4.r()) + " " + FloatToString(v4.g()) + " " + FloatToString(v4.b()) + " " + FloatToString(v4.a()) + " " ; 
                
                    v4 = l->getSpecular() ;
                    response += "SPECULAR " + FloatToString(v4.r()) + " " + FloatToString(v4.g()) + " " + FloatToString(v4.b()) + " " + FloatToString(v4.a()) + " " ; 
                
                    v4 = l->getPosition() ;
                    response += "POSITION " + FloatToString(v4.x()) + " " + FloatToString(v4.y()) + " " + FloatToString(v4.z()) + " " + FloatToString(v4.w()) + " " ; 
                
                    osg::Vec3 v3 = l->getDirection() ;
                    response += "DIRECTION " + FloatToString(v3.x()) + " " + FloatToString(v3.y()) + " " + FloatToString(v3.z()) + " " ; 

                    response += "ATTENUATION CONSTANT " + FloatToString(l->getConstantAttenuation()) + " " ;  
                
                    response += "ATTENUATION LINEAR " + FloatToString(l->getLinearAttenuation()) + " " ;  
                
                    response += "ATTENUATION QUADRATIC " + FloatToString(l->getQuadraticAttenuation()) + " " ;  
                
                    response += "SPOT " + FloatToString(l->getSpotExponent()) + " "  + FloatToString(l->getSpotCutoff()) + " ";  
                
                    response += "REF_FRAME " ;
                    if (ln->getReferenceFrame() == osg::LightSource::RELATIVE_RF) response += "RELATIVE " ;
                    else response += "ABSOLUTE " ; 

                    response += "\n" ;
                    return fifo->write(response) ;
                }
            }
        }

        else if (IsSubstring("ping",vec[2],3))
        {
            std::string response = "PING" ;
            for (int i=3; i<vec.size(); i++) response += std::string(" ") + vec[i] ;
            response += "\n" ;
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::query ping response %s",response.c_str()) ;
            return fifo->write(response) ;
        }

        else if (IsSubstring("jump",vec[2],3) && vec.size() == 3)
        {
            std::string response = "JUMP " ;
            if (getJump()) response += "ON POSITION " ;
            else response += "OFF POSITION " ;
            osg::Vec3 t = getJumpPosition() ;
            response += FloatToString(t.x()) + " " + FloatToString(t.y()) + " " + FloatToString(t.z()) + " " ;
            double h, p, r ;
            QuatToEuler(getJumpAttitude(), &h, &p, &r) ;
            response += "ORIENTATION " + DoubleToString(h) + " " + DoubleToString(p) + " " + DoubleToString(r) ;
            response += "\n" ;
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::query jump response %s",response.c_str()) ;
            return fifo->write(response) ;
        }

        else if (IsSubstring("boundingbox",vec[2],3) && (vec.size() == 3 || vec.size() == 4))
        {
            std::string nodeName ;
            if (vec.size() == 4) nodeName = vec[3] ;
            else nodeName = "world" ;

            std::string response = std::string("BOUNDINGBOX ") + nodeName + std::string(" ") ;
            osg::Node* node = findNode(nodeName) ;
            if (!node)
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::query: unable to find node named %s\n", nodeName.c_str()) ;
                response += "\' ERROR: NODE NOT FOUND\n" ;
                fifo->write(response) ;
                return false ;
            }
            osg::Group* group = node->asGroup() ;
            if (!group)
            {
                dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::query: unable to find group node named %s\n", nodeName.c_str()) ;
                response += "\' ERROR: NODE NOT FOUND\n" ;
                fifo->write(response) ;
                return false ;
            }
            osg::BoundingBox bb = GetBoundingBox(group) ;;
            response += DoubleToString(bb.xMin()) + std::string(" ") + DoubleToString(bb.xMax()) + std::string(" ") ;
            response += DoubleToString(bb.yMin()) + std::string(" ") + DoubleToString(bb.yMax()) + std::string(" ") ;
            response += DoubleToString(bb.zMin()) + std::string(" ") + DoubleToString(bb.zMax()) + std::string("\n") ;
            dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::query boundingbox %s response %s",nodeName.c_str(),response.c_str()) ;
            return fifo->write(response) ;
        }

    else if (IsSubstring("graph",vec[2],3) && (vec.size() == 4 || vec.size() == 5))
    {
        _GraphQueryVisitor::OutputType outputType;

        if (IsSubstring("json",vec[3],3))
        {
            outputType = _GraphQueryVisitor::JSON;
        }

        else
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::query: unknown graph output format %s\n", vec[3].c_str()) ;
            fifo->write("\n") ;
            return false ;
        }

        osg::Node* node;
        if (vec.size() == 5) node = findNode(vec[4]) ;
        else node = getWorldNode();

        if (!node)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::query: unable to find node named %s\n", vec[4].c_str()) ;
            fifo->write("\n") ;
            return false ;
        }

        _GraphQueryVisitor gqv(outputType);
        node->accept(gqv);

        return fifo->write(gqv.response());
    }

    else if (IsSubstring("children",vec[2],4) && vec.size() == 4)
    {
        osg::Node* node = findNode(vec[3]) ;
        std::string response ;
        response = "CHILDREN \'" ;
        response += vec[3] ;

        if (!node)
        {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::query: unable to find node named %s\n", vec[3].c_str()) ;
            response += "\' ERROR: NODE NOT FOUND\n" ;
            fifo->write(response) ;
            return false ;
            }

        osg::Group* g = node->asGroup();
        if (!g || g->getNumChildren() == 0) response += "\' 0\n";
        else
        {
            response += "\' " + IntToString(g->getNumChildren()) ;
            for (int i = 0; i < g->getNumChildren(); ++i) {
                response += " \'" + g->getChild(i)->getName() + "\'" ;
            }
            response += "\n" ;
        }
        return fifo->write(response);
    }

        else if (IsSubstring("parents",vec[2],3) && vec.size() == 4)
        {
            osg::Node* node = findNode(vec[3]) ;
            std::string response ;
            response = "PARENTS \'" ;
            response += vec[3] ;
            if (!node)
            {
            dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::query: unable to find node named %s\n", vec[3].c_str()) ;
            response += "\' ERROR: NODE NOT FOUND\n" ;
            fifo->write(response) ;
            return false ;
            }
            std::vector<osg::Group*> ps = node->getParents() ;
            if (ps.size()==0 || vec[3] == "scene") response += "\' 0\n" ;
            else
            {
            response += "\' " + IntToString(ps.size()) ;
            for (int i=0; i<ps.size(); i++)
            {
                response += " \'" +  ps[i]->getName() + "\'" ;
            }
            response += "\n" ;
            }
            return fifo->write(response) ;
        }

        else if (IsSubstring("panes",vec[2],3) && vec.size() == 3)
        {
            std::string response ;
            response = "PANES " ;
            std::vector<osg::ref_ptr<Pane> > panes = Pane::getPaneList() ;
            if (panes.size() == 0) response += "0\n" ;
            else
            {
                response += IntToString(panes.size()) ;
                for (unsigned int i=0; i<panes.size(); i++)
                {
                    if (panes[i]->getName()) response += " \'" + std::string(panes[i]->getName()) + "\'" ;
                    else
                    {
                        response += " \'\'" ;
                        dtkMsg.add(DTKMSG_WARNING, "iris::SceneGraph: pane has no name, \"\"\n") ;
                    }
                }
                response += "\n" ;
            }
            return fifo->write(response) ;
        }

        else if (IsSubstring("windows",vec[2],3))
        {
            if (vec.size() == 3)
            {
                std::string response ;
                response = "WINDOWS " ;
                std::vector<osg::ref_ptr<Window> > windows = Window::getWindowList() ;
                if (windows.size() == 0) response += "0\n" ;
                else
                {
                    response += IntToString(windows.size()) ;
                    for (unsigned int i=0; i<windows.size(); i++) response += " \'" + std::string(windows[i]->getName()) + "\'" ;
                    response += "\n" ;
                }
                return fifo->write(response) ;
            }
            else if (vec.size() == 4)
            {
                Window* window = Window::findWindow(vec[3].c_str());
                if (window)
                {
                    osg::GraphicsContext::Traits* traits = window->getTraits();
                    std::string response = "WINDOW \'" + std::string(window->getName()) + "\' : ";
                    response += " named \'" + traits->windowName + "\' ";
                    response += IntToString(traits->width) + "x" + IntToString(traits->height) + "+";
                    response += IntToString(traits->x) + "+" + IntToString(traits->y) + " ";
                    response += std::string("offscreen: ") + (traits->pbuffer?"YES":"NO") + " ";
                    response += "DISPLAY: " + traits->displayName() + " ";
                    response + "\n";
                    return fifo->write(response) ;
                }
                else
                {
                    dtkMsg.add(DTKMSG_ERROR, "iris::SceneGraph::query: unable to find window named %s\n", vec[3].c_str()) ;
                    fifo->write("WINDOW \' ERROR: WINDOW NOT FOUND\n") ;
                    return false ;
                }
            }
            else
            {
                std::string response = "ERROR! UNKNOWN COMMAND: " ;
                for (int i=2; i<vec.size(); i++) response += std::string(" ") + vec[i] ;
                response += "\n" ;
                fifo->write(response) ;
                return false ;
            }
        }

        else if (IsSubstring("navigation",vec[2],3) && vec.size() == 3)
        {
            std::string response ;
            response = "NAVIGATION " ;
            std::vector<osg::ref_ptr<Nav> > nl = Nav::getNavList() ;
            if (nl.size() == 0)
            {
                response += "0\n" ;
            }
            else
            {
                response += " " + IntToString(nl.size()) ;
                for (int i=0; i<nl.size(); i++)
                {
                    response += " \'" +  std::string(nl[i]->getName()) + "\'" ;
                }
                response += "\n" ;
            }
            //dtkMsg.add(DTKMSG_INFO,"iris::SceneGraph::query navigation response = %s",response.c_str()) ;
            return fifo->write(response) ;
        }

        else 
        {
            std::string response = "ERROR! UNKNOWN COMMAND: " ;
            for (int i=2; i<vec.size(); i++) response += std::string(" ") + vec[i] ;
            response += "\n" ;
            fifo->write(response) ;
            return false ;
        }
    }
    
    ////////////////////////////////////////////////////////////////////////
    FifoReader* SceneGraph::_getFifo(std::string name)
    {
        std::map <std::string, FifoReader*>::iterator itr ;
        itr = _queryFifos.find(name) ;

        if (itr == _queryFifos.end())
        {
            FifoReader* fifo = new FifoReader(name) ;
            fifo->open(O_RDWR | O_NONBLOCK) ;
            fifo->unlinkOnExit() ;
            _queryFifos[name] = fifo ;
            return fifo ;
        }
        else return itr->second ;
    }

    int SceneGraph::_deleteFifos()
    {
        std::map <std::string, FifoReader*>::iterator itr ;
        for (itr=_queryFifos.begin(); itr!=_queryFifos.end(); itr++)
        {
            delete(itr->second) ;
        }
    }

    ////////////////////////////////////////////////////////////////////////
    void SceneGraph::initEnvVariables() 
    { 
        std::string key ;
        std::string value ;
        //extern char** environ;
        // from BSD printenv.c by Bill Joy!
        for (char** ep = environ ; *ep; ep++)
        {
            std::string env(*ep) ;
            int eq = env.find("=") ;
            if (eq == env.npos)
            {
                dtkMsg.add(DTKMSG_WARNING,"iris::SceneGraph::initEnvVariables: non conforming envvar line: %s\n",env.c_str()) ;
            }
            else
            {
                key = "$" + env.substr(0,eq) ;
                value = env.substr(eq+1) ;
                //fprintf(stderr,"key = \"%s\", value = \"%s\"\n",key.c_str(),value.c_str()) ;
                addVariable(key,value) ;
            }
        }
    } ;

    ////////////////////////////////////////////////////////////////////////
    // replace ${key} with value if key is in _variables
    std::string SceneGraph::substituteVariable(const std::string& line) 
    { 
        dtkMsg.add(DTKMSG_DEBUG, "iris::SceneGraph::substituteVariable: input line \"%s\"\n",line.c_str()) ;
        std::string processed = line ;
        std::string::size_type c = processed.find("#") ;
        if (c != std::string::npos) processed = processed.substr(0,c) ;
        dtkMsg.add(DTKMSG_DEBUG, "iris::SceneGraph::substituteVariable: processing line \"%s\"\n",processed.c_str()) ;

        while (1)
        {
            // find start of variable
            std::string::size_type b = processed.find("$(") ;
            // return if none found
            if (b == std::string::npos) return processed ;
            // find end of variable
            std::string::size_type e = processed.substr(b).find(")") ;
            // return if none found
            if (e == std::string::npos) return processed ;
            // get the variable name- i.e. key
            std::string key = processed.substr(b+2,e-2) ;
            //printf("key = \"%s\"\n",key.c_str()) ;
            // see if the key is in the map
            std::map<std::string, std::string>::iterator pos ;
            pos = _variables.find(key) ;
            // if it is, substitute the value
            if (pos != _variables.end()) processed = processed.substr(0,b) + pos->second + processed.substr(b+e+1) ;
            else
                // ignore it but warn the user
            {
                dtkMsg.add(DTKMSG_WARNING, "iris::SceneGraph::substituteVariable: variable \"%s\" not found for string \"%s\"\n",key.c_str(),line.c_str()) ;
                processed = processed.substr(0,b) + processed.substr(b+e+1) ;
            }
        }
    } ;

    ////////////////////////////////////////////////////////////////////////
    void SceneGraph::addVariable(std::string key, std::string value) 
    { 
        std::map<std::string, std::string>::iterator pos ;

        std::string processed = value ;
        std::string::size_type c = processed.find("#") ;
        if (c != std::string::npos) processed = processed.substr(0,c) ;

        pos = _variables.find(key) ;
        if (pos == _variables.end())
        {
            _variables.insert(std::make_pair(key,processed)) ;
            dtkMsg.add(DTKMSG_DEBUG,"iris::SceneGraph::addVariable: adding variable %s with value \"%s\"\n",key.c_str(),processed.c_str()) ;
        }
        else
        {
            pos->second = processed ;
            dtkMsg.add(DTKMSG_DEBUG,"iris::SceneGraph::addVariable: updating variable %s with value \"%s\"\n",key.c_str(),processed.c_str()) ;
        }
    } ;

    ////////////////////////////////////////////////////////////////////////
    void SceneGraph::delVariable(std::string key) 
    { 
        std::map<std::string, std::string>::iterator pos ;
        _variables.erase(key) ;
    } ;

    ////////////////////////////////////////////////////////////////////////
    void  SceneGraph::listVariables()
    {
        std::map<std::string, std::string>::iterator pos ;
        for (pos = _variables.begin(); pos != _variables.end(); pos++)
        {
            fprintf(stderr,"%s = %s\n",pos->first.c_str(), pos->second.c_str()) ;
        }
    }

    ////////////////////////////////////////////////////////////////////////
    bool  SceneGraph::logLine(const std::string& format, ...)
    {

        if (!_logOn) return false ;

        static unsigned int len = 1024 ;
        char line[len] ;

        va_list args;
        va_start(args, format);
        vsnprintf(line, len, format.c_str(), args) ;
        va_end(args);

        osg::FrameStamp* fs = NULL ;
        if (getViewer()) fs = getViewer()->getFrameStamp() ;
        
        _LogEntry* entry = new _LogEntry ;
        if (fs) entry->frameNumber = fs->getFrameNumber() ;
        else entry->frameNumber = 0 ;
        entry->time =  osg::Timer::instance()->time_s();
        entry->line = line ;
        _logData.push_back(entry) ;

        return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool SceneGraph::setLog(bool onOff)
    {
        // disabled and enabling?
        if (!_logOn && onOff)
        {
            // open file if the first time getting enabled
            if (_logFirstTry)
            {
                _logFirstTry = false ;
                _logFILE = fopen(_logFileName.c_str(), "w") ;
                if (!_logFILE) 
                {
                    dtkMsg.add(DTKMSG_NOTICE, "iris::SceneGraph::setLog can't open log file %s, logging disabled\n",_logFileName.c_str()) ;
                }
                else 
                {
                    dtkMsg.add(DTKMSG_NOTICE, "iris::SceneGraph::setLog logging to file %s\n",_logFileName.c_str()) ;
                    _logOn = true ;
                    logLine("logging enabled") ;
                }
            }
            else
                if (_logFILE)
                {
                    dtkMsg.add(DTKMSG_NOTICE, "iris::SceneGraph::setLog logging enabled\n") ;
                    _logOn = true ;
                    logLine("logging enabled") ;
                }
                else
                {
                    dtkMsg.add(DTKMSG_NOTICE, "iris::SceneGraph::setLog can't enable logging because log file couldn't be opend\n") ;
                }
        }
        else if (_logOn && !onOff)
        {
            dtkMsg.add(DTKMSG_NOTICE, "iris::SceneGraph::setLog logging disabled\n") ;
            logLine("logging disbled") ;
            _logOn = false ;
        }

        return _logOn ;
    }

    ////////////////////////////////////////////////////////////////////////
    // create and init static class variables
    bool SceneGraph::_echo = false ;
    std::map<std::string, osg::ref_ptr<osg::Node> > SceneGraph::_nodeMap ;
#ifdef SCENEGRAPH_NODE_PTR_SET
    std::set<osg::ref_ptr<osg::Node> > SceneGraph::_nodePtrSet ;
#endif
    std::vector<SceneGraph*> SceneGraph::_instances ;
    bool SceneGraph::_instanced = false ;
    int SceneGraph::_useSceneGraphIndex = 0 ;
    int SceneGraph::_useViewerIndex = 0 ;
    osg::Node* SceneGraph::_lastNode = NULL ;

    std::set<int> _ViewerKeyboardMousePollingAndScenegraphUpdate::_keyState ;
    std::set<int> _ViewerKeyboardMousePollingAndScenegraphUpdate::_buttonState ;
    unsigned int _ViewerKeyboardMousePollingAndScenegraphUpdate::_buttonMask = 0 ;
}
