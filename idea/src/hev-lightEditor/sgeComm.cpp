
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iris.h>
// #include "dtk.h"

#include "sgeComm.h"


static FILE *sgeFP = stdout;

static int Echo = 0;


////////////////////////////////////////////////////////////////////
//
// First, a few utility routines
//


void
setSgeCommEcho (int echo)
	{
	Echo = echo;
	if (Echo)
		{
		sgeSendMsg ("ECHO ON\n");
		}
	else
		{
		sgeSendMsg ("ECHO OFF\n");
		}
	}  // end of setSgeCommEcho


inline int
sgeSendMsg (const char *msgIn)
	{
    char msg[2000];
    strncpy (msg, msgIn, sizeof (msg)-2);
	int len = strlen(msg);
	if (msg[len-1] != '\n')
		{
		msg[len] = '\n';
		msg[len+1] = 0;
		}
	fprintf (sgeFP, msg);
	if (Echo)
		{
		char lmsg[1000];
		strcpy (lmsg, msg);
		lmsg[len-1] = 0;
		fprintf (stderr, 
			"hev-lightEditor: Sending IRIS msg <%s>\n", 
			lmsg);
		}
	fflush (sgeFP);
	return 0;
	}  // end of sgeSendMsg

//
//
////////////////////////////////////////////////////////////////////
//
// The following code concerns light sources
//
//

inline void
sgeSendColor (LightSourceDesc *ls, int iColor)
	{
	char msg[200];
	static const char * colName[3] = {"AMBIENT", "DIFFUSE", "SPECULAR"};
	sprintf (msg, "LIGHTNODE %s %s %g %g %g %g\n", 
			ls->name, 
			colName[iColor], 
			CLAMP01(ls->color[iColor][0]),
			CLAMP01(ls->color[iColor][1]),
			CLAMP01(ls->color[iColor][2]),
			CLAMP01(ls->color[iColor][3]) );

	sgeSendMsg (msg);

	}  // end of sgeSendColor

void 
sgeSendCurrColor (int iColor)
	{
	sgeSendColor (LightSource + CurrentLightNum, iColor);
	}  // end of sgeSendCurrColor


inline void sgeSendLocalLocation(LightSourceDesc *ls)
	{
	char msg[200];
	sprintf (msg, "LIGHTNODE %s POSITION %g %g %g 1\n",
			ls->name,
			ls->localLocation[0],
			ls->localLocation[1],
			ls->localLocation[2] ) ;

	sgeSendMsg (msg);
	}  // end of sgeSendLocalLocation




inline void sgeSendInfDirection (LightSourceDesc *ls)
	{
	char msg[200];
	sprintf (msg, "LIGHTNODE %s POSITION %g %g %g 0\n",
			ls->name,
			ls->infDirection [0],
			ls->infDirection [1],
			ls->infDirection [2] ) ;

	sgeSendMsg (msg);
	}  // end of sgeSendInfDirection





void sgeSendCurrLocalLocation ()
	{
	sgeSendLocalLocation (LightSource + CurrentLightNum);
	}  // end of sgeSendCurrLocalLocation

void sgeSendCurrInfDirection ()
	{
	sgeSendInfDirection (LightSource + CurrentLightNum);
	}  // end of sgeSendCurrInfDirection





inline void sgeSendSpotDirection (LightSourceDesc *ls)
	{
	char msg[200];
	sprintf (msg, "LIGHTNODE %s DIRECTION %g %g %g\n",
			ls->name,
			ls->spotDirection[0],
			ls->spotDirection[1],
			ls->spotDirection[2] ) ;

	sgeSendMsg (msg);
	}  // end of sgeSendDirection

void sgeSendCurrSpotDirection ()
	{
	sgeSendSpotDirection (LightSource + CurrentLightNum);
	}  // end of sgeSendCurrDirection


inline void sgeSendAttenuations (LightSourceDesc *ls)
	{
	char msg[200];
	sprintf (msg, "LIGHTNODE %s ATTENUATION CONSTANT %g  ATTENUATION LINEAR %g  ATTENUATION QUADRATIC %g\n",
			ls->name,
			ls->atten[0],
			ls->atten[1],
			ls->atten[2]);


	sgeSendMsg (msg);
	}  // end of sgeSendAttenuations

void sgeSendCurrAttenuations ()
	{
	sgeSendAttenuations (LightSource + CurrentLightNum);
	}  // end of sgeSendCurrAttenuations 


inline void sgeSendSpotExpAng (LightSourceDesc *ls)
	{
	char msg[200];
	sprintf (msg, "LIGHTNODE %s SPOT %g %g\n",
			ls->name,
			ls->spotExponent,
			ls->spotCutoff );

	sgeSendMsg (msg);
	}  // end of sgeSendSpot

