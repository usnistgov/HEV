#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>

#include <iris.h>


#define MAX(a,b)  (((a)>(b))?(a):(b))
#define MIN(a,b)  (((a)<(b))?(a):(b))


/*


 */


// Global variables, and a lot of them!

static bool Running = true;

static bool Active = false;

static bool Verbose = false;
static bool Debug = false;

static char *ProgName;

static char TmpDirName[1000];

#define CTRL_ID_SHM_SIZE 200
#define MAX_NAME_SIZE (200)
#define MAX_NAME_ARRAY_SIZE (MAX_NAME_SIZE+1)

static dtkSharedMem *InTransformShm = NULL;
static dtkSharedMem *InTRButtonShm = NULL;
static dtkSharedMem *InTButtonShm = NULL;
static dtkSharedMem *InRButtonShm = NULL;

static iris::ShmString * InCtrlShm = NULL;
static std::string MyCtrlIDStr = "";


static dtkSharedMem *OutTransformShm = NULL;
// static dtkSharedMem *OutCoordShm = NULL;
// static dtkSharedMem *OutMatShm = NULL;
static dtkSharedMem *OutPlaneShm = NULL;
//static char OutCoordNodeName[MAX_NAME_ARRAY_SIZE];
static char OutMatNodeName[MAX_NAME_ARRAY_SIZE];
static char OutPlaneUniNodeName[MAX_NAME_ARRAY_SIZE];
static char OutPlaneUniVarName[MAX_NAME_ARRAY_SIZE];
static bool InitCoordSpecified = false;
static double CurrentOutCoord[9] = {0, 0, 0, 0, 0, 0, 1, 1, 1};
static double CurrentOutPlane[4] = {0, 1, 0, 0};
static bool Absolute = false;


static bool BoxConstrain = false;
// static bool BoxAdjust = false;
static double Box[2][3] = { {-1,-1,-1} , {1,1,1} };


static bool InMove = false;

static unsigned long SleepDurationUSec = 100000;

static iris::FifoReader* fifo = NULL ;

/////////////////////////////////////////////////


static void
activate ()
{
    // Does anything else have to be done here?
    Active = true;

    if (Verbose)
    {
	fprintf (stderr, "%s: Has been activated!\n", ProgName);
    }
}  // end of activate

static void
deactivate ()
{
    // Does anything else have to be done here?
    Active = false;
    InMove = false;

    if (Verbose)
    {
	fprintf (stderr, "%s: Has been deactivated!\n", ProgName);
    }
}  // end of deactivate


static bool
active ()
{
    if (InCtrlShm)
    {
	// Because we have two ways of activating and deactivating, we
	// have the problem of what to do when both are in use and
	// they give contradictory instructions.
	//
	// This problem is handled here.  The idea is that the control
	// shm id affects the activation state only when it changes
	// from a match to no-match or vice-versa.  This makes it
	// act more like a command rather than a state.

	static bool LastIDMatch = false;


	// we only do something if the id match has changed
	bool idMatch = (InCtrlShm->getString() == MyCtrlIDStr);

	if (idMatch != LastIDMatch)
	{
	    if (idMatch)
	    {
		// ctrl shm says that we should be active
		if ( ! Active )
		{
		    activate ();
		}
	    }
	    else
	    {
		// ctrl shm says that we should NOT be active
		if ( Active  )
		{
		    deactivate ();
		}
	    }

	    LastIDMatch = idMatch; 
	}

    }  // end of section for testing control shm 

    return Active;
}  // end of active ()



/////////////////////////////////////////////////

static int
boxConstrain (double coord[9])
{

    if ( ! BoxConstrain )
    {
	return 0;
    }


    for (int i = 0; i < 3; i++)
    {
	if (coord[i] < Box[0][i])
	{
	    coord[i] = Box[0][i];
	}
	else if (coord[i] > Box[1][i])
	{
	    coord[i] = Box[1][i];
	}
    }

    return 0;
} // end of boxConstrain


static int
absoluteMove (
	      double inCoord[9], 
	      double outCoord[9], 
	      double outPlane[4],
	      unsigned char tr,
	      unsigned char t,
	      unsigned char r
	      )
{

    memcpy (outCoord, CurrentOutCoord, 9*sizeof(double));

    if (tr || t)
    {
	// translation 
	memcpy (outCoord, inCoord, 3*sizeof(double));
    }

    if (tr || r)
    {
	// rotation 
	memcpy (outCoord+3, inCoord+3, 3*sizeof(double));
    }

    if (false)  // if scale
    {
	// scale
	memcpy (outCoord+6, inCoord+6, 3*sizeof(double));
    }

    boxConstrain (outCoord);

    memcpy (CurrentOutCoord, outCoord, 9*sizeof(double));



    // Now figure out the plane
    osg::Matrix rotMat;
    rotMat.makeRotate 
	(iris::EulerToQuat (
			    outCoord[3], 
			    outCoord[4], 
			    outCoord[5]));

    // The plane is perpendicular to the transformed Y axis
    outPlane[0] = rotMat(1, 0);
    outPlane[1] = rotMat(1, 1);
    outPlane[2] = rotMat(1, 2);

    // The new outCoord xyz lies on the plane
    outPlane[3] =  - ( outCoord[0] * outPlane[0] +
		       outCoord[1] * outPlane[1] +
		       outCoord[2] * outPlane[2] );

    return 0;
} // end of absoluteMove




static void
coord9ToMatInv (double coord[9], osg::Matrix & matInv)
{
    osg::Matrix rot;
    rot.makeRotate (iris::EulerToQuat (coord[3], coord[4], coord[5]));

    // OK, this is ridiculous; there should be a transpose method
    // http://www.openscenegraph.org/projects/osg/wiki/Support/Maths/MatrixTransformations
    matInv.invert (rot);

    matInv.postMultScale 
	( osg::Vec3d (1/coord[6], 1/coord[7], 1/coord[8]) );

    matInv.preMultTranslate 
	( osg::Vec3d (-coord[0], -coord[1], -coord[2]) );
}  // end of coord9ToMatInv


static void
coord9ToMat (double coord[9], osg::Matrix & mat)
{
    mat.makeRotate 
	(iris::EulerToQuat (coord[3], coord[4], coord[5]));
    mat.preMultScale 
	( osg::Vec3d (coord[6], coord[7], coord[8]) );
    mat.postMultTranslate 
	( osg::Vec3d (coord[0], coord[1], coord[2]) );
} // coord9ToMat

