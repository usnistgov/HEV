
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <string>

#include <iris.h>


#include "File_GUI_S.h"
#include "General_GUI_S.h"
#include "InitLights.h"
#include "sgeComm.h"

using namespace std ;






////////////////////////////////////////////////////////////////////////
//
// Global Data
//

static Fl_File_Chooser *OutputFileChooserWidget;
static Fl_File_Chooser *InputFileChooserWidget;
static Fl_Input *OutputFNWidget;
static Fl_Input *InputFNWidget;
static Fl_Group *WriteGroup;
static Fl_Group *ReadGroup;
static Msg_Window *WriteDoneWin;
static Msg_Window *ReadDoneWin;
static Fl_Button *InTemplateButton;
static Fl_Button *OutTemplateButton;
static Fl_Button *SelAll[2];
static Fl_Light_Button *LButtons[2][9];


static void setTemplateButton (int forInput);

static int writeLightToFile (int lNum, char *outFN);
static int readLightFromFile (int lNum, char *outFN);
static int writeLightModelToFile (char *outFN);
static int readLightModelFromFile (char *outFN);




//
// End of global data
//
//
///////////////////////////////////////////////////////////////////////////
//  
// Initialize data structures: both light source data and 
//


//
//  End of initializations...
//
/////////////////////////////////////////////////////////////////////
//
// Make the GUI agree with the light data
//



//
// End of reconciling of GUI with data
//
//////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
//
// Callbacks
//

static int
getTemplatePieces (char *fn, char *startFN, char *endFN)
	{

	strcpy (startFN, fn);
	endFN[0] = 0;

	char *tokenLoc = strstr (startFN, "%ID");
	if (tokenLoc == NULL)
		{
		return 0;
		}

	strcpy (endFN, tokenLoc+3);
	*tokenLoc = 0;

	return 1;
	}  // end of getTemplatePieces


static void
readFilesCB (Fl_Widget *w)
	{
	int nFiles = 0;

	for (int i = 0; i < 8; i++)
		{
		if (LButtons[1][i]->value())
			{
			nFiles++;
			}
		}

	char startFN[5000], endFN[5000], fn[5000];

	strcpy (fn, InputFNWidget->value());
	int templatePresent = getTemplatePieces (fn, startFN, endFN);
	
	for (int i = 0; i < 8; i++)
		{
		if (LButtons[1][i]->value())
			{
			if ((nFiles == 1) && (!templatePresent))
				{
				strcpy (fn, startFN);
				}
			else
				{
				sprintf (fn, "%s%1d%s", 
					startFN, i, endFN);
				}
			fprintf (stderr, "Reading light %d from file %s.\n", 
				i, fn);
			readLightFromFile (i, fn);
			sgeSendLightAll (LightSource + i);
			}
		}

	if (LButtons[1][8]->value())
		{
		// read light model file

		// first, make the file name
		if ((nFiles == 1) && (!templatePresent))
			{
			// take the file name as given
			strcpy (fn, startFN);
			}
		else
			{
			sprintf (fn, "%sModel%s", 
				startFN, endFN);
			int len = strlen (fn);
			if (len >= 4)
				{
				// if the template put .osg at the end, 
				// we're going to substitute .iris
				if (strcmp (fn+(len-5), ".osgt") == 0)
					{
					fn[len-5] = 0;
					strcat (fn, ".iris");
					}
				}
			}
		fprintf (stderr, "Reading light model from file %s.\n", fn);
		readLightModelFromFile (fn);
		sgeSendLightModel ();
		reconcileGuiToLightModel ();
		}




	reconcileGuiToCurrentLightSource ();

	int relPos[2];
	ReadDoneWin->getRelPos (relPos);
	ReadDoneWin->position (MainWindow->x()+relPos[0], 
                           MainWindow->y()+relPos[1]);
	ReadDoneWin->set_modal();
	ReadDoneWin->show();
	
	}  // end of readFilesCB




