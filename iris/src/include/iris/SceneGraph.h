#ifndef __IRIS_SCENE_GRAPH__
#define __IRIS_SCENE_GRAPH__

#include <dtk.h>
#include <osgGA/GUIEventHandler>
#include <osgViewer/Viewer>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Quat>

#include <iris/Utils.h>
#include <iris/FifoReader.h>
#include <iris/MatrixTransform.h>

namespace iris
{
    /**
       \brief 
       The %SceneGraph class is at the center of IRIS.  The %SceneGraph class is
       currently implemented as a singleton, although hooks are in place to allow
       multiple %SceneGraph objects to exist should the need arise. 
       \n \n The %SceneGraph class is based on the dtkManager class, as it manages
       DTK DSOs written using the Augment class.
       \n \n The %SceneGraph class does the following:
       - creates and manages the IRIS scenegraph
       - loads DSO and model files (including \c iris files) specified by the
       \c IRIS_FILES environment variable
       - creates an osgViewer::Viewer object (unless otherwise specified) and attaches the scenegraph to it.
       Currently only one Viewer object is allowed, but hooks are in
       place to allow multiple Viewer objects to exist per %SceneGraph
       object, should the need arise.
       - realizes the Viewer object, creating graphics windows
       - executes single or continuous graphcs frames as specified
       - reads mouse and keyboard input every graphics frame in an event handler callback
       - reads control commands every graphics frame in the event handler via the fifo named in the
       \c IRIS_CONTROL_FIFO environment variable.  Control commands are parsed and
       processed.
       - cleans up on exit
       
    */


    class SceneGraph : public dtkManager
    {
    public:

	
	/** 
	    \brief creates a %SceneGraph object with an optional set of parameters and an optional associated osgViewer::Viewer object

	    Using this constructor with its defaults is the usual way of creating a %SceneGraph object
	    \param argc and \param argv are currently not used
	    \param createViewer specifies whether to create an osg::Viewer object
	 */
	SceneGraph(int argc = 0, char **argv = NULL, bool createViewer = true) { _init(argc, argv, createViewer); } ;
	/** 
	    \brief creates a %SceneGraph object with an optional associated osgViewer::Viewer object

	    \param createViewer specifies whether to create an osg::Viewer
	    object.  Only one %Scenegraph object with a Viewer should be
	    running at a time, otherwise they'll compete for reading from
	    the fifo.  This can be avoided by changing $IRIS_CONTROL_FIFO
	    for each %SceneGraph object that has Viewers.

	    This method is here to support future develpment
	 */
	SceneGraph(bool createViewer) { _init(0, NULL, createViewer); } ;

	virtual ~SceneGraph() ;

	/**
	   \brief returns a pointer to the indicated %SceneGraph object.

	   \returns a pointer to a new %SceneGraph object if no %SceneGraph object has been created

	   \param i specifies the index of which %SceneGraph object to return.  Currently only one %SceneGraph object is supported, so it must be set to 0, which is the default if not supplied.

	 */
	static SceneGraph* const instance(int i = 0) ;
	int getIndex() { return _index ; } ;
	virtual void frame(double time = USE_REFERENCE_TIME) ;
	virtual void realize();
	virtual int run() { while (!_done) frame() ; return 0 ; } ;
	virtual void setDone(bool done);
	virtual bool done() { return _done ; } ;
	virtual bool isRealized() { return _realized ; } ; 
	virtual int addNewDefaultViewer() ;
	virtual int addViewer(osgViewer::Viewer* viewer) ;
	virtual osgViewer::Viewer* const getViewer(int i = 0) ;
	virtual MatrixTransform* const getSceneNode() { return _scene.get() ; } ;
	virtual MatrixTransform* const getEtherNode() { return _ether.get() ; } ;
	virtual MatrixTransform* const getWorldNode() { return _world.get() ; } ;
	virtual MatrixTransform* const getNavNode() { return _nav.get() ; } ;
	virtual MatrixTransform* const getPivotNode() { return _pivot.get() ; } ;
	virtual MatrixTransform* const getHeadNode() { return _head.get() ; }  ;
	virtual MatrixTransform* const getWandNode() { return _wand.get() ; }  ;
	virtual MatrixTransform* const getClearNode() { return _clear.get() ; }  ;
	virtual bool pollKey(int k) ;
	virtual bool pollButton(int b) ;
	virtual std::vector<int> getPressedKeys() ;
	virtual std::vector<int> getPressedButtons() ;
	virtual unsigned int getButtonMask() ;
	virtual bool control(const std::string& line) ;
	virtual bool control(const std::string& line, const std::vector<std::string>&) ;

