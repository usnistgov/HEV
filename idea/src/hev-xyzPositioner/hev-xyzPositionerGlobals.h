/*

  data is bounded from xmin, xmax, ymin, ymax, zmin, zmax.

  in below, A is any axis, x, y or z

  each axis has a center:  (Amax+Amin)/2

  each axis has a scale, relative to a normalized -1 to 1 cube.  (Amax-Amin)/2

  you also want to store the largest scale value of the three scales

  the rollers just change x, y and z values.  They are bounded between Amin and Amax.

  sge commands are sent to place and move objects as x, y, and z change.
  Normalized objects are hung under DCS nodes to scale and positioin the objects.

    box: box.osg

      the base object is a normalized cube with font-face culling.  It is
      slightly larger than the -1 to 1 size, say -1.05 to 1.05, to prevent z
      fighting, and to ensure the "shadows" on the axes are visible.

    point: point.osg

      the base object is a point at 0,0,0.  It moves to the position x, y, z.  

    glyph: plus3d.osg

      the base object is centered at 0,0,0, and has a radius of .1.  It
      moves to the position x, y, z.  It is uniformly scaled to the largest
      scale value

    axes: xPlusAxis.osg xMinusAxis.osg yPlusAxis.osg yMinusAxis.osg zPlusAxis.osg zMinusAxis.osg

      There're six of them, for -x, +x, -y, +y, -z, +z.  The axis for +A
      goes from .1 to 1 in the A axis direction.  The other axis values are
      0.  

      At the end furthest from the origin is a "shadow", which is a small "+"
      shaped object made of two perpendicular axis-aligned lines, both
      perpendicular to the axis A.  The shadow is centered on the axis, and
      each line segment is .2 units long (extending .1 away from the axis in
      four directions).  The -A axis is a reflection of the A axis, going
      from -.1 to -1.

      The +A axis is scaled and translated i inthe A axis such that the
      endpoint furthest from the origin is at Amax.  But the endpoint
      nearest to the origin is at .1 * the largest scale value.  The two
      non-A axes are scaled such that the shadow lines are .2 * the largest
      scale value.  Again, the -A axis is just the reflection of the A axis.

      As x, y and z change value, the axes are translated and scaled so the
      endpoint closest to the origin is always .1 * the largest scale value
      away from the point x, y, z.  The endpoint furthest away from the
      origin is always at Amax or Amin, as appropriate.  The shadows move
      with the axes' endpoints, but do not change scale.
    
    The scenegraph is simple: a no clip node under the world, and under it a
    DCS for each object to be displayed.  Calculating the DCS's values will
    be the fun part


*/

// widgets
Fl_Double_Window *main_window ;
Fl_Roller *xSlider1, *xSlider10, *xSlider100, 
    *ySlider1, *ySlider10, *ySlider100, 
    *zSlider1, *zSlider10, *zSlider100,
    *gapSizeRoller ;
Fl_Button *clickButton, *clickButton3, *resetButton ;
Fl_Light_Button *glyphButton, *lightButton, *lightButton3 ;
Fl_Input *xText, *yText, *zText ;

// shared memory that contains xyz, with hpr set to zero
std::string coordName ;
dtkSharedMem *coordShm = NULL ;
float coordData[6] = {0,0,0,0,0,0} ;
bool OutTransformIsCoord = false;


float oldX ;
float oldY ;
float oldZ ;

float xmin = -1.f ;
float xmax =  1.f ;
float ymin = -1.f ;
float ymax =  1.f ;
float zmin = -1.f ;
float zmax =  1.f ;

std::string boundingBoxNode ;
int ticks = iris::GetUsleep() ;
bool noEscape = false ;

float xscale, yscale, zscale, lscale ;
float xcenter, ycenter, zcenter ;
float offset ;
float gapSize = 1 ;
float inc = .0001 ;
std::string click[2] ;
std::string clickLabel = "click" ;
std::string click3[2] ;
std::string clickLabel3 = "click3" ;
std::string node = "world" ;
bool clip = false ;