static void
writeFilesCB (Fl_Widget *w)
	{
	int nFiles = 0;

// fprintf (stderr, "writeFilesDB called\n");

	for (int i = 0; i < 8; i++)
		{
		if (LButtons[0][i]->value())
			{
			nFiles++;
			}
		}
// fprintf (stderr, "nFiles %d\n", nFiles);

	char startFN[5000], endFN[5000], fn[5000];

	strcpy (fn, OutputFNWidget->value());
	int templatePresent = getTemplatePieces (fn, startFN, endFN);
	
	for (int i = 0; i < 8; i++)
		{
		if (LButtons[0][i]->value())
			{
			if ((nFiles == 1) && (!templatePresent))
				{
				strcpy (fn, startFN);
				}
			else
				{
				sprintf (fn, "%s%1d%s", 
					startFN, i, endFN);
				}
			fprintf (stderr, "Writing light %d to file %s.\n", 
				i, fn);
			writeLightToFile (i, fn);
			}
		}

	if (LButtons[0][8]->value())
		{
		// write light model file

		// first, make the file name
		if ((nFiles == 1) && (!templatePresent))
			{
			// take the file name as given
			strcpy (fn, startFN);
			}
		else
			{
			sprintf (fn, "%sModel%s", 
				startFN, endFN);
			int len = strlen (fn);
			if (len >= 4)
				{
				// if the template put .osgt at the end, 
				// we're going to substitute .iris
				if (strcmp (fn+(len-5), ".osgt") == 0)
					{
					fn[len-5] = 0;
					strcat (fn, ".iris");
					}
				}
			}
		fprintf (stderr, "Writing light model to file %s.\n", fn);
		writeLightModelToFile (fn);
		}

	int relPos[2];
	WriteDoneWin->getRelPos (relPos);
	WriteDoneWin->position (MainWindow->x()+relPos[0], 
                            MainWindow->y()+relPos[1]);
	WriteDoneWin->set_modal();
	WriteDoneWin->show();
	
	}  // end of writeFilesCB






static void
selAllCB (Fl_Widget *w, long forInput)
	{

	for (int i = 0; i < 9; i++)
		{
		LButtons[forInput][i]->set ();
		}

	}  // end of selAllCB

static void
fnCB (Fl_Widget *w, long forInput)
	{
	Fl_Input *fnIn = (Fl_Input *) w;

#if 0
	fprintf (stderr, "fnCB:  widget %p  < %s >  %d\n", 
				fnIn, fnIn->value(), forInput);
#endif

	setTemplateButton (forInput);

	}  // end of fnCB


static void
removeCWD (char *path)
	{
	char cwd[5000];

	if (getcwd (cwd, sizeof (cwd)) == NULL)
		{
		return;
		}

	int len = strlen (cwd);

	if (strncmp (cwd, path, len) == 0)
		{
		if (path[len] == '/')
			{
			len++;
			}

		strcpy (cwd, path+len);
		strcpy (path, cwd);
		}

	}  // end of removeCWD


static void
browserButtonCB (Fl_Widget *w, long forInput)
	{
	char locFN[4000];


	if (forInput)
		{
		InputFileChooserWidget->show ();
		while (InputFileChooserWidget->visible())
			{
			Fl::wait ();
			}
	
		const char *fn = InputFileChooserWidget->value();

#if 0
		printf ("In browserButtonCB: file chooser value is: <%s>\n", 
			InputFileChooserWidget->value());
#endif


		if (fn == NULL)
			{
			return;
			}

		strcpy (locFN, fn);
		
		removeCWD (locFN);
		int len = strlen (locFN);
		InputFNWidget->value (locFN);
		InputFNWidget->position (len-1,len-1);
		}
	else
		{
		OutputFileChooserWidget->show ();
		while (OutputFileChooserWidget->visible())
			{
			Fl::wait ();
			}
	
		const char *fn = OutputFileChooserWidget->value();

#if 0
		printf ("Out browserButtonCB: file chooser value is: <%s>\n", 
			OutputFileChooserWidget->value());
#endif

		if (fn == NULL)
			{
			return;
			}

		strcpy (locFN, fn);

		removeCWD (locFN);
		int len = strlen (locFN);

// fprintf (stderr, "setting OutputFNWidget to %s\n", locFN);

		OutputFNWidget->value (locFN);
		OutputFNWidget->position (len-1,len-1);
		}

	
	setTemplateButton (forInput);
	}  // end of browserButtonCB