static void
printMatrix (osg::Matrix & mat, char *comment)
{
    fprintf (stderr, "%s\n", comment);
    fprintf (stderr, "Matrix:\n");
    double *m = mat.ptr();
    for (int row = 0; row < 4; row++)
    {
	fprintf (stderr, "        ");
	for (int col = 0; col < 4; col++)
	{
	    fprintf (stderr, " %g ", *m);
	    m++;
	}
	fprintf (stderr, "\n");
    }

    fprintf (stderr, "\n");


} // end of printMatrix

static int
deriveOutputs (
	       double inCoord[9], 
	       unsigned char rtButton, 
	       unsigned char tButton, 
	       unsigned char rButton, 
	       double outCoord[9], 
	       double outPlane[4]
	       ) 
{
    // Here is where all the real computation is done

    // dtkMatrix - A Couple of Observations:
    //
    // Note that in dtkMatrix, the last ROW is the one
    // with zeros, and the last COL has the translations.
    // So points are column vectors and you transform a point
    // by have the matrix on the left and the pt on the right.
    // And if you multiply two matrices, the order of
    // transformations is right to left.
    // 
    // See the method element(row,col) for confirmation of this 
    // arrangement.
    //
    // The matrix whose mult method is being called is on
    // the right, and the matrix which is an arg to the
    // method is on the left.  The matrix on the right
    // is the first transformation, the matrix on the left
    // is the second.
    //
    //////////////////////////////////////////
    //
    // osg::Matrix - A Few More Observations:
    // 
    // This class seems to regard the matrices as being the transpose
    // of those in dtkMatrix.  Look at the () operator, which explicitly
    // names the first index as the row and the second index
    // as the column. Then look at internals of makeTranslate and
    // similar methods:  we get the matrix arrangement with the
    // last COL having the zeros and the last ROW being the translation.
    // So when you multiply two matrices, the order of transformations
    // is left to right.
    //
    // The preMult method takes the matrix which is the arg and 
    // puts it on the left, while the matrix at the current object
    // is on the right.  The result goes into the current object.
    // So A.preMult(B) is like A = B*A.
    //
    // The "make" methods replace the entire matrix.  So, for example,
    // A.makeRotate (...) replaces the matrix with a pure rotation matrix.
    //
    // The methods with names like preMultScale do the obvious things.
    // preMultScale( Vec3 v ); makes a scale matrix from v and 
    // multiplies that matrix with the current objects matrix with
    // the scale matrix on the left.  So A.preMultScale (v) is like
    // A = B.makeScale(v) * A 
    //
    // take a look at this link- includes discussion of transpose
    // http://www.openscenegraph.org/projects/osg/wiki/Support/Maths/MatrixTransformations

    static double StartInCoord[9] = {0,0,0,0,0,0,1,1,1};
    static double StartOutCoord[9] = {0,0,0,0,0,0,1,1,1};
    static osg::Matrix StartRotOut;
    static osg::Matrix StartRotInInv;


    if ( ! (rtButton || tButton || rButton) )
    {
	// buttons are up; don't do anything
	InMove = false;
	return 1;
    }

    // at least one button is down

    if (Absolute)
    {
	return absoluteMove (inCoord, outCoord, outPlane,
			     rtButton, tButton, rButton );
    }

    if ( ! InMove )
    {
	// grab the start coords
	memcpy (StartInCoord, inCoord, 9*sizeof(double));
	memcpy (StartOutCoord, CurrentOutCoord, 9*sizeof(double));


	osg::Matrix rot;
	rot.makeRotate 
	    (iris::EulerToQuat (inCoord[3], inCoord[4], inCoord[5]));
	// OK, this is ridiculous; there should be a transpose method
	// http://www.openscenegraph.org/projects/osg/wiki/Support/Maths/MatrixTransformations
	StartRotInInv.invert (rot);
	// printMatrix (StartRotInInv, "StartRotInInv:");

	StartRotOut.makeRotate 
	    (iris::EulerToQuat (StartOutCoord[3], StartOutCoord[4], StartOutCoord[5]));
	// printMatrix (StartRotOut, "StartRotOut:");

	InMove = true;
	return 1;
    }

    InMove = true;

    /*
      If we get here, it's because we are in the middle of a move

      We going to calculate a move based on inCoord relative to StartInCoord
      then apply that relative move to StartOutCoord to produce the current
      outCoord and outPlane.
    */

    // OK, we do the rotation and the translation separately

    // first the rotation:
    // The idea is that we find a "difference" transformation
    // that transforms the start input rotation to the 
    // current input rotation.
    //
    // Then we apply this difference rotation to the start output rotation
    // to get the current output rotation.
    //
    // D * Si = Ci
    // D = Ci*inverse(Si)
    // then D * So = Co
        
    bool doRotate = rtButton || rButton;
    bool doTranslate = rtButton || tButton;
    bool doScale = false;

    if (Verbose)
    {
	fprintf (stderr, "%s: rtButton %d  rButton %d  tButton %d\n",
		 ProgName, rtButton, rButton, tButton);
                
	fprintf (stderr, "%s: doRotate %d  doTranslate %d  doScale%d\n",
		 ProgName, doRotate, doTranslate, doScale);
    }


    if ( doRotate )
    {
	osg::Matrix currRotIn;
	currRotIn.makeRotate 
	    (iris::EulerToQuat (inCoord[3], inCoord[4], inCoord[5]));
	// printMatrix (currRotIn, "currRotIn");

	osg::Matrix diffMat (StartRotInInv);
	diffMat.postMult (currRotIn);
	// printMatrix (diffMat, "diffMat");

	osg::Matrix currRotOut (StartRotOut);
	currRotOut.postMult (diffMat);
	// printMatrix (currRotOut, "currRotOut");

	// get the output hpr 
	double hpr[3];
	iris::MatrixToEuler ( currRotOut, hpr+0, hpr+1, hpr+2 );
	outCoord[3] = hpr[0];
	outCoord[4] = hpr[1];
	outCoord[5] = hpr[2];


	// The plane is perpendicular to the transformed Y axis
	outPlane[0] = currRotOut(1, 0);
	outPlane[1] = currRotOut(1, 1);
	outPlane[2] = currRotOut(1, 2);


    }
    else
    {
	// HPR does not change
	memcpy (outCoord+3, CurrentOutCoord+3, sizeof(double) * 3);
	memcpy (outPlane, CurrentOutPlane, 3*sizeof(double));
    }
    // OK, we're done with HPR


    if ( doTranslate )
    {
	// XYZ is basically the same except that we don't have 
	// to fool around with matrices.
        
	for (int i = 0; i < 3; i++)
	{
	    outCoord[i] = StartOutCoord[i] + 
		(inCoord[i] - StartInCoord[i]);
	}
	// If requested, restrict xyz to box
	boxConstrain (outCoord);
    }
    else
    {
	// XYZ does not change
	memcpy (outCoord, CurrentOutCoord, sizeof(double) * 3);
    }

    if ( doScale )
    {
	for (int i = 5; i < 8; i++)
	{
	    outCoord[i] = StartOutCoord[i] * 
		(inCoord[i] / StartInCoord[i]);
	}
    }
    else
    {
	// scale does not change
	memcpy (outCoord+6, CurrentOutCoord+6, sizeof(double) * 3);
    }


    // We just have to get the final plane equation coefficient from 
    // what we've already calculated..

    // The new outCoord xyz lies on the plane
    outPlane[3] =  - ( outCoord[0] * outPlane[0] +
		       outCoord[1] * outPlane[1] +
		       outCoord[2] * outPlane[2] );


    memcpy (CurrentOutCoord, outCoord, 9*sizeof(double));
    memcpy (CurrentOutPlane, outPlane, 4*sizeof(double));

    return 0;
}  // end of deriveOutputs



