//
// hssCmd.cpp
//
// Routines that handle the commands that come in on stdin.
//

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <string>

// #include <hevUtils.h>
#include <iris.h>

#include "hev-segmentSelection.h"
#include "hssCmd.h"
#include "hssOps.h"
#include "hssDBHL.h"

#define MIN(x,y)   (((x)<(y))?(x):(y))

#define MAX_COMMAND_LEN   (1000)

static char CommandLine[MAX_COMMAND_LEN];


////////////////////////////////////////////////////////////////////////

//
// convert a C string to a vector of tokens.
//
static void
cStrToVec (char *cStr, std::vector<std::string> &vec)
        {
        static std::string line;

        line = cStr;;

        // chop off trailing spaces
        line = line.substr(0,line.find_last_not_of(" ")+1) ;

        // create vector of tokens
        vec = iris::ParseString(line) ;
        }  // end of cStrToVec


static void
removeComment (char *line)
	{
	for (char *c = line; *c; c++)
		{
		if ( *c == '#' )
			{
			*c = 0;
			break;
			}
		}
	}  // end of removeComment

static void 
makeLower (char *line)
	{
	for (char *c = line; *c; c++)
		{
		*c = tolower (*c);
		}
	}  // end of makeLower
///////////////////////////////////////////////////////

static void
badValueMsg ()
	{

	fprintf (stderr, "%s: Error in input command:\n", ProgName);
	fprintf (stderr, "    \"%s\"\n", CommandLine);
	fprintf (stderr, "    Unable to process value arguments.\n\n");
	}  // end of badValueMsg


static void
badCountMsg ()
	{
	fprintf (stderr, "%s: Error in input command:\n", ProgName);
	fprintf (stderr, "    \"%s\"\n", CommandLine);
	fprintf (stderr, "    Bad argument count.\n\n");
	}  // end of badCountMsg





///////////////////////////////////////////////////////




static int
getConstraint (
  std::vector<std::string> &vec, 
  Constraint &constraint )
	{

	if ((vec.size() < 4) || (vec.size() > 6))
		{
		badCountMsg ();
		return -1;
		}



	

	// currently, CO_LINE is the only type of constraint object
	constraint.obj = CO_LINE;

	constraint.type = CT_NONE;

	if (iris::IsSubstring ("all", vec[1]))
		{
		constraint.objID = -1;
		}
	else 
		{
		if (sscanf (vec[1].c_str(), "%d", &(constraint.objID)) != 1)
			{
			badValueMsg ();
			return -1;
			}
		}


	if (iris::IsSubstring ("para", vec[2]))
		{
		constraint.type = CT_PARA;
		for (int i = 0; i < 3; i++)
			{
			if (sscanf (vec[3+i].c_str(), "%lf", 
						constraint.vec+i) != 1)
				{
				badValueMsg ();
				return -1;
				}
			}
		}
	else if (iris::IsSubstring ("perp", vec[2]))
		{
		constraint.type = CT_PERP;
		for (int i = 0; i < 3; i++)
			{
			if (sscanf (vec[3+i].c_str(), "%lf", 
						constraint.vec+i) != 1)
				{
				badValueMsg ();
				return -1;
				}
			}
		}
	else if (iris::IsSubstring ("length", vec[2]))
		{
		constraint.type = CT_LEN;
		if (sscanf (vec[3].c_str(), "%lf", &(constraint.len)) != 1)
			{
			badValueMsg ();
			return -1;
			}
		}
	else
		{
		badValueMsg ();
		return -1;
		}

	return 0;
	}  // getConstraint


static int
constrainLine (std::vector<std::string> &vec)
	{
	Constraint constraint;

	if (getConstraint (vec, constraint))
		{
		return -1;
		}

	return constrain (constraint);
	}  // constrainLine

static int
forceLine (std::vector<std::string> &vec)
	{
	Constraint constraint;

	if (getConstraint (vec, constraint))
		{
		return -1;
		}

	return force (constraint);
	}  // forceLine



static int
getPts (int numPts, int startArg, std::vector<std::string> &vec, double pts[][3])
	{

	if ( vec.size() != (startArg + 3*numPts) )
		{
		badCountMsg ();
		return -1;
		}

	int iArg = startArg;
	for (int i = 0; i < numPts; i++)
		{
		for (int j = 0; j < 3; j++)
			{

			if (sscanf (vec[iArg].c_str(), "%lf", 
						&(pts[i][j])) != 1)
				{
				badValueMsg ();
				return -1;
				}
			iArg++;
			}
		}


	return 0;
	}  // end of getPts








static int
scaleMovement (std::vector<std::string> &vec)
	{
	double scale;

	if (vec.size() != 2)
		{
		badCountMsg ();
		return -1;
		}

	if (sscanf (vec[1].c_str(), "%lf", &scale) != 1)
		{
		badValueMsg ();
		return -1;
		}

	return scaleMovement (scale);
	}  // end of scaleMovement