	/** 
	    \brief add a line to the log file
	    
	    \param format is a string containing a "printf" compatible
	    format, followed by zero or more parameters to the format
	    string. The formatted line will be added to the log file if
	    logging is enabled.

	    \returns true if line was added to the log

	    Logging is enabled if the environment variable $IRIS_LOG_FILE is
	    set to a file name that can be successfully opened for writing
	    and logging is enabled by the setLog function.

	    The log data are kept in memory and written to $IRIS_LOG_FILE on exit.

	*/
	virtual bool logLine(const std::string& format, ...) ;

	/**
	   \brief enable or disable logging. Logging is initially disabled.

	   \returns true if logging was able to be enabled or is already enabled, false otherwise
	 */
	virtual bool setLog(bool onOff) ;

	virtual bool getLog() { return _logOn ; } ;

	/**
	   \brief specify the log file name. If not specified the default
	   name /tmp/iris-$USER.log will be used.
	*/
	virtual void setLogFile(const std::string& filename) { _logFileName = filename ; } ;
	virtual std::string getLogFile() { return _logFileName ; } ;

	//get vector of pids exec'd by the EXEC control command
	virtual std::vector<int> getPids() { return _childPids ; } ;

	// jump mode is a way of specifying an alternative viewing position and orientation
	// the default is the head position and orientation
	virtual void setJump(bool jump) { _jump = jump ; } ;
	virtual bool getJump() { return _jump ; } ;
	// where to jump to
	virtual void setJumpPosition(osg::Vec3 jumpPosition) { _jumpPosition = jumpPosition ; } ;
	virtual osg::Vec3 getJumpPosition() { return _jumpPosition ; } ;
	virtual void setJumpAttitude(osg::Quat jumpAttitude) { _jumpAttitude = jumpAttitude ; } ;
	virtual osg::Quat getJumpAttitude() { return _jumpAttitude ; } ;

	// returns the position and attitude of the viewer- either the position from the head node or the jump position
	virtual osg::Vec3 getViewerPosition() { if (_jump) return _jumpPosition ; else return _head->getPosition() ; } ;
	virtual osg::Quat getViewerAttitude() { if (_jump) return _jumpAttitude ; else return _head->getAttitude() ; } ;

	// the number of meters per normalized display unit
	// only used for immersive environments
	// normalized units go from -1 to 1
	// for ex: if you have screen 10' wide, half of that is 5', or 1.524 meters
	//         so call setMeters(1.524)
	virtual double getMeters() { return _meters ; } ;
	virtual void setMeters(double meters) { _meters = meters ; } ;
    
	// units are meters
	// distance between the eyes
	virtual double getInterocular() { return _interocular ; } ;
	virtual void setInterocular(double interocular) { _interocular = interocular ; } ;
	
	// units are normalized- 1 is at screen
	// distance until objects merge- used in desktop stereo
	virtual double getFusion() { return _fusion ; } ;
	virtual void setFusion(double fusion) { _fusion = fusion ; } ;
	
	// convert meters to normalized display units and vice versa
	virtual double metersToDisplay(double m) { return m/_meters ; } ;
	virtual double displayToMeters(double d) { return d*_meters ; } ;

	// for future use- these can be set and read so DSOs and callbacks know what
	// SceneGraph and Viewer objects to use
	bool setUseSceneGraphIndex(int i) ;
	bool setUseViewerIndex(int i) ;
	int getUseSceneGraphIndex() { return _useSceneGraphIndex ; } ;
	int getUseViewerIndex() { return _useViewerIndex ; } ;

	// to support the SET and UNSET control commands and variable substition
	void initEnvVariables() ;
	std::string substituteVariable(const std::string& line) ;
	void addVariable(std::string key, std::string value="") ;
	void delVariable(std::string key) ;
	void listVariables() ;

	// this is done once for all cameras in the Pane::realize() method
	// to change them later, or per-camera, write code to change for each camera
	// to change the color outside the viewport, change the color of the graphics context
	// this is set to black in Window::realize()
	virtual void setClearColor(osg::Vec4 clearColor) { _clearColor = clearColor ; } ;
	virtual osg::Vec4 getClearColor() { return _clearColor ; } ;

	// utilities related to nodes by name
	virtual osg::Node* findNode(const std::string& name) ;
	virtual bool getReferenceCount(const std::string& name) ;
	virtual bool addNode(const std::string& name, osg::Node* const node) ;
	virtual bool removeNode(const std::string& name) ;
	virtual bool mapNode(const std::string& name) ;
	virtual std::map<std::string, osg::ref_ptr<osg::Node> >* getNodeMap() { return &_nodeMap ; } ;

