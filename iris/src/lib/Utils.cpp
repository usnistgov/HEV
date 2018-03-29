#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wordexp.h>
#include <sstream>
#include <execinfo.h>

#include <dtk.h>

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/ComputeBoundsVisitor>
#include <osgUtil/Optimizer>
#include <osgUtil/Statistics>

#include <iris/SceneGraph.h>
#include <iris/Augment.h>
#include <iris/Utils.h>
#include <iris/ConvertNodeVisitor.h>

extern const char *__progname;

static double irisTime = .001 ;

namespace iris
{
    ////////////////////////////////////////////////////////////////////////
    osg::BoundingBox GetBoundingBox(osg::Group* group)
    {
	osg::BoundingBox bb ;
	for (unsigned int i=0; i<group->getNumChildren(); i++)
	{
	    osg::Node* cnode = group->getChild(i) ;
	    osg::ComputeBoundsVisitor  cbv;
	    cbv.setNodeMaskOverride(~0) ;
	    cnode->accept(cbv);
	    osg::BoundingBox cbvbb = cbv.getBoundingBox() ;
	    bb.expandBy(cbvbb) ;
	}
	return bb ;
    }
    ////////////////////////////////////////////////////////////////////////
    bool SetSleep(double t)
    {
	if (t<0.0)
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::SetSleep: negative time ignored\n") ;
	    return false ;
	}
	irisTime = t ;
	if (getenv("IRIS_SLEEP")) unsetenv("IRIS_SLEEP") ;
    }

    ////////////////////////////////////////////////////////////////////////
    double GetSleep()
    {
	char *ts = getenv("IRIS_SLEEP") ;
	if (ts)
	{
	    double t ;
	    if (StringToDouble(ts,&t) && t>= 0.0)
	    {
		SetSleep(t) ;
	    }
	    else
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::GetSleep: $IRIS_SLEEP not set to a valid non-negative double- returning default\n") ;
	    }
	}
	return irisTime ;
    }

    ////////////////////////////////////////////////////////////////////////
    unsigned long GetUsleep()
    {
	unsigned long us = static_cast<unsigned long>(GetSleep()*1000000.0) ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool GetLine(std::string* line, double t, bool* to, std::istream& stream)
    {
	
	fd_set rfds;
	struct timespec tv;
	int retval;
      
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);
      
	// no wait, just poll
	tv.tv_sec = long(t);
	tv.tv_nsec = long(1000000000.f*t) - 1000000000*tv.tv_sec;
	dtkMsg.add(DTKMSG_DEBUG, "iris::GetLine: waiting up to %ld (%f) seconds, %ld nanoseconds for stream to have data\n",tv.tv_sec,t,tv.tv_nsec) ;
	retval = pselect(1, &rfds, NULL, NULL, &tv, NULL);
      
	if (retval != 0)
	{
	    dtkMsg.add(DTKMSG_DEBUG, "iris::GetLine: reading from stream\n") ;
	    if (to) *to = false ;
	    if (std::getline(stream, *line)) return true ;
	    else return false ;
	}
	else
	{
	    if (to) *to = true ;
	    return false ;
	}
    }

    ////////////////////////////////////////////////////////////////////////
    bool Signal(sighandler_t handler)
    {
	// end with 0
	int defaultSignals[] = { SIGHUP, SIGINT, SIGQUIT, SIGABRT, SIGSEGV, SIGTERM, 0 } ;
	bool ret = true ;
	for (int i=0; ; i++)
	{
	    if (defaultSignals[i] == 0) break ;
	    sighandler_t prev = signal(defaultSignals[i], handler); 
	    if (handler != SIG_DFL)
	    {
		    if (prev == SIG_ERR)
		    {
		        dtkMsg.add(DTKMSG_ERROR,"iris::Signal: error setting signal %d\n",defaultSignals[i]);
		        ret = false ;
		    }
		    if (prev != SIG_DFL) dtkMsg.add(DTKMSG_WARN,"iris::Signal signal %d was already set, it just got clobbered\n",defaultSignals[i]);
	    }
	}
	return ret ;
    }

    ////////////////////////////////////////////////////////////////////////
    void DumpBacktrace()
    {
        static void* symbols[1024];
        int nsymbols = backtrace(symbols, 1024);

        char** symbolsStr = backtrace_symbols(symbols, nsymbols);
        if (!symbolsStr) {
            write(STDERR_FILENO, "iris::DumpBacktrace:\n", 21);
            backtrace_symbols_fd(symbols, nsymbols, STDERR_FILENO);
        } else {
            write(STDERR_FILENO, "iris::DumpBacktrace:\n", 21);
            for (int i=0; i<nsymbols; ++i) {
                write(STDERR_FILENO, symbolsStr[i], strlen(symbolsStr[i]));
                write(STDERR_FILENO, "\n", 1);
            }
            free(symbolsStr);
        }
    }

    ////////////////////////////////////////////////////////////////////////
    void RemoveEventHandler(osgGA::GUIEventHandler* eventHandler)
    {
	osgViewer::View::EventHandlers _eventHandlers = SceneGraph::instance()->getViewer()->getEventHandlers() ;
	osgViewer::View::EventHandlers::iterator it =
	    std::find(_eventHandlers.begin(), _eventHandlers.end(), eventHandler);
	if ( it != _eventHandlers.end() ) _eventHandlers.erase( it );
    }
	
    ////////////////////////////////////////////////////////////////////////
    osg::Matrix CoordToMatrix(const dtkCoord& c)
    {
	osg::Vec3d t(c.x, c.y, c.z) ;
	osg::Quat q = EulerToQuat(c.h, c.p, c.r) ;
	
	osg::Matrix m ;
	m.makeTranslate(t) ;
	m.preMultRotate(q) ;
	return m ;
    }

    ////////////////////////////////////////////////////////////////////////
    osg::Matrix CoordToMatrix(const double x, const double y, const double z, const double h, const double p, const double r)
    {
	osg::Vec3d t(x, y, z) ;
	osg::Quat q = EulerToQuat(h, p, r) ;
	
	osg::Matrix m ;
	m.makeTranslate(t) ;
	m.preMultRotate(q) ;
	return m ;
    }


    ////////////////////////////////////////////////////////////////////////
    dtkCoord MatrixToCoord(const osg::Matrix& m)
    {
	osg::Vec3d t ;
	osg::Quat q  ;
	iris::Decompose (m, &t, &q) ;
	
	dtkCoord w ;
	w.x = t.x() ; w.y = t.y() ; w.z = t.z() ;
	double h, p, r ;
	QuatToEuler(q, &h, &p, &r) ;
	w.h = h ; w.p = p ; w.r = r ;
	
	return w ;
    }
    
    ////////////////////////////////////////////////////////////////////////
    void MatrixToCoord(const osg::Matrix& mat,  double* x, double* y, double* z, double* h, double* p, double* r)
    {
	osg::Vec3d t ;
	osg::Quat q  ;
	iris::Decompose (mat, &t, &q) ;
	
	*x = t.x() ; *y = t.y() ; *z = t.z() ;
	QuatToEuler(q, h, p, r) ;				
    
	return ;

    }

    ////////////////////////////////////////////////////////////////////////
    osg::Quat EulerToQuat(double h, double p, double r)
    {
	osg::Quat qh(DegreesToRadians(h), osg::Vec3d(0.f,0.f,1.f)) ;
	osg::Quat qp(DegreesToRadians(p), osg::Vec3d(1.f,0.f,0.f)) ;
	osg::Quat qr(DegreesToRadians(r), osg::Vec3d(0.f,1.f,0.f)) ;
	    
	return qr*qp*qh ;
    }
	
    ////////////////////////////////////////////////////////////////////////
    void QuatToEuler(const osg::Quat& q, double* h, double* p, double* r)
    {
	osg::Matrix mat(q) ;

	// We choose to make pitch (Rx) between -pi/2 and +pi/2
	    
	/* I would some day like to know why this seems to work when pitch
	 * is close to +/- pi/2.  */
	    
	double p_angle = asinf(mat(1,2));
	*p = RadiansToDegrees(p_angle);
	double cos_p = cosf(p_angle);

	if(cos_p > 1.0e-6f || cos_p < -1.0e-6f)
	{
	    //            atan (  sin,      cos)
	    *r = RadiansToDegrees(atan2f(-mat(0,2)/cos_p, mat(2,2)/cos_p));
	    *h = RadiansToDegrees(atan2f(-mat(1,0)/cos_p, mat(1,1)/cos_p));
	}
	else // heading and roll have the same affect if pitch near +/- pi.
	{ /* Gimball lock has occurred */

	    *h = 0.0f;
	    *r = RadiansToDegrees(atan2f(mat(2,0), mat(0,0)));
	}
	return ;
    }

    ////////////////////////////////////////////////////////////////////////
    void MatrixToEuler(const osg::Matrix& mat, double* h, double* p, double* r)
    {
	osg::Quat q ;

	osg::Vec3d s = mat.getScale() ;
	if (s.x() != 1.0 || 
	    s.y() != 1.0 || 
	    s.z() != 1.0)
	{
	    iris::Decompose (mat, NULL, &q) ;
	}
	else
	{
	    q = mat.getRotate() ;
	}
	
	QuatToEuler(q, h, p, r) ;
    }

    ////////////////////////////////////////////////////////////////////////
    void PrintMatrix(const osg::Matrix& mat, FILE* file)
    {
	for(int i=0;i<4;i++) fprintf(file,"%+8.8f  %+8.8f  %+8.8f  %+8.8f\n", mat(i,0), mat(i,1), mat(i,2), mat(i,3));
	
#if 0
	double x,y,z, h,p,r;
	
	rotateHPR(&h, &p, &r);
	translate(&x, &y, &z);
	
	fprintf(file, "\n"
		"xyz = %+3.3f %+3.3f %+3.3f"
		"  hpr= %+3.3f %+3.3f %+3.3f"
		"  scale= %+3.3f\n",
		x,y,z,h,p,r,scale());
#endif
	fprintf(file, "*********************************************\n");
    }

    ////////////////////////////////////////////////////////////////////////
    void PrintMatrixControlCommand(const osg::Matrix& mat, const std::string& nodeName, FILE* file)
    {
	fprintf(file,"MATRIX %s",nodeName.c_str()) ;
	for(int i=0;i<4;i++) fprintf(file,"  %s %s %s %s", 
				     iris::DoubleToString(mat(i,0)).c_str(), 
				     iris::DoubleToString(mat(i,1)).c_str(), 
				     iris::DoubleToString(mat(i,2)).c_str(), 
				     iris::DoubleToString(mat(i,3)).c_str());
	fprintf(file,"\n") ;
	fflush(file) ;
    }

    ////////////////////////////////////////////////////////////////////////
    /*
      pass a string and break it into a vector of words, sort of like a shell
      does.  stuff within "" or '' is treated as one parameter. the \ character
      will escape the next character.  An unescaped # throws away the rest of
      the line.
  
      does not do error checking for terminating quotes- an eol or comment
      terminates the quote.
    */

    static void _parseString_addChar(char c, std::vector<std::string>* vec, int* inSpace)
    {
	if (*inSpace) 
	{
	    // add one more array element if first time, or exiting a "space"
	    std::string hold ;
	    vec->push_back(hold) ;
	    *inSpace = 0 ;
	}
  
	vec->back() += c ;
    }

    std::vector<std::string> ParseString(const std::string& line)
    {
  
	std::vector<std::string> vec ;
  
	int inSpace = 1 ;
	int inDoubleQuote = 0 ;
	int inSingleQuote = 0 ;
	int prevEscape = 0 ;
  
	for (unsigned int i=0; i<line.length(); i++) {
	    char c = line[i] ;
	    if (c == '\n') continue;
	    if  (c == '\\') prevEscape = 1 ;
	    else 
	    {
		if (prevEscape) 
		{
		    // if previous character was an escape, just add to arg
		    _parseString_addChar(c, &vec, &inSpace) ;
		} 
		else if (c == '"') 
		{
		    if (inSingleQuote) 
		    {
			_parseString_addChar(c, &vec, &inSpace) ;
		    } 
		    else 
			if (inDoubleQuote) 
			{
			    inDoubleQuote = 0 ;
			} 
			else 
			{
			    inDoubleQuote = 1 ;
			}
		} 
		else if  (c == '\'') 
		{
		    if (inDoubleQuote) _parseString_addChar(c, &vec, &inSpace) ;
		    else 
			if (inSingleQuote) 
			{
			    inSingleQuote = 0 ;
			} 
			else 
			{
			    inSingleQuote = 1 ;
			}
	
		} 
		else if  (c == ' ' || c == '	' ) 
		{
		    if (prevEscape || inSingleQuote || inDoubleQuote) 
		    {
			_parseString_addChar(c, &vec, &inSpace) ;
		    } 
		    else if (!inSpace) 
		    {
			inSpace = 1 ;
		    }
	
		} 
		else if (!prevEscape && c == '#') 
		{
		    return vec ;
	
		} 
		else 
		{
		    _parseString_addChar(c, &vec,  &inSpace) ;
		}
		prevEscape = 0 ;
	    }
	}
	return vec ;
    }
    ////////////////////////////////////////////////////////////////////////
    //  looks for the supplied string in the keyword string
    //  
    //  returns true if supplied matchs the first minLength characters of
    //  keyword, or all of the characters of supplied match keyword if minLength
    //  is zero or omitted.

    bool IsSubstring(const std::string& keyword, const std::string& supplied, 
		     const unsigned int minLength, bool matchCase)
    {
	unsigned int m = minLength ;
	if (m==0) m = supplied.length() ;
	if (m<supplied.length()) m = supplied.length() ;
	if (m==0) m = keyword.size() ;
	if (keyword.size() < m || supplied.length() < m) return false ;
	if (!matchCase && strncasecmp(keyword.c_str(), supplied.c_str(), m)) return false ;
	if (matchCase && strncmp(keyword.c_str(), supplied.c_str(), m)) return false ;
	else return true ;
    }

