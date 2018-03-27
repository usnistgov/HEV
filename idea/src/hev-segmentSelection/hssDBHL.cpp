
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dtk.h>
// #include <dosg/DOSGUtil.h>

#include "hev-segmentSelection.h"
#include "hssFSM.h"
#include "hssDBHL.h"
#include "hssOps.h"




static int SegmentInUse = -1;

static float Segment[PTSHM_MAX_NUM_SEGS][2][3];
static bool ValidSeg[PTSHM_MAX_NUM_SEGS];
static HLType SegBaseHL[PTSHM_MAX_NUM_SEGS];

static bool BillboardExists[PTSHM_MAX_NUM_SEGS];

static bool DisplayLenLabels = false;

static char LenLabelFmt[100] = "%g";

static double CoordScale = 1;  // this is used to scale coordinates 
                               // that are displayed to user or written
                               // to a file.

#define DEFAULT_LEN_LABEL_SIZE (0.027)
static double LenLabelSize = DEFAULT_LEN_LABEL_SIZE;

void
initSegs()
	{
	for (int i = 0; i < PTSHM_MAX_NUM_SEGS; i++)
		{
		ValidSeg[i] = false;
                SegBaseHL[i] = HL_NONE;
		BillboardExists[i] = false;
		writeSegShmValid (i, false);
		}
	}  // end of initSegs

void 
setLenLabelScale (double scale)
	{
	LenLabelSize = DEFAULT_LEN_LABEL_SIZE * scale;
	}  // end of setLenLabelScale

void 
displayLenLabels (bool disp, char *fmt)
	{
	DisplayLenLabels = disp;
        if (fmt != NULL)
                {
                strcpy (LenLabelFmt, fmt);
                }
	}  // end of displayLenLabels

void
setCoordScale (double scale)
        {
        CoordScale = scale;
        }  // end of setCoordScale

///////////////////////////////////////////////////////

static void
getNodeNames (int iSeg, char nodeNames[5][1000])
	{
	sprintf (nodeNames[0], "__hssSeg%04dEndPt0__", iSeg);
	sprintf (nodeNames[1], "__hssSeg%04dEndPt1__", iSeg);
	sprintf (nodeNames[2], "__hssSeg%04dLine__", iSeg);
	sprintf (nodeNames[3], "__hssSeg%04dCenter__", iSeg);
	sprintf (nodeNames[4], "__hssSeg%04dLabel__", iSeg);
	}  // end of getNodeNames


static void 
getHPR (float seg[2][3], float hpr[3])
	{
	double x = seg[1][0] - seg[0][0];
	double y = seg[1][1] - seg[0][1];
	double z = seg[1][2] - seg[0][2];

	double xy = sqrt (x*x + y*y);

	hpr[0] = (180.0/M_PI) * atan2 (y, x);

	hpr[2] = -(180.0/M_PI) *  atan2 (z, xy);

	hpr[1] = 0;
	}  // end of getHPR

static float
segLen (float seg[2][3])
	{
	return sqrt (
		((seg[0][0] - seg[1][0]) * (seg[0][0] - seg[1][0])) +
		((seg[0][1] - seg[1][1]) * (seg[0][1] - seg[1][1])) +
		((seg[0][2] - seg[1][2]) * (seg[0][2] - seg[1][2])) 
		    );
	}  // end of segLen


static void
moveGlyph (int iSeg, int whichPt)
	{
	char nodeNames[5][1000];
	float hpr[3], len, center[3];

	getNodeNames (iSeg, nodeNames);

	getHPR (Segment[iSeg], hpr);
	len = segLen (Segment[iSeg]);
	for (int i = 0; i < 3; i++)
		{
		center[i] = (Segment[iSeg][0][i] + Segment[iSeg][1][i]) / 2;

#if 0
fprintf (stderr, 
"i %d  center[i] %f   Segment[iSeg][0][i] %f    Segment[iSeg][1][i] %f\n",
i, center[i], Segment[iSeg][0][i], Segment[iSeg][1][i]);
#endif



		}



	sendSgeMsg ("DCS %s %g %g %g\n", nodeNames[whichPt], 
			Segment[iSeg][whichPt][0],
			Segment[iSeg][whichPt][1],
			Segment[iSeg][whichPt][2] );

	sendSgeMsg ("DCS %s %g %g %g  %g %g %g  %g 1 1\n", nodeNames[2],
			Segment[iSeg][0][0],
			Segment[iSeg][0][1],
			Segment[iSeg][0][2],
			hpr[0], 
			hpr[1], 
			hpr[2], 
			len );

	sendSgeMsg ("DCS %s %g %g %g\n", nodeNames[3],
			center[0],
			center[1],
			center[2] );

	if (DisplayLenLabels)
		{
#if 1
                char fmt[1000];
                sprintf (fmt, 
#if 0
                "TEXT %%s STRING \"%s\" ALIGNMENT CENTER_CENTER AXIS SCREEN\n", 
#else
                "TEXT %%s STRING \"%s\" ALIGNMENT CENTER_CENTER\n", 
#endif
                LenLabelFmt);
		sendSgeMsg (fmt, nodeNames[4], CoordScale*len);
#else
		sendSgeMsg ("TEXT %s STRING \"%g\"\n", nodeNames[4], len);
#endif
		}


	}  // end of moveGlyph