static int
writeOutputs (double outCoord[9], double outPlane[4])
{
    // Send the calculated outputs to their various destinations.

    if (Verbose)
    {
	fprintf (stderr, "%s: Output coord:  %g %g %g   %g %g %g\n", 
		 ProgName,
		 outCoord[0], outCoord[1], outCoord[2], 
		 outCoord[3], outCoord[4], outCoord[5] );
	fprintf (stderr, "%s: Output plane:  %g %g %g %g\n", 
		 ProgName,
		 outPlane[0], outPlane[1], outPlane[2], outPlane[3] );
	fprintf (stderr, "\n\n");
    }


    int rtn = 0;

    osg::Matrix outMat;
    bool outMatValid = false;

    if (OutTransformShm != NULL)
    {
	// convert outCoord to matrix and output to shm
	coord9ToMat (outCoord, outMat);
	OutTransformShm->write (outMat.ptr());
	outMatValid = true;
    }



    if (OutPlaneShm != NULL)
    {
	if (OutPlaneShm->write (outPlane))
	{
	    fprintf (stderr, 
		     "%s: Error writing plane shared memory.\n", 
		     ProgName);
	    rtn += -1;
	}
    }

    bool flush = false;
#if 0
    if (OutCoordNodeName[0])
    {
	fprintf (stdout, "DCS %s %g %g %g %g %g %g\n", 
		 OutCoordNodeName, 
		 outCoord[0], outCoord[1], outCoord[2],
		 outCoord[3], outCoord[4], outCoord[5]  );
	flush = true;
    }
#endif

    if (OutMatNodeName[0])
    {
	// convert outCoord to matrix and output IRIS command
	if ( ! outMatValid )
	{
	    coord9ToMat (outCoord, outMat);
	}
	double * m = outMat.ptr();
	fprintf (stdout, 
		 "MATRIX %s  "
		 "%g %g %g %g  %g %g %g %g  %g %g %g %g  %g %g %g %g\n", 
		 OutMatNodeName, 
		 m[0], m[1], m[2], m[3],
		 m[4], m[5], m[6], m[7],
		 m[8], m[9], m[10], m[11],
		 m[12], m[13], m[14], m[15]
		 );
	flush = true;
    }

    if (OutPlaneUniNodeName[0])
    {
	fprintf (stdout, "UNIFORM %s %s vec4 1 %g %g %g %g\n",
		 OutPlaneUniNodeName, OutPlaneUniVarName,
		 outPlane[0], outPlane[1], outPlane[2], outPlane[3] );
	flush = true;
    }

    if (flush)
    {
	fflush (stdout);
    }


    return rtn;
} // end of writeOutputs


static int
getInputs (
	   double inCoord[9], 
	   unsigned char & rtButton, 
	   unsigned char & tButton, 
	   unsigned char & rButton)
{
    static double LastCoord[9] = 
	{HUGE, HUGE, HUGE, HUGE, HUGE, HUGE, HUGE, HUGE, HUGE};
    static unsigned char LastRT = -1;
    static unsigned char LastT  = -1;
    static unsigned char LastR  = -1;

    // Get the changing inputs
    rtButton = 0;
    if (InTRButtonShm != NULL)
    {
	if (InTRButtonShm->read (&rtButton))
	{
	    fprintf (stderr, "%s: Error reading TR button shared memory.\n",
		     ProgName);
	    return -1;
	}
    }

    tButton = 0;
    if (InTButtonShm != NULL)
    {
	if (InTButtonShm->read (&tButton))
	{
	    fprintf (stderr, 
		     "%s: Error reading translate button shared memory.\n",
		     ProgName);
	    return -1;
	}
    }

    rButton = 0;
    if (InRButtonShm != NULL)
    {
	if (InRButtonShm->read (&rButton))
	{
	    fprintf (stderr, 
		     "%s: Error reading rotate button shared memory.\n",
		     ProgName);
	    return -1;
	}
    }




    double matvals[16];
    if (InTransformShm->read (matvals))
    {
	fprintf (stderr, "%s: Error reading input coordinates.\n",
		 ProgName);
	return -1;
    }
	
    osg::Matrix inMat(matvals);

    osg::Vec3d trans = inMat.getTrans ();
    inCoord[0] = trans[0];
    inCoord[1] = trans[1];
    inCoord[2] = trans[2];
	
    double hpr[3];
    iris::MatrixToEuler (inMat, hpr+0, hpr+1, hpr+2);
    inCoord[3] = hpr[0];
    inCoord[4] = hpr[1];
    inCoord[5] = hpr[2];
	
    osg::Vec3d scale = inMat.getScale ();
    inCoord[6] = scale[6];
    inCoord[7] = scale[7];
    inCoord[8] = scale[8];

    bool inputsChanged = 
	(LastCoord[0] != inCoord[0]) ||
	(LastCoord[1] != inCoord[1]) ||
	(LastCoord[2] != inCoord[2]) ||
	(LastCoord[3] != inCoord[3]) ||
	(LastCoord[4] != inCoord[4]) ||
	(LastCoord[5] != inCoord[5]) ||
	(LastRT != rtButton) ||
	(LastT  !=  tButton) ||
	(LastR  !=  rButton)                 ;

    memcpy (LastCoord, inCoord, 9*sizeof(double));
    LastRT = rtButton;
    LastT  =  tButton;
    LastR  =  rButton;

    if (inputsChanged && Verbose )
    {
	static bool noButtonsFirst = true;
	bool printInputs = false;

	if ( ! (rtButton || tButton || rButton) )
	{
	    if (noButtonsFirst)
	    {
		printInputs = true;
		noButtonsFirst = false;
	    }
	}
	else
	{
	    noButtonsFirst = true;
	    printInputs = true;
	}

                        
	if (printInputs)
	{
	    fprintf (stderr, "%s: Input coord:  %g %g %g   %g %g %g\n", 
		     ProgName,
		     inCoord[0], inCoord[1], inCoord[2], 
		     inCoord[3], inCoord[4], inCoord[5] );
	    fprintf (stderr, "%s: Buttons:  %d %d %d\n", 
		     ProgName, rtButton, tButton, rButton);
	}
    }


    return inputsChanged ? 0 : 1;

} // end of getInputs

