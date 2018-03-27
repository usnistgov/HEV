#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include <iris.h>

#include "utils.h"

void usage()
{
    fprintf(stderr,"Usage: hev-interpolateRotation [ --r0 h p r ] [ --r1 h p r ]  [ --absolute ] [ --startTime t ] [ --duration ] [ --frameRate ] [ --deltaRotation a ]\n") ;
}

////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{

    // send dtk messages to stdout
    dtkMsg.setFile(stderr) ;

    double r0[3] = {0, 0, 0} ;
    double r1[3] = {0, 0, 0} ;
    float startTime = 0 ;
    float duration = 1 ;
    float frameRate = -1 ;
    bool absolute = false ;
    double deltaRotation = .1 ;

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
		dtkMsg.add(DTKMSG_ERROR, "hev-interpolateRotation: invalid startTime value\n") ;
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
		dtkMsg.add(DTKMSG_ERROR, "hev-interpolateRotation: invalid duration value\n") ;
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
		dtkMsg.add(DTKMSG_ERROR, "hev-interpolateRotation: invalid frameRate value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c++ ;
	}
	else if (c>=argc || iris::IsSubstring("--deltaRotation",argv[c],3))
	{
	    c++ ;
	    if (c>=argc || !iris::StringToDouble(argv[c],&deltaRotation) || deltaRotation<0) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-interpolateRotation: invalid deltaRotation value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c++ ;
	}
	else if (c>=argc || !strcmp("--r0",argv[c]))
	{
	    c++ ;
	    if (c>=argc-2 || !iris::StringToDouble(argv[c],r0) || !iris::StringToDouble(argv[c+1],r0+1) || !iris::StringToDouble(argv[c+2],r0+2)) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-interpolateRotation: invalid --r0 value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c+=3 ;
	}
	else if (c>=argc || !strcmp("--r1",argv[c]))
	{
	    c++ ;
	    if (c>=argc-2 || !iris::StringToDouble(argv[c],r1) || !iris::StringToDouble(argv[c+1],r1+1) || !iris::StringToDouble(argv[c+2],r1+2)) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-interpolateRotation: invalid --r0 value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c+=3 ;
	}
	else 
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-hev-interpolateRotation: unrecognized option: %s\n",argv[c]) ;
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
    if (frameRate < 0) frameRate = getDefaultFrameRate() ;

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
    
    printf("# rotating from r0=(%f, %f, %f) to r1=(%f, %f, %f) ; start time = %f, duration = %f, frame rate = %f\n",r0[0], r0[1], r0[2], r1[0], r1[1], r1[2], startTime, duration, frameRate) ;

    Posn p0, p1;
    p0.att = iris::EulerToQuat(r0[0], r0[1], r0[2]) ; ;
    p0.t = startTime ;
    p1.att = iris::EulerToQuat(r1[0], r1[1], r1[2]) ;
    p1.t = startTime + duration ;

    Interpolate interp(p0, p1) ;

    // always write the first
    printf("# first t = %f\n",p0.t) ;
    printf("AFTER %s %f NAV ORIENTATION %f %f %f\n",timeString.c_str(), p0.t, r0[0], r0[1], r0[2]) ;
    Posn lastPosn = p0 ;

    for (int i=1; i<frameRate*duration; i++)
    {
	double t = p0.t + double(i)/frameRate ;
	Posn interpPosn ;
	printf("# interpolated t = %f\n",t) ;
	if (!interp.at(t, &interpPosn))
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-interpolateRotation: interpolation error at t=%f\n",t) ;
	}
	else
	{
	    // only write if it changed from last time
	    if (movedEnoughRot(interpPosn.att, lastPosn.att, deltaRotation))
	    {
		lastPosn.att = interpPosn.att ;
		double h, p, r ;
		iris::QuatToEuler(interpPosn.att, &h, &p, &r) ;
		printf("AFTER %s %f NAV ORIENTATION %f %f %f\n",timeString.c_str(), t, h, p, r) ;
		fflush(stdout) ;
	    }
	}
    }
    // always write the last
    printf("# last t = %f\n",p1.t) ;
    printf("AFTER %s %f NAV ORIENTATION %f %f %f\n",timeString.c_str(), p1.t, r1[0], r1[1], r1[2]) ;
    return 0 ;
}
