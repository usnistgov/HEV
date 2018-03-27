
#ifndef _INIT_LIGHTS_H_

#define _INIT_LIGHTS_H_ 

#include "General_GUI_S.h"

extern void setLightSourcePositionDefaults (LightSourceDesc *ls);

extern void setLightSourceColorDefaults (LightSourceDesc *ls);

extern void setLightSourceDefaults (int lightNum, LightSourceDesc *ls);

extern void initAllLightData ();

extern int sendAllLightData ();

extern void setLightModelDefaults ();



//
//  Verify that we have two-way communication with sge dso.
//  Returns 0 for success, non-zero for failure.
extern int establishCommunicationWithSGE ();

//
//  Communicates to SGE to get info about existing light sources
//  in the current scene graph.  Updates light editor data structures
//  to correspond to these lights.
//
extern int getCurrentLightSourcesFromSGE ();

extern int getLightModelFromSGE ();


#endif     // _INIT_LIGHTS_H


