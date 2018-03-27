

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>

#include <iris.h>

// NOTE: This code refers to sge. sge (the scene graph editor) is no longer
//       used and (in some respects) is replaced by iris.
//       For the purposes of this code, when you see sge, it just means IRIS.
//

#include "General_GUI_S.h"
#include "sgeComm.h"

#include "InitLights.h"


///////////////////////////////////////////////////////////////////////////
//  
// This section initializes LightSourceDesc and LightModelDesc data 
// structures to default values.
//


void
setLightModelDefaults ()
	{
	// These are the defaults from savgfly (dosg-fly?)
	LightModel.ambient[0] = 0.2;
	LightModel.ambient[1] = 0.2;
	LightModel.ambient[2] = 0.2;
	LightModel.ambient[3] = 1.0;

	LightModel.localViewer = 1;
	LightModel.twoSided = 1;
	LightModel.colorControlSeparate = 0;  // separate spec is off
	}  // end of lightModelDefaults



// extern
void
setLightSourcePositionDefaults (LightSourceDesc *ls)
	{
	// Vector toward infinite light source
	ls->infDirection[0] =  0;
	ls->infDirection[1] = -1;
	ls->infDirection[2] =  0;

	// Default location for omni and spot lights
	ls->localLocation[0] =  0;
	ls->localLocation[1] = -1;
	ls->localLocation[2] =  0;


	// the direction is used for spot lights
	ls->spotDirection[0] =  0;
	ls->spotDirection[1] =  1;
	ls->spotDirection[2] =  0;

	ls->spotLookAtPt[0] = 0;
	ls->spotLookAtPt[1] = 0;
	ls->spotLookAtPt[2] = 0;
	ls->spotDirectionType = SPOT_DIR_HP;


	ls->atten[0] = 1;
	ls->atten[1] = 0;
	ls->atten[2] = 0;

	ls->spotExponent = 0;

	// Note that spotCutoff can vary from 0 to 90 when specifying
	// a true spotlight.  If it is set to 180, then spotlighting is
	// disabled
	ls->spotCutoff = 20.0;



	}  // end of setLightSourcePositionDefaults



// extern
void
setLightSourceColorDefaults (LightSourceDesc *ls)
	{
	// ls->color[3][4];  // first index: 0 ambient, 1 diffuse, 2 spec

	// ambient
	ls->color[0][0] = 0.0;
	ls->color[0][1] = 0.0;
	ls->color[0][2] = 0.0;
	ls->color[0][3] = 1.0;

	// diffuse
	ls->color[1][0] = 1.0;
	ls->color[1][1] = 1.0;
	ls->color[1][2] = 1.0;
	ls->color[1][3] = 1.0;

	// specular
	ls->color[2][0] = 1.0;
	ls->color[2][1] = 1.0;
	ls->color[2][2] = 1.0;
	ls->color[2][3] = 1.0;


	}  // end of setLightSourceColorDefaults

// extern
void
setLightSourceDefaults (int lightNum, LightSourceDesc *ls)
	{

	sprintf (ls->name, "_HEV_LE_%1d_", lightNum);

	ls->glLightNum = lightNum;

	setLightSourceColorDefaults (ls);

	ls->type = INFINITE;

	setLightSourcePositionDefaults (ls);



	ls->referenceFrameIsRelative = 1; // 1 relative, 0 absolute

	// These are related to the GUI within this program
	ls->on = 0;
	ls->showGlyph = 0;
	ls->inRgbMode= 0;


	ls->lightType = 0;  // 0 infinite, 1 local omni, 2 local spot

	// 0 no node, 1 scene, 2 ether, 3 world, 4 wand, 5 head, 6 nav
	ls->node = 0;  

	// a test case:
	// ls->node = 7;  

	ls->currEditColor = 1;  // 0 ambient, 1 diffuse, 2 spec
	ls->linkAmbDiffSpec = 0;  

	ls->spotDirectionType = SPOT_DIR_HP;
	ls->spotLookAtPt[0] = 0;
	ls->spotLookAtPt[1] = 0;
	ls->spotLookAtPt[2] = 0;


	}  // end of setLightSourcDefaults


void
initAllLightData ()
	{
	int i;

	CurrentLightNum = 0;

	setLightModelDefaults ();
	
	for (i = 0; i < 8; i++)
		{
		setLightSourceDefaults (i, &(LightSource[i]));
		// sgeSendLightAll (&(LightSource[i]));
		}
	}  // end of initAllLightData