static void
addGlyphs (int iSeg)
	{

	// I'm going to assume that if we've gotten here, we don't need
	// to check if iSeg is in range or if ValidSeg[iSeg] is true.

	char nodeNames[5][1000];

	getNodeNames (iSeg, nodeNames);

	sendSgeMsg ("DCS %s\n", nodeNames[0]);
	sendSgeMsg ("ADDCHILD %s %s\n", nodeNames[0], OutCoordsNodeName);
	sendSgeMsg ("ADDCHILD %s %s\n", EndPtNodeName[0], nodeNames[0]);

	sendSgeMsg ("DCS %s\n", nodeNames[1]);
	sendSgeMsg ("ADDCHILD %s %s\n", nodeNames[1], OutCoordsNodeName);
	sendSgeMsg ("ADDCHILD %s %s\n", EndPtNodeName[0], nodeNames[1]);

	sendSgeMsg ("DCS %s\n", nodeNames[2]);
	sendSgeMsg ("ADDCHILD %s %s\n", nodeNames[2], OutCoordsNodeName);
	sendSgeMsg ("ADDCHILD %s %s\n", SegNodeName[0], nodeNames[2]);

	sendSgeMsg ("DCS %s\n", nodeNames[3]);
	sendSgeMsg ("ADDCHILD %s %s\n", nodeNames[3], OutCoordsNodeName);
	sendSgeMsg ("ADDCHILD %s %s\n", SegHandleNodeName[0], nodeNames[3]);


        sendSgeMsg (
#if 0
         "TEXT %s STRING \" \" SIZE %g ALIGNMENT CENTER_CENTER AXIS SCREEN\n", 
#else
         "TEXT %s STRING \" \" SIZE %g ALIGNMENT CENTER_CENTER\n", 
#endif
         nodeNames[4], LenLabelSize);

if ( ! BillboardExists[iSeg] )
	{
	sendSgeMsg ("BILLBOARD %s.BB\n", nodeNames[4]); 
	// sendSgeMsg ("GROUP %s.BB\n", nodeNames[4]); 
	BillboardExists[iSeg] = true;
	}
sendSgeMsg ("ADDCHILD %s.BB %s\n", nodeNames[4], nodeNames[3]); 
sendSgeMsg ("ADDCHILD %s %s.BB\n", nodeNames[4], nodeNames[4]); 


	moveGlyph (iSeg, 0);
	moveGlyph (iSeg, 1);
	}  // end of addGlyphs


///////////////////////////////////////////////////////////////////
//
// First the highlighting stuff.
//
// We're using the term "highlight" to mean both that an object is visually
// differentiated, and also to mean that the object is the one that is
// currently being manipulated.
//
// An object (a line segment representation, including the end points and
// mid point glyphs) can be in one of four visual states:
//    unhighlighted
//    user highlight
//    drag highlight
//    delete highlight
//
// The last two states are determined by whether and how the user is
// currently interacting with a line segment.  These states are dependent
// on the location of the wand and the state of the buttons.  At most one
// segment at a time can be in a drag or delete highlight state.
// There are only two types of things that can be highlighted: a segment 
// end point, and an entire segment.
//
// When a segment is not in drag or delete highlight, it is in one of the
// other two states.  When it comes out of drag or delete highlight it
// reverts to either unhighlighted or user highlight state.  Thus, the
// user highlight is more like unhighlight than the other two highlight
// states.
//
//
// In much of the code below, the term "highlight" is used to refer to
// drag and delete highlight, which "unhighlight" can be used to refer
// to user highlight as well as unhighlight.  This terminology is partly
// due to the fact that user highlighting was retro-fitted after the
// drag and delete highlighting was fully implemented.
//
// The term "base highlight" is used for either unhighlight or user highlight.
// Each segment has a base hightlight state.
//