void sgeSendCurrSpotExpAng ()
	{
	sgeSendSpotExpAng (LightSource + CurrentLightNum);
	}  // end of sgeSendCurrSpot


inline void sgeSendOmniFlag (LightSourceDesc *ls)
	{
	// For omni-directional lights, the spot cutoff angle
	// has to be set to 180.0
	char msg[200];
	sprintf (msg, "LIGHTNODE %s SPOT %g 180\n",
			ls->name,
			ls->spotExponent);

	sgeSendMsg (msg);
	}  // end of sgeSendSpot

void sgeSendCurrOmniFlag ()
	{
	sgeSendOmniFlag (LightSource + CurrentLightNum);
	}  // end of sgeSendCurrSpot




inline void sgeSendRefFrame (LightSourceDesc *ls)
	{
	char msg[200];
	sprintf (msg, "LIGHTNODE %s REF_FRAME %s\n",
		    ls->name,
		    ls->referenceFrameIsRelative ? "RELATIVE" : "ABSOLUTE" );
	sgeSendMsg (msg);
	} // end of sgeSendRefFrame 	


void sgeSendCurrRefFrame ()
	{
	sgeSendRefFrame (LightSource + CurrentLightNum);
	}  // end of sgeSendRefFrame



inline void sgeSendOnOff (LightSourceDesc *ls)
	{
	char msg[200];
	sprintf (msg, "LIGHTNODE %s ON %s\n",
		    ls->name,
		    ls->on ? "TRUE" : "FALSE" );
	sgeSendMsg (msg);
	} // end of sgeSendRefFrame 	


void sgeSendCurrOnOff ()
	{
	sgeSendOnOff (LightSource + CurrentLightNum);
	}  // end of sgeSendRefFrame









inline void sgeSendRmFromParentNode (LightSourceDesc *ls)
	{
	char msg[200];
	static const char * nodeName[8] = 
		{
		"NO_NODE",
		"scene",
		"ether",
		"world",
		"wand",
		"head",
		"nav",
		"UNKNOWN"
		};

	// If the node is "unknown" or NO_NODE then we're not going to do anything

	if ( (ls->node != 7) && (ls->node != 0) )
		{
		sprintf (msg, "REMOVECHILD %s %s\n", ls->name, nodeName[ls->node]);
		sgeSendMsg (msg);
		}

	}  // end of sgeSendRmFromParentNode




inline void sgeSendAddToParentNode (LightSourceDesc *ls)
	{
	char msg[200];
	static const char * nodeName[8] = 
		{
		"NO_NODE",
		"scene",
		"ether",
		"world",
		"wand",
		"head",
		"nav",
		"UNKNOWN"
		};

	// If the node is "unknown" or NO_NODE then we're not going to do anything

	if ( (ls->node != 7) && (ls->node != 0) )
		{
		sprintf (msg, "ADDCHILD %s %s\n", ls->name, nodeName[ls->node]);
		sgeSendMsg (msg);
		}

	}  // end of sgeSendAddToParentNode

void sgeSendCurrRmFromParentNode ()
	{
	sgeSendRmFromParentNode (LightSource + CurrentLightNum);
	}  // end of sgeSendCurrRmFromParentNode

void sgeSendCurrAddToParentNode ()
	{
	sgeSendAddToParentNode (LightSource + CurrentLightNum);
	}  // end of sgeSendCurrAddToParentNode






#if 0

inline void sgeSendParentNode (LightSourceDesc *ls)
	{
	char msg[200];
	static const char * nodeName[8] = 
		{
		"NO_NODE",
		"scene",
		"ether",
		"world",
		"wand",
		"head",
		"nav",
		"UNKNOWN"
		};

	// If the node is "unknown" then we're not going to do anything

	if (ls->node == 0) 
		{
		sprintf (msg, "OFF %s\n", ls->name);
		sgeSendMsg (msg);
		}
	else if ( ls->node != 7 )
		{
		sprintf (msg, "OFF %s\n", ls->name);
		sgeSendMsg (msg);
		sprintf (msg, "ON %s %s\n", ls->name, 
			nodeName[ls->node]);
		sgeSendMsg (msg);
		}

	}  // end of sgeSendParentNode

void sgeSendCurrParentNode ()
	{
	sgeSendParentNode (LightSource + CurrentLightNum);
	}  // end of sgeSendCurrParentNode
#endif


void sgeSendLightAll (LightSourceDesc *ls)
	{
	sgeSendOnOff (ls);
	sgeSendColor (ls, 0);	
	sgeSendColor (ls, 1);	
	sgeSendColor (ls, 2);	
	switch (ls->type)
		{
		case INFINITE:
			sgeSendInfDirection (ls);
			sgeSendOmniFlag (ls);
			break;

		case OMNI:
			sgeSendLocalLocation (ls);
			sgeSendOmniFlag (ls);
			break;

		case SPOT:
			sgeSendLocalLocation (ls);
			sgeSendSpotDirection (ls);
			sgeSendSpotExpAng (ls);
			break;
		}

	sgeSendAttenuations (ls);
	sgeSendRefFrame (ls);
	// sgeSendParentNode (ls);  // JGH : I don't thinki this is needed
	}  // end of sgeSendLightAll

