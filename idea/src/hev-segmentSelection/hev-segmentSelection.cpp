#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dtk.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <iris.h>

#include "hssOps.h"
#include "hssDBHL.h"
#include "hssCmd.h"
#include "hssFSM.h"

#include "hev-segmentSelection.h"




/*


Things to do:

	cmd line option to do calc input and output coord transformations
		either continuously or only once at the start.

	don't send DCS transforms to sge if they are unchanged

	route all error messages through a single wrapper to fprintf (stderr,

	test input & output coord node calculations

	insert verbose output

	data structures for end points and segment handles
	(Maybe make a class with access methods to keep the in-memory data
	 and the shared mem data in sync.)
	

	implement fsm

	complete usage message


*/




bool Verbose = false;
bool Debug = false;

char ProgName[1000];

char TmpDirName[1000];

static bool ActivateOnStartup = false;


static double GlyphScale = 1.0;

static void
usage ()
	{


fprintf (stderr, 
"\n"
"Usage:   hev-segmentSelection  [ options ]\n"
"\n"
"    Options:\n"
"        --verbose\n"
"        --help\n"
"        --active\n"
"        --ptShm        outPointsShmName  \n"
"                            (default: hev-segmentSelection/segmentData)\n"
"        --buttons      editButShmName delButShmName  \n"
"                            (default: idea/buttons/left idea/buttons/right)\n"
"        --fifo\n"
"        --selectorShm  shmName              (default: idea/selector)\n"
"        --selectorStr  string               (default: hev-segmentSelection)\n"
"        --inCoordShm   trackShmName         (default: idea/worldOffsetWand)\n"
"        --outNode      nodeName             (default: world)\n"
"        --maxSegments  n                    (default: 200)\n"
"        --lenLabels    [fmt]                (default: no labels)\n"
"        --coordScale   scale                (default: 1)\n"
"        --glyphScale   factor               (default: 1)\n"
"        --ptDist       distance             (default: 0.02)\n"
"        --lnDist       distance             (default: 0.02)\n"
"        --usleep       t                    (default: iris::GetUsleep()\n"
"        --ptMarkerFN   ptFN  hl1PtFN hl2PtFN hl3PtFN\n"
"        --lnFN         lineFN hl1LineFN  hl2LineFN  hl3LineFN\n"
"        --lnHandleFN   handleFN hl1HandleFN hl2HandleFN hl3HandleFN\n"
"\n"
"    For more information, see the man page.\n"
"\n"
);
	} // end of usage


static void
badArgValue (char *optionName, char *val)
	{
	fprintf (stderr, "%s: Option \"%s\" had bad value \"%s\".\n",
		ProgName, optionName, val);
	}  // end of badArgValue

static int
extractOption ( 
  int argc, 
  char **argv, 
  int iArg, 
  int *argMap, 
  int minVals, 
  int maxVals,
  int &nVals, 
  int &startVal)

	{
	if (argMap != NULL)
		{
		argMap[iArg] = 1;
		}

	startVal = iArg + 1;
	nVals = 0;


	for (int i = 0; i < maxVals; i++)
		{

		// the variable tooFew signals whether there are fewer
		// than maxVals found

		// first check if we're going beyond the end of the arg array
		bool tooFew = (startVal+i) >= argc;
		if ( argMap != NULL )
			{
			// and check if the next arg has already been taken
			// as an option
		    	tooFew |= (argMap[iArg+1+i] == 1);
		    	}

		// check if next arg starts with "--"
		tooFew |= (strncmp (argv[startVal+i], "--", 2) == 0);

		if (tooFew )
			{
			if (i < minVals)
				{
				// if there are fewer than minVals found
				fprintf (stderr, 
					"%s: Error parsing option \"%s\"; "
					"not enough values.\n", 
					ProgName, argv[iArg]);
				return -1;
				}
			break;
			}


		nVals++;
		if (argMap)
			{
			argMap[iArg+1+i] = 1;
			}
		}


	return 1;
	}  // end of extractOption