static int
lenLabel (std::vector<std::string> &vec)
	{
	bool doLabel;

	if (vec.size() != 2)
		{
		badCountMsg ();
		return -1;
		}

	if (iris::IsSubstring ("true", vec[1]))
		{
		doLabel = true;
		}
	else if (iris::IsSubstring ("false", vec[1]))
		{
		doLabel = false;
		}
	else
		{
		badValueMsg ();
		return -1;
		}

	return lenLabel (doLabel);
	}  // end of lenLabel






static int
limitBox (std::vector<std::string> &vec)
	{
	double box[2][3];

	if ( vec.size() != 7 )
		{
		badCountMsg ();
		return -1;
		}

	if (getPts (2, 1, vec, box))
		{
		return -1;
		}


	return limitBox (box);
	}  // end of limitBox


static int
readSegFile (std::vector<std::string> &vec)
	{

	if ( vec.size() != 2 )
		{
		badCountMsg ();
		return -1;
		}

	return readSegFile (vec[1].c_str());
	}  // end of readSegFile


static int
writeSegFile (std::vector<std::string> &vec)
	{

	if ( vec.size() != 2 )
		{
		badCountMsg ();
		return -1;
		}

	return writeSegFile (vec[1].c_str());
	}  // end of writeSegFile



static int
userHighlight (std::vector<std::string> &vec)
	{

	if ( vec.size() != 3 )
		{
		badCountMsg ();
		return -1;
		}

        bool all = false;
        int iSeg = -1;;
        bool segOn;

	if (iris::IsSubstring ("all", vec[1]))
                {
                all = true;
                }
        else if (sscanf (vec[1].c_str(), "%d", &iSeg) != 1)
                {
                badValueMsg ();
                return -1;
                }
        
	if (iris::IsSubstring ("on", vec[2]))
                {
                segOn = true;
                }
	else if (iris::IsSubstring ("off", vec[2]))
                {
                segOn = false;
                }
        else
                {
                badValueMsg ();
                return -1;
                }

	return userHighlight (all, iSeg, segOn);
	}  // end of writeSegFile




//////////////////////////////////////////////////////////

static bool StdinCommandInputActive = true;

static int
getCmdLineFromStdin (char *line, int lineLen)
	{
	// Get commands from stdin.
	// If we read commands from stdin, we need to be able to do
	// non-blocking reads (maybe fgets) from stdin.
	// One possibility is to use std::cin.rdbuf() -> in_avail()
	// to test if there is input waiting.  This does not work.
	//
	// We will do this using select().  Note that it appears that
	// select() does not report that input is waiting until there
	// is a new line in the stream; this is fine for us.

	if ( ! StdinCommandInputActive )
		{
		return 0;
		}


	int fd = fileno (stdin);

	struct timeval timeOut;

	// set timeOut to zero
	timeOut.tv_sec = 0;
	timeOut.tv_usec = 0;

	fd_set fileSet;
	
	FD_ZERO (&fileSet);
	FD_SET (fd, &fileSet);

	errno = 0;
	int rtn = select (fd+1, &fileSet, NULL, NULL, &timeOut);
	int errnoSaved = errno;

	if (rtn == 0)
		{
		// there is nothing waiting to be read.
		return 0;
		}

	if (rtn < 0)
		{
		char *errStr = strerror (errnoSaved);

		fprintf (stderr, 
		  "%s: Possible error on command input (A):\n      \"%s\"\n", 
			ProgName, errStr);
		StdinCommandInputActive = false;

		return -1;
		}

	// At this point, we know that there is some input waiting to be read.

	errno = 0;
	if (fgets (line, lineLen, stdin) == NULL)
		{
		int errnoSaved2 = errno;

		if (errnoSaved2 == 0)
			{
			char *errStr = strerror (errnoSaved);
			fprintf (stderr, 
		  	"%s: Possible error on command input (B):\n"
			 	"      \"%s\"\n", 
				ProgName, errStr);
			}
		StdinCommandInputActive = false;
		return -1;
		}

// fprintf (stderr, "fgets got <%s>\n", line);

	return strlen (line);
	}  // end of getCmdLineFromStdin




#if 0

//////////////////////////////////////////////////////////
//// Here's the beginning of the explicit fifo implementation

#define CMD_FIFO_DEFAULT_BUFFER_LEN (200)
static int CmdFifoFD = -1;
static int CmdFifoBufferLen = CMD_FIFO_DEFAULT_BUFFER_LEN;
static char *CmdFifoBuffer = NULL;
static bool UseFifoForCmdLineInput = false;