std::string line ;


// function templates
int arg(int argc, char **argv, int &i) ;
void signal_catcher(int sig) ;
void usage() ;
void active(bool) ;
void displayGlyph(bool) ;
void writeCoord() ;
void runCommand(std::string) ;
void writeButton() ;
void writeButton(bool) ;
void writeButton3() ;
void writeButton3(bool) ;
void setup(int argc, char **argv) ;
void updateDeltaX(float f) ;
void updateDeltaY(float f) ;
void updateDeltaZ(float f) ;
void updateX(float f) ;
void updateY(float f) ;
void updateZ(float f) ;
void updateXYZ() ;
void reset() ;
void updateGapSize(float f) ;

////////////////////////////////////////////////////////////////////////
// for catching the KILL, QUIT, ABRT, TERM and INT signals
void signal_catcher(int sig)
{
    dtkMsg.add(DTKMSG_INFO,"PID %d, hev-xyzPositioner caught signal %d, starting exit sequence ...\n", getpid(), sig);
    printf("UNLOAD xyzPositionZMinusAxis\n"	
	   "UNLOAD xyzPositionZMinusAxisDCS\n"	
	   "UNLOAD xyzPositionZPlusAxis\n"	
	   "UNLOAD xyzPositionZPlusAxisDCS\n"	
	   "UNLOAD xyzPositionYMinusAxis\n"	
	   "UNLOAD xyzPositionYMinusAxisDCS\n"	
	   "UNLOAD xyzPositionYPlusAxis\n"	
	   "UNLOAD xyzPositionYPlusAxisDCS\n"	
	   "UNLOAD xyzPositionXMinusAxis\n"	
	   "UNLOAD xyzPositionXMinusAxisDCS\n"	
	   "UNLOAD xyzPositionXPlusAxis\n"	
	   "UNLOAD xyzPositionXPlusAxisDCS\n"	
	   "UNLOAD xyzPositionGlyph\n"		
	   "UNLOAD xyzPositionGlyphDCS\n"	
	   "UNLOAD xyzPositionPoint\n"		
	   "UNLOAD xyzPositionPointDCS\n"	
	   "UNLOAD xyzPositionBox\n"		
	   "UNLOAD xyzPositionBoxDCS\n"		
	   "UNLOAD xyzPositionGroup\n") ;
    exit(0) ;

}

////////////////////////////////////////////////////////////////////////
void usage()
{
    fprintf(stderr,"Usage: hev-xyzPositioner [ --boundingBox nodeName ] [ --xmin f ] [ --xmax f ] [ --ymin f ] [ --ymax f ] [ --zmin f ] [ --zmax f ] [ --click cmd ] [ --click3 cmd ] [ --button label on off ] [ --button3 label on off ] [ --shm name ] [ --node name ] [ --clip ] [ --noEscape ] [ --usleep ticks ]\n") ;
}