	// control commands to modify a node:
	virtual bool examine(const std::string& nodeName = "world") ;
	virtual bool bound(const std::string& nodeName = "world", double radius = 1.f) ;
	virtual bool center(const std::string& nodeName = "world", const osg::Vec3& center = osg::Vec3(0.f, 0.f, 0.f)) ;
	virtual bool normals(const std::string& nodeName = "world") ;
	virtual bool optimize(const std::string& nodeName = "world", const std::string& optimizations="") ;
	virtual bool staticDataVariance(const std::string& nodeName = "world") ;
	virtual bool copyPrimitives(const std::string& nodeName = "world") ;
	virtual bool noStateSets(const std::string& nodeName = "world") ;
	virtual bool noLighting(const std::string& nodeName = "world") ;
	virtual bool exec(const std::string& command, bool waitChild=false) ;
	virtual bool kill(const std::string& pid) ;
	virtual bool background(float r, float g, float b) ;
#if 0
	virtual bool print(const std::string& nodeName, const std::string& fileName) ;
#endif
	virtual bool dump(const std::string& nodeName, const std::string& fileName) ;
	virtual bool clipnode(const std::vector<std::string>& vec) ;
	virtual bool lightnode(const std::vector<std::string>& vec) ;
	virtual bool lightmodel(const std::vector<std::string>& vec) ;
	virtual bool billboard(const std::vector<std::string>& vec) ;
	virtual bool nodemask(const std::string& nodeName, const std::string& state) ;
	virtual bool polygonMode(const std::string& nodeName = "world", const std::string& modeStr = "fill", const std::string& faceStr = "front_and_back" ) ;
	
	// get a pointer to the fifo object
	virtual FifoReader* getFifo() { return &_fifo ; } ;

	// add a PID to the list of PIDs to kill on exit
	virtual void addPID(pid_t pid) ;

	// puts when command in appropriate set
	virtual bool recordWhenCommand(const std::string& line, const std::vector<std::string>& vec) ;
	// runs the commands in the set as appropriate
	virtual void processBeforeRealizeWhenCommands() ;
	virtual void processAfterRealizeWhenCommands() ;
	virtual void processBeforeFrameWhenCommands() ;
	virtual void processAfterFrameWhenCommands() ;
	virtual void processAfterTimeWhenCommands() ;
	virtual void processBeforeCleanupWhenCommands() ;
	virtual void processAfterCleanupWhenCommands() ;

	// make it public so the iris file loader can access it
	virtual bool getEcho() { return _echo ; } ;

	// for STARTMODEL/ENDMODEL
	virtual std::string getStartModelDataFileName() { return _inModelDataFileName ; } ;
	virtual std::ofstream* getStartModelDataFileStream() { return _inModelDataFileStream ; } ;
	virtual std::string getStartModelDataNodeName() { return _inModelDataNodeName ; } ;
	virtual void resetStartModel() { _inModelDataFileName = "" ;  _inModelDataFileStream = NULL ; _inModelDataNodeName = "" ; } ;

    protected:
	virtual void _init(int argc, char **argv, bool createViewer) ;
	bool _realized ;
	bool _done ;
	bool _createViewer ;
	std::vector<int> _childPids; ;
	// true if any SceneGraph object has been created
	static bool _instanced ;
	// all the SceneGraph objects
	static std::vector<SceneGraph*> _instances ;
	// the index into the _instance vector for this SceneGraph object
	int _index ;
	// vector of these, so later it can support multiple viewers
	std::vector<osg::ref_ptr<osgViewer::Viewer> > _viewers ;
	static int _useSceneGraphIndex ;
	static int _useViewerIndex ;
	osg::ref_ptr<MatrixTransform> _scene ;
	osg::ref_ptr<MatrixTransform> _ether ;
	osg::ref_ptr<MatrixTransform> _world ;
	osg::ref_ptr<MatrixTransform> _nav ;
	osg::ref_ptr<MatrixTransform> _pivot ;
	osg::ref_ptr<MatrixTransform> _head ;
	osg::ref_ptr<MatrixTransform> _wand ;
	osg::ref_ptr<MatrixTransform> _clear ;
	virtual void _listNodes() ;

