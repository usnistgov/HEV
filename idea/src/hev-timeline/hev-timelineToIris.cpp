#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include <iris.h>

#include "utils.h"

////////////////////////////////////////////////////////////////////////
void usage()
{
    fprintf(stderr,"Usage: hev-timelineToIris [ --frameRate f ] [ --startTime t ] [ --absolute ] [ --deltaTrans t ] [ --deltaRot a ] [ --deltaScale s ]\n") ;
}

////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{

    // send dtk messages to stdout
    dtkMsg.setFile(stderr) ;
    
    // default command line parameters
#if 1
    // default to deltas "on"
    float deltaTrans = .001 ;
    float deltaRot = .1 ;
    float deltaScale = 1.001 ;
#else
    // default to deltas "ignore"
    float deltaTrans = -1.0 ;
    float deltaRot = -1.0 ;
    float deltaScale = -1.0 ;
#endif
    float frameRate = -1.f ;
    float startTime = -1.f ;
    bool absolute = false ;

    int c = 1 ;
    while (c < argc && argv[c][0] == '-' && argv[c][1] == '-')
    {
        if (c>=argc || iris::IsSubstring("--absolute",argv[c],3))
        {
            c++ ;
            absolute = true ;
        }
        else if (c>=argc || iris::IsSubstring("--frameRate",argv[c],3))
        {
            c++ ;
            if ( c>=argc || 
                 !iris::StringToFloat(argv[c],&frameRate) || 
                 frameRate<=0) 
            {
                dtkMsg.add(DTKMSG_ERROR, 
            "hev-timelineToIris: invalid frameRate value\n") ;
                usage() ;
                return 1 ; 
            }
            c++ ;
        }
        else if (c>=argc || iris::IsSubstring("--startTime",argv[c],3))
        {
            c++ ;
            if ( c>=argc || 
                 !iris::StringToFloat(argv[c],&startTime) || 
                 startTime<0) 
            {
                dtkMsg.add(DTKMSG_ERROR, 
            "hev-timelineToIris: invalid startTime value\n") ;
                usage() ;
                return 1 ; 
            }
            c++ ;
        }
        else if (c>=argc || iris::IsSubstring("--deltaTrans",argv[c],8))
        {
            c++ ;
            if (c>=argc || !iris::StringToFloat(argv[c],&deltaTrans)) 
            {
                dtkMsg.add(DTKMSG_ERROR, 
            "hev-timelineToIris: invalid deltaTrans value\n") ;
                usage() ;
                return 1 ; 
            }
            c++ ;
        }
        else if (c>=argc || iris::IsSubstring("--deltaRot",argv[c],8))
        {
            c++ ;
            if (c>=argc || !iris::StringToFloat(argv[c],&deltaRot)) 
            {
                dtkMsg.add(DTKMSG_ERROR, 
            "hev-timelineToIris: invalid deltaRot value\n") ;
                usage() ;
                return 1 ; 
            }
            c++ ;
        }
        else if (c>=argc || iris::IsSubstring("--deltaScale",argv[c],8))
        {
            c++ ;
            if (c>=argc || !iris::StringToFloat(argv[c],&deltaScale)) 
            {
                dtkMsg.add(DTKMSG_ERROR, 
            "hev-timelineToIris: invalid deltaScale value\n") ;
                usage() ;
                return 1 ; 
            }
            c++ ;
        }
        else 
        {
            dtkMsg.add(DTKMSG_ERROR, 
            "hev-timelineToIris: unrecognized option: %s\n",argv[c]) ;
            usage() ;
            return 1 ;
        }
    }

    // We store the transformation given by the input file into 
    // the vector posn. We store the inverse of these transformations
    // into the vector posnInverse.  
    //
    // We do the interpolation on the inverse transforms because the
    // XYZ coordinates of these transforms express the "camera" motion.
    // In other words, these coords in the inverse position give the
    // coordinates in the scene that correspond to our viewpoint.
    // We get more natural motion by interpolating these coordinates.

    // read the path file in a loop into a vector
    std::string line ;
    std::vector<Posn> posn ;
    std::vector<Posn> posnInverse ;
    std::vector<std::string> pathLines ;
    
    // needs to be outside the loop so previous values will get used
    Posn p ;
    Posn pInverse ;

    while (iris::GetLine(&line,999999.f))
    {
        std::vector<std::string> vec = iris::ParseString(line) ;
        // skip comments and blank lines
        if (vec.size() == 0) continue ;
        else
        {
            p.init();
            pInverse.init();

            pathLines.push_back(line) ;
            if (!p.get(vec, line))
            {
                dtkMsg.add(DTKMSG_ERROR, 
                    "hev-timelineToIris: invalid line, number %d, skipping\n",
                    pathLines.size()) ;
            }
            else
            {

                // If the line is not a position reuse the last position.
                // This simplifies the interpolation code below.
                if ( p.hold | (p.controlCommand != "") )
                {
                    int j = posn.size() - 1;
                    if (j >= 0)
                    {
                        // but only use last position if this is not the first
                        p.pos = posn[j].pos;
                        p.att = posn[j].att;
                        p.scale = posn[j].scale;
                        for (int k = 0; k < 7; k++)
                        {
                            p._lastPos[k] = posn[j]._lastPos[k];
                        }
                    }

                }


                // make inverse of p in pInverse
                pInverse = p;
                pInverse.invert();

                posn.push_back(p) ;
                posnInverse.push_back(pInverse);

                // write out some useful info as comments
                printf ("# KEY_LINE %s\n", line.c_str());
                printf ("# KEY_POS_CAMERA_XYZ %g %g %g\n",
                    pInverse.pos[0], pInverse.pos[1], pInverse.pos[2]);
                int prev = posnInverse.size() - 2;
                double dist = 0;
                if (prev >= 0)
                {
                    for (int ii = 0; ii < 3; ii++)
                    {
                        double d = pInverse.pos[ii] - posnInverse[prev].pos[ii];
                        dist += d*d;
                    }
                    dist = sqrt (dist);
                }
                else
                {
                    dist = 0;
                }
            
                printf ("# KEY_DIST_TO_PREV %g\n", dist);
                printf ("\n");

            } // end of section for non-empty line
        }
    }  // end of loop over input lines


    dtkMsg.add(DTKMSG_INFO, 
        "hev-timelineToIris: %d waypoints read\n",posn.size()) ;
    if (posn.size() == 0)
    {
            dtkMsg.add(DTKMSG_ERROR, 
            "hev-timelineToIris: no waypoints read, exiting\n");
        return 1 ;
    }

    // now navigate!

    if (startTime < 0) startTime = 0.f ;

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
            printf("# time is relative to start of first graphics frame, ") ;
    }
    else 
    {
        timeString = "TIME" ;
        printf("# time is relative to when control commands are received, ") ;
    }

    printf("start time = %f, frame rate = %f\n",startTime, frameRate) ;
    dtkMsg.add(DTKMSG_INFO, 
        "hev-timelineToIris: start time = %f, frame rate = %f\n",
        startTime, frameRate) ;

    // save the previous position so only changes are written
    Posn* lastPosn = NULL ;
    Posn* lastPosnInverse = NULL ;

    // handle the first line
    printf("\n# path line 1: %s\n",pathLines[0].c_str()) ;
    if (posn[0].hold) // just a time?
    {
        printf("# waiting for %f seconds\n", posn[0].t) ;
        startTime += posn[0].t ;
    }
    else // a time and a position or control command
    {
        printf("# waiting for %f seconds\n", posn[0].t) ;
        startTime += posn[0].t ;
        if (posn[0].controlCommand == "") // a position
        {
            printf("# jumping to first coordinate\n") ;
            printf("AFTER %s %f NAV SCALE %f\n",
            timeString.c_str(), startTime, posn[0].scale) ;     

#ifdef NAV_ATTITUDE
            printf("AFTER %s %f NAV ATTITUDE %f %f %f %f\n",
                timeString.c_str(), startTime, 
                posn[0].att.x(), posn[0].att.y(), posn[0].att.z(), 
                posn[0].att.w());
#else
            double h, p, r ;
            iris::QuatToEuler(posn[0].att, &h, &p, &r) ;
            printf("AFTER %s %f NAV ORIENTATION %.17g %.17g %.17g\n",
            timeString.c_str(), startTime, h, p, r) ;
#endif

            printf("AFTER %s %f NAV POSITION %f %f %f\n",
            timeString.c_str(), startTime, 
            posn[0].pos.x(), posn[0].pos.y(), posn[0].pos.z()) ;
            fflush(stdout) ;
            lastPosn = &(posn[0]) ;
            lastPosnInverse = &(posnInverse[0]) ;
        }
        else // a control command
        {
            printf("# issuing control command\n") ;
            printf("AFTER %s %f %s\n",
                timeString.c_str(), startTime, 
                posn[0].controlCommand.c_str()) ;  
        }
    }

    //loop, issuing "after time nav ..." control commands
    // Loop over each position, interpolating between previous
    // position and the current position
    for (unsigned int i=1; i<posn.size(); i++)
    {
        printf("\n# path line %d: %s\n",i+1, pathLines[i].c_str()) ;
        if (posn[i].controlCommand != "") // a control command
        {
            startTime += posn[i].t ;
            printf("# issuing control command\n") ;
            printf("AFTER %s %f %s\n",
            timeString.c_str(), startTime, 
            posn[i].controlCommand.c_str()) ;   
            continue ;
        }

        // no previous position?
        if (!lastPosn)
        {
            startTime += posn[i].t ;
            if (!posn[i].hold) 
            {
                // jump to the coordinate instead of moving there
                printf(
                    "# incrementing clock and jumping to first coordinate, "
                    "on line %d\n",i+1) ;
                printf("AFTER %s %f NAV SCALE %f\n",
                    timeString.c_str(), startTime, posn[i].scale) ;     

#ifdef NAV_ATTITUDE
                printf("AFTER %s %f NAV ATTITUDE %f %f %f %f\n",
                    timeString.c_str(), startTime, 
                    posn[i].att.x(), posn[i].att.y(), posn[i].att.z(), 
                    posn[i].att.w());
#else
                double h, p, r ;
                iris::QuatToEuler(posn[i].att, &h, &p, &r) ;
                printf("AFTER %s %f NAV ORIENTATION %.17g %.17g %.17g\n",
                timeString.c_str(), startTime, h, p, r) ;
#endif

                printf("AFTER %s %f NAV POSITION %f %f %f\n",
                    timeString.c_str(), startTime, 
                    posn[i].pos.x(), posn[i].pos.y(), posn[i].pos.z()) ;
                fflush(stdout) ;
                lastPosn = &(posn[i]) ;
                lastPosnInverse = &(posnInverse[i]) ;
            }
            else printf("# waiting for %f seconds\n", posn[i].t) ;
            continue ;
        }
        
        if (posn[i].hold)
        {
            printf("# waiting for %f seconds\n", posn[i].t) ;
            startTime += posn[i].t ;
            continue ;
        }

        if (posn[i].t<=1.0/frameRate)
        {
            printf(
                "# delta time <= frame rate, so jumping to coordinate, "
                "on line %d\n",i+1) ;
            printf("AFTER %s %f NAV SCALE %f\n",
            timeString.c_str(), startTime, posn[i].scale) ;     

#ifdef NAV_ATTITUDE
            printf("AFTER %s %f NAV ATTITUDE %f %f %f %f\n",
            timeString.c_str(), startTime, 
            posn[i].att.x(), posn[i].att.y(), posn[i].att.z(), posn[i].att.w());
#else
            double h, p, r ;
            iris::QuatToEuler(posn[i].att, &h, &p, &r) ;
            printf("AFTER %s %f NAV ORIENTATION %.17g %.17g %.17g\n",
            timeString.c_str(), startTime, h, p, r) ;
#endif

            printf("AFTER %s %f NAV POSITION %f %f %f\n",
            timeString.c_str(), startTime, 
            posn[i].pos.x(), posn[i].pos.y(), posn[i].pos.z()) ;
            fflush(stdout) ;
            startTime += posn[i].t ;
            lastPosn = &(posn[i]) ;
            lastPosnInverse = &(posnInverse[i]) ;
            continue ;
        }


        // If we get here, we are interpolating between posn[i-1] and posn[i]
        lastPosn = &(posn[i-1]);
        lastPosnInverse = &(posnInverse[i-1]);
        lastPosn->t = 0.f ;
        lastPosnInverse->t = 0.f ;


        // As mentioned above, we interpolate the inverse positions to
        // give us "camera" motion through scene coordinates, rather than 
        // scene motion through view coordinates.

        Interpolate interp(*lastPosnInverse, posnInverse[i]) ;
        Posn interpPosn ;

        // loop over each frame between last position and current position
        // calc new position for each frame
        for ( double t=1.0/frameRate; 
              t<posn[i].t+1.0/frameRate; t+=1.0/frameRate)
        {

            // this is to ensure we always hit the position at the end
            if (t>=posn[i].t) t=posn[i].t ;
            
            printf("# interpolated t = %f\n",t) ;
            if (!interp.at(t, &interpPosn))
            {
                dtkMsg.add(DTKMSG_ERROR, 
                    "hev-timelineToIris: interpolation error at t=%f, t1=%f\n",
                    t, posn[i].t) ;
            }
            else
            {

                printf ("# CAMERA XYZ %g %g %g\n",
                    interpPosn.pos[0], interpPosn.pos[1], interpPosn.pos[2]);

                interpPosn.invert();

                // only write if it changed from last time
                if (movedEnoughScale(interpPosn.scale, 
                                        lastPosn->scale, deltaScale))
                {
                    lastPosn->scale = interpPosn.scale ;
                    printf("AFTER %s %f NAV SCALE %f\n",
                timeString.c_str(), startTime+t, interpPosn.scale) ;
                }
                if (movedEnoughRot(interpPosn.att, lastPosn->att, deltaRot))
                {
                    lastPosn->att = interpPosn.att ;

#ifdef NAV_ATTITUDE
                    printf("AFTER %s %f NAV ATTITUDE %f %f %f %f\n",
                        timeString.c_str(), startTime+t, 
                        interpPosn.att.x(), 
                        interpPosn.att.y(), 
                        interpPosn.att.z(), 
                        interpPosn.att.w()) ;
#else
                    double h, p, r ;
                    iris::QuatToEuler(interpPosn.att, &h, &p, &r) ;
                    printf("AFTER %s %f NAV ORIENTATION %.17g %.17g %.17g\n",
                    timeString.c_str(), startTime+t, h, p, r) ;
#endif

                }
                if (movedEnoughTrans(interpPosn.pos,lastPosn->pos, deltaTrans))
                {
                    lastPosn->pos = interpPosn.pos ;
                    printf("AFTER %s %f NAV POSITION %f %f %f\n",
                timeString.c_str(), startTime+t, 
                interpPosn.pos.x(), interpPosn.pos.y(), interpPosn.pos.z()) ;
                }
            }
            // just to be sure we don't do the last one twice
            if (t==posn[i].t) break ;

        } // end of loop over interpolated frames

        startTime += posn[i].t ;
        fflush(stdout) ;

    }  // end of loop over positions
    
    return 0 ;    

}  // end of main