////////////////////////////////////////////////////////////////////////
// argument parser callback
int arg(int argc, char **argv, int &i)
{

    //fprintf(stderr,"arg called, i=%d, argv[%d] = %s\n",i,i,argv[i]) ;

    if (iris::IsSubstring("--xmin", argv[i], 5))
    {
	i++ ;
	if (sscanf(argv[i],"%f",&xmin) != 1)
	{
	    usage() ;
	    exit(1) ;
	}
    }
    else if (iris::IsSubstring("--xmax", argv[i], 5))
    {
	i++ ;
	if (sscanf(argv[i],"%f",&xmax) != 1)
	{
	    usage() ;
	    exit(1) ;
	}
    }
    else if (iris::IsSubstring("--ymin", argv[i], 5))
    {
	i++ ;
	if (sscanf(argv[i],"%f",&ymin) != 1)
	{
	    usage() ;
	    exit(1) ;
	}
    }
    else if (iris::IsSubstring("--ymax", argv[i], 5))
    {
	i++ ;
	if (sscanf(argv[i],"%f",&ymax) != 1)
	{
	    usage() ;
	    exit(1) ;
	}
    }
    else if (iris::IsSubstring("--zmin", argv[i], 5))
    {
	i++ ;
	if (sscanf(argv[i],"%f",&zmin) != 1)
	{
	    usage() ;
	    exit(1) ;
	}
    }
    else if (iris::IsSubstring("--zmax", argv[i], 5))
    {
	i++ ;
	if (sscanf(argv[i],"%f",&zmax) != 1)
	{
	    usage() ;
	    exit(1) ;
	}
    }
    else if (iris::IsSubstring("--click", argv[i], 5))
    {
	i++ ;
	click[0] = argv[i] ;
    }
    else if (iris::IsSubstring("--usleep", argv[i], 5))
    {
	i++ ;
	if (!iris::StringToInt(argv[i],&ticks))
	{
	    dtkMsg.add(DTKMSG_ERROR,"hev-xyzPositioner: invalid --usleep value, ignoring\n") ;
	}
    }
    else if (iris::IsSubstring("--boundingbox", argv[i], 5))
    {
	i++ ;
	boundingBoxNode = argv[i] ;
    }
    else if (iris::IsSubstring("--click3", argv[i], 5))
    {
	i++ ;
	click3[0] = argv[i] ;
    }
    else if (iris::IsSubstring("--button", argv[i], 5))
    {
	i++ ;
	clickLabel = argv[i] ;
	i++ ;
	click[0] = argv[i] ;
	i++ ;
	click[1] = argv[i] ;
    }
    else if (iris::IsSubstring("--button3", argv[i], 5))
    {
	i++ ;
	clickLabel3 = argv[i] ;
	i++ ;
	click3[0] = argv[i] ;
	i++ ;
	click3[1] = argv[i] ;
    }
    else if (iris::IsSubstring("--outShm", argv[i], 5))
    {
	i++ ;
	coordName = argv[i] ;
    }
    else if (iris::IsSubstring("--node", argv[i], 5))
    {
	i++ ;
	node = argv[i] ;
    }
    else if (iris::IsSubstring("--clip", argv[i], 5))
    {
	clip = true ;
    }
    else if (iris::IsSubstring("--noescape", argv[i], 5))
    {
	noEscape = true ;
    }
    else
    {
	return 0 ;
    }
    
    // point to the next word to test
    i++ ;

    //fprintf(stderr,"arg returning i=%d\n",i) ;

    return i ;
}

////////////////////////////////////////////////////////////////////////

static dtkSharedMem *
openTransformShm (
  const char *shmName,
  bool mustExist,
  bool createCoord,
  bool & existingIsCoord
  )
        {
        existingIsCoord = true;

        // try to open existing coord file
        dtkSharedMem * shm = NULL;

        shm = new dtkSharedMem (shmName, 0);
        if ( ! shm->isInvalid () )
                {
                int sz = shm->getSize();
                if (sz == 6*sizeof(float))
                        {
                        existingIsCoord = true;
                        return shm;
                        }
                else if (sz == 16*sizeof(double))
                        {
                        existingIsCoord = false;
                        return shm;
                        }
                delete shm;
                }

        // file doesn't exist

        existingIsCoord = false;

        if (mustExist)
                {
	        dtkMsg.add(DTKMSG_ERROR,
                        "%s: Unable to open shared memory transformation %s.\n"
                        "     Note that existing files must hold either "
                        " 6 floats (XYZHPR) or 16 doubles (matrix).\n",
                        "hev-xyzPositioner", shmName);
                return NULL;
                }


        if (createCoord)
                {
                shm = new dtkSharedMem (6*sizeof(float), shmName);
                }
        else
                {
                shm = new dtkSharedMem (16*sizeof(double), shmName);
                }

        if ( shm->isInvalid () )
                {
	        dtkMsg.add(DTKMSG_ERROR,
                        "%s: Unable to open shared memory transformation %s.\n"
                        "     Note that existing files must hold either "
                        " 6 floats (XYZHPR) or 16 doubles (matrix).\n",
                        "hev-xyzPositioner", shmName);
                delete shm;
                shm = NULL;
                }

        return shm;
        } // end of openTransformShm





