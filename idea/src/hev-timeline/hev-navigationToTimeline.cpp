#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include <iris.h>

#include "utils.h"

void usage()
{
    fprintf(stderr,"Usage: hev-navigationToTimeline [ --frameRate f ] [ --deltaTrans t ] [ --deltaRot a ] [ --deltaScale s ]\n") ;
}

////////////////////////////////////////////////////////////////////////
class WritePath
{
public:
    WritePath(double deltaTrans=0.f, double deltaRot=0.f, double deltaScale=0.f) : _first(true)
    {
	setDeltaTrans(deltaTrans) ;
	setDeltaRot(deltaRot) ;
	setDeltaScale(deltaScale) ;

	iris::ShmState shmState  ;
	std::map<std::string, iris::ShmState::DataElement>::iterator pos ;
	pos = shmState.getMap()->find("nav") ;
	
	if (pos == shmState.getMap()->end())
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-navigationToTimeline: can't find ShmState object \"nav\"\n") ;
	    return ;
	}

	_d = pos->second ;
	if (_d.type != iris::ShmState::MATRIX)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-navigationToTimeline: ShmState object \"nav\" is not a matrix\n") ;
	    return ;
	}
    } 

    ////////////////////////////////////////////////////////////////////////
    void comment(std::string s) 
    { 
	_pathLines.push_back(s) ; 
    } ;

    ////////////////////////////////////////////////////////////////////////
    void writeIfMoved()
    {
	
	// get nav node matrix
	osg::Matrix mat ;
	_d.shm->read(mat.ptr()) ;

	osg::Timer_t now = osg::Timer::instance()->tick();
	if (_first)
	{
	    // init last command time and last matrix
	    _lastTick = now ;
	    _lastPathTick = now ;
	    _lastMat = mat ;
	    // start delta timer on first call
	    _firstTick = _lastTick ;
	    _first = false ;
	}

	if (_lastMat != mat) // time to move?
	{

	    osg::Vec3d t ;
	    osg::Quat q ;
	    osg::Vec3d s ;
	    iris::Decompose(mat, &t, &q, &s) ;
	    
	    if (movedEnoughTrans(t, _lastT, _deltaTrans) ||
		movedEnoughRot(q, _lastQ, _deltaRot) ||
		movedEnoughScale(s.x(), _lastS, _deltaScale)) // yup!
	    {
		// time since we started running
		double elapsed = osg::Timer::instance()->delta_s(_firstTick, now) ;
		std::string comment = "# time: " + iris::DoubleToString(elapsed) ;
		
		// the time between this call and the last path command
		double delta = osg::Timer::instance()->delta_s(_lastPathTick, now) ;
		std::string out  = iris::DoubleToString(delta) + "    " ;
		
		// if a path command was not written the last time we were called and the matrix has changed
		// that means we were motionless for a while, so kick out a "T" command
		if (_lastTick != _lastPathTick)
		{
		    double elapsedWait = osg::Timer::instance()->delta_s(_firstTick, _lastTick) ;
		    _pathLines.push_back("# time: " + iris::DoubleToString(elapsedWait) + ", format: T") ;
		    double deltaWait = osg::Timer::instance()->delta_s(_lastPathTick, _lastTick) ;
		    _pathLines.push_back(iris::DoubleToString(deltaWait)) ;
		    _lastPathTick = _lastTick ;
		}
		
		delta = osg::Timer::instance()->delta_s(_lastPathTick, now) ;
		out = iris::DoubleToString(delta) + "    " ;
		
		// check for uniform scale
		if (closeEnough(s.x(),s.y()) && closeEnough(s.x(),s.z()) && closeEnough(s.y(),s.z()))
		{

		    double h, p, r ;
		    iris::QuatToEuler(q, &h, &p, &r) ;
		    // always print position
		    out += iris::DoubleToString(t.x()) + " " ;
		    out += iris::DoubleToString(t.y()) + " " ;
		    out += iris::DoubleToString(t.z()) + "    " ;
		    comment += ", format: T XYZ " ;
		    // need to write hpr if non-zero of scale != 1
		    if (s.x() != 1.0 || h != 0.f || p != 0.f || r != 0.f)
		    {
			out += iris::DoubleToString(h) + " " ;
			out += iris::DoubleToString(p) + " " ;
			out += iris::DoubleToString(r) + "    " ;
			comment += "HPR " ;
			// and scale if non-zero
			if (s.x() != 1.0) 
			{
			    out += iris::DoubleToString(s.x()) ;
			    comment += "S " ;
			}
		    }
		    
		}
		else
		{
		    dtkMsg.add(DTKMSG_WARNING, "hev-navigationToTimeline: skipping path line because scale is non-uniform\n") ;
		}
		
		_pathLines.push_back(comment) ;
		_pathLines.push_back(out) ;
		
		_lastMat = mat ;
		_lastT = t ;
		_lastQ = q ;
		_lastS = s.x() ;
		_lastPathTick = now ;
	    }
	}
	_lastTick = now ;
    }

    ////////////////////////////////////////////////////////////////////////
    void print()
    {
	dtkMsg.add(DTKMSG_INFO, "hev-navigationToTimeline: writing %d path lines\n",_pathLines.size()) ;
	for (unsigned int i=0; i<_pathLines.size(); i++) printf("%s\n",_pathLines[i].c_str()) ;
    }

    void setDeltaTrans(double f) { _deltaTrans = f ; } ;
    void setDeltaRot(double f) { _deltaRot = f ; } ;
    void setDeltaScale(double f) { _deltaScale = f ; } ;