int
sendAllLightData ()
        {


        for (int i = 0; i < 8; i++)
                {
                sgeSendLightAll (&(LightSource[i]));
                }

        return 0;
        }  // end of sendAllLightData

//
//
/////////////////////////////////////////////////////////////////////
//
//
// The following section implements the querying of light source and
// light model data within hev-lightEditor.
//
//




//
// verify that we have two-way communication between this program
// and sge.
//
int establishCommunicationWithSGE ()
	{
	char msgSend[1000];
	char msgRecv[1000];
	std::vector<std::string> vec;

	msgRecv[0] = 0;

	sprintf (msgSend, "QUERY %s PING 0", sgeRecvShmName);
	sgeSendMsg (msgSend);

	if (sgeReceiveMsgQWait (msgRecv))
		{
		fprintf (stderr, "hev-lightEditor: "
		   "Did not receive ping response from IRIS.\n");
		return -1;
		} 
		
	cStrToVec (msgRecv, vec);

	if (vec.size () != 2)
		{
		fprintf (stderr, "hev-lightEditor: "
			"Expected message \"PING_RESPONSE 0\"; "
			"but received message : \n    \"%s\".",
			msgRecv);
		return -1;
		}

	if (iris::IsSubstring("ping_response", vec[0]) &&
		    			iris::IsSubstring("0", vec[1]))
		{
		// all is well
		return 0;
		}


	fprintf (stderr, "hev-lightEditor: "
			"Expected message \"PING_RESPONSE 0\"; "
			"but received message : \n    \"%s\".",
			msgRecv);
	return -2;
	}  // end of establishCommunicationWithSGE




static bool 
vecToDoubles (
  std::vector<std::string> &vec, 
  int startVecIndex, 
  double *d, 
  int numD )
	{
	bool bad = 0;

	for (int i = 0; i < numD; i++)
		{
		int ii = i + startVecIndex;
		bad = bad || (!iris::StringToDouble (vec[ii], d+i));
		}

	return bad;
	}  // end of vecToDoubles

static void
ignoreWarning (char *msg)
	{
	fprintf (stderr, 
		"hev-lightEditor : Ignoring message \"%s\".\n", msg);
	}  // end of ignoreWarning

static void
ignorePartialWarning (char *msg)
	{
	fprintf (stderr, 
		"B hev-lightEditor : Ignoring parts of message \"%s\".\n", msg);
	}  // end of ignorePartialWarning


static void
ignoreWarning (std::vector<std::string> &vec)
	{
	fprintf (stderr, "hev-lightEditor : Ignoring message \"");
	for (int i; i < vec.size(); i++)
		{
		fprintf (stderr, " %s", vec[i].c_str());
		}
	fprintf (stderr, "\".\n");
	}  // end of ignoreWarning

static void
ignorePartialWarning (std::vector<std::string> &vec)
	{
	fprintf (stderr, "A hev-lightEditor : Ignoring parts of message \"");
	for (int i = 0; i < vec.size(); i++)
		{
		fprintf (stderr, " %s", vec[i].c_str());
		}
	fprintf (stderr, "\".\n");
	}  // end of ignorePartialWarning



static void
msgToDoubles (
   const char *name,
   std::vector<std::string> &vec,
   int correctVecLen, 
   int startVecIndex, 
   double *d, 
   int numD
   )
	{


	if (vec.size() != correctVecLen)
		{
		fprintf (stderr, "hev-lightEditor : "
		  "Error in number of tokens in \"%s\" light message.\n", name);
		ignoreWarning (vec);
		}
	else
		{
		if (vecToDoubles (vec, startVecIndex, d, numD))
			{
			fprintf (stderr, "hev-lightEditor : "
		   	   "Error in parsing \"%s\" light message.\n", name);
			ignorePartialWarning (vec);
			}
		}
	}  // end of msgToDoubles


static void
stripQuotes (const char *inStr, char *outStr, int *quoted)
	{


	char localIn[1000];
	strcpy (localIn, inStr);
	char *in = localIn;

	*quoted = 0;
	// strip off at most two sets of single or double quotes
	for (int i = 0; i < 2; i++)
		{
		int len = strlen (in);
		if (len < 2)
			{
			break;
			}
		else if ( (in[0] == '"') || (in[0] == '\'') )
			{
			if (in[0] == in[len-1])
				{
				in[len-1] = 0;
				in++;
				*quoted = 1;
				}
			}
		else
			{
			break;
			}
		}

	strcpy (outStr, in);

	} // end of stripQuotes