// segment index of currently drag or delete highlighted segment
static int CurrentSegHL = -1;

// 0, 1 indicates an end point; 2 means entire segment
static int CurrentEndPtHL = -1;  

static HLType CurrentHLType = HL_NONE;


void
getCurrentHL (int &iSeg, int &whichPt, int &hlType)
	{

	iSeg = CurrentSegHL;
	whichPt = CurrentEndPtHL;
	hlType = CurrentHLType;
	}  // end of getCurrentHL

static void
removeGlyphs (int iSeg)
	{
	// I'm going to assume that if we've gotten here, we don't need
	// to check if iSeg is in range or if ValidSeg[iSeg] is true.

	if (CurrentHLType != HL_NONE)
		{
		unhighlight ();
		}

	char nodeNames[5][1000];

	getNodeNames (iSeg, nodeNames);
        int baseHL = SegBaseHL[iSeg];

	sendSgeMsg ("REMOVECHILD %s %s\n", EndPtNodeName[baseHL], nodeNames[0]);
	sendSgeMsg ("REMOVECHILD %s %s\n", nodeNames[0], OutCoordsNodeName);

	sendSgeMsg ("REMOVECHILD %s %s\n", EndPtNodeName[baseHL], nodeNames[1]);
	sendSgeMsg ("REMOVECHILD %s %s\n", nodeNames[1], OutCoordsNodeName);

	sendSgeMsg ("REMOVECHILD %s %s\n", SegNodeName[baseHL], nodeNames[2]);
	sendSgeMsg ("REMOVECHILD %s %s\n", nodeNames[2], OutCoordsNodeName);



	sendSgeMsg ("REMOVECHILD %s %s.BB\n", nodeNames[4], nodeNames[4]);
	sendSgeMsg ("REMOVECHILD %s.BB %s\n", nodeNames[4], nodeNames[3]);


	sendSgeMsg ("REMOVECHILD %s %s\n", SegHandleNodeName[baseHL], 
                                                                nodeNames[3]);
	sendSgeMsg ("REMOVECHILD %s %s\n", nodeNames[3], OutCoordsNodeName);




	}  // end of removeGlyphs

int 
switchSgeHL (int iSeg, int whichPt, HLType fromHL, HLType toHL)
        {

/*
JGH OK this code is going to assume that HLType values are actually
valid indices into the array
SegNodeName
EndPtNodeName
SegHandleNodeName
*/

        if (fromHL == toHL)
                {
                return 0;
                }

	char nodeNames[5][1000];
	getNodeNames (iSeg, nodeNames);

	if  ( (whichPt == -1) || (whichPt == 2) )
		{
                // fprintf (stderr, "highlighting  entire seg\n"); 
		// highlight end pts, segment, segment center
		sendSgeMsg ("ADDCHILD %s %s\n", 
			EndPtNodeName[toHL], nodeNames[0]);
		sendSgeMsg ("REMOVECHILD %s %s\n", 
			EndPtNodeName[fromHL], nodeNames[0]);

		sendSgeMsg ("ADDCHILD %s %s\n", 
			EndPtNodeName[toHL], nodeNames[1]);
		sendSgeMsg ("REMOVECHILD %s %s\n", 
			EndPtNodeName[fromHL], nodeNames[1]);

		sendSgeMsg ("ADDCHILD %s %s\n", 
			SegNodeName[toHL], nodeNames[2]);
		sendSgeMsg ("REMOVECHILD %s %s\n", 
			SegNodeName[fromHL], nodeNames[2]);

		sendSgeMsg ("ADDCHILD %s %s\n", 
			SegHandleNodeName[toHL], nodeNames[3]);
		sendSgeMsg ("REMOVECHILD %s %s\n", 
			SegHandleNodeName[fromHL], nodeNames[3]);
		}
	else
		{
                // fprintf (stderr, "highlighting  endpt %d\n", whichPt); 
		// highlight only the end pt
		sendSgeMsg ("ADDCHILD %s %s\n", 
			EndPtNodeName[toHL], nodeNames[whichPt]);
		sendSgeMsg ("REMOVECHILD %s %s\n", 
			EndPtNodeName[fromHL], nodeNames[whichPt]);
		}

        return 0;
        } // end of switchSgeHL