	FifoReader _fifo ;
	osg::Vec3 _headPosition ;
	osg::Quat _headAttitude ;
	double _meters ;
	double _interocular ;
	double _fusion ;
	bool _jump ;
	osg::Vec3 _jumpPosition ;
	osg::Quat _jumpAttitude ;
	osg::Vec4 _clearColor ;
	std::list<std::string> _beforeRealize ;
	std::list<std::string> _afterRealize ;
	std::list<std::pair<std::string,int> > _beforeFrameN ;
	std::list<std::pair<std::string,int> > _afterFrameN ;
	std::list<std::pair<std::string,double> > _afterTimeN ;
	std::list<std::string> _beforeCleanup ;
	std::list<std::string> _afterCleanup ;
	virtual bool _setEcho(const std::string& echo) ;
	virtual bool _setLog(const std::string& log) ;
	static bool _echo ;
	class mapNodeVisitor ;
	static std::map<std::string, osg::ref_ptr<osg::Node> > _nodeMap ;
	// all this nodePtrSet stuff is so we can see if a node has already
	// been inserted in the node map without having to traverse the entire map
#define SCENEGRAPH_NODE_PTR_SET
#ifdef SCENEGRAPH_NODE_PTR_SET
	static std::set<osg::ref_ptr<osg::Node> > _nodePtrSet ;
#endif
	static osg::Node* _lastNode ;

	// for startModelData/endModelData
	std::string _inModelDataFileName ;
	std::ofstream* _inModelDataFileStream ;
	std::string _inModelDataNodeName ;

	// old sge commands
	virtual bool _addChild(const std::string& nodeName, const std::string& groupName, const std::string& index="") ;
	virtual bool _dcs(bool dynamic, bool quaternion, const std::vector<std::string>& vec) ;
	virtual bool _matrix(const std::vector<std::string>& vec) ;
	virtual bool _group(const std::string& nodeName) ;
	virtual bool _noclipnode(const std::string& nodeName) ;
	virtual bool _nolightnode(const std::string& nodeName) ;
	virtual bool _load(const std::string& nodeName, const std::string& fileName) ;
	virtual bool _load(const std::string& fileName) ;
	virtual bool _copyState(const std::string& fromNodeName, const std::string& toNodeName) ;
	virtual bool _mergeState(const std::string& fromNodeName, const std::string& toNodeName) ;
	virtual bool _cloneNode(const std::string& nodeName, const std::string& copyName, osg::CopyOp::CopyFlags op=osg::CopyOp::DEEP_COPY_ALL) ;
	virtual bool _removeChild(const std::string& nodeName, const std::string& groupName) ;
	virtual bool _unload(const std::string& nodeName) ;
	virtual bool _mat(const std::string& nodeName, bool dynamic, osg::Vec3 pos = osg::Vec3(0.f, 0.f, 0.f), osg::Quat attitude = osg::Quat(0.f, 0.f, 0.f, 1.f), osg::Vec3 scale = osg::Vec3(1.f, 1.f, 1.f)) ;
	virtual bool _shmMatrix(const std::vector<std::string>& vec) ;
	virtual bool _query(const std::string& line, const std::vector<std::string>& vec) ;
	std::map <std::string, FifoReader*> _queryFifos ;
        virtual bool _shaderProg (const std::vector<std::string> & vec);
        virtual bool _uniformSyntaxA (std::vector<std::string> vec);
        virtual bool _uniformSyntaxB (std::vector<std::string> vec);
        virtual bool _uniform (std::vector<std::string> vec);
        virtual bool _shaderstoragebuffer (std::vector<std::string> vec);
        virtual bool _txtr (const std::vector<std::string> & vec);
        virtual bool _txtradd (const std::vector<std::string> & vec);
        virtual bool _txtrc (const std::vector<std::string> & vec);       
        // add also _txtrremove
        virtual bool _text (std::vector<std::string> vec);
        virtual bool _cullingOnOff (std::vector<std::string> vec);
	virtual bool _startModelData(std::string nodeName, std::string extension) ;
	virtual bool _endModelData() ;
	virtual FifoReader* _getFifo(std::string name) ;
	virtual int _deleteFifos() ;
	virtual bool _map(const std::string& nodeName) ;
        virtual bool _switch (std::vector<std::string> vec) ;

	// to support the SET and USET control commands and variable substition
	std::map<std::string, std::string> _variables ;

    private:
	// logging support
	std::string _logFileName ;
	FILE* _logFILE ;
	struct _LogEntry
	{
	    double time ;
	    int frameNumber ;
	    std::string line ;
	} ;
	bool _logOn ;
	bool _logFirstTry ;
	std::vector<_LogEntry*> _logData ;

    // control command statistics
    std::map<std::string, size_t> _ccStats;

	// keep texture references in _textures (including filenames)
	static std::map<std::string, osg::ref_ptr<osg::Texture> > _textures;
    } ;
}

#endif