static bool
getKeywordValues ( 
  const std::vector<std::string> &inVec, 
  const std::string & keyword, 
  int nVal, 
  std::string vals []
  )
    {

    for (int i = 0; i < inVec.size()-nVal; i++)
        {
	    if ( keyword == inVec[i] )
            {
            for (int j = 0; j < nVal; j++)
                {
                int jj = j + i + 1;
                vals[j] = inVec[jj];
                }
            return true;
            }
    
        }  // end of loop over vector

	fprintf (stderr, "hev-lightEditor: "
		   "Could not find keyword %s in IRIS query response.\n", 
            keyword.c_str());
    return false;
    }  // end of getKeywordValues ( string array version)



static bool
getKeywordValues ( 
  const std::vector<std::string> &inVec, 
  const std::string & keyword, 
  int nVal, 
  double vals [] 
  )
    {

    for (int i = 0; i <= inVec.size()-nVal; i++)
        {
	    if ( keyword == inVec[i] )
            {
            for (int j = 0; j < nVal; j++)
                {
                int jj = j + i + 1;
                if ( ! iris::StringToDouble (inVec[jj], vals+j) )
                    {
	                fprintf (stderr, "hev-lightEditor: "
		                "Expected double but found token (%s) in IRIS query response.\n", 
                        inVec[jj].c_str());
                    return false;
                    }
                }
            return true;
            }
    
        }  // end of loop over vector


	fprintf (stderr, "hev-lightEditor: "
		   "Could not find keyword %s in IRIS query response.\n", 
            keyword.c_str());
            
    return false;
    }  // end of getKeywordValues (double array version)