////////////////////////////////////////////////////////////////////////
// set up shared memory and initialize data and set timer
void setup(int argc, char **argv)
{
    // catch signals that kill us off
    iris::Signal(signal_catcher);

    // send messages to stderr
    dtkMsg.setFile(stderr) ;

    if (boundingBoxNode != "")
    {
	std::string tmpFifo = std::string("/tmp/hev-xyzPositioner-fifo-") + std::string(getenv("USER")) ;
	iris::FifoReader fifo(tmpFifo) ;
	fifo.unlinkOnExit() ;
	if (!fifo.open()) 
	{
	    dtkMsg.add(DTKMSG_ERROR,"hev-xyzPositioner: can't open fifo %s, can't use bounding box\n",tmpFifo.c_str()) ;
	    boundingBoxNode = "" ;
	}
	else
	{
	    printf("QUERY %s BOUNDINGBOX %s\n",tmpFifo.c_str(),boundingBoxNode.c_str()) ; fflush(stdout) ;
	    std::string response ;
	    while (1)
	    {
		if (fifo.readLine(&response)) 
		{
		    std::vector<std::string> vec ;
		    vec = iris::ParseString(response) ;
		    if (vec.size() != 8)
		    {
			dtkMsg.add(DTKMSG_ERROR,"hev-xyzPositioner: unexpected QUERY response, can't use bounding box\n",tmpFifo.c_str()) ;
			boundingBoxNode = "" ;
		    }
		    else
		    {
			//for (unsigned int i = 0; i<vec.size(); i++) fprintf(stderr,"vec[%d] = %s\n",i,vec[i].c_str()) ;
			if (vec[0] != "BOUNDINGBOX" || vec[1] != boundingBoxNode ||
			    !iris::StringToFloat(vec[2],&xmin) || !iris::StringToFloat(vec[3],&xmax) || 
			    !iris::StringToFloat(vec[4],&ymin) || !iris::StringToFloat(vec[5],&ymax) || 
			    !iris::StringToFloat(vec[6],&zmin) || !iris::StringToFloat(vec[7],&zmax))
			{
			    dtkMsg.add(DTKMSG_ERROR,"hev-xyzPositioner: unexpected QUERY response, can't use bounding box\n",tmpFifo.c_str()) ;
			    boundingBoxNode = "" ;
			}
		    }
		    break ; // just one response
		}
		else usleep(ticks) ;
	    }
	}
    }
    xscale = (xmax-xmin)/2.f ;
    lscale = xscale ;
    yscale = (ymax-ymin)/2.f ;
    if (yscale>lscale) lscale = yscale ;
    zscale = (zmax-zmin)/2.f ;
    if (zscale>lscale) lscale = zscale ;

    inc *= lscale ;
    // how much of the axis is chopped off? .05 is radius of plus3d.osg
    offset = .05*lscale*gapSize ;

    xcenter = (xmax+xmin)/2.f ;
    ycenter = (ymax+ymin)/2.f ;
    zcenter = (zmax+zmin)/2.f ;

    oldX = coordData[0] = xcenter ; 
    oldY = coordData[1] = ycenter ; 
    oldZ = coordData[2] = zcenter ;

#if 0
    fprintf(stderr,"xscale = %f, yscale = %f. zscale = %f, lscale = %f\n",xscale, yscale, zscale, lscale) ;
    fprintf(stderr, "xmin = %f, xmax = %f, ymin = %f, ymax = %f, zmin = %f, zmax = %f\n", xmin,xmax,ymin,ymax,zmin,zmax) ;
    fprintf(stderr,"xcenter = %f, ycenter = %f. zcenter = %f\n",xcenter, ycenter, zcenter) ;
#endif

    if (clip)
    {
	printf("GROUP xyzPositionGroup\n") ;
    }
    else
    {
	printf("NOCLIP xyzPositionGroup\n") ;
    }

    printf("NODEMASK xyzPositionGroup OFF\n") ;
    printf("ADDCHILD xyzPositionGroup %s\n", node.c_str()) ;

    printf("DCS xyzPositionBoxDCS %f %f %f 0 0 0 %f %f %f\n",
	   xcenter, ycenter, zcenter, xscale, yscale, zscale) ;
    printf("ADDCHILD xyzPositionBoxDCS xyzPositionGroup\n") ;
    printf("LOAD xyzPositionBox %s/idea/etc/hev-xyzPositioner/data/box.osg\n",getenv("HEVROOT")) ;
    printf("ADDCHILD xyzPositionBox xyzPositionBoxDCS\n") ;

    printf("DCS xyzPositionPointDCS %f %f %f\n",
	   xcenter, ycenter, zcenter) ;
    printf("ADDCHILD xyzPositionPointDCS xyzPositionGroup\n") ;
    printf("LOAD xyzPositionPoint %s/idea/etc/hev-xyzPositioner/data/point.osg\n",getenv("HEVROOT")) ;
    printf("ADDCHILD xyzPositionPoint xyzPositionPointDCS\n") ;

    printf("DCS xyzPositionGlyphDCS %f %f %f 0 0 0 %f %f %f\n",
	   xcenter, ycenter, zcenter, lscale*gapSize, lscale*gapSize, lscale*gapSize) ;
    printf("ADDCHILD xyzPositionGlyphDCS xyzPositionGroup\n") ;
    printf("LOAD xyzPositionGlyph %s/idea/etc/hev-xyzPositioner/data/plus3d.osg\n",getenv("HEVROOT")) ;
    printf("NODEMASK xyzPositionGlyph OFF\n") ;
    printf("ADDCHILD xyzPositionGlyph xyzPositionGlyphDCS\n") ;
    
    // +X
    printf("DCS xyzPositionXPlusAxisDCS %f %f %f 0 0 0 %f %f %f\n",
	   xcenter + offset, ycenter, zcenter, xscale - offset, lscale, lscale) ;
    printf("ADDCHILD xyzPositionXPlusAxisDCS xyzPositionGroup\n") ;
    printf("LOAD xyzPositionXPlusAxis %s/idea/etc/hev-xyzPositioner/data/xPlusAxis.osg\n",getenv("HEVROOT")) ;
    printf("ADDCHILD xyzPositionXPlusAxis xyzPositionXPlusAxisDCS\n") ;

    // -X
    printf("DCS xyzPositionXMinusAxisDCS %f %f %f 0 0 0 %f %f %f\n",
	   xcenter - offset, ycenter, zcenter, xscale - offset, lscale, lscale) ;
    printf("ADDCHILD xyzPositionXMinusAxisDCS xyzPositionGroup\n") ;
    printf("LOAD xyzPositionXMinusAxis %s/idea/etc/hev-xyzPositioner/data/xMinusAxis.osg\n",getenv("HEVROOT")) ;
    printf("ADDCHILD xyzPositionXMinusAxis xyzPositionXMinusAxisDCS\n") ;

    // +Y
    printf("DCS xyzPositionYPlusAxisDCS %f %f %f 0 0 0 %f %f %f\n",
	   xcenter, ycenter + offset, zcenter, lscale, yscale - offset, lscale) ;
    printf("ADDCHILD xyzPositionYPlusAxisDCS xyzPositionGroup\n") ;
    printf("LOAD xyzPositionYPlusAxis %s/idea/etc/hev-xyzPositioner/data/yPlusAxis.osg\n",getenv("HEVROOT")) ;
    printf("ADDCHILD xyzPositionYPlusAxis xyzPositionYPlusAxisDCS\n") ;
    
    // -Y
    printf("DCS xyzPositionYMinusAxisDCS %f %f %f 0 0 0 %f %f %f\n",
	   xcenter, ycenter - offset, zcenter, lscale, yscale - offset, lscale) ;
    printf("ADDCHILD xyzPositionYMinusAxisDCS xyzPositionGroup\n") ;
    printf("LOAD xyzPositionYMinusAxis %s/idea/etc/hev-xyzPositioner/data/yMinusAxis.osg\n",getenv("HEVROOT")) ;
    printf("ADDCHILD xyzPositionYMinusAxis xyzPositionYMinusAxisDCS\n") ;
    
    // +Z
    printf("DCS xyzPositionZPlusAxisDCS %f %f %f 0 0 0 %f %f %f\n",
	   xcenter, ycenter, zcenter + offset, lscale, lscale, zscale - offset) ;
    printf("ADDCHILD xyzPositionZPlusAxisDCS xyzPositionGroup\n") ;
    printf("LOAD xyzPositionZPlusAxis %s/idea/etc/hev-xyzPositioner/data/zPlusAxis.osg\n",getenv("HEVROOT")) ;
    printf("ADDCHILD xyzPositionZPlusAxis xyzPositionZPlusAxisDCS\n") ;

    // -Z
    printf("DCS xyzPositionZMinusAxisDCS %f %f %f 0 0 0 %f %f %f\n",
	   xcenter, ycenter, zcenter - offset, lscale, lscale, zscale - offset) ;
    printf("ADDCHILD xyzPositionZMinusAxisDCS xyzPositionGroup\n") ;
    printf("LOAD xyzPositionZMinusAxis %s/idea/etc/hev-xyzPositioner/data/zMinusAxis.osg\n",getenv("HEVROOT")) ;
    printf("ADDCHILD xyzPositionZMinusAxis xyzPositionZMinusAxisDCS\n") ;

    fflush(stdout) ;

    if (coordName.size()>0)
    {
        coordShm = openTransformShm (coordName.c_str(), 
                                        false, false, OutTransformIsCoord);

        if (coordShm == NULL) exit (1);

        writeCoord ();
    }

}

