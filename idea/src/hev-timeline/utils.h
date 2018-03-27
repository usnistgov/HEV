////////////////////////////////////////////////////////////////////////
class Posn
{
public:
    Posn() : t(0), pos(osg::Vec3d(0,0,0)), att(osg::Quat(0,0,0,1)), scale(1), hold(false)
    { 
        _lastPos[0] = 0 ; 
        _lastPos[1] = 0 ; 
        _lastPos[2] = 0 ; 
        _lastPos[3] = 0 ; 
        _lastPos[4] = 0 ; 
        _lastPos[5] = 0 ; 
        _lastPos[6] = 1 ; 
        inverted = false;
    } ;

    double t ;
    bool hold ;
    osg::Vec3d pos ;
    osg::Quat att ;
    float scale ;
    std::string controlCommand ;
    bool inverted;
    float _lastPos[7] ;

    void init ()
    {
        _lastPos[0] = 0 ; 
        _lastPos[1] = 0 ; 
        _lastPos[2] = 0 ; 
        _lastPos[3] = 0 ; 
        _lastPos[4] = 0 ; 
        _lastPos[5] = 0 ; 
        _lastPos[6] = 1 ; 
        t = 0;
        pos[0] = pos[1] = pos[2] = 0;
        att[0] = att[1] = att[2] = 0;
        att[3] = 1;
        scale = 1;
        controlCommand = "";
        hold = false;
        inverted = false;
    } // end of init


    bool invert ()
    {
        // Invert the transformation expressed by this object

        

        // build the matrix given by t pos, att, and scale
        osg::Matrix mat;
        mat.makeIdentity();
        mat.setTrans (pos);
        mat.setRotate (att);
        mat.scale (scale, scale, scale);

        osg::Vec3d newTrans;
        osg::Quat newRot;
        osg::Vec3d newScale;

#if 0
        printf ("old trans %g %g %g\n",
                pos.x(),
                pos.y(),
                pos.z());
        printf ("old rot %g %g %g %g\n",
                att.x(),
                att.y(),
                att.z(),
                att.w());
        printf ("old scale %g \n", scale);
#endif

        iris::Decompose (mat, &newTrans, &newRot, &newScale);
#if 0
        printf ("new trans %g %g %g\n",
                newTrans.x(),
                newTrans.y(),
                newTrans.z());
        printf ("new rot %g %g %g %g\n",
                newRot.x(),
                newRot.y(),
                newRot.z(),
                newRot.w());
        printf ("new scale %g %g %g\n",
                newScale.x(),
                newScale.y(),
                newScale.z());
        printf ("\n\n");
#endif


        // invert the matrix given by t pos, att, and scale
        osg::Matrix matInverse;
        matInverse.invert (mat);
        

        // extract the pos, att, scale
        iris::Decompose (matInverse, &pos, &att, &newScale);
        scale = newScale.x();


#if 0
        printf ("revised trans %g %g %g\n",
                pos.x(),
                pos.y(),
                pos.z());
        printf ("revised rot %g %g %g %g\n",
                att.x(),
                att.y(),
                att.z(),
                att.w());
        printf ("revised scale %g \n", scale);
#endif

        inverted = true;
        return true;
    } // end of invert


    bool get(std::vector<std::string> vec, std::string& line)
    {
        
        controlCommand = "" ;
        hold = false;

        if (vec.size() == 0) return false ;

        if (!iris::StringToDouble(vec[0],&t) || t<0.0) return false ;

        if (vec.size() == 1)
        {
            // sit and rest a while
            hold = true ;
            return true ;
        }

        // control command?
        {
            float x ;
            if (!(vec[1] == "-") && !(iris::StringToFloat(vec[1], &x)))
            {
                controlCommand = iris::ChopFirstWord(line) ;
                return true ;
            }
        }

        // if not, must be a coordinate
        if (vec.size() == 4 || vec.size() == 7 || vec.size() == 8)
        {

            // always get xyz
            float xyz[3] ;
            for (int i=0; i<3; i++)
            {
                if (vec[i+1] == "-") xyz[i] = _lastPos[i] ;
                else if (iris::StringToFloat(vec[i+1], xyz+i)) _lastPos[i] = xyz[i] ;
                else return false ;
            }
            pos.set(xyz[0], xyz[1], xyz[2]) ;
            
            if (vec.size() >= 7) // optional hpr
            {
                float hpr[3] ;
                for (int i=0; i<3; i++)
                {
                    if (vec[i+3+1] == "-") hpr[i] = _lastPos[i+3] ;
                    else if (iris::StringToFloat(vec[i+3+1], hpr+i)) _lastPos[i+3] = hpr[i] ;
                    else return false ;
                }
                att = iris::EulerToQuat(hpr[0], hpr[1], hpr[2]) ;
            }
            else
            {
                // set hpr to 0, s to 1
                att.set(0,0,0,1) ;
                scale = 1 ;
            }

            if (vec.size() == 8) // optional s
            {
                if (vec[7] == "-") scale = _lastPos[6] ;
                else if (iris::StringToFloat(vec[7], &scale)) _lastPos[6] = scale ;
                else return false ;
            }
            else
            {
                // set s to 1
                scale = 1 ;
            }
            return true ;
        }
        else return false ;
        ;
    } ;



private:
    // float _lastPos[7] ;
} ;