#if 1
int unhighlight ()
        {
	if (CurrentHLType == HL_NONE)
		{
		CurrentSegHL = -1;
		CurrentEndPtHL = -1;
		return 0;
		}

	if (CurrentSegHL == -1)
		{
		CurrentHLType = HL_NONE;
		CurrentEndPtHL = -1;
		return 0;
		}

	if ( (CurrentEndPtHL < -1) || (CurrentEndPtHL > 2) )
		{
		return -1;
		}

        int rtn = switchSgeHL (CurrentSegHL, CurrentEndPtHL, 
                               CurrentHLType, SegBaseHL[CurrentSegHL] );

	CurrentSegHL = -1;
	CurrentEndPtHL = -1;
	CurrentHLType = HL_NONE;

        return rtn;

        } // end of unhighlight

#else
int
unhighlight ()
	{
	if (CurrentHLType == HL_NONE)
		{
		CurrentSegHL = -1;
		CurrentEndPtHL = -1;
		return 0;
		}

	if (CurrentSegHL == -1)
		{
		CurrentHLType = HL_NONE;
		CurrentEndPtHL = -1;
		return 0;
		}

	if ( (CurrentEndPtHL < -1) || (CurrentEndPtHL > 2) )
		{
		return -1;
		}


	char nodeNames[5][1000];
	getNodeNames (CurrentSegHL, nodeNames);

	int hlNameIndex = (CurrentHLType == HL_DRAG) ? 1 : 2;

	if ( (CurrentEndPtHL == -1) || (CurrentEndPtHL == 2) )
		{
		// unhighlight all elements of segment
		sendSgeMsg ("REMOVECHILD %s %s\n", 
			EndPtNodeName[hlNameIndex], nodeNames[0]);
		sendSgeMsg ("ADDCHILD %s %s\n", 
			EndPtNodeName[0], nodeNames[0]);

		sendSgeMsg ("REMOVECHILD %s %s\n", 
			EndPtNodeName[hlNameIndex], nodeNames[1]);
		sendSgeMsg ("ADDCHILD %s %s\n", 
			EndPtNodeName[0], nodeNames[1]);

		sendSgeMsg ("REMOVECHILD %s %s\n", 
			SegNodeName[hlNameIndex], nodeNames[2]);
		sendSgeMsg ("ADDCHILD %s %s\n", 
			SegNodeName[0], nodeNames[2]);

		sendSgeMsg ("REMOVECHILD %s %s\n", 
			SegHandleNodeName[hlNameIndex], nodeNames[3]);
		sendSgeMsg ("ADDCHILD %s %s\n", 
			SegHandleNodeName[0], nodeNames[3]);
		}
	else
		{
		// unhighlight only the end pt
		sendSgeMsg ("REMOVECHILD %s %s\n", 
			EndPtNodeName[hlNameIndex], nodeNames[CurrentEndPtHL]);
		sendSgeMsg ("ADDCHILD %s %s\n", 
			EndPtNodeName[0], nodeNames[CurrentEndPtHL]);
		}


	CurrentSegHL = -1;
	CurrentEndPtHL = -1;
	CurrentHLType = HL_NONE;

	return 0;
	}  // end of unhighlight

#endif


#if 1
int
highlight (int iSeg, int whichPt, HLType toHL)
	{
	if (toHL == HL_NONE)
		{
		unhighlight ();
		return 0;
		}

	if ( (iSeg < 0) || (iSeg >= MaxSegments) )
		{
		return -1;
		}

	if ( ! ValidSeg[iSeg] )
		{
		return -1;
		}

	if ( (whichPt < -1) || (whichPt > 2) )
		{
		return -1;
		}

	unhighlight ();

	CurrentSegHL = iSeg;
	CurrentEndPtHL = whichPt;
	CurrentHLType = toHL;

        return switchSgeHL (iSeg, whichPt, SegBaseHL[iSeg], toHL);

        }  // end of highlight


#else