////////////////////////////////////////////////////////////////////////
void active(bool b)
{
    if (b)
    {
	xSlider1->activate() ;
	xSlider10->activate() ;
	xSlider100->activate() ;
	ySlider1->activate() ;
	ySlider10->activate() ;
	ySlider100->activate() ;
	zSlider1->activate() ;
	zSlider10->activate() ;
	zSlider100->activate() ;
	if (click[1].size()>0) lightButton->activate() ;
	else if (click[0].size()>0) clickButton->activate() ;
	if (click3[1].size()>0) lightButton3->activate() ;
	else if (click3[0].size()>0) clickButton3->activate() ;
	resetButton->activate() ;
	gapSizeRoller->activate() ;
	xText->activate() ;
	yText->activate() ;
	zText->activate() ;
	glyphButton->activate() ;
	printf("NODEMASK xyzPositionGroup ON\n") ;

    }
    else
    {
	xSlider1->deactivate() ;
	xSlider10->deactivate() ;
	xSlider100->deactivate() ;
	ySlider1->deactivate() ;
	ySlider10->deactivate() ;
	ySlider100->deactivate() ;
	zSlider1->deactivate() ;
	zSlider10->deactivate() ;
	zSlider100->deactivate() ;
	clickButton->deactivate() ;
	lightButton->deactivate() ;
	clickButton3->deactivate() ;
	lightButton3->deactivate() ;
	resetButton->deactivate() ;
	gapSizeRoller->deactivate() ;
	xText->deactivate() ;
	yText->deactivate() ;
	zText->deactivate() ;
	glyphButton->deactivate() ;
	printf("NODEMASK xyzPositionGroup OFF\n") ;
    }
    fflush(stdout) ;
}

