

#ifndef __HSSSEGDBHL_H__

#define __HSSSEGDBHL_H__





///////////////////////////////////////////////////////////////////
//
//
// The basic assumption is that only one "thing" at a time can
// be highlighted and there are only two types of things that
// can be highlighted: a segment end point, and an entire segment.
//
// There are two types of highlighting, HL for dragging and HL for
// deletion.
//
// We use "highlight" to mean both that the object is visually 
// differentiated, and also to mean that the object is the one
// currently being manipulated.
//



enum HLType
	{
	HL_NONE = 0,
	HL_DRAG = 1,
	HL_DELETE = 2,
	HL_USER = 3
	}  ;

void getCurrentHL (int &iSeg, int &whichPt, int &hlType);

int unhighlight ();

int highlight (int iSeg, int whichPt, HLType hlType);

void highlightCursor ();
void unhighlightCursor ();


void initSegs();




int getFreeSegIndex ();

int deleteSeg (int iSeg);

int setSeg (int iSeg, float seg[2][3]);
int getSeg (int iSeg, float seg[2][3]);

int setSegEndPt (int iSeg, int whichPt, float pt[3]);

void checkCursorNearFar (bool &near);

void setLenLabelScale (double scale);  // scales size of displayed len label
void displayLenLabels (bool display, char *fmt);
void setCoordScale (double scale);  // scales the calculated coords

int readSegFile (const char * fileName);
int writeSegFile (const char * fileName);

int userHighlight (bool all, int iSeg, bool hlOn);

#endif    // __HSSSEGDBHL_H__