int 
highlight (int iSeg, int whichPt, HLType hlType)
	{

#if 0
fprintf (stderr, "highlight  %d  %d  %d\n", iSeg, whichPt, hlType);
fprintf (stderr, "    HL_DRAG %d    HL_DELETE %d    HL_NONE %d\n",
		HL_DRAG, HL_DELETE, HL_NONE);
#endif


	if (hlType == HL_NONE)
		{
// fprintf (stderr, "NONE branch %d\n", (hlType == HL_NONE));
		unhighlight ();
		return 0;
		}

	if ( (iSeg < 0) || (iSeg >= MaxSegments) )
		{
// fprintf (stderr, "branch a\n");
		return -1;
		}

	if ( ! ValidSeg[iSeg] )
		{
// fprintf (stderr, "branch b\n");
		return -1;
		}

	if ( (whichPt < -1) || (whichPt > 2) )
		{
// fprintf (stderr, "branch c\n");
		return -1;
		}

	unhighlight ();







// fprintf (stderr, "highlight  after unhighlight  whichPt = %d\n", whichPt);
	
	int hlNameIndex = (hlType == HL_DRAG) ? 1 : 2;

	char nodeNames[5][1000];
	getNodeNames (iSeg, nodeNames);

// fprintf (stderr, "            whichPt = %d\n", whichPt);
	if  ( (whichPt == -1) || (whichPt == 2) )
		{
// fprintf (stderr, "highlighting  entire seg\n"); 
		// highlight end pts, segment, segment center
		sendSgeMsg ("ADDCHILD %s %s\n", 
			EndPtNodeName[hlNameIndex], nodeNames[0]);
		sendSgeMsg ("REMOVECHILD %s %s\n", 
			EndPtNodeName[0], nodeNames[0]);

		sendSgeMsg ("ADDCHILD %s %s\n", 
			EndPtNodeName[hlNameIndex], nodeNames[1]);
		sendSgeMsg ("REMOVECHILD %s %s\n", 
			EndPtNodeName[0], nodeNames[1]);

		sendSgeMsg ("ADDCHILD %s %s\n", 
			SegNodeName[hlNameIndex], nodeNames[2]);
		sendSgeMsg ("REMOVECHILD %s %s\n", 
			SegNodeName[0], nodeNames[2]);

		sendSgeMsg ("ADDCHILD %s %s\n", 
			SegHandleNodeName[hlNameIndex], nodeNames[3]);
		sendSgeMsg ("REMOVECHILD %s %s\n", 
			SegHandleNodeName[0], nodeNames[3]);
		}
	else
		{
// fprintf (stderr, "highlighting  endpt %d\n", whichPt); 
		// highlight only the end pt
		sendSgeMsg ("ADDCHILD %s %s\n", 
			EndPtNodeName[hlNameIndex], nodeNames[whichPt]);
		sendSgeMsg ("REMOVECHILD %s %s\n", 
			EndPtNodeName[0], nodeNames[whichPt]);
		}

	CurrentSegHL = iSeg;
	CurrentEndPtHL = whichPt;
	CurrentHLType = hlType;
// fprintf (stderr, "exiting highlight\n");
	return 0;
	} // end of highlight
#endif





int
userHighlight (bool all, int iSeg, bool hlOn)
        {

        // unhighlight ();

        // JGH
        HLType newBaseHL = hlOn ? HL_USER : HL_NONE;

        if (all)
                {
                for (int i = 0; i < MaxSegments; i++)
                    {
                    if ( ValidSeg[i] )
                        {
	                if (i != CurrentSegHL)
                            {
                            switchSgeHL (i, -1, SegBaseHL[i], newBaseHL);
                            }
                        SegBaseHL[i] = newBaseHL;
                        }
                    }
                }
        else if ( (iSeg >= 0) && (iSeg < MaxSegments) && ValidSeg[iSeg] )
                {
	        if (iSeg != CurrentSegHL)
                        {
                        switchSgeHL (iSeg, -1, SegBaseHL[iSeg], newBaseHL);
                        }
                SegBaseHL[iSeg] = newBaseHL;
                }

        return -1;
        }  // userHighlight

///////////////////////////////////////////////////////////////////

int getFreeSegIndex ()
	{
	// look through list and get free slot

	for (int i = 0; i < MaxSegments; i++)
		{
		if ( ! ValidSeg[i] )
			{
			return i;
			}
		}

	return -1;
	}  // end getFreeSegIndex