static int
getNewLightInfo (int myLightIndex, const char *nodeName)
	{

	LightSourceDesc *ls = LightSource + myLightIndex;
	char msgSend[1000];
	char msgRecv[1000];
	int ii;
	std::vector<std::string> vec;
	std::string valS[10];
	double valD[10];


    ls->glLightNum = myLightIndex;
	strcpy (ls->name, nodeName);



	sprintf (msgSend, "QUERY %s PARENTS %s", 
				sgeRecvShmName, nodeName);
	sgeSendMsg (msgSend);

	if (sgeReceiveMsgQWait (msgRecv))
		{
		fprintf (stderr, "hev-lightEditor: "
		   "Did not receive light node parent message from IRIS.\n");
		return -1;
		}

	cStrToVec (msgRecv, vec);
	if ( (!iris::IsSubstring("parents", vec[0]))  || 
		 (!iris::IsSubstring(nodeName, vec[1]) )  || 
         (vec.size() < 3) ||
         (vec[2] == "ERROR:") )
		{
		// we didn't get what we expected
		fprintf (stderr, "hev-lightEditor: "
		   "Light node parent message not in proper format: \"%s\"\n",
		   msgRecv);
		return -1;
		}

    char parentName[1000];
    int quoted;

    if (strcmp (vec[2].c_str(), "0") == 0)
        {
        ls->node = 0;  // no parent
        }
    else
        {
        stripQuotes (vec[3].c_str(), parentName, &quoted);

	    if (strcmp (parentName, "scene") == 0)
		    {
		    ls->node = 1;
		    }
	    else if (strcmp (parentName, "ether") == 0)
		    {
		    ls->node = 2;
		    }
	    else if (strcmp (parentName, "world") == 0)
		    {
		    ls->node = 3;
		    }
        else if (strcmp (parentName, "wand") == 0)
		    {
		    ls->node = 4;
		    }
        else if (strcmp (parentName, "head") == 0)
		    {
		    ls->node = 5;
		    }
        else if (strcmp (parentName, "nav") == 0)
		    {
		    ls->node = 6;
		    }
        else
		    {
		    // a real node name but it's not one we know
		    // 7 means "unknown"
		    ls->node = 7;
	        }

	    }



	// The way I'm handling the data is a little wasteful.  I am reading 
	// the data from the message into local variables, then after 
	// receiving all of the messages, I convert it into the form
	// needed in the LightSourceDesc data structure.
	// Some (most) of the data could be read directly from the message 
	// into the data structure.




    // Now the parameters of the light source
	sprintf (msgSend, "QUERY %s LIGHTNODE %s", 
				sgeRecvShmName, nodeName);
	sgeSendMsg (msgSend);

	if (sgeReceiveMsgQWait (msgRecv))
		{
		fprintf (stderr, "hev-lightEditor: "
		   "Did not receive light node message from IRIS.\n");
		return -1;
		}

	cStrToVec (msgRecv, vec);
	if ( (!iris::IsSubstring("lightnode", vec[0]))  || 
	    			( ! iris::IsSubstring(nodeName, vec[1]) ) )
		{
		// we didn't get what we expected
		fprintf (stderr, "hev-lightEditor: "
		   "Light node message not in proper format: \"%s\"\n",
		   msgRecv);
		return -1;
		}

	bool on;
	double color[3][4];
	double position[4];
	double direction[4];
	double attenuation[3];
	double spotExpAng[2];
	bool ref_frame;

    // now parse out the message into the ls struct

    if ( ! getKeywordValues (vec, "ON", 1, valS) )
        {
        return -1;
        }
    on = (valS[0] == "TRUE");

    if ( ! getKeywordValues (vec, "AMBIENT", 4, color[0]) )
        {
        return -1;
        }

    if ( ! getKeywordValues (vec, "DIFFUSE", 4, color[1]) )
        {
        return -1;
        }

    if ( ! getKeywordValues (vec, "SPECULAR", 4, color[2]) )
        {
        return -1;
        }

    if ( ! getKeywordValues (vec, "POSITION", 4, position) )
        {
        return -1;
        }

    if ( ! getKeywordValues (vec, "DIRECTION", 3, direction) )
        {
        return -1;
        }

    if ( ! getKeywordValues (vec, "CONSTANT", 1, attenuation+0) )
        {
        return -1;
        }

    if ( ! getKeywordValues (vec, "LINEAR", 1, attenuation+1) )
        {
        return -1;
        }

    if ( ! getKeywordValues (vec, "QUADRATIC", 1, attenuation+2) )
        {
        return -1;
        }

    if ( ! getKeywordValues (vec, "SPOT", 2, spotExpAng) )
        {
        return -1;
        }

    if ( ! getKeywordValues (vec, "REF_FRAME", 1, valS) )
        {
        return -1;
        }
    ref_frame = (valS[0] == "RELATIVE");



	// Now convert all of the data we got from sge and put it into the 
	// LightSourceDesc data structure at ls

	ls->on = on;

	// Note the following code should be equivalent to the code that
	// fills in the LightSourceDesc data structure based on an OSG
	// LightSource node.  This is done when we read in an OSG file.

	for (int i = 0; i < 3; i++)
		{
		for (int j = 0; j < 4; j++)
			{
			ls->color[i][j] = color[i][j];
			}
		ls->atten[i] = attenuation[i];
		ls->spotDirection[i] = direction[i];
		}


	ls->referenceFrameIsRelative = ref_frame;
	ls->spotExponent = spotExpAng[0];



        if (position[3] == 0)
                {
		// fprintf (stderr, "inf\n");

                ls->type = INFINITE;
                ls->infDirection[0] = position[0];
                ls->infDirection[1] = position[1];
                ls->infDirection[2] = position[2];

#if 0
fprintf (stderr, "infDir %f %f %f \n",
ls->infDirection[0],
ls->infDirection[1],
ls->infDirection[2]);
#endif

                // In case we want to convert this to OMNI or SPOT,
                // we set the local location and spotCutoff.
                ls->localLocation[0] = position[0];
                ls->localLocation[1] = position[1];
                ls->localLocation[2] = position[2];
                ls->spotCutoff = 
			(spotExpAng[1] == 180.0) ? 20.0 : spotExpAng[1];

                }
        else
                {
                ls->localLocation[0] = position[0] / position[3];
                ls->localLocation[1] = position[1] / position[3];
                ls->localLocation[2] = position[2] / position[3];
                if (spotExpAng[1] == 180.0)
                        {
                        ls->type = OMNI;
// fprintf (stderr, "omni\n");
                        // In case we want to convert light type,
                        // we set infDirection and spotCutoff.
                        ls->spotCutoff = 20.0;
                        ls->infDirection[0] = ls->localLocation[0];
                        ls->infDirection[1] = ls->localLocation[1];
                        ls->infDirection[2] = ls->localLocation[2];
                        }
                else
                        {
// fprintf (stderr, "spot\n");
                        ls->type = SPOT;
                        ls->spotCutoff = spotExpAng[1];
                        // In case we want to convert light type,
                        // we set infDirection.
                        ls->infDirection[0] = - ls->spotDirection[0];
                        ls->infDirection[1] = - ls->spotDirection[1];
                        ls->infDirection[2] = - ls->spotDirection[2];
                        }
                }

	if (ls->spotCutoff > 90.0)
		{
		ls->spotCutoff = 90.0;
		}

	ls->spotLookAtPt[0] = ls->spotDirection[0]+ls->localLocation[0];
	ls->spotLookAtPt[1] = ls->spotDirection[1]+ls->localLocation[1];
	ls->spotLookAtPt[2] = ls->spotDirection[2]+ls->localLocation[2];


#if 0
fprintf (stderr, "infDir %f %f %f \n",
ls->infDirection[0],
ls->infDirection[1],
ls->infDirection[2]);
#endif 




	return 0;
	}  // getNewLightInfo


