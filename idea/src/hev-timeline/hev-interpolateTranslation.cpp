#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include <iris.h>

#include "utils.h"

void usage()
{
    fprintf(stderr,"Usage: hev-interpolateTranslation [ --t0 x y z ] [ --t1 x y z ] [ --absolute ] [ --startTime t ] [ --duration t ] [ --frameRate r ] [ --deltaTranslation t ]\n") ;
}

////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{

    // send dtk messages to stdout
    dtkMsg.setFile(stderr) ;

    osg::Vec3d t0(0,0,0) ;
    osg::Vec3d t1(0,0,0) ;
    float startTime = 0 ;
    float duration = 1 ;
    float frameRate = -1 ;
    bool absolute = false ;
    double deltaTranslation = .001 ;

    int c = 1 ;
    while (c < argc && argv[c][0] == '-' && argv[c][1] == '-')
    {
	if (c>=argc || iris::IsSubstring("--absolute",argv[c],3))
	{
	    c++ ;
	    absolute = true ;
	}
	else if (c>=argc || iris::IsSubstring("--startTime",argv[c],4))
	{
	    c++ ;
	    if (c>=argc || !iris::StringToFloat(argv[c],&startTime) || startTime<0) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-interpolateTranslation: invalid startTime value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c++ ;
	}
	else if (c>=argc || iris::IsSubstring("--duration",argv[c],4))
	{
	    c++ ;
	    if (c>=argc || !iris::StringToFloat(argv[c],&duration) || duration<=0) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-interpolateTranslation: invalid duration value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c++ ;
	}
	else if (c>=argc || iris::IsSubstring("--frameRate",argv[c],4))
	{
	    c++ ;
	    if (c>=argc || !iris::StringToFloat(argv[c],&frameRate) || frameRate<=0) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-interpolateTranslation: invalid frameRate value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c++ ;
	}
	else if (c>=argc || iris::IsSubstring("--deltaTranslation",argv[c],3))
	{
	    c++ ;
	    if (c>=argc || !iris::StringToDouble(argv[c],&deltaTranslation) || deltaTranslation<0 ) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-interpolateTranslation: invalid deltaTranslation value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c++ ;
	}
	else if (c>=argc || !strcmp("--t0",argv[c]))
	{
	    c++ ;
	    if (c>=argc-2 || !iris::StringToDouble(argv[c],&(t0.x())) || !iris::StringToDouble(argv[c+1],&(t0.y())) || !iris::StringToDouble(argv[c+2],&(t0.z())))
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-interpolateTranslation: invalid --t0 value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c+=3 ;
	}
	else if (c>=argc || !strcmp("--t1",argv[c]))
	{
	    c++ ;
	    if (c>=argc-2 || !iris::StringToDouble(argv[c],&(t1.x())) || !iris::StringToDouble(argv[c+1],&(t1.y())) || !iris::StringToDouble(argv[c+2],&(t1.z())))
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-interpolateTranslation: invalid --t1 value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c+=3 ;
	}
	else 
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-hev-interpolateTranslation: unrecognized option: %s\n",argv[c]) ;
	    usage() ;
	    return 1 ;
	}
    }

#if 0
    // get the frame rate from the system if frameRate not passed as a parameter
    if (frameRate < 0) frameRate = getDefaultFrameRate() ;
#else
    if (frameRate < 0) frameRate = 60 ;
#endif

    std::string timeString ;
    if (absolute) 
    {
	timeString = "ABSTIME" ;
	printf("# time is relative to start of first graphics frame\n") ;
    }
    else 
    {
	timeString = "TIME" ;
	printf("# time is relative to when control commands are received\n") ;
    }
    
    printf("# moving from t0 = (%f %f %f) to t1 = (%f %f %f) ; start time = %f, duration = %f, frame rate = %f\n",t0.x(), t0.y(), t0.z(), t1.x(), t1.y(), t1.z(), startTime, duration, frameRate) ;

    Posn p0, p1;
    p0.pos = t0 ;
    p0.t = startTime ;
    p1.pos = t1 ;
    p1.t = startTime + duration ;

    Interpolate interp(p0, p1) ;

    // always write the first
    printf("# first t = %f\n",p0.t) ;
    printf("AFTER %s %f NAV POSITION %f %f %f\n",timeString.c_str(), p0.t, p0.pos.x(), p0.pos.y(), p0.pos.z()) ;
    Posn lastPosn = p0 ;

    for (int i=1; i<frameRate*duration; i++)
    {
	double t = p0.t + double(i)/frameRate ;
	Posn interpPosn ;
	printf("# interpolated t = %f\n",t) ;
	if (!interp.at(t, &interpPosn))
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-interpolateTranslation: interpolation error at t=%f\n",t) ;
	}
	else
	{
	    // only write if it changed from last time
	    if (movedEnoughTrans(interpPosn.pos, lastPosn.pos, deltaTranslation))
	    {
		lastPosn.pos = interpPosn.pos ;
		printf("AFTER %s %f NAV POSITION %f %f %f\n",timeString.c_str(), t, interpPosn.pos.x(), interpPosn.pos.y(), interpPosn.pos.z()) ;
		fflush(stdout) ;
	    }
	}
    }
    // always write the last
    printf("# last t = %f\n",p1.t) ;
    printf("AFTER %s %f NAV POSITION %f %f %f\n",timeString.c_str(), p1.t, p1.pos.x(),  p1.pos.y(),  p1.pos.z()) ;
    return 0 ;
}
