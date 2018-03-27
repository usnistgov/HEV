#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <dtk.h>
#include <string>


#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Adjuster.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_XBM_Image.H>
#include <FL/Fl_GIF_Image.H>
#include <FL/Fl_XPM_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_PNM_Image.H>

#define MIN(x,y)	(((x)<(y))?(x):(y))
#define MAX(x,y)	(((x)>(y))?(x):(y))



#define MAX_POINTS 1000

const char *ProgName = "hev-alphaAdjust";
char UserName[200];

// ignore escapes and close window buttons
void doNothingCB(Fl_Widget*, void*) { } ;


static
Fl_Box *makeLabel (int x, int y, int w, int h, char *str)
        {
        Fl_Box *label = new Fl_Box (FL_FLAT_BOX, x, y, w, h, "");
	label->label(str);
        label->align (FL_ALIGN_INSIDE | FL_ALIGN_CENTER);

        return label;
        }       // end of makeLabel

//////////////////////////////////////////////////////////////////


Fl_Adjuster *Adj[3];

Fl_Output *OutP[3];

Fl_Button *Reset[3];

float DefaultValues[3] = {0.0, 1.0, 1.0};
float mins[3] = {-1.0,   0.0, 0.0001};
float maxs[3] = { 1.0, 200.0, 100.0};
const char *UniNames[3] = {"AlphaOffset", "AlphaScale", "OpticalScale"};

int Index[3] = {0, 1, 2};

char NodeName[1000];
float InitAlphaOffset = 0;
float InitAlphaScale = 1;
float InitDensityScale = 1;

static void resetCallback (Fl_Widget *w, void *data)
    {
    int index = *(int*) data;

    char s[100];
    sprintf (s, " %.5g", DefaultValues[index]);
    OutP[index]->value(s);
    Adj[index]->value(DefaultValues[index]);
    printf ("UNIFORM %s %s float 1 %.9g\n", 
            NodeName, UniNames[index], Adj[index]->value());
    fflush (stdout);
    } // end of resetCallback

static void adjCallback (Fl_Widget *w, void *data)
    {
    int index = *(int*) data;
    char s[100];

    if (index == 0)
        {
        float val = Adj[index]->value();
        if (val < -1)
            {
            Adj[index]->value(-1.0);
            }
        else if (val > 1)
            {
            Adj[index]->value(1.0);
            }
        }
    else if (index == 2)
        {
        float val = Adj[index]->value();
        if (val < 0.0001)
            {
            Adj[index]->value(0.0001);
            }
        }
    sprintf (s, " %.5g", Adj[index]->value());
    OutP[index]->value(s);
    printf ("UNIFORM %s %s float 1 %.9g\n", 
            NodeName, UniNames[index], Adj[index]->value());
    fflush (stdout);
    }