int
getCurrentLightSourcesFromSGE ()
	{
	char msgSend[1000];
	char msgRecv[1000];
	int ii, numLights;
	std::vector<std::string> vec;

	msgRecv[0] = 0;

	sprintf (msgSend, "QUERY %s LIGHTNODE", sgeRecvShmName);
	sgeSendMsg (msgSend);

	if (sgeReceiveMsgQWait (msgRecv))
		{
		fprintf (stderr, "hev-lightEditor: "
	   		   "Did not receive light list message from IRIS.\n");
		return -1;
		}

	cStrToVec (msgRecv, vec);
	if (!iris::IsSubstring("lightnode", vec[0])) 
		{
		// we didn't get what we expected
		fprintf (stderr, "hev-lightEditor: "
   		   "Expected light list message from IRIS, "
		   "but got message:\n   \"%s\".\n", msgRecv);
		return -1;
		}

	if (sscanf (vec[1].c_str(), "%d", &numLights) != 1)
		{
		fprintf (stderr, "hev-lightEditor: "
   		   "Can't parse light list message from IRIS: \n"
		   "   \"%s\".\n", msgRecv);
		return -1;
		}

	int newLight = 0;

	for (int iLight = 0; iLight < numLights; iLight++)
		{
		getNewLightInfo (iLight, vec[iLight+2].c_str());
		}  // end of loop over lights

	return 0;
	}  // end of getCurrentLightSourcesFromSGE


int
getLightModelFromSGE ()
	{
	char msgSend[1000];
	char msgRecv[1000];
	int ii, numLights;
	std::vector<std::string> vec;

	msgRecv[0] = 0;



/////////////////////////////
/////////////////////////////
/////////////////////////////

	sprintf (msgSend, "QUERY %s LIGHTMODEL", sgeRecvShmName);
	sgeSendMsg (msgSend);

	if (sgeReceiveMsgQWait (msgRecv))
		{
		fprintf (stderr, "hev-lightEditor: "
   		   "Did not receive light model ambient message from IRIS.\n");
		return -1;
		}


	cStrToVec (msgRecv, vec);


    // JGH: there is a bug in the LIGHTMODEL query response; it does not
    //      have the COLOR_CONTROL param.  For now, we are going to 
    //      assume that it has the default of single color
#if 0
	if ( (vec.size () != 12) || (!iris::IsSubstring("lightmodel", vec[0])) )
#else
	if ( (vec.size () != 10) || (!iris::IsSubstring("lightmodel", vec[0])) )
#endif
        {
		fprintf (stderr, "hev-lightEditor: "
          "Received bad light model message from iris: %s.\n", msgRecv);
		return -1;
        } 

    if ( ! getKeywordValues (vec, "AMBIENT", 4, LightModel.ambient) )
        {
        return -1;
        }

    std::string valS[10];

#if 0
    if ( ! getKeywordValues (vec, "COLOR_CONTROL", 1, valS) )
        {
        return -1;
        }
	LightModel.colorControlSeparate = (valS[0]=="SINGLE") ? 0 : 1;
#else
	LightModel.colorControlSeparate = 0;
#endif

    if ( ! getKeywordValues (vec, "LOCAL_VIEWER", 1, valS) )
        {
        return -1;
        }
	LightModel.localViewer = (valS[0]=="TRUE") ? 1 : 0;

    if ( ! getKeywordValues (vec, "TWO_SIDED", 1, valS) )
        {
        return -1;
        }
	LightModel.twoSided = (valS[0]=="TRUE") ? 1 : 0;


	return 0;
	}  // end of getLightModelFromSGE

