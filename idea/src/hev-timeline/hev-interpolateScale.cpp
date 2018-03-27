#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include <iris.h>

#include "utils.h"

void usage()
{
    fprintf(stderr,"Usage: hev-interpolateScale [ --s0 s ] [ --s1 s ] [ --absolute ] [ --startTime t ] [ --duration ] [ --frameRate ] [ --deltaScale s ]\n") ;
}

////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{

    // send dtk messages to stdout
    dtkMsg.setFile(stderr) ;

    double s0 = 1 ;
    double s1 = 1 ;
    float startTime = 0 ;
    float duration = 1 ;
    float frameRate = -1 ;
    bool absolute = false ;
    double deltaScale = .001 ;

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
		dtkMsg.add(DTKMSG_ERROR, "hev-interpolateScale: invalid startTime value\n") ;
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
		dtkMsg.add(DTKMSG_ERROR, "hev-interpolateScale: invalid duration value\n") ;
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
		dtkMsg.add(DTKMSG_ERROR, "hev-interpolateScale: invalid frameRate value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c++ ;
	}
	else if (c>=argc || iris::IsSubstring("--deltaScale",argv[c],3))
	{
	    c++ ;
	    if (c>=argc || !iris::StringToDouble(argv[c],&deltaScale) || deltaScale <0) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-interpolateScale: invalid deltaScale value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c++ ;
	}
	else if (c>=argc || !strcmp("--s0",argv[c]))
	{
	    c++ ;
	    if (c>=argc || !iris::StringToDouble(argv[c],&s0)) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-interpolateScale: invalid --s0 value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c++ ;
	}
	else if (c>=argc || !strcmp("--s1",argv[c]))
	{
	    c++ ;
	    if (c>=argc || !iris::StringToDouble(argv[c],&s1)) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-hev-interpolateScale: invalid --s1 value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c++ ;
	}
	else 
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-hev-interpolateScale: unrecognized option: %s\n",argv[c]) ;
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
    
    printf("# scaling from s0 = %f to s1 = %f; start time = %f, duration = %f, frame rate = %f\n",s0, s1, startTime, duration, frameRate) ;

    Posn p0, p1;
    p0.scale = s0 ;
    p0.t = startTime ;
    p1.scale = s1 ;
    p1.t = startTime + duration ;

    Interpolate interp(p0, p1) ;

    // always write the first
    printf("# first t = %f\n",p0.t) ;
    printf("AFTER %s %f NAV SCALE %f\n",timeString.c_str(), p0.t, p0.scale) ;
    Posn lastPosn = p0 ;

    for (int i=1; i<frameRate*duration; i++)
    {
	double t = p0.t + double(i)/frameRate ;
	Posn interpPosn ;
	printf("# interpolated t = %f\n",t) ;
	if (!interp.at(t, &interpPosn))
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-interpolateScale: interpolation error at t=%f\n",t) ;
	}
	else
	{
	    // only write if it changed from last time
	    if (movedEnoughScale(interpPosn.scale, lastPosn.scale, deltaScale))
	    {
		lastPosn.scale = interpPosn.scale ;
		printf("AFTER %s %f NAV SCALE %f\n",timeString.c_str(), t, interpPosn.scale) ;
		fflush(stdout) ;
	    }
	}
    }
    // always write the last
    printf("# last t = %f\n",p1.t) ;
    printf("AFTER %s %f NAV SCALE %f\n",timeString.c_str(), p1.t, p1.scale) ;
    return 0 ;
}