int 
getOption (
  int argc, 
  char **argv, 
  int *argMap, 
  char *name, 
  int minNameLen, 
  int minNumArgs, 
  int maxNumArgs,
  int &nArgs, 
  int &startArg)
	{
	// return 0 if the argument was not matched
	// return 1 if the argument was matched 
	// return -1 on error


	for (int i = 0; i < argc; i++)
		{

		bool checkArg ;

		if (argMap != NULL)
			{
			checkArg = ! argMap[i];
			}
		else
			{
			checkArg = 1;
			}

		if (checkArg)
			{
			// if argMap says this arg is unused

			if (strncmp (argv[i], "--", 2) == 0)
				{
				// first two chars are --


				int len = strlen (argv[i]+2);

				if ( (len >= minNameLen) && 
						(len <= strlen (name) ) )
					{
					// length of arg is within range

					if (strncasecmp (argv[i]+2, 
						              name, len) == 0)
						{
						// name matches

						if (argMap)
							{
							argMap[i] = 1;
							}

						return extractOption (
							argc, 
							argv, 
							i, 
							argMap, 
							minNumArgs, 
							maxNumArgs,
							nArgs, 
							startArg);

						}
					}
				}
			}
		}

		
	return 0;
	}  // end of getOption



static int
housekeepingArgs (int argc, char **argv, int *argMap)
	{
	int rtn;
	int nVals, startVal;
	int maxSeg;
	

	rtn = getOption ( argc, argv, argMap, 
			  const_cast<char*>("verbose"), 1, 0, 0, nVals, startVal);
	if (rtn < 0)
		{
		return -1;
		}
	else if (rtn > 0)
		{
		Verbose = true;
		// sendSgeMsg ("ECHO ON\n");
		}



	rtn = getOption ( argc, argv, argMap, 
			  const_cast<char*>("debug"), 1, 0, 0, nVals, startVal);
	if (rtn < 0)
		{
		return -1;
		}
	else if (rtn > 0)
		{
		Debug = true;
		sendSgeMsg ("ECHO ON\n");
		}





	rtn = getOption ( argc, argv, argMap, 
			  const_cast<char*>("help"), 1, 0, 0, nVals, startVal);
	if (rtn < 0)
		{
		return -1;
		}
	else if (rtn > 0)
		{
		usage ();
		EXIT (0);
		}

	rtn = getOption ( argc, argv, argMap, 
			  const_cast<char*>("active"), 1, 0, 0, nVals, startVal);
	if (rtn < 0)
		{
		return -1;
		}
	else if (rtn > 0)
		{
		ActivateOnStartup = true;
		}


	rtn = getOption ( argc, argv, argMap, 
			  const_cast<char*>("maxSegments"), 3, 1, 1, nVals, startVal);
	if (rtn < 0)
		{
		return -1;
		}
	else if (rtn > 0)
		{
		if (sscanf (argv[startVal], "%d", &maxSeg) != 1)
			{
			badArgValue (argv[startVal-1], argv[startVal]);
			return -1;
			}
		}
	else
		{
		maxSeg = 200;
		}

	setMaxSegments (maxSeg);


	rtn = getOption ( argc, argv, argMap, 
			  const_cast<char*>("usleep"), 3, 1, 1, nVals, startVal);
        unsigned long uslp;
	if (rtn < 0)
		{
		return -1;
		}
	else if (rtn > 0)
		{
                int u;
		if (sscanf (argv[startVal], "%d", &u) != 1)
			{
			badArgValue (argv[startVal-1], argv[startVal]);
			return -1;
			}
                uslp = u;
		}
	else
                {
                uslp = iris::GetUsleep();
                }

        setSleep (uslp);


	return 0;
	} // end of housekeepingArgs