// if always is set to true then always update even if no new inputs
static int
update (bool always=false)
{

    unsigned char rtButton, tButton, rButton;
    double inCoord[9], outCoord[9], outPlane[4];
        
    // Get the changing inputs
    int ret = getInputs (inCoord, rtButton, tButton, rButton) ;
    if ( (ret == -1) || (!always && ret == 1))
    {
	// failure or no change
	return -1;
    }
    
    // Calculate the outputs from the inputs
    if (deriveOutputs (inCoord, rtButton, tButton, rButton, outCoord, outPlane))
    {
	// no change or button up
#if 0
	if (Verbose)
	{
	    fprintf (stderr, "%s: No Outputs derived.\n", ProgName);
	}
#endif
	return 0;
    }

    return writeOutputs (outCoord, outPlane);

} // end of update

static void
processFifo()
{
    if (!fifo) return ;
    std::string line ;
    while (fifo->readLine(&line))
    {
	std::vector<std::string> vec ;
	vec = iris::ParseString(line) ;
	if (vec.size()>0)
	{
	    if (!iris::IsSubstring("reset", vec[0], 3) || vec.size() < 3 || vec.size() > 10)
	    {
		dtkMsg.add (DTKMSG_WARNING,"invalid fifo command: \"%s\"\n",line.c_str()) ;
		return ;
	    }

	    double coord[9] ;
	    vec.erase(vec.begin()) ;
	    if (iris::VectorOfStringsToCoord(vec, coord))
	    {
		if (Verbose)
		{
		    
		    fprintf(stderr,"%s: RESET fifo command: old CurrentOutCoord = %.17g %.17g %.17g,  %.17g %.17g %.17g, %.17g %.17g %.17g\n",
			    ProgName, CurrentOutCoord[0], CurrentOutCoord[1], CurrentOutCoord[2], CurrentOutCoord[3], CurrentOutCoord[4], CurrentOutCoord[5], CurrentOutCoord[6], CurrentOutCoord[7], CurrentOutCoord[8]) ;
		    fprintf(stderr,"%s: RESET fifo command: new initial coord  = %.17g %.17g %.17g,  %.17g %.17g %.17g, %.17g %.17g %.17g\n",
			    ProgName, coord[0], coord[1], coord[2], coord[3], coord[4], coord[5], coord[6], coord[7], coord[8]) ;
#if 0
		    osg::Matrix mat ;
		    coord9ToMat(coord, mat) ;
		    printMatrix(mat, "") ;
#endif
		}
		// set new initial coord and plane
		memcpy (CurrentOutCoord, coord, 9*sizeof(double));
		
		// Now figure out the plane
		osg::Matrix rotMat;
		rotMat.makeRotate 
		    (iris::EulerToQuat (
					CurrentOutCoord[3], 
					CurrentOutCoord[4], 
					CurrentOutCoord[5]));
		
		// The plane is perpendicular to the transformed Y axis
		CurrentOutPlane[0] = rotMat(1,0);
		CurrentOutPlane[1] = rotMat(1,1);
		CurrentOutPlane[2] = rotMat(1,2);
		
		// The new outCoord xyz lies on the plane
		CurrentOutPlane[3] =  - ( CurrentOutCoord[0] * CurrentOutPlane[0] +
					  CurrentOutCoord[1] * CurrentOutPlane[1] +
					  CurrentOutCoord[2] * CurrentOutPlane[2] );

		// force an update from a new starting point. the true
		// boolean to update() specifies that it should update even
		// if no data have changed.

		// the first update, with InMove set to false, fakes a
		// button press and InMove gets set true
		InMove = false ;
		update(true) ;

		// the second update, with InMove now set to false, fakes new data.
		update(true) ;
	    }
	    else
	    {
		dtkMsg.add (DTKMSG_WARNING,"invalid fifo parameters: \"%s\"\n",line.c_str()) ;
		return ;
	    }
	}
    }
} // end of processFifo



/////////////////////////////////////////////////

static dtkSharedMem * 
openTransformShm (
		  char *shmName
		  )
{

    // try to open existing coord file
    dtkSharedMem * shm = NULL;

    shm = new dtkSharedMem (16*sizeof(double), shmName, 0);
    if ( shm->isInvalid () )
    {
	delete shm;
	fprintf (stderr, 
		 "%s: Unable to open shared memory transformation %s.\n"
		 "     Note that existing files must hold 16 doubles (matrix).\n",
		 ProgName, shmName);
	shm = NULL ;
    }
    return shm;
} // end of openTransformShm


/////////////////////////////////////////////////

// This section does initialization based on command line 
// arguments and/or defaults

static int
setupInputs (
	     // char *inCoordShmName, 
	     char *inTransformShmName, 
	     char *inTRButtonShmName, 
	     char *inTButtonShmName, 
	     char *inRButtonShmName, 
	     char *inCtrlShmName, 
	     char *ctrlID
	     )
{

    if ( (InTransformShm = openTransformShm (inTransformShmName)) == NULL)
    {
	return -1;
    }

    if ( (inTRButtonShmName[0] == 0) &&
	 (inTButtonShmName[0] == 0)  &&
	 (inRButtonShmName[0] == 0)      )
    {
	fprintf (stderr, 
		 "%s: At least one button must be "
		 "specified on the command line.\n",
		 ProgName);
	return -1;
    }

    if (inTRButtonShmName[0])
    {
	InTRButtonShm =
	    new dtkSharedMem (sizeof(unsigned char), inTRButtonShmName);
	if (InTRButtonShm->isInvalid())
	{
	    fprintf (stderr, "%s: Unable to open shared memory %s.\n",
		     ProgName, inTRButtonShmName);
	    return -1;
	}
    }

    if (inTButtonShmName[0])
    {
	InTButtonShm =
	    new dtkSharedMem (sizeof(unsigned char), inTButtonShmName);
	if (InTButtonShm->isInvalid())
	{
	    fprintf (stderr, "%s: Unable to open shared memory %s.\n",
		     ProgName, inTButtonShmName);
	    return -1;
	}
    }

    if (inRButtonShmName[0])
    {
	InRButtonShm =
	    new dtkSharedMem (sizeof(unsigned char), inRButtonShmName);
	if (InRButtonShm->isInvalid())
	{
	    fprintf (stderr, "%s: Unable to open shared memory %s.\n",
		     ProgName, inRButtonShmName);
	    return -1;
	}
    }




    if (inCtrlShmName[0])
    {

	InCtrlShm = new iris::ShmString (inCtrlShmName);
	if (InCtrlShm->isInvalid())
	{
	    fprintf (stderr, "%s: Unable to open shared memory %s.\n",
		     ProgName, inCtrlShmName);
	    return -1;
	}
        
	MyCtrlIDStr = ctrlID;

    }  // end of section for setting up control ID shm

    return 0; // success
} // end of setupInputs