static int 
setupCmdFifo (char *fifoName, int len)
	{


	// See if the fifo already exists
	bool fifoExists = false;
	struct stat buf ;
	int ret = stat(fifoName, &buf) ;
	// does file exist but isn't a fifo?
  	if (ret == 0)
    		{
      		if (! (S_ISFIFO(buf.st_mode)))
        		{
          		fprintf(stderr,
                	  "%s: File %s exists, but isn't a fifo.\n"
			  "               File will be removed.\n\n",
                  		ProgName, fifoName) ;
			unlink (fifoName);
			fifoExists = false;
        		}
		else
			{
			fifoExists = true;
			}
    		}
  	else
    		{
		fifoExists = false;
		}

	if ( ! fifoExists )
		{
      		// If it doesn't exist, then create it as a fifo
      		ret = mkfifo(fifoName, 0777);
      		if (ret != 0)
        		{
			char *errStr = strerror (errno);
			fprintf (stderr, "%s: Error in mkfifo: %s\n", 
				ProgName, errStr);
          		// perror("hev-writeFifo, mkfifo: ");
          		return -1 ;
        		}
    		}


	CmdFifoFD = open (fifoName, O_RDONLY | O_NONBLOCK) ;

	if (CmdFifoFD == -1)
                {
                fprintf (stderr,
                        "%s: Unable to open input FIFO \"%s\".\n",
                        ProgName, fifoName);
                return (-1);
                }

	CmdFifoBufferLen = len;

	// Allocate buffer for reading from fifo
        if ( (CmdFifoBuffer = (char *) malloc (CmdFifoBufferLen+1)) == NULL )
                {
                fprintf (stderr,
                 "%s: Unable to allocate memory for input buffer.\n", ProgName);
                return (-1);
                }

	UseFifoForCmdLineInput = true;

	return 0;
	}  // end of setupCmdFifo (fifoName, len)

int
setupCmdFifo ()
	{
	char fifoName[1000];

	strcpy (fifoName, "/dev/shm/hev-segSelCmd-");
	char *userName = getenv ("USER");

	if (userName != NULL)
		{
		strcat (fifoName, userName);
		}
	else
		{
		fprintf (stderr, "%s: Unable to get user name ($USER).\n",
			ProgName);
		return -1;
		}

	return setupCmdFifo (fifoName, CMD_FIFO_DEFAULT_BUFFER_LEN);
	}  // end of setupCmdFifo ()

static int
getCmdLineFromFifo (char *line, int lineLen)
	{
	if ( ! UseFifoForCmdLineInput )
		{
		fprintf (stderr, 
		    "%s: Command FIFO not properly set up.\n", ProgName);
		return -1;
		}


	int nBytes = read(CmdFifoFD, CmdFifoBuffer, CmdFifoBufferLen);

        // When doing non-blocking reads, nBytes could
        // be -1 when there is no error.  I think that
        // nBytes == -1 when there is no process trying
        // to write to the fifo.

        if (nBytes > 0)
        	{
		strncpy (line, CmdFifoBuffer, MIN (lineLen, CmdFifoBufferLen));
		line[lineLen-1] = 0;
                if (nBytes < lineLen)
                        {
                        line[nBytes - 1] = 0;
                        }
		return strlen (line);
        	}
        else if (nBytes < -1)
        	{

		char *errStr = strerror (errno);
		fprintf (stderr, 
		  	"%s: Possible error on command input (B):\n"
			 	"      \"%s\"\n", 
			ProgName, errStr);
        	return (-1);
        	}


	return 0;
	}  // end of getCmdLineFromFifo

/////////// Here's the end of the explicit Fifo code
//////////////////////////////////////////////////////////

#else

// Here the beginning of the Fifo code that uses iris::FifoReader
static bool UseFifoForCmdLineInput = false;

static iris::FifoReader * CmdFifoReader = NULL;
int
setupCmdFifo ()
        {
        char fifoName[1000];

        strcpy (fifoName, "/dev/shm/hev-segSelCmd-");
        char *userName = getenv ("USER");

        if (userName != NULL)
                {
                strcat (fifoName, userName);
                }
        else
                {
                fprintf (stderr, "%s: Unable to get user name ($USER).\n",
                        ProgName);
                return -1;
                }

        if (CmdFifoReader == NULL)
                {
                CmdFifoReader = new iris::FifoReader (fifoName);
                if (CmdFifoReader == NULL)
                        {
                        fprintf (stderr, 
                                "%s: Unable to create command fifo %s.\n",
                                ProgName, fifoName);
                        return -1;
                        }
                }


        if ( ! CmdFifoReader->open() )
                {
                fprintf (stderr, "%s: Unable to open command fifo %s.\n",
                                ProgName, fifoName);
                delete CmdFifoReader;
                CmdFifoReader = NULL;
                return -1;
                }

	UseFifoForCmdLineInput = true;

        return 0;

        }  // end of setupCmdFifo