int
deleteSeg (int iSeg)
	{

	if (Verbose)
		{
		fprintf (stderr, "%s: Deleting segment %d\n", ProgName, iSeg);
		}

	if ( (iSeg < 0) || (iSeg >= MaxSegments) )
		{
		return -1;
		}

	if ( ! ValidSeg[iSeg])
		{
		return -1;
		}

	if (iSeg == CurrentSegHL)
		{
		unhighlight ();
		}


	removeGlyphs (iSeg);

	ValidSeg[iSeg] = false;
	writeSegShmValid (iSeg, false);

	return 0;
	}  // end of removeSeg

int
getSeg (int iSeg, float seg[2][3])
	{
	if ( (iSeg < 0) || (iSeg >= MaxSegments) )
		{
		return -1;
		}

	if ( ! ValidSeg[iSeg])
		{
		return -1;
		}

	memcpy ( seg, Segment[iSeg], 6*sizeof(float));

	return 0;
	}  // end of getSeg


int
setSeg (int iSeg, float seg[2][3])
	{

	if ( (iSeg < 0) || (iSeg >= MaxSegments) )
		{
		return -1;
		}

	memcpy ( Segment[iSeg], seg, 6*sizeof(float));

	writeSegShm (iSeg, seg, CoordScale);

	if ( ! ValidSeg[iSeg])
		{
		if (Verbose)
			{
			fprintf (stderr, "%s: Creating segment %d.\n", 
				ProgName, iSeg);
			}
		ValidSeg[iSeg] = true;
		writeSegShmValid (iSeg, true);
		addGlyphs (iSeg);
		}
	else
		{	
		moveGlyph (iSeg, 0);
		moveGlyph (iSeg, 1);
		}

	return 0;
	}  // end of setSeg

int
setSegEndPt (int iSeg, int whichPt, float pt[3])
	{

	if ( (iSeg < 0) || (iSeg >= MaxSegments) )
		{
		return -1;
		}

	if ( ! ValidSeg[iSeg] )
		{
		return -1;
		}

	if ( (whichPt < 0) || (whichPt > 1) )
		{
		return -1;
		}

	
	memcpy (Segment[iSeg][whichPt], pt, 3*sizeof(float));
	writeSegShm (iSeg, whichPt, pt, CoordScale);
	
	moveGlyph (iSeg, whichPt);

	return 0;
	}  // end of setSegEndPt


static bool
lineCenterIsNear (float loc[3], int iSeg)
	{
	double dSq = 0;
	// check against center of iSeg
	for (int i = 0; i < 3; i++)
		{
		float center = (Segment[iSeg][0][i] + Segment[iSeg][1][i] ) / 2;
		dSq += (loc[i] - center) * (loc[i] - center) ;
		}

	return (dSq <= LnHotSpotDistSq);
	}  // end of lineCenterIsNear

static bool
endPtIsNear (float loc[3], int iSeg, int endPt)
	{
	double dSq = 0;
	// check against endPt of iSeg
	for (int i = 0; i < 3; i++)
		{
		dSq += (loc[i] - Segment[iSeg][endPt][i]) *
		       (loc[i] - Segment[iSeg][endPt][i]) ;
		}

	return (dSq <= PtHotSpotDistSq);
	}  // end of endPtIsNear


void
checkCursorNearFar (bool &near)
	{

	// first check to see if we are in proximity to currently 
	// highlighted thing.

	float cursorLoc[3];

	getStoredCursorLocation (cursorLoc);
	
	if (CurrentHLType != HL_NONE)
		{
		if (CurrentEndPtHL == 2)
			{
			if (lineCenterIsNear (cursorLoc, CurrentSegHL))
				{
				near = true;
				if (Debug)
					{
					fprintf (stderr, 
					 "Found cursor still is near seg %d\n",
					 CurrentSegHL);
					}
				return;
				}
			else if ( (CurrentHLType == HL_DELETE) &&
				  (endPtIsNear (cursorLoc, CurrentSegHL, 0) || 
				   endPtIsNear (cursorLoc, CurrentSegHL, 1) ) ) 
				{
				near = true;
				if (Debug)
					{
					fprintf (stderr, 
				      "Found cursor still is near del seg %d\n",
					 CurrentSegHL);
					}
				return;
				}
			    
			}
		else
			{
			if (endPtIsNear (cursorLoc, 
                                            CurrentSegHL, CurrentEndPtHL))
				{
				near = true;
				if (Debug)
					{
					fprintf (stderr, 
					 "Found cursor still is near seg %d"
					 " end pt %d\n",
					 CurrentSegHL, CurrentEndPtHL);
					}
				return;
				}
			}
		}

	// anything that was highlighted is no longer in proximity, so
	// unhighlight everything.
	unhighlight ();

	// first check against each end pt
	for (int i = 0; i < MaxSegments; i++)
		{
		if ( ValidSeg[i] )
			{
			for (int j = 0; j < 2; j++)
				{
				if (endPtIsNear (cursorLoc, i, j))
					{
					highlight (i, j, HL_DRAG);
					near = true;
					if (Debug)
					   {
					   fprintf (stderr, 
					    "Found cursor now is near seg %d"
					    " end pt %d\n", i, j);
					   }
					return;
					}
				}
			}
		}

	// now check against each center pt
	for (int i = 0; i < MaxSegments; i++)
		{
		if ( ValidSeg[i] )
			{
			if (lineCenterIsNear (cursorLoc, i))
				{
				highlight (i, 2, HL_DRAG);
				near = true;
				if (Debug)
				   {
				   fprintf (stderr, 
				    "Found cursor now is near seg %d\n", i);
				   }
				return;
				}
			}
		}



	// just to be sure:
	unhighlight ();

	if (Debug)
		{
		fprintf (stderr, "Found cursor is not near anything.\n");
		}

	near = false;
	return;
	}  // end of checkCursorNearFar