private:
    iris::ShmState::DataElement _d ;
    osg::Timer_t _lastPathTick  ;
    osg::Timer_t _lastTick  ;
    osg::Timer_t _firstTick  ;
    bool _first ;
    double _deltaTrans ;
    double _deltaRot ;
    double _deltaScale ;
    osg::Matrix _lastMat ;
    osg::Vec3d _lastT ;
    osg::Quat _lastQ ;
    double _lastS ;
    std::vector<std::string> _pathLines ;
} ;

// sucky global variable so the signal catcher can call wp.print() 
WritePath wp ;
bool done = false ;

////////////////////////////////////////////////////////////////////////
static void signal_catcher(int sig)
{
    dtkMsg.add(DTKMSG_INFO,"hev-navigationToTimeline:: PID %d, caught signal %d, starting exit sequence ...\n", getpid(), sig);
    wp.print() ;
    done = true ;
    // reset the signals
    iris::Signal(SIG_DFL); 
}


////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{

    // send dtk messages to stdout
    dtkMsg.setFile(stderr) ;
    
    // default command line parameters
#if 1
    // default to deltas "on"
    double deltaTrans = .05 ;
    double deltaRot = 5 ;
    double deltaScale = 1.05 ;
#else
    // default to deltas "off"
    double deltaTrans = -1.0 ;
    double deltaRot = -1.0 ;
    double deltaScale = -1.0 ;
#endif
    double frameRate = -1 ;

    int c = 1 ;
    while (c < argc && argv[c][0] == '-' && argv[c][1] == '-')
    {
	if (c>=argc || iris::IsSubstring("--frameRate",argv[c],4))
	{
	    c++ ;
	    if (c>=argc || !iris::StringToDouble(argv[c],&frameRate) || frameRate<=0) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-navigationToTimeline: invalid frameRate value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c++ ;
	}
	else if (c>=argc || iris::IsSubstring("--deltaTrans",argv[c],8))
	{
	    c++ ;
	    if (c>=argc || !iris::StringToDouble(argv[c],&deltaTrans)) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-navigationToTimeline: invalid deltaTrans value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c++ ;
	}
	else if (c>=argc || iris::IsSubstring("--deltaRot",argv[c],8))
	{
	    c++ ;
	    if (c>=argc || !iris::StringToDouble(argv[c],&deltaRot)) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-navigationToTimeline: invalid deltaRot value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c++ ;
	}
	else if (c>=argc || iris::IsSubstring("--deltaScale",argv[c],8))
	{
	    c++ ;
	    if (c>=argc || !iris::StringToDouble(argv[c],&deltaScale)) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-navigationToTimeline: invalid deltaScale value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c++ ;
	}
	else 
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-navigationToTimeline: unrecognized option: %s\n",argv[c]) ;
	    usage() ;
	    return 1 ;
	}
    }

    wp.setDeltaTrans(deltaTrans) ;
    wp.setDeltaRot(deltaRot) ;
    wp.setDeltaScale(deltaScale)  ;

    if (frameRate < 0) frameRate = getDefaultFrameRate() ;
    dtkMsg.add(DTKMSG_INFO, "hev-navigationToTimeline: frame rate = %f\n", frameRate) ;

    // catch signals that kill us off
    iris::Signal(signal_catcher) ;

    while (!done)
    {

	wp.writeIfMoved() ;

	usleep((unsigned long)(1.0/frameRate*1000000.0)) ;
    }
    
    return 0 ;

}