static int
shmAndNodeArgs (int argc, char **argv, int *argMap)
	{
	  char *ptsShmName = const_cast<char*>("hev-segmentSelection/segmentData");
	  char *editButShmName = const_cast<char*>("idea/buttons/left");
	  char *delButShmName = const_cast<char*>("idea/buttons/right");
	  char *inCoordShmName = const_cast<char*>("idea/worldOffsetWand");
	  char *outNode = const_cast<char*>("world");

	int rtn;
	int nVal, startVal;

// fprintf(stderr, "argc = %d  argv[13] %s (%d)  argv[14] (%d) %s\n", argc,
// argv[13], argMap[13], argv[14], argMap[14] );

	rtn = getOption ( argc, argv, argMap, 
			  const_cast<char*>("ptShm"), 3, 1, 1, nVal, startVal);
	if (rtn < 0)
		{
		return -1;
		}
	else if (rtn > 0)
		{
		ptsShmName = argv[startVal];
		}



	rtn = getOption ( argc, argv, argMap, 
			  const_cast<char*>("outNode"), 4, 1, 1, nVal, startVal);
	if (rtn < 0)
		{
		return -1;
		}
	else if (rtn > 0)
		{
		outNode = argv[startVal];
		}
        else
                {
                // if arg outNode not found then try outCoordNode
	        rtn = getOption ( argc, argv, argMap, 
				  const_cast<char*>("outCoordNode"), 8, 1, 1, nVal, startVal);
	        if (rtn < 0)
		        {
		        return -1;
		        }
	        else if (rtn > 0)
		        {
		        outNode = argv[startVal];
		        }
                }

	setupPtsShm (ptsShmName, outNode);


	rtn = getOption ( argc, argv, argMap, 
			  const_cast<char*>("buttons"), 3, 2, 2, nVal, startVal);
	if (rtn < 0)
		{
		return -1;
		}
	else if (rtn > 0)
		{
		editButShmName = argv[startVal];
		delButShmName = argv[startVal+1];
		}


	if (setupButtons (editButShmName, delButShmName))
		{
		return -1;
		}


	rtn = getOption ( argc, argv, argMap, 
			  const_cast<char*>("inCoordShm"), 8, 1, 1, nVal, startVal);
	if (rtn < 0)
		{
		return -1;
		}
	else if (rtn > 0)
		{
		inCoordShmName = argv[startVal];
		}


	
	if (setupInShm (inCoordShmName))
		{
		return -1;
		}


	rtn = getOption ( argc, argv, argMap, 
			  const_cast<char*>("fifo"), 4, 0, 0, nVal, startVal);
	if (rtn < 0)
		{
		return -1;
		}
	else if (rtn > 0)
		{
		setupCmdFifo ();
		}


        char *inCtrlShm = const_cast<char*>("idea/selector");
        char *ctrlID = const_cast<char*>("hev-segmentSelection");


        rtn = getOption ( argc, argv, argMap,
			  const_cast<char*>("selectorShm"), 10, 1, 1, nVal, startVal);
        if (rtn < 0)
                {
                return -1;
                }
        else if (rtn > 0)
                {
                inCtrlShm = argv[startVal];

                }


        rtn = getOption ( argc, argv, argMap,
			  const_cast<char*>("selectorStr"), 10, 1, 1, nVal, startVal);
        if (rtn < 0)
                {
                return -1;
                }
        else if (rtn > 0)
                {
                ctrlID = argv[startVal];
                }



        if (setupCtrlShm (inCtrlShm, ctrlID))
                {
                return -1;
                }




	return 0;
	}  // end of shmAndNodeArgs



static int
objFileArgs (int argc, char **argv, int *argMap)
	{
	char objFNames[3][4][1000];
	int rtn;
	int nVal, startVal;
	double glyphScale = 1.0;
	float yOffset = 0.08;


	for (int i = 0; i < 3; i++)
		{
		for (int j = 0; j < 4; j++)
			{
			objFNames[i][j][0] = 0;
			}
		}

	rtn = getOption ( argc, argv, argMap, 
			  const_cast<char*>("glyphScale"), 3, 1, 1, nVal, startVal);
	if (rtn < 0)
		{
		return -1;
		}
	else if (rtn > 0)
		{
		if (sscanf (argv[startVal], "%lf", &glyphScale) != 1)
			{
			badArgValue (argv[startVal-1], argv[startVal]);
			return -1;
			}
        GlyphScale = glyphScale;
		}



#if 0
	rtn = getOption ( argc, argv, argMap, 
			  const_cast<char*>("offset"), 4, 1, 1, nVal, startVal);
	if (rtn < 0)
		{
		return -1;
		}
	else if (rtn > 0)
		{
		if (sscanf (argv[startVal], "%f", &yOffset) != 1)
			{
			badArgValue (argv[startVal-1], argv[startVal]);
			return -1;
			}
		}

	setYOffset (yOffset);
#endif




	rtn = getOption ( argc, argv, argMap, 
			  const_cast<char*>("ptMarkerFN"), 3, 4, 4, nVal, startVal);
	if (rtn < 0)
		{
		return -1;
		}
	else if (rtn > 0)
		{
		strcpy (objFNames[0][0], argv[startVal]);
		strcpy (objFNames[0][1], argv[startVal+1]);
		strcpy (objFNames[0][2], argv[startVal+2]);
		strcpy (objFNames[0][3], argv[startVal+3]);
		}

	rtn = getOption ( argc, argv, argMap, 
			  const_cast<char*>("lnFN"), 3, 4, 4, nVal, startVal);
	if (rtn < 0)
		{
		return -1;
		}
	else if (rtn > 0)
		{
		strcpy (objFNames[1][0], argv[startVal]);
		strcpy (objFNames[1][1], argv[startVal+1]);
		strcpy (objFNames[1][2], argv[startVal+2]);
		strcpy (objFNames[1][3], argv[startVal+3]);
		}


	rtn = getOption ( argc, argv, argMap, 
			  const_cast<char*>("lnHandleFN"), 3, 4, 4, nVal, startVal);
	if (rtn < 0)
		{
		return -1;
		}
	else if (rtn > 0)
		{
		strcpy (objFNames[2][0], argv[startVal]);
		strcpy (objFNames[2][1], argv[startVal+1]);
		strcpy (objFNames[2][2], argv[startVal+2]);
		strcpy (objFNames[2][3], argv[startVal+3]);
		}

	return setupObjects (objFNames, glyphScale);
	}  // end of objFileArgs