void sgeSendCurrLightAll ()
	{
	sgeSendLightAll (LightSource + CurrentLightNum);
	}  

//
// End of code concerning light sources
//
//
////////////////////////////////////////////////////////////////////
//
// LightModel 
//


void sgeSendLightModel ()
	{
	sgeSendLightModelColor ();
	sgeSendLightModelColorControl ();
	sgeSendLightModelLocalViewer ();
	sgeSendLightModelTwoSided ();
	}  // end of sgeSendLightModel

void sgeSendLightModelColor ()
	{
	char msg[200];
	sprintf (msg, "LIGHTMODEL AMBIENT  %g %g %g %g\n", 
			CLAMP01(LightModel.ambient[0]),
			CLAMP01(LightModel.ambient[1]),
			CLAMP01(LightModel.ambient[2]),
			CLAMP01(LightModel.ambient[3]) );
	sgeSendMsg (msg);
	}  // end of sgeSendLightModelColor


void sgeSendLightModelColorControl ()
	{
	char msg[200];
	sprintf (msg, "LIGHTMODEL COLOR_CONTROL %s\n",
	   LightModel.colorControlSeparate ?  "SEPARATE_SPECULAR" : "SINGLE" );
	sgeSendMsg (msg);
	}  // end of sgeSendLightModelColorControl


void sgeSendLightModelLocalViewer ()
	{
	char msg[200];
	sprintf (msg, "LIGHTMODEL LOCAL_VIEWER %s\n",
	   LightModel.localViewer ?  "TRUE" : "FALSE" );
	sgeSendMsg (msg);
	}  // end of sgeSendLightModelLocalViewer


void sgeSendLightModelTwoSided ()
	{
	char msg[200];
	sprintf (msg, "LIGHTMODEL TWO_SIDED %s\n",
	   LightModel.twoSided ?  "TRUE" : "FALSE" );
	sgeSendMsg (msg);
	}  // end of sgeSendLightModelTwoSided


//
// End of code concerning the light model
//
////////////////////////////////////////////////////////////////////
//
// The following code concerns the reception of messages from sge
// by way of a shared memory file.
//

char sgeRecvShmName[1000];

static int sgeShmSize = 1000;
static int sgeShmQLen = 256;
// static dtkSharedMem *sgeShm = NULL;
static double sgeShmWait = 5.0;

static iris::FifoReader * RecvFifo = NULL;


int setupSgeRecvShm (const char *name, int size, int qLen)
	{
	strcpy (sgeRecvShmName, name);

    RecvFifo = new iris::FifoReader (sgeRecvShmName);

    if (RecvFifo == NULL)
        {
			fprintf (stderr, 
			 "hev-lightEditor : "
			 "Unable to open FIFO file \"%s\".\n", name);
            return -1;
        }

    if ( ! RecvFifo->open() )
        {
        fprintf (stderr, "hev-lightEditor: Unable to open receive fifo %s.\n",
                        sgeRecvShmName);
        delete RecvFifo;
        RecvFifo = NULL;
        return -1;
        }


	return 0;
	}  // end of setSgeRecvShmNameSize



int sgeReceiveMsgQWait (char *msg)
	{
	int nIter = (int) (0.5 + (sgeShmWait / 0.01));

	for (int i = 0; i < nIter; i++)
		{
		int rtn;
        std::string lineStr;

        if ( RecvFifo->readLine ( & lineStr) )
			{
			if (Echo)
				{
				fprintf (stderr, 
				   "hev-lightEditor: Received IRIS msg <%s>\n", 
					msg);
				}
            strcpy (msg, lineStr.c_str());
			return 0;  // we have a message
			}

		usleep (10000);
		}

	return 1;  // we've exhausted the wait
	}


int sgeReceiveMsgQ (char *msg)
	{
    msg[0] = 0;
    std::string lineStr;

    if ( RecvFifo->readLine ( & lineStr) )
		{
        if (Echo)
            {
		    fprintf (stderr, 
			    "hev-lightEditor: Received IRIS msg <%s>\n", 
			    msg);
            }
        strcpy (msg, lineStr.c_str());
        return strlen (msg);
		}
	return 0;
	}  // end of sgeReceiveMsgQ

void sgeSetWait (double seconds)
	{
	sgeShmWait = seconds;
	}  // end of sgeSetWait

//
//
////////////////////////////////////////////////////////////////////
