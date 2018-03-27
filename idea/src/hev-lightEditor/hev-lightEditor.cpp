//////////////////////////////////////////////////////////////////
//
// FLTK program to control lighting through the sge fifo dso.
//
//
// If I were doing this again, I would probably do more subclassing
// of the FLTK widgets.  This would reduce the amount of repeated
// code, and would make things cleaner.
// 
// On the whole, my C++ code tends to look mostly like C. So be it.
// 
// 
// 
// 


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <string>

#include "General_GUI_S.h"
#include "InitLights.h"
#include "sgeComm.h"

using namespace std ;



/////////////////////////////////////////////////////////////////////
//
// Init and run
//

static int
initData ()
	{
	initAllLightData ();
	return 0;
	}  // end of initData


static int
getSgeLights ()
	{
    char fifoName[1000];
	int rtn = 0;

    strcpy (fifoName, "/tmp/hev-lightEditor-fifo-");

    char *userName = getenv ("USER");
    if (userName == NULL)
        {
        fprintf (stderr, 
				"hev-lightEditor: "
				"Unable to get user name ($USER).\n");
        return -1;
        }

    strcat (fifoName, userName);

	if ( rtn |= setupSgeRecvShm (fifoName, 1000, 100) )
		{
		return rtn;
		}

	if ( ! (rtn |= establishCommunicationWithSGE ()) )
		{
		rtn |= getCurrentLightSourcesFromSGE ();
		rtn |= getLightModelFromSGE ();
		}

	return rtn;
	}  // end of getSgeLights



static int
init ()
	{
	int rtn = 0;

	rtn |= initData ();
        rtn |= getSgeLights ();
	rtn |= sendAllLightData ();
	rtn |= initGUI ();
	rtn |= reconcileGuiToData ();

	return rtn;
	}  // end of init

static void
usage ()
	{

	fprintf (stderr, "\n");
	fprintf (stderr, "Usage:  hev-lightEditor [ options ]\n");
	fprintf (stderr, "\n");
	fprintf (stderr, "  options:\n");
	fprintf (stderr, "\n");
	fprintf (stderr, "    -e  echo all communication with IRIS\n");
	fprintf (stderr, "\n");
	fprintf (stderr, "    -h  display help message\n");
	fprintf (stderr, "\n");

	fprintf (stderr, 
	"  For more information, see: \n");
	fprintf (stderr, "\n");
	fprintf (stderr, 
	"    file:///usr/local/HEV/idea/src/hev-lightEditor/doc/hev-lightEditor.html\n");
	fprintf (stderr, "\n");
	fprintf (stderr, 
    "  or at the corresponding location in /usr/local/HEV-beta.\n");
	fprintf (stderr, "\n");
	fprintf (stderr, "\n");

	} // end of usage


static void
handleArgs (int argc, char **argv)
	{

	for (int i = 1; i < argc; i++)
		{
		if (strcmp (argv[i], "-e") == 0)
			{
			setSgeCommEcho (1);
			}
		else if (strcmp (argv[i], "-h") == 0)
			{
			usage ();
			exit (0);
			}
		else
			{
			fprintf (stderr, 
				"hev-lightEditor: "
				"Bad command line argument: < %s >.\n", 
				argv[i]);
			usage ();
			exit (-1);
			}
		}

	}  // end of handleArgs

int main(int argc, char **argv) 
	{

	handleArgs (argc, argv);

	init ();

	runGUI (1, argv);

	return 0 ;

	}  // end of main
//
// End of init and run
//
////////////////////////////////////////////////////////////////////////