static int
operationalArgs (int argc, char **argv, int *argMap)
	{
	bool doLenLabel = false;
        double coordScale = 1;
	double ptDist = 0.02 * GlyphScale;
	double lnDist = 0.02 * GlyphScale;
	int rtn;
	int nVal, startVal;
        char lenFmt[100] = "%g";

	rtn = getOption ( argc, argv, argMap, 
			  const_cast<char*>("lenLabels"), 2, 0, 1, nVal, startVal);
	if (rtn < 0)
		{
		return -1;
		}
	else if (rtn > 0)
		{
		doLenLabel = true;
                if (nVal == 1)
                        {
                        strcpy (lenFmt, argv[startVal]);
                        }
		}
	displayLenLabels (doLenLabel, lenFmt);


	rtn = getOption ( argc, argv, argMap, 
			  const_cast<char*>("coordScale"), 2, 1, 1, nVal, startVal);
	if (rtn < 0)
		{
		return -1;
		}
	else if (rtn > 0)
		{
                if (sscanf (argv[startVal], "%lf", &coordScale) != 1)
                        {
			badArgValue (argv[startVal-1], argv[startVal]);
                        coordScale = 1;
                        }
		}
	setCoordScale (coordScale);


	rtn = getOption ( argc, argv, argMap, 
			  const_cast<char*>("ptDist"), 3, 1, 1, nVal, startVal);
	if (rtn < 0)
		{
		return -1;
		}
	else if (rtn > 0)
		{
		if (sscanf (argv[startVal], "%lf", &ptDist) != 1)
			{
			badArgValue (argv[startVal-1], argv[startVal]);
			return -1;
			}
		}


	rtn = getOption ( argc, argv, argMap, 
			  const_cast<char*>("lnDist"), 3, 1, 1, nVal, startVal);
	if (rtn < 0)
		{
		return -1;
		}
	else if (rtn > 0)
		{
		if (sscanf (argv[startVal], "%lf", &lnDist) != 1)
			{
			badArgValue (argv[startVal-1], argv[startVal]);
			return -1;
			}
		}

	return setEditDistances (ptDist, lnDist);
	}  // end of operationalArgs


int
processCmdLineArgs (int argc, char **argv)
	{

	int *argMap = (int *) malloc (argc*sizeof(int));
	if (argMap == NULL)
		{
		fprintf (stderr, "%s: Unable to allocate sufficient memory.\n",
			ProgName);
		EXIT (-1);
		}

	argMap[0] = 1;
	for (int i = 1; i < argc; i++)
		{
		argMap[i] = 0;
		}
 
	int rtn = 0;

	rtn += housekeepingArgs (argc, argv, argMap);

	rtn += shmAndNodeArgs (argc, argv, argMap);

	rtn += objFileArgs (argc, argv, argMap);

        rtn += operationalArgs (argc, argv, argMap);
	
	if (rtn)
		{
		return -1;
		}


	// Note that there are no non-option arguments
	int nUnused = 0;
	for (int i = 0; i < argc; i++)
		{
		if (argMap[i] == 0)
			{
			nUnused++;
			}
		}

	if (nUnused > 0)
		{
		fprintf (stderr, "%s: Unrecognized command line argument%s:\n",
			ProgName, (nUnused > 1) ? "s" : "");
		for (int i = 0; i < argc; i++)
			{
			if (argMap[i] == 0)
				{
				fprintf (stderr, "    \"%s\"\n", argv[i]);
				}
			}
		fprintf (stderr, "\n");
		return -1;
		}
		

	return 0;
	}  // end of processCmdLineArgs