static int
setupCoord (char *initCoord[9], bool absolute)
{

    for (int i = 0; i < 9; i++)
    {
	if (sscanf (initCoord[i], "%lf", CurrentOutCoord+i) != 1)
	{
	    fprintf (stderr, "%s: Invalid initial coord %d: %s .\n",
		     ProgName, i, initCoord[i]);
	    return -1;
	}
    }


    Absolute = absolute;

    return 0; // success
} // end of setupCoord


static int
setupBox (
	  bool boxConst, 
	  // bool boxAdj, 
	  char *boxStr[2][3]
	  ) 
{

#if 0
    if (boxConst && boxAdj)
    {
	fprintf (stderr, 
		 "%s: Cannot specify both boxConstraint and "
		 "boxAdjust command line arguments.\n", ProgName);
	return -1;
    }

    if ( ! (boxConst || boxAdj) )
    {
	return 0;
    }

    BoxAdjust = boxAdj;
#else
    if ( ! boxConst )
    {
	return 0;
    }
#endif

    BoxConstrain = boxConst;

    for (int i = 0; i < 2; i++)
    {
	for (int j = 0; j < 3; j++)
	{
	    if (sscanf (boxStr[i][j], "%lf", &(Box[i][j])) != 1)
	    {
		fprintf (stderr, 
			 "%s: Unable to read Box command line "
			 "argument (%s).\n", ProgName, boxStr[i][j]);
		return -1;
	    }
	}
    }
                
    return 0;
}  // end of setupBox

static int
setupFifo ( char *fifoName ) 
{
    if (fifoName[0] != '\0')
    {
	fifo = new iris::FifoReader(fifoName) ;
	if ( ! fifo->open())
	{
	    dtkMsg.add (DTKMSG_ERROR,"unable to open fifo \"%s\"\n",fifoName) ;
	    delete fifo ;
	    fifo = NULL ;
	    return -1 ;
	}
	else
	{
	    fifo->unlinkOnExit() ;
	}
    }
    return 0 ;
}  // end of setupFifo

static int
setupOutputs (
	      char *outTransformShmName, 
	      // char *outCoordShmName, 
	      //char *outCoordNodeName, 
	      // char *outMatShmName, 
	      char *outMatNodeName, 
	      char *outPlaneShmName, 
	      char *outPlaneUniNodeName,
	      char *outPlaneUniVarName
	      )
{


    // At least one of the outputs has to be specified
    if ( ! (outTransformShmName[0]  ||
	    //outCoordNodeName[0] ||
	    outMatNodeName[0] ||
	    outPlaneShmName[0]  ||
	    outPlaneUniNodeName[0] ) )
    {
	fprintf (stderr, 
		 "%s: At least one of the outputs must be specified.\n", 
		 ProgName);
	return -1;
    }

    //strcpy (OutCoordNodeName, outCoordNodeName);
    strcpy (OutMatNodeName, outMatNodeName);
    strcpy (OutPlaneUniNodeName, outPlaneUniNodeName);
    strcpy (OutPlaneUniVarName, outPlaneUniVarName);

    if (outTransformShmName[0])
    {
	if ( ( OutTransformShm = openTransformShm (outTransformShmName)) == NULL )
	{
	    return -1;
	}
    }


    if (outPlaneShmName[0])
    {
	OutPlaneShm = 
	    new dtkSharedMem (4*sizeof(double), outPlaneShmName);
	if (OutPlaneShm->isInvalid())
	{
	    fprintf (stderr, 
		     "%s: Unable to open shared memory %s.\n",
		     ProgName, outPlaneShmName);
	    return -1;
	}
    }


    return 0; // success
} // end of setupOutputs

static int
setupSleep ( char * nMicroSecSleep )
{
    int m;

    if (nMicroSecSleep[0])
    {
	if (sscanf (nMicroSecSleep, "%d", &m) != 1)
	{
	    fprintf (stderr, "%s: Invalid usleep argument %s.\n",
		     ProgName, nMicroSecSleep);
	    return -1;
	}
	SleepDurationUSec = m;
    }
    else
    {
	SleepDurationUSec = iris::GetUsleep ();
    }

    return 0;
}  // setupSleep



static int
writePID (char *fn)
{
    FILE *outFP;

    if ( (outFP = fopen (fn, "w")) == NULL)
    {
	fprintf (stderr, "%s: Unable to open PID file %s.\n",
		 ProgName, fn);
	return -1;
    }

    fprintf (outFP, "%d\n", getpid());

    fclose (outFP);

    return 0;
} // end of writePID

static int
checkParamConflict ()
{


    // if (InitCoordSpecified && (BoxConstrain || BoxAdjust) )
    if (InitCoordSpecified && BoxConstrain)
    {
	if (
	    (CurrentOutCoord[0] < Box[0][0]) ||
	    (CurrentOutCoord[1] < Box[0][1]) ||
	    (CurrentOutCoord[2] < Box[0][2]) ||
	    (CurrentOutCoord[0] > Box[1][0]) ||
	    (CurrentOutCoord[1] > Box[1][1]) ||
	    (CurrentOutCoord[2] > Box[1][2])    )
	{
	    fprintf (stderr, 
		     "%s: Specified initial coordinate is "
		     "outside of specified box.\n", 
		     ProgName);
	    return -1;
	}
    }

    return 0;
} // end of checkParamConflict

// End of initialization section

/////////////////////////////////////////////////