////////////////////////////////////////////////////////////////////////
void displayGlyph(bool b)
{
    if (b) printf("NODEMASK xyzPositionGlyph ON\n") ;
    else printf("NODEMASK xyzPositionGlyph OFF\n") ;
    fflush(stdout) ;
}

////////////////////////////////////////////////////////////////////////
void writeCoord()
{
    //fprintf(stderr,"writeCoord: coord = %f %f %f\n",coordData[0], coordData[1], coordData[2]) ;
    // if (coordName.size()>0) coordShm->write(coordData) ;

    if (coordName.size()<=0) return;

    if (OutTransformIsCoord)
        {
        coordShm->write(coordData) ;
        }
   else   
        {
        static double matrix[4][4] =
              {
                {1.0, 0.0, 0.0, 0.0},
                {0.0, 1.0, 0.0, 0.0},
                {0.0, 0.0, 1.0, 0.0},
                {0.0, 0.0, 0.0, 1.0}
              };

        matrix[3][0] = coordData[0];
        matrix[3][1] = coordData[1];
        matrix[3][2] = coordData[2];
        coordShm->write(matrix) ;
        }


}  // end of writeCoord


////////////////////////////////////////////////////////////////////////
void runCommand(std::string cmd) 
{

    pid_t pid  = fork() ;
    if (pid == -1)
    {
	perror("hev-xyzPositioner: Fork failed") ;
	exit(1) ;
    }
    else if (pid == 0)
    {
	dtkMsg.add(DTKMSG_INFO,"hev-xyzPositioner: running command \"%s\"\n",cmd.c_str()) ;
	
	int ret = execl("/bin/sh", "sh", "-c", cmd.c_str(), NULL) ;
	if (ret) perror("hev-xyzPositioner: : Child failed to exec command") ;
	exit(1) ;
    }
    
}