static int
getCmdLineFromFifo (char *line, int lineLen)
        {
        if (CmdFifoReader == NULL)
                {
                return -1;
                }

        std::string lineStr;

        if ( ! CmdFifoReader->readLine ( & lineStr) )
                {
                // not an error, just means there's no line available
                return 0;
                }

        if (lineLen <= lineStr.length())
                {
                fprintf (stderr, "%s: Warning: Command fifo returned a "
                                "command longer than the maximum length.\n",
                                ProgName );
                fprintf (stderr, "%s:          Will only process "
                                "the first %d characters.\n", 
                                ProgName, lineLen-1);
                }

        strncpy (line, lineStr.c_str(), lineLen-1);
        line[lineLen-1] = 0;

	return strlen (line);
        }  // end of getCmdLineFromFifo



#endif

















static int
getCmdLine (char *line, int lineLen)
	{

	if (UseFifoForCmdLineInput)
		{
		return getCmdLineFromFifo (line, lineLen);
		}

	return getCmdLineFromStdin (line, lineLen);
	}  // end of getCmdLine





//////////////////////////////////////////////////////////

static bool CommandInputActive = true;


int
processCommands ()
    {

	// Commands come in on stdin.
	// If we read commands from stdin, we need to be able to do
	// non-blocking reads (maybe fgets) from stdin.
	// One possibility is to use std::cin.rdbuf() -> in_avail()
	// to test if there is input waiting.  This does not work.
	//
	// We will do this using select().  Note that it appears that
	// select() does not report that input is waiting until there
	// is a new line in the stream; this is fine for us.

// fprintf (stderr, "processCommands A\n");

	if ( ! CommandInputActive )
		{
		return 0;
		}

// fprintf (stderr, "processCommands B\n");


	char line[MAX_COMMAND_LEN];






#if 0
	int rtn = getCmdLine (line, sizeof (line));


	if (rtn < 0)
		{
		return -1;
		}
	else if (rtn == 0)
		{
		// there is nothing waiting to be read.
		return 0;
		}

#else

int ncmd = 0;
    int rtn;
    while ( (rtn = getCmdLine (line, sizeof (line))) > 0 )
        {
                

#endif

        ncmd ++;



	if (Verbose)
		{
		fprintf (stderr, 
		   "%s: Just got command \"%s\".\n", ProgName, line);
		}




	int len = strlen (line);
	if (len != 0)
		{
		if (line[len-1] == '\n')
			{
			line[len-1] = 0;
			}
		}
	strcpy (CommandLine, line);

	removeComment (line);
	makeLower (line);

	std::vector<std::string> vec;

	// use John K's approach to parsing out the line, a la sge.

	cStrToVec (line, vec);

	if (vec.size() == 0)
		{
		// don't do anything
		}
	else if (iris::IsSubstring ("constrain_line", vec[0]))
		{
		constrainLine (vec);
		}
	else if (iris::IsSubstring ("unconstrain", vec[0]))
		{
		unconstrain ();
		}
	else if (iris::IsSubstring ("force_line", vec[0]))
		{
		forceLine (vec);
		}
	else if (iris::IsSubstring ("limit", vec[0]))
		{
		limitBox (vec);
		}
	else if (iris::IsSubstring ("unlimit", vec[0]))
		{
		unlimitBox ();
		}
	else if (iris::IsSubstring ("activate", vec[0]))
		{
		activate ();
		}
	else if (iris::IsSubstring ("deactivate", vec[0]))
		{
		deactivate ();
		}
	else if (iris::IsSubstring ("hide", vec[0]))
		{
		hide ();
		}
	else if (iris::IsSubstring ("show", vec[0]))
		{
		show ();
		}
	else if (iris::IsSubstring ("scale_movement", vec[0]))
		{
		scaleMovement (vec);
		}
	else if (iris::IsSubstring ("unscale_movement", vec[0]))
		{
		unscaleMovement ();
		}
	else if (iris::IsSubstring ("len_label", vec[0]))
		{
		lenLabel (vec);
		}
	else if (iris::IsSubstring ("reset", vec[0]))
		{
		reset ();
		}
	else if (iris::IsSubstring ("quit", vec[0]))
		{
		quit ();
		}
	else if (iris::IsSubstring ("read", vec[0]))
		{
		readSegFile (vec);
		}
	else if (iris::IsSubstring ("write", vec[0]))
		{
		writeSegFile (vec);
		}
	else if (iris::IsSubstring ("userhl", vec[0]))
		{
		userHighlight (vec);
		}
        else
                {
	        fprintf (stderr, "%s: Unrecognized command keyword \"%s\".\n",
			ProgName, vec[0].c_str());
                }

        }  // end of while getCmdLine > 0

        return rtn;

    }  // end of processCommands