static void
usage ()
{


    fprintf (stderr, 
	     "\n"
	     "Usage:   hev-relativeMove [ options ]\n"
	     "\n"
	     "    Options:\n"
	     "       --inShm          shmName      (default: wandMatrix)\n"
	     "       --inTRButtonShm  shmName      (default: idea/buttons/left)\n"
	     "       --inTButtonShm   shmName      (default: idea/buttons/right)\n"
	     "       --inRButtonShm   shmName\n"
	     "       --initCoord      X Y Z [H P R [Xscale [Yscale Zscale]]] \n"
	     "                                     (default: 0 0 0 0 0 0 1 1 1)\n"
	     "       --outShm         shmName\n"
	     "       --outPlaneShm    shmName\n"
	     "       --outPlaneUni    nodeName uniformName\n"
	     "       --selectorShm    shmName      (default: idea/selector)\n"
	     "       --selectorStr    string       (default: hev-relativeMove)\n"
	     "       --absolute\n"
	     "       --usleep         t            (default: iris::GetUsleep())\n"
	     "       --pid            fileName\n"
	     "       --boxConstrain   Xmin Ymin Zmin   Xmax Ymax Zmax\n"
	     "       --verbose\n"
	     "       --fifo           fileName\n"
	     "       --help\n"
	     "\n"
	     "    Note: At least one of the --out options must be specified.\n"
	     "\n"
	     "    See man page for more details.\n"
	     "\n"
	     );

} // end of usage


////////////////////////////////////////////////////////////////////////

// This section parses the command line arguments

static void
badArgValue (char *optionName, char *val)
{
    fprintf (stderr, "%s: Option \"%s\" had bad value \"%s\".\n",
	     ProgName, optionName, val);
}  // end of badArgValue
	


static int
extractOption ( 
	       int argc, 
	       char **argv, 
	       int iArg, 
	       int *argMap, 
	       int minVals, 
	       int maxVals,
	       int &nVals, 
	       int &startVal)

{

    if (argMap != NULL)
    {
	argMap[iArg] = 1;
    }

    startVal = iArg + 1;
    nVals = 0;


    for (int i = 0; i < maxVals; i++)
    {


	// the variable tooFew signals whether there are fewer
	// than maxVals found

	// first check if we're going beyond the end of the arg array
	bool tooFew = (startVal+i) >= argc;
	if ( ! tooFew )
	{
	    if ( argMap != NULL )
	    {
		// and check if the next arg has already been taken
		// as an option
		tooFew |= (argMap[iArg+1+i] == 1);
	    }

	    // check if next arg starts with "--"
	    tooFew |= (strncmp (argv[startVal+i], "--", 2) == 0);
	}

	if (tooFew )
	{
	    if (i < minVals)
	    {
		// if there are fewer than minVals found
		fprintf (stderr, 
			 "%s: Error parsing option \"%s\"; "
			 "not enough values.\n", 
			 ProgName, argv[iArg]);
		return -1;
	    }
	    break;
	}


	nVals++;
	if (argMap)
	{
	    argMap[iArg+1+i] = 1;
	}
    }


    return 1;
}  // end of extractOption


int 
getOption (
	   int argc, 
	   char **argv, 
	   int *argMap,   // an array that keeps track of which options have been used
	   char *name, 
	   int minNameLen, 
	   int minNumArgs, 
	   int maxNumArgs,
	   int &nArgs, 
	   int &startArg)
{
    // return 0 if the argument was not matched
    // return 1 if the argument was matched 
    // return -1 on error


    for (int i = 0; i < argc; i++)
    {

	bool checkArg ;

	if (argMap != NULL)
	{
	    checkArg = ! argMap[i];
	}
	else
	{
	    checkArg = 1;
	}

	if (checkArg)
	{
	    // if argMap says this arg is unused

	    if (strncmp (argv[i], "--", 2) == 0)
	    {
		// first two chars are --


		int len = strlen (argv[i]+2);

		if ( (len >= minNameLen) && 
		     (len <= strlen (name) ) )
		{
		    // length of arg is within range

		    if (strncasecmp (argv[i]+2, 
				     name, len) == 0)
		    {
			// name matches

			if (argMap)
			{
			    argMap[i] = 1;
			}

			return extractOption (
					      argc, 
					      argv, 
					      i, 
					      argMap, 
					      minNumArgs, 
					      maxNumArgs,
					      nArgs, 
					      startArg);

		    }
		}
	    }
	}
    }

		
    return 0;
}  // end of getOption