////////////////////////////////////////////////////////////////////////
void writeButton()
{
    runCommand(click[0]) ;
}

////////////////////////////////////////////////////////////////////////
void writeButton(bool v)
{
    if (v) runCommand(click[0]) ;
    else runCommand(click[1]) ;
}

////////////////////////////////////////////////////////////////////////
void writeButton3()
{
    runCommand(click3[0]) ;
}

////////////////////////////////////////////////////////////////////////
void writeButton3(bool v)
{
    if (v) runCommand(click3[0]) ;
    else runCommand(click3[1]) ;
}

////////////////////////////////////////////////////////////////////////
void updateDeltaX(float f)
{
  updateX(coordData[0] + inc*f)  ;
}

////////////////////////////////////////////////////////////////////////
void updateDeltaY(float f)
{
  updateY(coordData[1] + inc*f)  ;
}

////////////////////////////////////////////////////////////////////////
void updateDeltaZ(float f)
{
  updateZ(coordData[2] + inc*f)  ;
}

static void
floatToString (double f, char str[])
    {
    sprintf (str, "%.5f", f);
    }

////////////////////////////////////////////////////////////////////////
void updateX(float f)
{
  coordData[0] = f ;
  if (coordData[0] > xmax) coordData[0] = xmax ;
  if (coordData[0] < xmin) coordData[0] = xmin ;

  char fstr[100];
  floatToString (coordData[0], fstr);
  
  xText->value(fstr);
  updateXYZ()  ;
}

////////////////////////////////////////////////////////////////////////
void updateY(float f)
{
  coordData[1] = f ;
  if (coordData[1] > ymax) coordData[1] = ymax ;
  if (coordData[1] < ymin) coordData[1] = ymin ;

  char fstr[100];
  floatToString (coordData[1], fstr);
  
  yText->value(fstr);
  updateXYZ() ;
}

////////////////////////////////////////////////////////////////////////
void updateZ(float f)
{
  coordData[2] = f ;
  if (coordData[2] > zmax) coordData[2] = zmax ;
  if (coordData[2] < zmin) coordData[2] = zmin ;
  
  char fstr[100];
  floatToString (coordData[2], fstr);
  
  zText->value(fstr);
  updateXYZ() ;
}