////////////////////////////////////////////////////////////////////////
class Interpolate
{
public:
    Interpolate(Posn p0, Posn p1) : _p0(p0), _p1(p1) { ; } ;

    bool at(double t, Posn* p)
    {
        if (!p)
        {
            dtkMsg.add(DTKMSG_ERROR, "hev-timelineToIris: NULL Posn passed to interpolate\n") ;
            return false ;
        }
        if (t < _p0.t || t > _p1.t)
        {
            dtkMsg.add(DTKMSG_ERROR, "hev-timelineToIris: time t=%f is not between t0=%f and t1=%f inclusive\n",t, _p0.t, _p1.t) ;
            return false ;
        }

        // normalize t to be between 0 and 1
        double nt = (t-_p0.t)/(_p1.t-_p0.t) ;

        // interpolate position
        (*p).pos = (_p1.pos - _p0.pos)*nt + _p0.pos ;

        // slerp the rotation- magic!
        (*p).att.slerp(nt, _p0.att, _p1.att) ;

        // interpolate scale
        (*p).scale = (_p1.scale - _p0.scale)*nt + _p0.scale ;

        //printf("# interp: t= %f, _p1.t = %f, nt = %f, (*p).pos = %f %f %f\n", t, _p1.t, nt, (*p).pos.x(), (*p).pos.y(), (*p).pos.z()) ;
        
        return true ;
    }

private:
    Posn _p0 ;
    Posn _p1 ;
} ;

////////////////////////////////////////////////////////////////////////
bool closeEnough(float a, float b=0.f, float d=.00001)
{
    if (fabs(a-b)<d) return true ;
    else return false ;
}

////////////////////////////////////////////////////////////////////////
// translation
bool movedEnoughTrans(osg::Vec3d& t1, osg::Vec3d& t2, float deltaTrans)
{
    if (deltaTrans < 0) return false ;
    if (deltaTrans == 0) return true ;
    
    double d = (t1-t2).length2() ;
    if (d >=deltaTrans*deltaTrans) return true ;
    return false ;
}
    
////////////////////////////////////////////////////////////////////////
// rotation
bool movedEnoughRot(osg::Quat& q1, osg::Quat& q2, float deltaRot)
{
    if (deltaRot < 0) return false ;
    if (deltaRot == 0) return true ;

    double d ;
    osg::Vec3d v ;
    (q1.inverse()*q2).getRotate(d, v) ;
    if (d>=iris::DegreesToRadians(deltaRot)) return true ;

    return false ;
}

////////////////////////////////////////////////////////////////////////
// scale
bool movedEnoughScale(float s1, float s2, float deltaScale)
{
    if (deltaScale < 0) return false ;
    if (deltaScale == 0) return true ;

    float ds(fmax(fabs(s1/s2), fabs(s2/s1))) ;
    //printf("# s1 = %f, s2 = %f, ds = %f, deltaScale = %f\n", s1, s2, ds, deltaScale) ;
    if (ds>=deltaScale) return true ;

    return false ;
}

////////////////////////////////////////////////////////////////////////
// get the frame rate from the system and double it
float getDefaultFrameRate()
{ 
    float frameRate ;
    iris::ShmState shmState  ;
    std::map<std::string, iris::ShmState::DataElement>::iterator ratePos ;
    ratePos = shmState.getMap()->find("frameRate") ;
    if (ratePos == shmState.getMap()->end())
    {
        dtkMsg.add(DTKMSG_ERROR, "hev-path: can't find ShmState object \"frameRate\"\n") ;
        return 1 ;
    }
    iris::ShmState::DataElement rateD = ratePos->second ;
    if (rateD.type != iris::ShmState::FLOAT)
    {
            dtkMsg.add(DTKMSG_ERROR, "hev-path: ShmState object \"frameRate\" is not a float\n") ;
            return 1 ;
    }
    if (!rateD.shm || rateD.shm->isInvalid())
    {
        // twice the frame rate as in the RAVE
        frameRate = 96*2 ;
        dtkMsg.add(DTKMSG_NOTICE, "hev-path: can't read \"frameRate\" shared memory, setting to %f\n", frameRate) ;
    }
    else
    {
        rateD.shm->read(&frameRate) ;
        frameRate *= 2 ;
    }
    return frameRate ;
}

////////////////////////////////////////////////////////////////////////
// get the start time from the system 
float getDefaultStartTime()
{
    float startTime ;
    iris::ShmState shmState  ;

    std::map<std::string, iris::ShmState::DataElement>::iterator timePos ;
    timePos = shmState.getMap()->find("refTime") ;
    if (timePos == shmState.getMap()->end())
    {
        dtkMsg.add(DTKMSG_ERROR, "hev-timelineToIris: can't find ShmState object \"refTime\"\n") ;
        return 1 ;
    }
    iris::ShmState::DataElement timeD = timePos->second ;
    if (timeD.type != iris::ShmState::FLOAT)
    {
        dtkMsg.add(DTKMSG_ERROR, "hev-timelineToIris: ShmState object \"refTime\" is not a float\n") ;
        return 1 ;
    }
    if (!timeD.shm || timeD.shm->isInvalid())
    {
        startTime = 0 ;
        dtkMsg.add(DTKMSG_NOTICE, "hev-timelineToIris: can't read \"refTime\" shared memory, setting to start time to %f\n", startTime) ;
    }
    else timeD.shm->read(&startTime) ;
}

