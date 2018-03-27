
#ifndef SGE_COMM_H
#define SGE_COMM_H



#include "General_GUI_S.h"


extern void setSgeCommEcho (int echo);

extern int sgeSendMsg (const char *);

extern void sgeSendColor (LightSourceDesc *ls, int iColor);
extern void sgeSendCurrColor (int iColor);


extern void sgeSendLocalLocation(LightSourceDesc *ls);
extern void sgeSendCurrLocalLocation ();

extern void sgeSendInfDirection (LightSourceDesc *ls);
extern void sgeSendCurrInfDirection ();

extern void sgeSendSpotDirection (LightSourceDesc *ls);
extern void sgeSendCurrSpotDirection ();

extern void sgeSendAttenuations (LightSourceDesc *ls);
extern void sgeSendCurrAttenuations ();

extern void sgeSendSpotExpAng (LightSourceDesc *ls);
extern void sgeSendCurrSpotExpAng ();

extern void sgeSendOmniFlag (LightSourceDesc *ls);
extern void sgeSendCurrOmniFlag ();

extern void sgeSendRefFrame (LightSourceDesc *ls);
extern void sgeSendCurrRefFrame ();

extern void sgeSendOnOff (LightSourceDesc *ls);
extern void sgeSendCurrOnOff ();

extern void sgeSendRmFromParentNode (LightSourceDesc *ls);
extern void sgeSendCurrRmFromParentNode ();
extern void sgeSendAddToParentNode (LightSourceDesc *ls);
extern void sgeSendCurrAddToParentNode ();

extern void sgeSendLightAll (LightSourceDesc *ls);
extern void sgeSendCurrLightAll ();


void sgeSendLightModel ();
void sgeSendLightModelColor ();
void sgeSendLightModelColorControl ();
void sgeSendLightModelLocalViewer ();
void sgeSendLightModelTwoSided ();

extern char sgeRecvShmName[1000];
extern int setupSgeRecvShm (const char *name, int size, int qLen);
extern int sgeReceiveMsgQWait (char *);
extern int sgeReceiveMsgQ (char *);
extern void sgeSetWait (double seconds);

#endif     //  SGE_COMM_H