static void
fileChooserCB (Fl_File_Chooser *fc, void *data)
	{
	// printf ("fileChooserCB : <%s>\n", fc->value());
	}  // end of fileChooserCB


void rwModeCB (Fl_Widget *w, long whichButton)
        {

        // printf ("Button %d was selected for widget %p.\n", whichButton, w);
        switch (whichButton)
                {
                case 1:
                        WriteGroup->hide();
                        ReadGroup->show();
                        break;

                case 2:
                        WriteGroup->show();
                        ReadGroup->hide();
                        break;
                }


        }  // end of editModeCB


static void templateButtonCB (Fl_Widget *w, long forInput)
	{
	Fl_Input *fnw = (forInput) ? InputFNWidget : OutputFNWidget;

	char fn[2000];

	strcpy (fn, fnw->value());


	if (strstr (fn, "%ID") != NULL)
		{
		return;
		}

	int len = strlen (fn);


	if (len == 0)
		{
		strcpy (fn, "Light_%ID.osgt");
		}
	else if (len > 3)
		{
		if (strcmp (".osgt", fn+(len-5)) == 0)
			{

			fn[len-4] = 0;
			len -= 4;
			if (len != 0)
				{
				if ( isdigit (*(fn+len-1)) )
					{
					fn[len-1] = 0;
					}
				else if (len >= 5)
					{
					if ( (strcasecmp (fn+(len-5), "model") 
								== 0) )
						{
						fn[len-5] = 0;
						}
					}
				}


			strcat (fn, "%ID.osgt");
			}
		else
			{
			strcat (fn, ".%ID.osgt");
			}
		}


	len = strlen (fn);
	fnw->value (fn);
	fnw->position (len-1,len-1);

	setTemplateButton (forInput);

	}  // end of templateButtonCB

static void setTemplateButton (int forInput)
	{
	Fl_Button *but;
	Fl_Input *fnw;

	if (forInput)
		{
		fnw = InputFNWidget;
		but = InTemplateButton;
		}
	else
		{
		fnw = OutputFNWidget;
		but = OutTemplateButton;
		}


	if (strstr (fnw->value(), "%ID") == NULL)
		{
		but->activate ();
		}
	else
		{
		but->deactivate ();
		}

	}  // end of setTemplateButton


//
// End of callbacks
//
///////////////////////////////////////////////////////////////////////////
//
// Build the GUI
//



Fl_Group *buildLightSelectionGroup (int forInput, int x, int y, int w, int h)
	{
	static char idStr[8][20];


	Fl_Group *lselGroup = new Fl_Group (x, y, w, h);
	lselGroup->end ();

	int currX = x;
	int currY = y;

	currX += 45;

	Fl_Box *lLabel = new Fl_Box (currX, currY, 100, 30, "Select Lights:");
	lLabel->labelfont (FL_HELVETICA_ITALIC);
	lselGroup->add (lLabel);

	Fl_Button *selAllButton = 
	  new Fl_Button (currX+5, currY+40, 80, 80, "Select\nAll");
	lselGroup->add (selAllButton);
	SelAll[forInput] = selAllButton;
	SelAll[forInput]->callback (selAllCB, forInput);


	for (int i = 0; i < 8; i++)
		{
		int ii, jj, xx, yy;
		ii = i / 4;
		jj = i % 4;
		xx = currX + 130 + ii*80;
		yy = currY + 30*jj;
		sprintf (idStr[i], "Light %1d\n", i);
		Fl_Light_Button *lButton = 
			new Fl_Light_Button (xx, yy, 75, 30, idStr[i]);
		lselGroup->add (lButton);
		LButtons[forInput][i] = lButton;
		}

	currY += 125;

	Fl_Light_Button *lButton = 
	  new Fl_Light_Button (currX + 145, currY, 120, 30, "  Light Model  ");
	lselGroup->add (lButton);
	// lButton->deactivate ();
	LButtons[forInput][8] = lButton;
	currY += 35;


	return lselGroup;

	}  // end of buildLightSelectionGroup