//////////////////////////////////////

int
readSegFile (const char *fileName)
        {
        FILE *fp;
        int lineNo;
        float seg[2][3];
        char line[1000];
        char tok[1000];

        if ( (fp = fopen (fileName, "r")) == NULL)
                {
		fprintf (stderr, "%s: Unable to open segment file %s.\n", 
                        ProgName, fileName);
                return -1;
                }

        lineNo = 0;
        while (fgets (line, sizeof(line), fp) == line)
            {
            lineNo++;

            if (sscanf (line, "%s", tok) != 0)  // if not a blank line
                {
                if (tok[0] != '#') // if it's not a comment
                    {
                    if (sscanf (line, "%f %f %f  %f %f %f",
                            &(seg[0][0]), &(seg[0][1]), &(seg[0][2]),
                            &(seg[1][0]), &(seg[1][1]), &(seg[1][2])) != 6)
                        {
		        fprintf (stderr, 
                          "%s: Error reading line %d in segment file %s.\n", 
                          ProgName, lineNo, fileName);
                        }
                    else
                        {
                        seg[0][0] /= CoordScale;
                        seg[0][1] /= CoordScale;
                        seg[0][2] /= CoordScale;
                        seg[1][0] /= CoordScale;
                        seg[1][1] /= CoordScale;
                        seg[1][2] /= CoordScale;
                        int iSeg = getFreeSegIndex ();
                        if (iSeg < 0)
                            {
                            // we're trying to create a new seg, 
                            // but we're beyond max # of segments
		            fprintf (stderr, 
                                "%s: Exceeded maximum number of segments "
                                "while reading segment file %s.\n", 
                                ProgName, fileName);
                            fclose (fp);
                            return -1;
                            }
                
                        if (setSeg (iSeg, seg))
                            {
		            fprintf (stderr, 
                                "%s: Error updating segment data base "
                                "while reading segment file %s.\n", 
                                ProgName, fileName);
                            fclose (fp);
                            return -1;
                            }
                        }
                    } // end of if not a comment
                }  // end of if not a blank line

            }  // end of if got a line

        fclose (fp);
        return 0;
        }  // end of readSegFile

int
writeSegFile (const char *fileName)
        {
        FILE *fp;

        if ( (fp = fopen (fileName, "w")) == NULL)
                {
		fprintf (stderr, "%s: Unable to open output segment file %s.\n",
                        ProgName, fileName);
                return -1;
                }

        for (int iSeg = 0; iSeg < MaxSegments; iSeg++)
                {
                if ( ValidSeg[iSeg] )
                        {
                        fprintf (fp, "%g %g %g  %g %g %g\n",
                            Segment[iSeg][0][0]*CoordScale, 
                            Segment[iSeg][0][1]*CoordScale, 
                            Segment[iSeg][0][2]*CoordScale, 
                            Segment[iSeg][1][0]*CoordScale, 
                            Segment[iSeg][1][1]*CoordScale, 
                            Segment[iSeg][1][2]*CoordScale );
                        }
                }  // end of loop over all segments

        fclose (fp);
        return 0;
        }  // end of writeSegFile


//////////////////////////////////////


//////////////////////////////////////