static int
setup ()
	{
	strcpy (ProgName, "hev-segmentSelection");

	sprintf (TmpDirName, "/var/tmp/%s-", ProgName);
	char *userName = getenv ("USER");
	if (userName != NULL)
		{
		strcat (TmpDirName, userName);
		}

	struct stat buf;
	if (stat (TmpDirName, &buf) != 0)
		{
		if (mkdir (TmpDirName, 0777))
			{
			fprintf (stderr, 
		  	   "%s: Error creating temporary directory \"%s\":\n"
			   "      %s\n",
		  	ProgName, TmpDirName, strerror (errno));
			return -1;
			}
		}

	return 0;
	}  // end of setup

static int
removeTmpDir ()
	{

	if ( ! Debug )
		{

                executeCmd ("rm --force %s/*.savg %s/*.osg", 
					TmpDirName, TmpDirName);

		if (rmdir (TmpDirName))
			{
			fprintf (stderr, 
		  	   "%s: Error deleting temporary directory \"%s\":\n"
			   "      %s\n",
		  	ProgName, TmpDirName, strerror (errno));
			return -1;
			}
		}

	return 0;
	}  // end of removeTmpDir

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>


static void
sigHandler (int sig)
	{
	fprintf (stderr, 
	 "\n----------> pid %d sigHandler received sig %d\n\n", getpid(), sig);
	if (sig == SIGSEGV)
		{
		fprintf (stderr, "SIGSEGV\n");
		sleep (1000);
		}
		
	}  // end of sigHandler

static void
setupSigHandlers ()
	{

	signal (SIGHUP, sigHandler);
	signal (SIGINT, sigHandler);
	signal (SIGQUIT, sigHandler);
	signal (SIGILL, sigHandler);
	signal (SIGTRAP, sigHandler);
	signal (SIGABRT, sigHandler);
	signal (SIGBUS, sigHandler);
	signal (SIGFPE, sigHandler);
	signal (SIGKILL, sigHandler);
	signal (SIGUSR1, sigHandler);
	signal (SIGSEGV, sigHandler);
	signal (SIGUSR2, sigHandler);
	signal (SIGPIPE, sigHandler);
	signal (SIGALRM, sigHandler);
	signal (SIGTERM, sigHandler);
	signal (SIGSTKFLT, sigHandler);
	signal (SIGCHLD, sigHandler);
	signal (SIGCONT, sigHandler);
	signal (SIGSTOP, sigHandler);
	signal (SIGTSTP, sigHandler);
	signal (SIGTTIN, sigHandler);
	signal (SIGTTOU, sigHandler);
	signal (SIGURG, sigHandler);
	signal (SIGXCPU, sigHandler);
	signal (SIGXFSZ, sigHandler);
	signal (SIGVTALRM, sigHandler);
	signal (SIGPROF, sigHandler);
	signal (SIGWINCH, sigHandler);
	signal (SIGPOLL, sigHandler);
	signal (SIGIO, sigHandler);
	signal (SIGPWR, sigHandler);
	signal (SIGSYS, sigHandler);
	signal (SIGUNUSED, sigHandler);

	}  // end of setupSigHandlers

static void 
atExitFunc ()
	{
	removeTmpDir ();
	if (Verbose)
		{
		fprintf (stderr, "hev-segmentSelection: About to exit.\n");
		}
	}  // end of atExitFunc

main (int argc, char **argv)

	{


        // This will cause all dtk messages to be written to stderr with
        // and appropriate prefix.
        dtkMsg.setFile (stderr);
        dtkMsg.setPreMessage ("hev-segmentSelection: ");
        // dtkMsg.add (DTKMSG_ERROR, 
        //             "This is an example of a error msg < %d >\n", 3);


	atexit (atExitFunc);
	// setupSigHandlers ();

	if (setup ())
		{
		EXIT (-1);
		}

	if (processCmdLineArgs (argc, argv))
		{
		EXIT (-1);
		}

	if (ActivateOnStartup)
		{
		activate ();
		}


#if 0
	while (1)
		{
		processCommands ();
		userInteractions ();
		usleep (100000);
		}
#else
	runFiniteStateMachine ();
#endif

	}  // end of main