static int
processRecognizedArgs (int argc, char **argv, int *argMap)
{
    int rtn;
    int nVal, startVal;


    // . . . . . . . . . . . . . . . . . . . . 
    // first check if we have --help
    rtn = getOption ( argc, argv, argMap, 
		      const_cast<char*>("help"), 4, 0, 0, nVal, startVal);
    if (rtn < 0)
    {
	return -1;
    }
    else if (rtn > 0)
    {
	usage ();
	exit (0);
    }
    // . . . . . . . . . . . . . . . . . . . . 



    char *fifoName = const_cast<char*>("") ;
    // char *inCoordShm = const_cast<char*>("wand");
    char *inTransformShm = const_cast<char*>("wandMatrix");
    // char *outCoordShm = const_cast<char*>("");
    char *outTransformShm = const_cast<char*>("");
    char *outCoordNode = const_cast<char*>("");
    // char *outMatShmName = const_cast<char*>("");
    char *outMatNodeName = const_cast<char*>("");
    char *outPlaneShm = const_cast<char*>("");
    char *outPlaneUni = const_cast<char*>("");
    char *outPlaneUniNodeName = const_cast<char*>("");
    char *outPlaneUniVarName  = const_cast<char*>("");
    char *inTRButtonShm = const_cast<char*>("idea/buttons/left");
    char *inTButtonShm = const_cast<char*>("idea/buttons/right");
    char *inRButtonShm = const_cast<char*>("");
    char *inCtrlShm = const_cast<char*>("idea/selector");
    char *ctrlID = const_cast<char*>("hev-relativeMove");
    bool absolute = false;
    char *nMicroSecSleep = const_cast<char*>("");
    char *initCoord[9] = { const_cast<char*>("0"), const_cast<char*>("0"), const_cast<char*>("0"), const_cast<char*>("0"), const_cast<char*>("0"), const_cast<char*>("0"), const_cast<char*>("1"), const_cast<char*>("1"), const_cast<char*>("1") };



    rtn = getOption ( argc, argv, argMap, 
		      const_cast<char*>("fifo"), 4, 1, 1, nVal, startVal);
    if (rtn < 0)
    {
	return -1;
    }
    else if (rtn > 0)
    {
	fifoName = argv[startVal];
    }

    rtn = getOption ( argc, argv, argMap, 
		      const_cast<char*>("inShm"), 4, 1, 1, nVal, startVal);
    if (rtn < 0)
    {
	return -1;
    }
    else if (rtn > 0)
    {
	inTransformShm = argv[startVal];
    }


    rtn = getOption ( argc, argv, argMap, 
		      const_cast<char*>("outShm"), 4, 1, 1, nVal, startVal);
    if (rtn < 0)
    {
	return -1;
    }
    else if (rtn > 0)
    {
	outTransformShm = argv[startVal];
    }


#if 0
    rtn = getOption ( argc, argv, argMap, 
		      const_cast<char*>("outCoordNode"), 9, 1, 1, nVal, startVal);
    if (rtn < 0)
    {
	return -1;
    }
    else if (rtn > 0)
    {
	outCoordNode = argv[startVal];
    }
#endif


    rtn = getOption ( argc, argv, argMap, 
		      const_cast<char*>("outMatrixNode"), 7, 1, 1, nVal, startVal);
    if (rtn < 0)
    {
	return -1;
    }
    else if (rtn > 0)
    {
	outMatNodeName = argv[startVal];
    }


    rtn = getOption ( argc, argv, argMap, 
		      const_cast<char*>("outPlaneShm"), 9, 1, 1, nVal, startVal);
    if (rtn < 0)
    {
	return -1;
    }
    else if (rtn > 0)
    {
	outPlaneShm = argv[startVal];
    }


    rtn = getOption ( argc, argv, argMap, 
		      const_cast<char*>("outPlaneUni"), 9, 2, 2, nVal, startVal);
    if (rtn < 0)
    {
	return -1;
    }
    else if (rtn > 0)
    {
	outPlaneUniNodeName = argv[startVal];
	outPlaneUniVarName  = argv[startVal+1];
    }


    rtn = getOption ( argc, argv, argMap, 
		      const_cast<char*>("inTRButtonShm"), 4, 1, 1, nVal, startVal);
    if (rtn < 0)
    {
	return -1;
    }
    else if (rtn > 0)
    {
	inTRButtonShm = argv[startVal];
    }


    rtn = getOption ( argc, argv, argMap, 
		      const_cast<char*>("inTButtonShm"), 4, 1, 1, nVal, startVal);
    if (rtn < 0)
    {
	return -1;
    }
    else if (rtn > 0)
    {
	inTButtonShm = argv[startVal];
    }


    rtn = getOption ( argc, argv, argMap, 
		      const_cast<char*>("inRButtonShm"), 4, 1, 1, nVal, startVal);
    if (rtn < 0)
    {
	return -1;
    }
    else if (rtn > 0)
    {
	inRButtonShm = argv[startVal];
    }


    rtn = getOption ( argc, argv, argMap, 
		      const_cast<char*>("selectorShm"), 10, 1, 1, nVal, startVal);
    if (rtn < 0)
    {
	return -1;
    }
    else if (rtn > 0)
    {
	inCtrlShm = argv[startVal];
    }


    rtn = getOption ( argc, argv, argMap, 
		      const_cast<char*>("selectorStr"), 10, 1, 1, nVal, startVal);
    if (rtn < 0)
    {
	return -1;
    }
    else if (rtn > 0)
    {
	ctrlID = argv[startVal];
    }


    rtn = getOption ( argc, argv, argMap, 
		      const_cast<char*>("absolute"), 3, 0, 0, nVal, startVal);
    if (rtn < 0)
    {
	return -1;
    }
    else if (rtn > 0)
    {
	absolute = true;
    }



    rtn = getOption ( argc, argv, argMap, 
		      const_cast<char*>("usleep"), 3, 1, 1, nVal, startVal);
    if (rtn < 0)
    {
	return -1;
    }
    else if (rtn > 0)
    {
	nMicroSecSleep = argv[startVal];
    }


    rtn = getOption ( argc, argv, argMap, 
		      const_cast<char*>("initCoord"), 5, 3, 9, nVal, startVal);
    if (rtn < 0)
    {
	return -1;
    }
    else if (rtn > 0)
    {
	InitCoordSpecified = true;
	if (nVal == 3)
	{
	    initCoord[0] = argv[startVal+0];
	    initCoord[1] = argv[startVal+1];
	    initCoord[2] = argv[startVal+2];
	}
	else if (nVal == 6)
	{
	    initCoord[0] = argv[startVal+0];
	    initCoord[1] = argv[startVal+1];
	    initCoord[2] = argv[startVal+2];
	    initCoord[3] = argv[startVal+3];
	    initCoord[4] = argv[startVal+4];
	    initCoord[5] = argv[startVal+5];
	}
	else if (nVal == 7)
	{
	    initCoord[0] = argv[startVal+0];
	    initCoord[1] = argv[startVal+1];
	    initCoord[2] = argv[startVal+2];
	    initCoord[3] = argv[startVal+3];
	    initCoord[4] = argv[startVal+4];
	    initCoord[5] = argv[startVal+5];
	    initCoord[6] = argv[startVal+6];
	    initCoord[7] = argv[startVal+6];
	    initCoord[8] = argv[startVal+6];
	}
	else if (nVal == 9)
	{
	    initCoord[0] = argv[startVal+0];
	    initCoord[1] = argv[startVal+1];
	    initCoord[2] = argv[startVal+2];
	    initCoord[3] = argv[startVal+3];
	    initCoord[4] = argv[startVal+4];
	    initCoord[5] = argv[startVal+5];
	    initCoord[6] = argv[startVal+6];
	    initCoord[7] = argv[startVal+7];
	    initCoord[8] = argv[startVal+8];
	}
	else
	{
	    fprintf (stderr, 
		     "%s: Bad number of values for "
		     "option --initCoord (%d).\n",
		     ProgName, nVal);
	    return -1;
	}

    }


    if (setupFifo ( fifoName ))
    {
	return -1;
    }

        
    if (setupInputs (inTransformShm, 
		     inTRButtonShm, inTButtonShm, inRButtonShm,
		     inCtrlShm, ctrlID))
    {
	return -1;
    }

    if (setupCoord (initCoord, absolute))
    {
	return -1;
    }

    if (setupOutputs (outTransformShm, 
		      //outCoordNode, 
		      outMatNodeName,
		      outPlaneShm, outPlaneUniNodeName, outPlaneUniVarName))
    {
	return -1;
    }

    if (setupSleep ( nMicroSecSleep ))
    {
	return -1;
    }

        
    bool boxConst = false;
    // bool boxAdj = false;
    char * boxStr[2][3] = { {NULL, NULL, NULL} , {NULL, NULL, NULL} };

    rtn = getOption ( argc, argv, argMap, 
		      const_cast<char*>("boxConstrain"), 4, 6, 6, nVal, startVal);
    if (rtn < 0)
    {
	return -1;
    }
    else if (rtn > 0)
    {
	boxConst = true;
	boxStr[0][0] = argv[startVal+0];
	boxStr[0][1] = argv[startVal+1];
	boxStr[0][2] = argv[startVal+2];
	boxStr[1][0] = argv[startVal+3];
	boxStr[1][1] = argv[startVal+4];
	boxStr[1][2] = argv[startVal+5];
    }


    if ( setupBox (boxConst, boxStr) )
    {
	return -1;
    }

    // .....................................................

    rtn = getOption ( argc, argv, argMap, 
		      const_cast<char*>("verbose"), 4, 0, 0, nVal, startVal);
    if (rtn < 0)
    {
	return -1;
    }
    else if (rtn > 0)
    {
	Verbose = true;
    }


    rtn = getOption ( argc, argv, argMap, 
		      const_cast<char*>("pid"), 3, 1, 1, nVal, startVal);
    if (rtn < 0)
    {
	return -1;
    }
    else if (rtn > 0)
    {
	if (writePID (argv[startVal]))
	{
	    return -1;
	}
    }


    return 0;
}  // end of processRecognizedArgs