static Fl_Group *makeAdjGroup (int x, int y, int w, int h, const char *label, int index, const char * toolTip)
    {
    Fl_Group *adjGroup = new Fl_Group (x, y, w, h);

    y += 1;
    h -= 2;

    Fl_Box *lab = new Fl_Box (FL_FLAT_BOX, x, y, 98, h, "");
	lab->label(label);
    lab->align (FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
    lab->tooltip (toolTip);

    Fl_Output *outp = new Fl_Output (x+100, y, 98, h);

    Fl_Adjuster *adj = new Fl_Adjuster (x+200, y, 148, h, "");


    Fl_Button *resetButton = new Fl_Button (x+350, y, 48, h, "reset");

    Adj[index] = adj;
    OutP[index] = outp;
    Reset[index] = resetButton;

    adj->callback (adjCallback, (void *) (Index+index));
    adj->range (mins[index], maxs[index]);
    resetButton->callback (resetCallback, (void *) (Index+index));

    adjGroup->add(lab);
    adjGroup->add(outp);
    adjGroup->add(adj);
    adjGroup->add(resetButton);

    resetCallback (resetButton, (void *) (Index+index) );

    return adjGroup;
    }  // end of makeAdjGroup


static Fl_Group *makeUIGroup ()
    {
    Fl_Group *uiGroup = new Fl_Group (0, 0, 400, 90);

    const char * toolTips[3] = {
        " new alpha = (alpha + offset) * scale ",
        " new alpha = (alpha + offset) * scale ",
        " relative density of the volume material " };

    Fl_Group *o = makeAdjGroup (0,   0, 400, 30, "Alpha Offset", 0, toolTips[0]);
    Fl_Group *s = makeAdjGroup (0,  30, 400, 30, "Alpha Scale", 1, toolTips[1]);
    Fl_Group *d = makeAdjGroup (0,  60, 400, 30, "Density", 2, toolTips[2]);

    uiGroup->add (o);
    uiGroup->add (s);
    uiGroup->add (d);
    return uiGroup;
    }

static Fl_Window *makeUI ()
	{
	Fl_Window *window = new Fl_Window (400, 90);

	// window->resizable (window);
	// window->size_range (50, 50);
	window->end();

	// Fl_Pack * uiGroup = makeUIGroup ();
	Fl_Group * uiGroup = makeUIGroup ();
	window->add (uiGroup);

	return window;
	}  // end of makeUI

static void
usage ()
    {
    fprintf (stderr, "Usage: \n"
    "  hev-volOpacityAdjust [--init offset scale density] [--noescape] nodeName\n");
    }  // end of usage()


int main(int argc, char **argv)
	{


    // send dtk messages to stdout
    dtkMsg.setFile(stderr) ;

    // separate out the fltk arguments from the others
    int fltk_argc;
    char **fltk_argv;
    int other_argc;
    char **other_argv;

    if(dtkFLTKOptions_get(argc, (const char **) argv,
                          &fltk_argc,  &fltk_argv,
                          &other_argc, &other_argv))
        {
        exit (-1);
        dtkMsg.add (DTKMSG_ERROR, 
            "hev-volOpacityAdjust: Error parsing command line arguments.\n");
        usage ();
        return -1;
        }

    // printf ("other_argc = %d\n", other_argc);
    // printf ("other_argv[0] = %s\n", other_argv[0]);
    if ( (other_argc != 2) && 
         (other_argc != 3) &&
         (other_argc != 6) &&
         (other_argc != 7)     )
        {
        dtkMsg.add (DTKMSG_ERROR, 
                "hev-volOpacityAdjust: Bad argument count.\n");
        usage ();
        return -1;
        }

    strcpy (NodeName, other_argv[other_argc-1]);

    bool noEscape = false;

    if (other_argc == 2)
        {
        // nothing has to be done
        }
    else if (other_argc == 3)
         {
        if (strcasecmp (other_argv[1], "--noescape") == 0)
            {
            noEscape = true;
            }
        else
            {
            dtkMsg.add (DTKMSG_ERROR, 
                "hev-volOpacityAdjust: Unrecognized argument: %s\n", 
                other_argv[1]);
            usage ();
            return -1;
            }
        }
    else 
        {
        int initArg = 1;
        if (other_argc == 7)
            {
            if (strcasecmp (other_argv[1], "--noescape") == 0)
                {
                noEscape = true;
                initArg = 2;
                }
            else if (strcasecmp (other_argv[5], "--noescape") == 0)
                {
                noEscape = true;
                initArg = 1;
                }
            else
                {
                dtkMsg.add (DTKMSG_ERROR, 
                  "hev-volOpacityAdjust: Bad arguments.\n" );
                usage ();
                return -1;
                }

            }

        if (strcasecmp (other_argv[initArg], "--init") != 0)
            {
            dtkMsg.add (DTKMSG_ERROR, 
                  "hev-volOpacityAdjust: Bad arguments.\n" );
            usage ();
            return -1;
            }

        if (sscanf (other_argv[initArg+1], "%f", &InitAlphaOffset) != 1)
            {
            dtkMsg.add (DTKMSG_ERROR, 
                "hev-volOpacityAdjust: Error in alpha offset argument : %s.\n",
                other_argv[initArg+1] );
            usage ();
            return -1;
            }

        if (sscanf (other_argv[initArg+2], "%f", &InitAlphaScale) != 1)
            {
            dtkMsg.add (DTKMSG_ERROR, 
                "hev-volOpacityAdjust: Error in alpha scale argument : %s.\n",
                other_argv[initArg+2] );
            usage ();
            return -1;
            }

        if (sscanf (other_argv[initArg+3], "%f", &InitDensityScale) != 1)
            {
            dtkMsg.add (DTKMSG_ERROR, 
                "hev-volOpacityAdjust: Error in density argument : %s.\n",
                other_argv[initArg+3] );
            usage ();
            return -1;
            }


        }


    if (fabs (InitAlphaOffset > 1) )
        {
        mins[0] = -fabs(InitAlphaOffset);
        maxs[0] =  fabs(InitAlphaOffset);
        }

    if (InitAlphaScale < 0)
        {
        dtkMsg.add (DTKMSG_ERROR, 
               "hev-volOpacityAdjust: Error : alpha scale must be positive.\n");
        usage ();
        return -1;
        }

    maxs[1] = MAX (maxs[1], 20*InitAlphaScale);

    if (InitDensityScale <= 0)
        {
        dtkMsg.add (DTKMSG_ERROR, 
             "hev-volOpacityAdjust: Error : density scale must be positive.\n");
        usage ();
        return -1;
        }

    mins[2] = MIN (mins[2], InitDensityScale*0.05);
    if (mins[2] == 0.0)
        {
        mins[2] = InitDensityScale;
        }
    
    maxs[2] = MAX (200.0, 20*InitDensityScale);

    DefaultValues[0] = InitAlphaOffset;
    DefaultValues[1] = InitAlphaScale;
    DefaultValues[2] = InitDensityScale;

	Fl_Window *window = makeUI ();
    if (noEscape)
        {
        window->callback (doNothingCB);
        }
	window->show (fltk_argc, fltk_argv);

	return Fl::run();
	}  // end of main