Fl_Group *buildFileNameGroup (int forInput, int x, int y, int w, int h)
	{
	int currX = x;
	currX += 8;
	int currY = y;
	currY += 2;
	// static char labelText[2][100];
	const static char *labelText[2] = 
		{
		"Output file name / template",
		"Input file name / template"
		};

	Fl_Group *fnGroup = new Fl_Group (x, y, w, h);
	fnGroup->end ();

#if 0
	if (forInput)
		{
		strcpy (labelText[, "Input file name / template");
		}
	else
		{
		strcpy (labelText, "Output file name / template");
		}
#endif

	Fl_Box *fnLabel = new Fl_Box (currX, currY, 200, 30, 
						labelText[forInput]);
	fnLabel->align (FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	fnLabel->labelfont (FL_HELVETICA_ITALIC);
	fnGroup->add (fnLabel);

	currY += 25;

	if (forInput)
		{
		InputFNWidget = new Fl_Input (currX, currY, (w-currX)-10, 30);
		InputFNWidget->value ("Light_%ID.osgt");
		fnGroup->add (InputFNWidget);
		InputFNWidget->callback (fnCB, forInput);
		}
	else
		{
		OutputFNWidget = new Fl_Input (currX, currY, (w-currX)-10, 30);
		OutputFNWidget->value ("Light_%ID.osgt");
		fnGroup->add (OutputFNWidget);
		OutputFNWidget->callback (fnCB, forInput);
		}

	currY += 35;

	Fl_Button *browserButton = 
		new Fl_Button (currX, currY, 120, 30, "File Browser...");
	browserButton->callback (browserButtonCB, forInput);
	fnGroup->add (browserButton);

	Fl_Button *templateButton = 
		new Fl_Button (w-190, currY, 180, 30, 
					"Add Light ID Template");
	templateButton->callback (templateButtonCB, forInput);
	fnGroup->add (templateButton);
	templateButton->deactivate ();

	if (forInput)
		{
		InTemplateButton = templateButton;
		}
	else
		{
		OutTemplateButton = templateButton;
		}


	return fnGroup;
	}  // end of buildFileNameGroup





Fl_Group *buildWriteGroup (int x, int y, int w, int h)
	{
	static char idStr[8][20];

	Fl_Group *writeGroup = new Fl_Group (x, y, w, h);
	writeGroup->end ();

	int currX = x+5;
	int currY = y+5;
	int currW = w-5;
	int currH = 155;

	currY += 15;

	Fl_Group *lightSelGroup = 
		buildLightSelectionGroup (0, currX, currY,  currW, currH);
	writeGroup->add (lightSelGroup);


	currY += 170;
	currH = 100;



	Fl_Group *fnGroup =
		buildFileNameGroup (0, currX, currY,  currW, currH);
	writeGroup->add (fnGroup);


	////////////////////
	OutputFileChooserWidget = new Fl_File_Chooser
		(".", "*", Fl_File_Chooser::SINGLE, "Select Output File");

  	OutputFileChooserWidget->callback(fileChooserCB);

	OutputFileChooserWidget->newButton->hide();
	OutputFileChooserWidget->previewButton->hide();

	WriteDoneWin = new Msg_Window (100, y+h-150, 200, 100, 
						"File write complete.");

	Fl_Button *doIO = 
		new Fl_Button (x+w/2-100, y+h-48, 200, 40, "Write File(s)");
	doIO->box (FL_ROUND_UP_BOX);
	doIO->labelsize (18);
	doIO->callback (writeFilesCB);
	writeGroup->add (doIO);


	return writeGroup;
	}  // end of buildWriteGroup


Fl_Group *buildReadGroup (int x, int y, int w, int h)
	{

	Fl_Group *readGroup = new Fl_Group (x, y, w, h);
	readGroup->end ();


	int currX = x+5;
	int currY = y+8;
	int currW = w-5;
	int currH = 100;


	Fl_Group *fnGroup =
		buildFileNameGroup (1, currX, currY,  currW, currH);
	readGroup->add (fnGroup);

	currY += currH + 16;
	currH = 155;

	Fl_Group *lightSelGroup = 
		buildLightSelectionGroup (1, currX, currY,  currW, currH);
	readGroup->add (lightSelGroup);

	InputFileChooserWidget = new Fl_File_Chooser
		(".", "*", Fl_File_Chooser::SINGLE, "Select Input File");

  	InputFileChooserWidget->callback(fileChooserCB);

	InputFileChooserWidget->newButton->hide();
	InputFileChooserWidget->previewButton->hide();


	ReadDoneWin = new Msg_Window (100, y+h-150, 200, 100, 
						"File read complete.");

	Fl_Button *doIO = 
		new Fl_Button (x+w/2-100, y+h-48, 200, 40, "Read File(s)");
	doIO->box (FL_ROUND_UP_BOX);
	doIO->labelsize (18);
	doIO->callback (readFilesCB);

	readGroup->add (doIO);
	


	return readGroup;
	}  // end of buildReadGroup









Fl_Group *buildFileIOGroup (int x, int y, int w, int h)
	{

	Fl_Group *fileIOGroup = new Fl_Group (x, y, w, h, " Files ");
	fileIOGroup->end();


        Fl_Group *modeGroup = new Fl_Group (x, y, w, 30);
        modeGroup->end();

        Fl_Round_Button *readMode =
                new Fl_Round_Button (x+100, y+5, 100, 30, "Read");
	readMode->labelsize (20);
        readMode->type (FL_RADIO_BUTTON);
        readMode->set ();
        readMode->callback (rwModeCB, 1);
	modeGroup->add (readMode);

        Fl_Round_Button *writeMode =
                new Fl_Round_Button (x+220, y+5, 100, 30, "Write");
	writeMode->labelsize (20);
        writeMode->type (FL_RADIO_BUTTON);
        writeMode->callback (rwModeCB, 2);
	modeGroup->add (writeMode);

	fileIOGroup->add (modeGroup);

	Fl_Box *sep = new Fl_Box (H_CENTER_LINE_BOX, x+40, y+36, w-80, 3, "");
	fileIOGroup->add (sep);




	WriteGroup = buildWriteGroup (x, y+30, w, h-30);
	fileIOGroup->add (WriteGroup);
	WriteGroup->hide ();

	ReadGroup = buildReadGroup (x, y+30, w, h-30);
	fileIOGroup->add (ReadGroup);
	ReadGroup->show ();



	
	return fileIOGroup;
	}  // end of buildFileIOGroup


// End of building of the GUI
//
/////////////////////////////////////////////////////////////////////

#include <iostream>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osg/LightSource>

// #include <dtk.h>

static int
writeLightToFile (int lNum, char *outFN)
	{

// fprintf (stderr, "writeLightToFile %d %s\n", lNum, outFN);

	osg::LightSource *lsNode = new osg::LightSource ();
	osg::Light *light = lsNode->getLight();
	osg::StateSet *ss = lsNode->getOrCreateStateSet ();

	LightSourceDesc *ls = LightSource + lNum;



	// set lsNode according to ls

	// regardless of type, we set
	//     attenuations
	//     colors
	//     ref frame

	light->setConstantAttenuation (ls->atten[0]);
	light->setLinearAttenuation (ls->atten[1]);
	light->setQuadraticAttenuation (ls->atten[2]);

	osg::Vec4 color;

	color[0] = ls->color[0][0];
	color[1] = ls->color[0][1];
	color[2] = ls->color[0][2];
	color[3] = ls->color[0][3];
	light->setAmbient (color);

	color[0] = ls->color[1][0];
	color[1] = ls->color[1][1];
	color[2] = ls->color[1][2];
	color[3] = ls->color[1][3];
	light->setDiffuse (color);

	color[0] = ls->color[2][0];
	color[1] = ls->color[2][1];
	color[2] = ls->color[2][2];
	color[3] = ls->color[2][3];
	light->setSpecular (color);

	if (ls->referenceFrameIsRelative)
		{
		lsNode->setReferenceFrame (osg::LightSource::RELATIVE_RF);
		}
	else
		{
		lsNode->setReferenceFrame (osg::LightSource::ABSOLUTE_RF);
		}


	// Now we do slightly different things depending on the 
	// light type.

	osg::Vec4 position;
	osg::Vec3 direction;
	float cutoff;

	// We set the spot direction and exponent
	// even though they might not be operational based on the
	// other parameters.
	//
	// Note that the cutoff angle will be over-ridden below when we
	// have an OMNI light.

	direction[0] = ls->spotDirection[0];
	direction[1] = ls->spotDirection[1];
	direction[2] = ls->spotDirection[2];

	light->setDirection (direction);
	light->setSpotExponent (ls->spotExponent);

       
	// Now we set position and cutoff angle based on light type
        switch (ls->type)
                {
                case INFINITE:
			position[0] = ls->infDirection[0];
			position[1] = ls->infDirection[1];
			position[2] = ls->infDirection[2];
			position[3] = 0;
			// cutoff = ls->spotCutoff;
			cutoff = 180.0;
                        break;

                case OMNI:
			position[0] = ls->localLocation[0];
			position[1] = ls->localLocation[1];
			position[2] = ls->localLocation[2];
			position[3] = 1;
			cutoff = 180.0;
                        break;

                case SPOT:
			position[0] = ls->localLocation[0];
			position[1] = ls->localLocation[1];
			position[2] = ls->localLocation[2];
			position[3] = 1;
			cutoff = ls->spotCutoff;
                        break;
                }


	light->setPosition (position);
	light->setSpotCutoff (cutoff);


	///////////
	// We are removing any GL_LIGHT mode from the state set.
	// Do this as the last step before writing out the file.
	for (int i = 0; i < 8; i++)
		{
		ss->removeMode (GL_LIGHT0+i);
		}


	// Now write the light source node out to the specified file.
        // if ( ! osgDB::writeObjectFile(*lsNode, std::string(outFN)) )
        // seems like new osg prefers writeNodeFile to writeObjectFile
    if ( ! osgDB::writeNodeFile(*lsNode, std::string(outFN)) )
                {
		        fprintf (stderr, 
                       "Unable to write file \"%s\".\n", outFN) ;
                return 1 ;
                }


	return 0;
	}  // end of writeLightToFile

// extern void setLightSourceDefaults (int lightNum, LightSourceDesc *ls);

static int
readLightFromFile (int lNum, char *inFN)
	{
        osg::Node *inNode = osgDB::readNodeFile(std::string(inFN)) ;
        if (inNode == NULL)
                {
		fprintf (stderr, 
                       "Unable to read file \"%s\".\n", inFN) ;
		return 1;
                }

	osg::LightSource *lsNode = dynamic_cast<osg::LightSource *> (inNode);
	if (lsNode == NULL)
                {
		fprintf (stderr, 
                       "File \"%s\" is not a light file.\n", inFN) ;
		return 1;
                }

	osg::Light *light = lsNode->getLight();
	osg::StateSet *ss = lsNode->getOrCreateStateSet ();

	LightSourceDesc *ls = LightSource + lNum;

	// We're going to fill in the LightSourceDesc strcture based
	// on the contents of lsNode.

	// There are some items in our data structure that are not
	// in the node data structure, so we are first going to set
	// everything (except the node name, on/off, and parent node) 
	// to default values.
	char tmpName[1000];
	strcpy (tmpName, ls->name);
	int on = ls->on;
	int node = ls->node;

	setLightSourceDefaults (lNum, ls);

	strcpy (ls->name, tmpName);
	ls->on = on;
	ls->node = node;



	// set *ls structure according to lsNode
	osg::Vec4 color;
	osg::Vec4 position;
	osg::Vec3 direction;
	double sc;

        ls->atten[0] = light->getConstantAttenuation ();
        ls->atten[1] = light->getLinearAttenuation ();
        ls->atten[2] = light->getQuadraticAttenuation ();

	color = light->getAmbient ();
	ls->color[0][0] = color[0];
	ls->color[0][1] = color[1];
	ls->color[0][2] = color[2];
	ls->color[0][3] = color[3];

	color = light->getDiffuse ();
	ls->color[1][0] = color[0];
	ls->color[1][1] = color[1];
	ls->color[1][2] = color[2];
	ls->color[1][3] = color[3];

	color = light->getSpecular ();
	ls->color[2][0] = color[0];
	ls->color[2][1] = color[1];
	ls->color[2][2] = color[2];
	ls->color[2][3] = color[3];


	ls->referenceFrameIsRelative =  
	    (lsNode->getReferenceFrame () == osg::LightSource::RELATIVE_RF);


	direction = light->getDirection ();
	ls->spotDirection[0] = direction[0];
	ls->spotDirection[1] = direction[1];
	ls->spotDirection[2] = direction[2];
	ls->spotExponent = light->getSpotExponent ();




	position = light->getPosition ();
	sc = light->getSpotCutoff ();


	if (position[3] == 0)
		{
		ls->type = INFINITE;
		ls->infDirection[0] = position[0];
		ls->infDirection[1] = position[1];
		ls->infDirection[2] = position[2];


		// In case we want to convert this to OMNI or SPOT
		// set the local location and spotCutoff
		ls->localLocation[0] = position[0];
		ls->localLocation[1] = position[1];
		ls->localLocation[2] = position[2];
		ls->spotCutoff = sc;

		}
	else
		{
		ls->localLocation[0] = position[0] / position[3];
		ls->localLocation[1] = position[1] / position[3];
		ls->localLocation[2] = position[2] / position[3];
		if (sc == 180.0)
			{
			ls->type = OMNI;
			// in case we want to convert light type, 
			// we must set infDirection and spotCutoff 
			ls->spotCutoff = 20.0;
			ls->infDirection[0] = ls->localLocation[0];
			ls->infDirection[1] = ls->localLocation[1];
			ls->infDirection[2] = ls->localLocation[2];
			}
		else
			{
			ls->type = SPOT;
			ls->spotCutoff = sc ;
			// in case we want to convert light type, 
			// we must set infDirection 
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

	return 0;
	}  // end of readLightFromFile

static int
writeLightModelToFile (char *outFN)
	{
	FILE *outFP = fopen (outFN, "w");

	if (outFP == NULL)
		{
		fprintf (stderr, 
		  "hev-lightEditor: "
		  "Unable to open output light model file \"%s\"\n", outFN);
		return -1;
		}

	fprintf (outFP, "LIGHTMODEL AMBIENT %g %g %g %g\n", 
		LightModel.ambient[0], 
		LightModel.ambient[1], 
		LightModel.ambient[2], 
		LightModel.ambient[3]);

	fprintf (outFP, "LIGHTMODEL TWO_SIDED %s\n",
	    LightModel.twoSided ? "TRUE" : "FALSE" );

	fprintf (outFP, "LIGHTMODEL LOCAL_VIEWER %s\n",
	    LightModel.localViewer ? "TRUE" : "FALSE" );

	fprintf (outFP, "LIGHTMODEL COLOR_CONTROL %s\n",
	    LightModel.colorControlSeparate ? "SEPARATE_SPECULAR" : "SINGLE" );

	fclose (outFP);

	return 0;
	} // end of writeLightModelToFile


static void
displayInputParseError (char *line, char *inFN)
	{
	fprintf (stderr, 
	  "hev-lightEditor: Error parsing light model file \"%s\" on line:\n",
		 inFN);
	fprintf (stderr, "   \"%s\"\n", line);
	}  // end of displayInputParseError


static int
readLightModelFromFile (char *inFN)
	{
	FILE *inFP = fopen (inFN, "r");
	LightModelDesc savedLightModel;
	char line[1000];

	if (inFP == NULL)
		{
		fprintf (stderr, 
		  "hev-lightEditor: "
		  "Unable to open input light model file \"%s\"\n", inFN);
		return -1;
		}


	// preserve the current light model settings
	memcpy (&savedLightModel, &LightModel, sizeof (LightModel));

	// set the current light model to the defaults
	setLightModelDefaults ();


	// We're reading an sge file, so it could contain sge commands
	// other than LIGHTMODEL commands as well as comments.  
	// Let's just ignore unrecognized commands, then report errors if 
	// any of the LIGHTMODEL lines are incorrectly formed or if there 
	// are no valid LIGHTMODEL lines at all.
	//
	// Chances are that we are being more lenient than sge will
	// be in parsing these lines.

	int validLineFound = 0;

	while (fgets (line, sizeof(line), inFP) != NULL)
		{
		std::vector<std::string> vec;

		cStrToVec (line, vec);

		if (! iris::IsSubstring ("lightmodel", vec[0]) )
			{
			// not a light model command; that's OK
			}

    		else if (iris::IsSubstring("ambient", vec[1]))
    			{

        		if (vec.size() < 6)
        			{
				displayInputParseError (line, inFN);
        			}
       			else 
        			{
        			// vec[2] through vec[5] are ambient rgba
				int lineOK = 1;
				double ambient[4];
        			for (int i = 0; i < 4; i++)
                			{
                			if (sscanf (vec[2+i].c_str(), "%lf",
                                	                  &(ambient[i])) != 1)
                        			{
						displayInputParseError (line, 
									 inFN);
                        			lineOK = 0;
                        			}
                			}

				if (lineOK)
					{
					validLineFound = 1;
					memcpy (LightModel.ambient, 
						  ambient, 4*sizeof(double));
					}
					
        			}
    			}  // end of "ambient" subcommand

    		else if (iris::IsSubstring("color_control", vec[1]))
    			{

        		if (vec.size() < 3)
        			{
				displayInputParseError (line, inFN);
        			}
        		else if (iris::IsSubstring("SINGLE", vec[2]))
        			{
				LightModel.colorControlSeparate = 0;
				validLineFound = 1;
        			}
        		else if (iris::IsSubstring("SEPARATE_SPECULAR", 
								vec[2]))
        			{
				LightModel.colorControlSeparate = 1;
				validLineFound = 1;
        			}
        		else
        			{
				displayInputParseError (line, inFN);
        			}

    			}  // end of color_control subcommand

    		else if (iris::IsSubstring("local_viewer", vec[1]))
    			{

        		if (vec.size() < 3)
        			{
				displayInputParseError (line, inFN);
        			}
        		else if (iris::IsSubstring("TRUE", vec[2]))
        			{
				LightModel.localViewer = 1;
				validLineFound = 1;
        			}
        		else if (iris::IsSubstring("FALSE", vec[2]))
        			{
				LightModel.localViewer = 0;
				validLineFound = 1;
        			}
        		else
        			{
				displayInputParseError (line, inFN);
        			}

    			}  // end of local_viewer subcommand

    		else if (iris::IsSubstring("two_sided", vec[1]))
    			{

        		if (vec.size() < 3)
        			{
				displayInputParseError (line, inFN);
        			}
        		else if (iris::IsSubstring("TRUE", vec[2]))
        			{
				LightModel.twoSided = 1;
				validLineFound = 1;
        			}
        		else if (iris::IsSubstring("FALSE", vec[2]))
        			{
				LightModel.twoSided = 0;
				validLineFound = 1;
        			}
        		else
        			{
				displayInputParseError (line, inFN);
        			}
			}
		}

	if ( ! validLineFound )
		{
		memcpy (&LightModel, &savedLightModel, sizeof (LightModel));
		fprintf (stderr, 
		  "hev-lightEditor: "
		  "No valid light model commands found in input file \"%s\"\n",
		  inFN);
		}

	fclose (inFP);

	return 0;
	} // end of readLightModelFromFile