static int
processCmdLineArgs (int argc, char **argv)
{

    for (int i = 1; i < argc; i++)
    {
	if (strlen(argv[i]) > MAX_NAME_SIZE)
	{
	    fprintf (stderr, 
		     "%s: Command line argument %d exceeds "
		     "maximum length (%d):\n"
		     "    %s\n",
		     ProgName, i, MAX_NAME_SIZE, argv[i]);
	    exit (-1);
	}
    }

    int *argMap = (int *) malloc (argc*sizeof(int));
    if (argMap == NULL)
    {
	fprintf (stderr, "%s: Unable to allocate sufficient memory.\n",
		 ProgName);
	exit (-1);
    }

    argMap[0] = 1;
    for (int i = 1; i < argc; i++)
    {
	argMap[i] = 0;
    }
 
    int rtn = processRecognizedArgs (argc, argv, argMap);

	
    if (rtn)
    {
	return -1;
    }

    if (checkParamConflict ())
    {
	return -1;
    }

    // Note that there are no non-option arguments
    int nUnused = 0;
    for (int i = 0; i < argc; i++)
    {
	if (argMap[i] == 0)
	{
	    nUnused++;
	}
    }

    if (nUnused > 0)
    {
	fprintf (stderr, "%s: Unrecognized command line argument%s:\n",
		 ProgName, (nUnused > 1) ? "s" : "");
	for (int i = 0; i < argc; i++)
	{
	    if (argMap[i] == 0)
	    {
		fprintf (stderr, "    \"%s\"\n", argv[i]);
	    }
	}
	fprintf (stderr, "\n");
	return -1;
    }
		

    return 0;
}  // end of processCmdLineArgs


// End of section for parsing the command line arguments
//
/////////////////////////////////////////////////////////////////////////////


static int
setup ()
{
    ProgName = strdup ("hev-relativeMove");


    return 0;
}  // end of setup



static void
sigHandler (int sig)
{

    if (Verbose)
    {
	fprintf (stderr, 
		 "\n----------> pid %d sigHandler received sig %d\n\n", 
		 getpid(), sig);
    }


    switch (sig)
    {
    case SIGUSR1:
	activate ();
	break;

    case SIGUSR2:
	deactivate ();
	break;

    }

		
}  // end of sigHandler

static void
setupSigHandlers ()
{
    signal (SIGUSR1, sigHandler);
    signal (SIGUSR2, sigHandler);
}  // end of setupSigHandlers

static void exitSignalCatcher(int sig)
{
    if (Verbose)
    {
	fprintf (stderr, 
		 "\n----------> pid %d exitSignalHandler received sig %d\n\n", 
		 getpid(), sig);
    }
    exit(0) ;
}

static void 
atExitFunc ()
{
    // removeTmpDir ();
    if (Verbose)
    {
	fprintf (stderr, "%s: About to exit.\n", ProgName);
    }
    if (fifo) 
    {
	delete fifo ;
    }
}  // end of atExitFunc


static int
writeInitialOutputs ()
{

    osg::Matrix rotMat;
    rotMat.makeRotate 
	(iris::EulerToQuat (
			    CurrentOutCoord[3], 
			    CurrentOutCoord[4], 
			    CurrentOutCoord[5] ) );

    // The plane is perpendicular to the transformed Y axis
    CurrentOutPlane[0] = rotMat(1,0);
    CurrentOutPlane[1] = rotMat(1,1);
    CurrentOutPlane[2] = rotMat(1,2);

    // The new outCoord xyz lies on the plane
    CurrentOutPlane[3] =  - ( CurrentOutCoord[0] * CurrentOutPlane[0] +
			      CurrentOutCoord[1] * CurrentOutPlane[1] +
			      CurrentOutCoord[2] * CurrentOutPlane[2] );

    return writeOutputs (CurrentOutCoord, CurrentOutPlane);

}  // writeInitialOutputs

main (int argc, char **argv)

{

    // This will cause all dtk messages to be written to stderr with
    // an appropriate prefix.
    dtkMsg.setFile (stderr);
    dtkMsg.setPreMessage ("hev-relativeMove: ");
    // dtkMsg.add (DTKMSG_ERROR, 
    //             "This is an example of a error msg < %d >\n", 3);


    atexit (atExitFunc);

    // catch the usual ones to exit
    iris::Signal(exitSignalCatcher);
    // and catch some special ones to activate/deactivate
    setupSigHandlers ();

    if (setup ())
    {
	exit (-1);
    }

    if (processCmdLineArgs (argc, argv))
    {
	usage ();
	exit (-1);
    }

    if (Verbose)
    {
	fprintf (stderr, 
		 "%s: INIT: curr coords = %f %f %f   %f %f %f\n",
		 ProgName,
		 CurrentOutCoord[0],
		 CurrentOutCoord[1],
		 CurrentOutCoord[2],
		 CurrentOutCoord[3],
		 CurrentOutCoord[4],
		 CurrentOutCoord[5]);
    }

    if (writeInitialOutputs ())
    {
	exit (-1);
    }

    // double sleepDurationSec = 1/10.0;
    // int sleepDurationUSec = (int)(1000000 * sleepDurationSec);


    while (Running)
    {
	// always read from the fifo even if not active
	processFifo() ;

	if ( active() )
	{
	    update ();
	}
	usleep (SleepDurationUSec);
    }

    exit (0);
}  // end of main


