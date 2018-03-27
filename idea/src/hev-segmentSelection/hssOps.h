

#ifndef _HEV_SEG_SEL_OPS_H_
#define _HEV_SEG_SEL_OPS_H_


#include <dtk.h>

#define PTSHM_MAX_NUM_SEGS          (200)

extern bool Active;

extern int MaxSegments;
extern double PtHotSpotDistSq;
extern double LnHotSpotDistSq;

extern char *EndPtNodeName[4];
extern char *SegNodeName[4];
extern char *SegHandleNodeName[4];
extern char OutCoordsNodeName[1000];



void sendSgeMsg (const char *format, ...);
void executeCmd (const char *format, ...);

extern dtkSharedMem *CreateButtonShm;
extern dtkSharedMem *DeleteButtonShm;

typedef struct
        {
        long int inUse;
        float pt[2][3];
        }  PtShmSegment;

int writeSegShm (int iSeg, float seg[2][3], double coordScale);
int writeSegShm (int iSeg, int whichPt, float pt[3], double coordScale);
int writeSegShm (int iSeg, PtShmSegment *seg, double coordScale);
int writeSegShmValid (int iSeg, bool valid);


enum ConstraintObj
	{
	CO_LINE,
#if 0
	CO_PT_0,
	CO_PT_1,
	CO_PT_BOTH,
#endif
	CO_NONE
	} ;

enum ConstraintType
	{
	CT_PARA,
	CT_PERP,
	CT_LEN,
	CT_NONE
	} ;

typedef struct constraint
	{
	ConstraintType type;
	ConstraintObj obj;
	int objID;
	double vec[3];
	double len;
	} Constraint;
	

int constrain (Constraint &constraint);
int force (Constraint &constraint);
int unconstrain ();

int setupCtrlShm (char *inCtrlShmName, char *ctrlID);

int activate ();
int deactivate ();

bool active ();

int show ();
int hide ();
int quit ();
int reset ();


int scaleMovement (double scale);
int unscaleMovement ();  // sets scale to 1

int limitBox (double box[2][3]);
int unlimitBox ();

int lenLabel (bool doLenLabel);

void setYOffset (float yOffset);


//////////////////////////////////////////////////

void getCursorLocation (float xyz[3]);

int userInteractions ();

// shared memory stuff
int setupButtons ( char *editButShmName, char *delButShmName);

int setupPtsShm (char *ptsShmName, char *ptsCoordNode);

int setEditDistances (double ptDist, double lnDist);

int setupObjects (char objectFileNames[3][4][1000], double glyphScale);

int setupInShm (char *inCoordShmName);

int setMaxSegments (int maxSeg);

int trackInCoords ();

void highlightCursor ();
void unhighlightCursor ();

#endif    // _HEV_SEG_SEL_OPS_H_