#if 0
    ////////////////////////////////////////////////////////////////////////
    osg::Matrix GetMatrix(const osg::Transform* t) 
    {
	osg::NodeVisitor nv = osg::NodeVisitor() ; 
	osg::Matrix mat ;
	t->computeLocalToWorldMatrix(mat,&nv);
	return mat ;
    }
#endif

    ////////////////////////////////////////////////////////////////////////
    osg::Matrix GetMatrix(const osg::Group* parent, const osg::Node* child)
    {
	osg::Matrix mat ;
	std::vector<osg::Matrix> matVector = child->getWorldMatrices(parent) ;
	if (matVector.size() == 0)
	{
	    dtkMsg.add(DTKMSG_ERROR,"iris::GetMatrix: no path from child to parent\n") ;
	}
	else if (matVector.size() > 1)
	{
	    dtkMsg.add(DTKMSG_ERROR,"iris::GetMatrix: %d paths from child to parent\n",matVector.size()) ;

	}
	else mat = matVector[0] ;
	
	return mat ;
    }

    ////////////////////////////////////////////////////////////////////////
    // helper class for FindNode
    class _findNodeVisitor : public osg::NodeVisitor
    {
    public:
	_findNodeVisitor(const std::string& name, const bool useNodeName, const osg::NodeVisitor::TraversalMode travMode) : 
	    osg::NodeVisitor(travMode), 
	    _name(name), _useNodeName(useNodeName) {} ;
	void apply(osg::Node& node)
	{
	    if((_useNodeName && node.getName() == _name) || !_useNodeName) _nodes.insert(&node) ;
	    traverse(node) ;
	}
	std::set<osg::Node*> getNodes() { return _nodes ; } ;
    protected:
	std::string _name;
	bool _useNodeName;
	std::set<osg::Node*> _nodes ;
    } ;
    
    std::set<osg::Node*> FindNode(const std::string& name, const bool useNodeName, const osg::NodeVisitor::TraversalMode travMode)
    {
	std::set<osg::Node*> _nodes ;
	_findNodeVisitor fnv(name, useNodeName, travMode) ;
	if (SceneGraph::instance())
	{
	    fnv.setNodeMaskOverride(~0) ;
	    fnv.apply(*SceneGraph::instance()->getSceneNode()) ;
	}
	return fnv.getNodes() ;
    } ;

    ////////////////////////////////////////////////////////////////////////
    std::string DoubleToString(double n)
    {
#if 1
	char s[32] ;
	sprintf(s,"%.17g",n) ;
	return std::string(s) ;
#else
	std::stringstream foo ;
	std::string s ;
	foo << n ;
	foo >> s;
	return s ;
#endif
    }

    ////////////////////////////////////////////////////////////////////////
    std::string FloatToString(float n)
    {
#if 1
	char s[32] ;
	sprintf(s,"%.9g",n) ;
	return std::string(s) ;
#else
	std::stringstream foo ;
	std::string s ;
	foo << n ;
	foo >> s;
	return s ;
#endif
    }

    ////////////////////////////////////////////////////////////////////////
    std::string LongToString(long n)
    {
	std::stringstream foo ;
	std::string s ;
	foo << n ;
	foo >> s;
	return s ;
    }

    ////////////////////////////////////////////////////////////////////////
    std::string IntToString(int n)
    {
	std::stringstream foo ;
	std::string s ;
	foo << n ;
	foo >> s;
	return s ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool StringToDouble(const std::string& line, double* d) 
    {
	char* e ;
	*d = strtod(line.c_str(), &e) ;
	if (e[0] == 0) return true ;
	else return false ;
    } ;

    ////////////////////////////////////////////////////////////////////////
    bool StringToFloat(const std::string& line, float* d) 
    {
	char* e ;
	*d = strtof(line.c_str(), &e) ;
	if (e[0] == 0) return true ;
	else return false ;
    } ;

    ////////////////////////////////////////////////////////////////////////
    bool StringToLong(const std::string& line, long* l) 
    {
	char* e ;
	*l = strtol(line.c_str(), &e, 10) ;
	if (e[0] == 0) return true ;
	else return false ;
    } ;

    ////////////////////////////////////////////////////////////////////////
    bool StringToInt(const std::string& line, int* i) 
    {
	char* e ;
	*i = int(strtol(line.c_str(), &e, 10)) ;
	if (e[0] == 0) return true ;
	else return false ;
    } ;

    ////////////////////////////////////////////////////////////////////////
    bool StringToUInt(const std::string& line, unsigned int* i) 
    {
	char* e ;
	int ii;
	ii = int(strtol(line.c_str(), &e, 10)) ;

	if ( (e[0] == 0) )
	{
	    if (ii >= 0) 
	    {
		*i = (unsigned int) ii;
		return true;
	    }
	    else
	    {
		return false;
	    }
	}

	return false ;

    } ;

    ////////////////////////////////////////////////////////////////////////
    double DegreesToRadians(double x) { return x*M_PI/180.0 ; } ;
    double RadiansToDegrees(double x) { return x*180.0/M_PI ; } ;

    ////////////////////////////////////////////////////////////////////////
    bool OnOff(const std::string& str, bool* onOff)
    {
	if (IsSubstring("on",str,2) || IsSubstring("true",str,3) || IsSubstring("yes",str,3) || IsSubstring("enable",str,3) || IsSubstring("active",str,3) || str=="1") 
	{
	    *onOff = true ;
	}
	else if (IsSubstring("off",str,3) || IsSubstring("false",str,3) || IsSubstring("no",str,2) || IsSubstring("disable",str,3) || IsSubstring("inactive",str,3) || str=="0")
	{
	    *onOff = false ;
	}
	else
	{
	    dtkMsg.add(DTKMSG_DEBUG, "iris::OnOff:: invalid on/off value\n") ;
	    return false ;
	}
	return true ;
    }
    
    ////////////////////////////////////////////////////////////////////////
    static ::Window GetWindowByNameTop(::Display* dpy, const ::Window top, const char* name)
    {
	::Window* children, dummy;
	unsigned int nchildren;
	int i;
	::Window w=0;
	char* window_name;
	
	if (XFetchName(dpy, top, &window_name) && !strcmp(window_name, name))
	    return(top);
	
	if (!XQueryTree(dpy, top, &dummy, &dummy, &children, &nchildren))
	    return(0);
	
	for (i=0; i<nchildren; i++) {
	    w = GetWindowByNameTop(dpy, children[i], name);
	    if (w)
		break;
	}
	if (children) XFree ((char* )children);
	return(w);
    }
    
    ::Window GetWindowByName(::Display* dpy, const char* name)
    {
	return GetWindowByNameTop(dpy, XDefaultRootWindow(dpy), name) ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool LoadFile(const std::string& name, osg::Group* group, bool setName)
    {
	// does file have a "." in it?  if so, load as a model file

	// unadvertised special- if file name is "NULL" skip it
	if (name == "NULL") return true ;

	if (name.find(".") != std::string::npos)
	{
	    osg::Node* childNode = osgDB::readNodeFile(name) ;
	    if (group && childNode)
	    {
		group->addChild(childNode) ;
	    }
	    if (childNode)
	    {
		if (childNode->getName() == "" && setName)
		{
		    dtkMsg.add(DTKMSG_INFO,"iris::LoadFile: setting node name of file %s to %s\n",name.c_str(),name.c_str()) ;
		    childNode->setName(name) ;
		}
	    }
	    else
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::LoadFile: unable to load model file \"%s\"\n",name.c_str()) ;
		return false ;
	    }
	}
	else
	{
	    Augment* dso = Augment::getOrLoadDSO(name.c_str()) ;
	    if (!dso) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::LoadFile: unable to load DSO file \"%s\"\n",name.c_str()) ;
		return false ;
	    }
	}
	
	return true ;
    }

    bool LoadFile(ArgumentParser* args, osg::Group* group, bool setName)
    {
	bool ret = true ;
	// skip args[0] as it's the executable name
	for (unsigned int i=1; i<args->argc(); i++)
	{
	    if (!LoadFile((*args)[i], group, setName)) ret = false ;
	    else
	    {
		args->remove(i) ;
		i-- ;
	    }
	}
	return ret ;
    }

    bool LoadFile(const std::vector<std::string>& names, osg::Group* group, bool setName) 
    {
	bool ret = true ;
	for (unsigned int i=0; i<names.size(); i++)
	{
	    if (!LoadFile(names[i], group, setName)) ret = false ;
	}
	return ret ;
    }

    bool LoadFile(int argc, char** argv, osg::Group* group, bool setName)
    {
	bool ret = true ;
	// skip args[0] as it's the executable name
	for (unsigned int i=1; i<argc; i++)
	{
	    if (!LoadFile(argv[i], group, setName)) ret = false ;
	}
	return ret ;
    }

    ////////////////////////////////////////////////////////////////////////
    std::string ChopFirstWord(std::string* line)
    {
	std::string firstWord ;

	// toss leading spaces
	while ((*line)[0] == ' ') line->erase(0,1) ;

	// find first blank after first word
	std::string::size_type pos ;
	pos = line->find(" ") ;
	if (pos != std::string::npos) 
	{
	    firstWord = line->substr(0,pos) ;

	    // toss the first word
	    line->erase(0,pos) ;
	
	    // toss trailing spaces
	    while ((*line)[0] == ' ') line->erase(0,1) ;
	}
	else 
	{
	    // all you have is the first word!
	    firstWord = *line ;
	    line->clear() ;
	}

	return firstWord ;
    }

    ////////////////////////////////////////////////////////////////////////
    std::string ChopFirstWord(const std::string& line)
    {
	std::string ret = line ;
	ChopFirstWord(&ret) ;
	return ret ;
    }


    ////////////////////////////////////////////////////////////////////////
    char *GetTempFileName(const char *suffix, const char *dirname, const char *basename)
    {

	// find a dir
	struct stat buf ;
	std::string tempDir ;
	if (dirname)
	{
	    tempDir = dirname ;
	    if (stat(tempDir.c_str(), &buf))
	    {
		return NULL;
	    }
	}
	else
	{
	    tempDir = "/dev/shm" ;
	    if (stat(tempDir.c_str(), &buf))
	    {
		tempDir = "/tmp" ;
		if (stat(tempDir.c_str(), &buf))
		{
		    tempDir = "/var/tmp" ;
		    if (stat(tempDir.c_str(), &buf))
		    {
			char *home = getenv("HOME") ;
			if (home) 
			{
			    tempDir = home ;
			}
			else
			{
			    tempDir = "." ;
			}
			if (stat(tempDir.c_str(), &buf))
			{
			    return NULL;
			}
		    
		    }
		}
	    }
	}

	// try and find a unique name
	std::stringstream foo ;
	std::string tempFile ;
	std::string tempFullFile ;
	foo << basename << "-" << getuid() << "-" << getpid() << "-" << time(NULL) ;
	foo >> tempFile ;

	if (suffix) {
	    tempFullFile = tempDir + "/" + tempFile + "." + suffix ;
	}
	else
	{
	    tempFullFile = tempDir + "/" + tempFile ;
	}
	
	if (stat(tempFullFile.c_str(), &buf))
	{
	    return strdup(tempFullFile.c_str()) ;
	}
	else
	{
	    for (int foo=0; foo<1000000; foo++)
	    {
		std::string ugh = IntToString(foo) ;
		if (suffix) 
		{
		    tempFullFile = tempDir + "/" + tempFile + ugh + "." + suffix ;
		}
		else
		{
		    tempFullFile = tempDir + "/" + tempFile + ugh ;
		}

		if (stat(tempFullFile.c_str(), &buf))
		{
		    return strdup(tempFullFile.c_str()) ;
		}
	    }
	    dtkMsg.add(DTKMSG_ERROR, "iris::GetTempFileName- returning NULL, unable to find file\n") ;
	    return NULL ;
	}
    
    }

    bool Optimize(osg::Node* node, const std::string& optimizations)
    {

	if (!node)
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::Optimize: NULL node pointer passed\n") ;
	    return false ;
	}

	// convert from a string to a vector the old fashioned way! downcase while you're at it
	// handles both : and space separated tokens
	const std::string allOpts(" DEFAULT "
				  "FLATTEN_STATIC_TRANSFORMS "
				  "FLATTEN_STATIC_TRANSFORMS_DUPLICATING_SHARED_SUBGRAPHS "
				  "REMOVE_REDUNDANT_NODES "
				  "REMOVE_LOADED_PROXY_NODES "
				  "COMBINE_ADJACENT_LODS "
				  "SHARE_DUPLICATE_STATE "
				  "MERGE_GEODES "
				  "MERGE_GEOMETRY "
				  "SPATIALIZE_GROUPS "
				  "COPY_SHARED_NODES "
				  "TESSELLATE_GEOMETRY "
				  "TRISTRIP_GEOMETRY "
				  "OPTIMIZE_TEXTURE_SETTINGS "
				  "CHECK_GEOMETRY "
				  "MAKE_FAST_GEOMETRY "
				  "FLATTEN_BILLBOARDS "
				  "TEXTURE_ATLAS_BUILDER "
				  "STATIC_OBJECT_DETECTION "
				  "INDEX_MESH "
				  "VERTEX_POSTTRANSFORM "
				  "VERTEX_PRETRANSFORM ") ;

	std::vector<std::string> optVec ;
	bool inOptimization ;
	if (optimizations[0] == ' ' || optimizations[0] == ':') inOptimization = false ;
	else inOptimization = true ;
	int b = 0 ;
	for (unsigned int p=0; p<optimizations.length(); p++)
	{
	    if (optimizations[p] == ' ' || optimizations[p] == ':')
	    {
		if (inOptimization)
		{
		    std::string opt = optimizations.substr(b,p-b) ;
		    // I wanted to do it this way but couldn't get the damn thing to compile!
		    //std::transform(opt.begin(), opt.end(), opt.begin(), std::toupper) ;
		    for (unsigned int l=0; l<opt.size(); l++)
		    {
			if (opt[l] >= 'a' && opt[l] <= 'z') opt[l] -= 32 ;
		    }
		    inOptimization = false ;
		    if (allOpts.find(" " + opt + " ") == std::string::npos) dtkMsg.add(DTKMSG_WARNING, "iris::Optimize: skipping unknown optimization \"%s\"\n",opt.c_str()) ;
		    else optVec.push_back(opt) ;
		}
	    }
	    else
	    {
		if (!inOptimization)
		{
		    b = p ;
		    inOptimization = true ;
		}
	    }
	}
	if (inOptimization)
	{
	    std::string opt = optimizations.substr(b) ;
	    for (unsigned int l=0; l<opt.size(); l++)
	    {
		if (opt[l] >= 'a' && opt[l] <= 'z') opt[l] -= 32 ;
	    }
	    if (allOpts.find(" " + opt + " ") == std::string::npos) dtkMsg.add(DTKMSG_WARN, "iris::Optimize: skipping unknown optimization \"%s\"\n",opt.c_str()) ;
	    else optVec.push_back(opt) ;
	}

	if (optVec.size() == 0)
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::Optimize: no known optimizations passed\n") ;
	    return false ;
	}

	// do 'em!
	// print initial scenegraph specs
	osgUtil::StatsVisitor stats;
	if (dtkMsg.isSeverity(DTKMSG_INFO))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::Optimize: initial statistics:\n") ;
	    node->accept(stats);
	    stats.totalUpStats();
	    stats.print(std::cerr) ;
	}


	// for printing out how long the optimizations took
	dtkTime t(0.f,1) ;

	char* oldEnv  = getenv("OSG_OPTIMIZER") ;
	if (oldEnv)
	{
	    dtkMsg.add(DTKMSG_WARN,"iris::Optimize: ignoring options in $OSG_OPTIMIZER\n") ;
	}

	for (int i=0; i<optVec.size(); i++)
	{

	    if (optVec[i] == "MERGE_GEOMETRY")
	    {
		t.reset(0.f,1) ;
		ConvertNodeVisitor nv ;
		nv.setCopyPrimitives(true) ;
		node->accept(nv) ;
		dtkMsg.add(DTKMSG_INFO,"iris::Optimize: copy primitive sets before doing MERGE_GEOMETRY optimization took %f seconds\n",float(t.get())) ;
	    }
	    osgUtil::Optimizer optimizer ;
	    setenv("OSG_OPTIMIZER",optVec[i].c_str(),1) ;
	    dtkMsg.add(DTKMSG_INFO,"iris::Optimize: applying optimization \"%s\"\n",optVec[i].c_str()) ;
	    t.reset(0.f,1) ;
	    optimizer.optimize(node) ;

	    if (i < optVec.size()-1)
	    {
		// print intermediate scenegraph specs
		if (dtkMsg.isSeverity(DTKMSG_INFO))
		{
		    dtkMsg.add(DTKMSG_INFO, "iris::Optimize: took %f seconds, intermediate statistics:\n",float(t.get())) ;
		    stats.reset() ;
		    node->accept(stats);
		    stats.totalUpStats();
		    stats.print(std::cerr) ;
		}
		
	    }
	}

	// print final scenegraph specs
	if (dtkMsg.isSeverity(DTKMSG_INFO))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::Optimize: took %f seconds, final statistics:\n",float(t.get())) ;
	    stats.reset() ;
	    node->accept(stats);
	    stats.totalUpStats();
	    stats.print(std::cout) ;
	}
	
	if (oldEnv) setenv("OSG_OPTIMIZER", oldEnv, 1) ;
	else unsetenv("OSG_OPTIMIZER") ;
	return true ;
    }
    ////////////////////////////////////////////////////////////////////////
    std::vector<std::string> Wordexp(const std::string& s)
    {
	wordexp_t p;
	wordexp(s.c_str(), &p, 0);
	std::vector<std::string> retVector ;
	for (int i=0; i<p.we_wordc; i++) retVector.push_back("\"" + std::string(p.we_wordv[i]) + "\"") ;
	wordfree(&p);
	
	return retVector ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool EnvvarReplace(std::string& str)
    {
	bool ret = true ;

	std::string::size_type lidx, ridx ;
	int i=0 ;
	while (i<5 && (lidx = str.find("${")) != std::string::npos)
	{
	    i++ ;
	    ridx = str.substr(lidx+2).find("}") ;
	    //fprintf(stderr, "str = %s, lidx = %d, ridx = %d, size = %d\n",str.c_str(), lidx,ridx,str.size()) ;
	    if (ridx != std::string::npos) // closing } found
	    {
		//fprintf(stderr, "looking for envvar \"%s\"\n",str.substr(lidx+2,ridx).c_str()) ;
		char* env = getenv(str.substr(lidx+2,ridx).c_str()) ;
		std::string envvar ;
		if (!env)
		{
		    dtkMsg.add (DTKMSG_ERROR,"iris:EnvvarReplace:: envvar %s not found\n",str.substr(lidx+2,ridx).c_str()) ;
		    ret = false ;
		}
		else
		{
		    envvar = env ;
		    //fprintf(stderr, "%s = %s\n",str.substr(lidx,ridx+3).c_str(),env) ;
		}
		std::string before ;
		std::string after ;
		if (lidx>0) before = str.substr(0,lidx) ;
		str = before + envvar + str.substr(lidx+ridx+3) ;
	    }
	    else
	    {
		dtkMsg.add (DTKMSG_ERROR,"iris:EnvvarReplace:: no closing '}' found\n") ;
		if (lidx>0) str = str.substr(0,lidx) ;
		else str = "" ;
		ret = false ;
	    }
	}
	return ret ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool VectorOfStringsToCoord(const std::vector<std::string>& vec, double coord[9])
    {
	if (vec.size() != 0 && vec.size() != 3 && vec.size() != 6 && vec.size() != 7 && vec.size() != 9 && vec.size() != 16)
	{
	    dtkMsg.add (DTKMSG_WARNING,"iris::VectorOfStringsToCoord: invalid number of parameters\n") ;
	    return false ;
	}
	
	if (vec.size() == 16)
	{
	    osg::Matrix mat ;
	    for (int i=0; i<4; i++)
	    {
		for (int j=0; j<4; j++)
		{
		    if (!iris::StringToDouble(vec[i*4 + j], &(mat(i,j))))
		    {
			dtkMsg.add (DTKMSG_WARNING,"iris::VectorOfStringsToCoord: invalid matrix element (%d,%d) \"%s\"\n",i,j,vec[i].c_str()) ;
			return false ;
		    }
		}
	    }
	    osg::Vec3d t ;
	    osg::Quat q  ;
	    osg::Vec3d s ; 
	    iris::Decompose (mat, &t, &q, &s) ;

	    coord[0] = t.x() ;
	    coord[1] = t.y() ;
	    coord[2] = t.z() ;
	    
	    iris::QuatToEuler(q, coord+3, coord+4, coord+5) ;
	    
	    coord[6] = s.x() ;
	    coord[7] = s.y() ;
	    coord[8] = s.z() ;
	    
	}
	else
	{
	    std::string coordText[9] ;
	    coordText[0] = coordText[1] = coordText[2] = "0" ;
	    coordText[3] = coordText[4] = coordText[5] = "0" ;
	    coordText[6] = coordText[7] = coordText[8] = "1" ;
	    
	    if (vec.size() >= 3)
	    {
		coordText[0] = vec[0] ;	
		coordText[1] = vec[1] ;	
		coordText[2] = vec[2] ;	
	    }
	    if (vec.size() >= 6)
	    {
		coordText[3] = vec[3] ;	
		coordText[4] = vec[4] ;	
		coordText[5] = vec[5] ;	
	    }
	    if (vec.size() == 7)
	    {
		coordText[6] = vec[6] ;	
		coordText[7] = vec[6] ;	
		coordText[8] = vec[6] ;	
	    }
	    if (vec.size() == 9)
	    {
		coordText[6] = vec[6] ;	
		coordText[7] = vec[7] ;	
		coordText[8] = vec[8] ;	
	    }
	    
	    for (int i=0; i<9; i++)
	    {
		if (!iris::StringToDouble(coordText[i], coord+i))
		{
		    dtkMsg.add (DTKMSG_WARNING,"iris::VectorOfStringsToCoord: invalid coord parameter %d, \"%s\"\n",i+1,coordText[i].c_str()) ;
		    return false ;
		}
	    }
	}
	return true ;
    }

    ////////////////////////////////////////////////////////////////////////
    void Decompose (const osg::Matrix &matrix, osg::Vec3d *translation, osg::Quat *rotation, osg::Vec3d *scale, osg::Quat *scaleOrientation)
    {
	osg::Vec3d t ;
	osg::Quat q  ;
	osg::Vec3d s ; 
	osg::Quat so ;
	matrix.decompose(t, q, s, so) ;
	if (translation) *translation = t ;
	if (rotation) *rotation = q ;
	if (scale) *scale = s ;
	if (scaleOrientation) *scaleOrientation = so ;
    }

}