////////////////////////////////////////////////////////////////////////
void updateXYZ()
{
    writeCoord() ;

    printf("DCS xyzPositionPointDCS %f %f %f 0 0 0 %f %f %f\n", coordData[0], coordData[1], coordData[2], lscale, lscale, lscale) ;
    printf("DCS xyzPositionGlyphDCS %f %f %f 0 0 0 %f %f %f\n", coordData[0], coordData[1], coordData[2], lscale*gapSize, lscale*gapSize, lscale*gapSize) ;

    // +X
    printf("DCS xyzPositionXPlusAxisDCS %f %f %f 0 0 0 %f %f %f\n",
	   xcenter + offset + (coordData[0]-xcenter), 
	   ycenter + (coordData[1]-ycenter), 
	   zcenter + (coordData[2]-zcenter), 
	   xscale - (offset + (coordData[0]-xcenter)), lscale, lscale) ;

    // -X
    printf("DCS xyzPositionXMinusAxisDCS %f %f %f 0 0 0 %f %f %f\n",
	   xcenter - offset + (coordData[0]-xcenter), 
	   ycenter + (coordData[1]-ycenter), 
	   zcenter + (coordData[2]-zcenter), 
	   xscale - offset + (coordData[0]-xcenter), lscale, lscale) ;

    // +Y
    printf("DCS xyzPositionYPlusAxisDCS %f %f %f 0 0 0 %f %f %f\n",
	   xcenter + (coordData[0]-xcenter), 
	   ycenter + offset + (coordData[1]-ycenter), 
	   zcenter + (coordData[2]-zcenter), 
	   lscale, yscale - (offset + (coordData[1]-ycenter)), lscale) ;

    // -Y
    printf("DCS xyzPositionYMinusAxisDCS %f %f %f 0 0 0 %f %f %f\n",
	   xcenter + (coordData[0]-xcenter), 
	   ycenter - offset + (coordData[1]-ycenter), 
	   zcenter + (coordData[2]-zcenter), 
	   lscale, yscale - offset + (coordData[1]-ycenter), lscale) ;

    // +Z
    printf("DCS xyzPositionZPlusAxisDCS %f %f %f 0 0 0 %f %f %f\n",
	   xcenter + (coordData[0]-xcenter), 
	   ycenter + (coordData[1]-ycenter), 
	   zcenter + offset + (coordData[2]-zcenter), 
	   lscale, lscale, zscale - (offset + (coordData[2]-zcenter))) ;

    // -Z
    printf("DCS xyzPositionZMinusAxisDCS %f %f %f 0 0 0 %f %f %f\n",
	   xcenter + (coordData[0]-xcenter), 
	   ycenter + (coordData[1]-ycenter), 
	   zcenter - offset + (coordData[2]-zcenter), 
	   lscale, lscale, zscale - offset + (coordData[2]-zcenter)) ;

    fflush(stdout) ;

}

////////////////////////////////////////////////////////////////////////
void reset()
{
    // which buttons to make visible
    if (click[1].size()>0) 
    {
	lightButton->show() ;
	lightButton->label(clickLabel.c_str()) ;
    }
    else if (click[0].size()>0) 
    {
	clickButton->show() ; 
	clickButton->label(clickLabel.c_str()) ;
    }

    if (click3[1].size()>0) 
    {
	lightButton3->show() ;
	lightButton3->label(clickLabel3.c_str()) ;
    }
    else if (click3[0].size()>0) 
    {
	clickButton3->show() ; 
	clickButton3->label(clickLabel3.c_str()) ;
    }

    oldX = coordData[0] = xcenter ; 
    oldY = coordData[1] = ycenter ; 
    oldZ = coordData[2] = zcenter ;
    xSlider1->value(0.f) ;
    xSlider10->value(0.f) ;
    xSlider100->value(0.f) ;
    ySlider1->value(0.f) ;
    ySlider10->value(0.f) ;
    ySlider100->value(0.f) ;
    zSlider1->value(0.f) ;
    zSlider10->value(0.f) ;
    zSlider100->value(0.f) ;
    gapSize = 1.f ;
    offset = .05*lscale*gapSize ;
    gapSizeRoller->value(gapSize) ;
    updateX(xcenter) ;
    updateY(ycenter) ;
    updateZ(zcenter) ;
}

////////////////////////////////////////////////////////////////////////
void updateGapSize(float f) 
{
    gapSize = f ;
    offset = .05*lscale*gapSize ;
    updateXYZ() ;
}

