#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "gle.h"
#include "savgIOUtils.h"



#define MIN(x,y)	(((x)<(y))?(x):(y))
#define MAX(x,y)	(((x)>(y))?(x):(y))

#define OUT_FORMAT_SAVG		('s')
#define OUT_FORMAT_OSG		('o')

///////////////////////////////////////////////////////
//
// The following block of code deals with parsing the
// command line.
//

static int
compareOptStrings (char *s1, char *s2)
	{
	int len;

	len = MIN (strlen (s1), strlen(s2));

	if (len == 0)
		{
		return -1;
		}

	return strncasecmp (s1, s2, len);
	}  // end of compareOptStrings



static int
getOptions 
  (
  int argc, 
  char **argv, 
  int *join, 
  int *cap, 
  int *norm, 
  int *divisions, 
  double *radius, 
  int *format,
  int *help
  )
	{
	int option_index;
	int c;
	static struct option long_options[] = 
			{
			{"join",      1, 0, 0},
			{"caps",      1, 0, 0},
			{"normals",   1, 0, 0},
			{"divisions", 1, 0, 0},
			{"radius",    1, 0, 0},
			{"format",    1, 0, 0},
			{"help",    1, 0, 0},
			{0, 0, 0, 0}
			};
	static char *optString = "j:c:n:d:r:f:h";
	int rtn;

	rtn = 0;

	// defaults
	*join = TUBE_JN_ANGLE;
	*cap = 0;
	*norm = TUBE_NORM_EDGE;
	*divisions = 8;
	*radius = 0.1;
	*format = OUT_FORMAT_OSG;
	*help = 0;



	// Loop over all options.
	while (1) 
		{
               c = getopt_long (argc, argv, optString,
                        long_options, &option_index);
               if (c == -1)
			{
                   	break;
			}

		if (c != 0)
			{
			option_index = c;
			}

		switch (option_index) 
			{
			case 0:
			case 'j':
				// --join
				if (compareOptStrings (optarg, "RAW") == 0)
					{
					*join = TUBE_JN_RAW;
					}
				else if (compareOptStrings (optarg, "ANGLE") == 0)
					{
					*join = TUBE_JN_ANGLE;
					}
				else if (compareOptStrings (optarg, "CUT") == 0)
					{
					*join = TUBE_JN_CUT;
					}
				else if (compareOptStrings (optarg, "ROUND") == 0)
					{
					*join = TUBE_JN_ROUND;
					}
				else
					{
					fprintf (stderr, "%s: Error in join option < %s >.\n", argv[0], optarg);
					rtn = -1;
					}
				break;



			case 1:
			case 'c':
				// --caps
				if (compareOptStrings (optarg, "Y") == 0)
					{
					*cap = TUBE_JN_CAP;
					}
				else if (compareOptStrings (optarg, "N") == 0)
					{
					*cap = 0;
					}
				else
					{
					fprintf (stderr, "%s: Error in cap option < %s >.\n", argv[0], optarg);
					rtn = -1;
					}
				break;



			case 2:
			case 'n':
				// --normals
				if (compareOptStrings (optarg, "FACET") == 0)
					{
					*norm = TUBE_NORM_FACET;
					}
				else if (compareOptStrings (optarg, "SMOOTH") == 0)
					{
					*norm = TUBE_NORM_EDGE;
					}

#if 0
I believe that TUBE_NORM_PATH_EDGE is not relevant to cylindrical
extrusions.
				else if (compareOptStrings (optarg, "PATH_EDGE") == 0)
					{
					*norm = TUBE_NORM_PATH_EDGE;
					}
#endif


				else
					{
					fprintf (stderr, "%s: Error in normals option < %s >.\n", argv[0], optarg);
					rtn = -1;
					}
				break;



			case 3:
			case 'd':
				// --divisions
				if (sscanf (optarg, "%d", divisions) != 1)
					{
					fprintf (stderr, "%s: Error in divisions option < %s >.\n", argv[0], optarg);
					rtn = -1;
					}
				break;



			case 4:
			case 'r':
				// --radius
				if (sscanf (optarg, "%lf", radius) != 1)
					{
					fprintf (stderr, "%s: Error in radius option < %s >.\n", argv[0], optarg);
					rtn = -1;
					}
				break;


			case 5:
			case 'f':
				// --format
				if (compareOptStrings (optarg, "SAVG") == 0)
					{
					*format = OUT_FORMAT_SAVG;
					}
				else if (compareOptStrings (optarg, "OSG") == 0)
					{
					*format = OUT_FORMAT_OSG;
					}
				else
					{
					fprintf (stderr, "%s: Error in format option < %s >.\n", argv[0], optarg);
					rtn = -1;
					}
				break;

			case 6:
			case 'h':
				*help = 1;
				break;

			default:
				fprintf (stderr, "%s: Error in command line options.\n", argv[0]);
				rtn = -1;

			}

		}


	if (optind < argc) 
		{
		int i;
		fprintf (stderr, "%s: Unrecognized command line arguments:\n", argv[0]);
		for (i = optind; i < argc; i++)
			{
			fprintf (stderr, "     %s\n" , argv[i]);
			}
		fprintf (stderr, "\n");
		rtn = -1;
		}

	
	return rtn;
	}  // end of getOptions
//
//
/////////////////////////////////////////////////////////////////////////
//
//  print the usage message.
//

static void usage ()
  {
  fprintf (stderr, "\n");
  fprintf (stderr, "Usage:   hev-tubify  [ options ]\n");
	
  fprintf (stderr, "\n");
  
  fprintf (stderr, "hev-tubify generates a 3D extruded tube based on 3D paths.\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "The 3D paths are read from standard input and can be either in the \n");
  fprintf (stderr, "form of an SAVG file or a file containing a series of 3D coordinates.  \n");
  fprintf (stderr, "\n");
  fprintf (stderr, "If an SAVG file is used as input, each 'line' or 'polygon' primitive \n");
  fprintf (stderr, "is extruded as a separate path.  Point primitives are ignored.\n");

  fprintf (stderr, "\n");

  fprintf (stderr, "If a file containing just a sequence of 3D coordinates is used \n");
  fprintf (stderr, "as input, these coordinates will be used as a single extrusion\n");
  fprintf (stderr, "path. Each 3D point must be on a separate line.\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "Ouput is written either as an SAVG or OSG polygon file.\n");
  fprintf (stderr, "\n");
 
  fprintf (stderr, "Example:\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "   hev-tubify --join ROUND --radius 0.04  <  path.savg  >  extrusion.osg\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "Options:\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "   -j joinType\n");
  fprintf (stderr, "   --join joinType       Method for joining tubes.\n");
  fprintf (stderr, "                         joinType can be RAW, ANGLE, CUT, or ROUND.\n");
  fprintf (stderr, "                         Default is ANGLE.\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "   -c YES|NO\n");
  fprintf (stderr, "   --cap YES|NO          Put caps at the beginning and end of\n");
  fprintf (stderr, "                         the tubes.\n");
  fprintf (stderr, "                         Default is NO.\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "   -n  normalType\n");
  fprintf (stderr, "   --normals normalType  Type of normals that are generated.\n");
  fprintf (stderr, "                         normalType can be FACET or SMOOTH.\n");
  fprintf (stderr, "                         Default is SMOOTH.\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "   -d nDiv\n");
  fprintf (stderr, "   --divisions nDiv      Number of divisions in tube circumference.\n");
  fprintf (stderr, "                         nDiv is an integer greater than 2.\n");
  fprintf (stderr, "                         Default is 8.\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "   -r rad\n");
  fprintf (stderr, "   --radius rad          Radius of tubes.\n");
  fprintf (stderr, "                         rad is a floating point number.\n");
  fprintf (stderr, "                         Default is 0.01\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "   -f outFormat\n");
  fprintf (stderr, "   --format outFormat    Output file format.\n");
  fprintf (stderr, "                         outFormat can be SAVG or OSG.\n");
  fprintf (stderr, "                         Default is OSG.\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "   -h\n");
  fprintf (stderr, "   --help                Display this usage message and exit.\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "Option keywords such as RAW, EDGE, and YES are case-insensitive.\n");
  fprintf (stderr, "They may be abbreviated to the shortest unique string.\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "NOTES: \n");
  fprintf (stderr, "       Color information from input .savg files is not used in the\n");
  fprintf (stderr, "       output.  No color information at all appears in the output\n");
  fprintf (stderr, "       files.\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "       Some bugs have been observed in the polygonization of the\n");
  fprintf (stderr, "       tubes when using the CUT join method.\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "       When using round joins to extrude along the length of a closed\n");
  fprintf (stderr, "       polygon in an input .savg file, the final join between the last\n");
  fprintf (stderr, "       point of the polygon and the first point of the polygon is not\n");
  fprintf (stderr, "       provided.\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "       This program uses GLE, a package for creating extruded\n");
  fprintf (stderr, "       surfaces that was originally developed by Linas Vepstas.\n");
  fprintf (stderr, "       We have slightly modified the code for use in this program.\n");
  fprintf (stderr, "       We are using this code under the GNU General Public License.\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "\n");
  }  // end of usage

//
//
/////////////////////////////////////////////////////////////////////////

#define MAX_PTS 10000
#define PT_ARRAY_SIZE (MAX_PTS+10)

static int finishPath 
  (
  int primType, 
  int numReadPts, 
  int *numPts, 
  double points[PT_ARRAY_SIZE][3]
  )
	{

	// We need to put fake first and last points onto the the
	// list of points so that the extrusion join methods can
	// do what they need to do. These fake points are generated
	// differently for polygon and line primitives.


	if (primType == SAVG_POLYPRIM_KEYWORD)
		{

		// For polygons, In addition to the fake points, 
		// we have to close the polygon by connecting
		// the last point to the first point.

		// put fake first pt at the beginning by duplicating last pt
		points[0][0] = points[numReadPts][0];
		points[0][1] = points[numReadPts][1];
		points[0][2] = points[numReadPts][2];

		// now close the polygon by duplicating first point at end
		points[numReadPts+1][0] = points[1][0];
		points[numReadPts+1][1] = points[1][1];
		points[numReadPts+1][2] = points[1][2];

		// fake last point is second point
		points[numReadPts+2][0] = points[2][0];
		points[numReadPts+2][1] = points[2][1];
		points[numReadPts+2][2] = points[2][2];

		*numPts = numReadPts + 3;
		}
	else if (primType == SAVG_LINEPRIM_KEYWORD)
		{

		// for lines, we add a fake first and last point
		// by extending the first and last segments

		points[0][0] = points[1][0] - (points[2][0] - points[1][0]);
		points[0][1] = points[1][1] - (points[2][1] - points[1][1]);
		points[0][2] = points[1][2] - (points[2][2] - points[1][2]);


		points[numReadPts+1][0] = points[numReadPts][0] - 
			   (points[numReadPts-1][0] - points[numReadPts][0]);
		points[numReadPts+1][1] = points[numReadPts][1] - 
			   (points[numReadPts-1][1] - points[numReadPts][1]);
		points[numReadPts+1][2] = points[numReadPts][2] - 
			   (points[numReadPts-1][2] - points[numReadPts][2]);


		*numPts = numReadPts + 2;
		}


	return 0;
	}  // end of finishPath


static int
printPath (FILE *fp, int numPts, double points[PT_ARRAY_SIZE][3])
	{
	int i;

	fprintf (fp, "\n");
	fprintf (fp, "Path:  %d points\n", numPts);

	for (i = 0; i < numPts; i++)
		{
		fprintf (fp, "  %d  %f %f %f\n",
			i,
			points[i][0],
			points[i][1],
			points[i][2]);
		}
	fprintf (fp, "\n");


	}  // end of printPath


static int
getPath (FILE *inFP, int *numPts, double points[PT_ARRAY_SIZE][3])
	{
	char line[SAVG_MAX_LINE_LEN];
	int lineType;
	double xyz[3], rgba[4], norm[3];
	int which;
	int keywordType;
	int numReadPts;
	int rdRtn;

	static int newPrimType = SAVG_LINEPRIM_KEYWORD;
	int currPrimType; 
	static int lineNum = 0;
	static int eofReached = 0;

	numReadPts = 0;

	if (eofReached)
		{
		return -1;
		}

	currPrimType = newPrimType;

	while (1)
	    {

	    rdRtn = savgGetLine (inFP, line, &lineType);
	    if (rdRtn == SAVG_EOF)
		{
		eofReached = 1;
		break;
		}


	    lineNum++;


	    if (lineType == SAVG_KEYWORD_LINE)
		{
		keywordType = savgGetKeywordType (line);
		if (keywordType != SAVG_UNKNOWN_KEYWORD)
		    {
		    newPrimType = keywordType;
		    if (numReadPts > 0)
			{
			return finishPath (currPrimType, 
					numReadPts, numPts, points);
			}
		    currPrimType = newPrimType;
		    }
		}  // end of if keyword line
	    else if (lineType == SAVG_VERTEX_LINE)
		{
		if (! savgGetVertexComponents (line, &which, xyz, rgba, norm) )
		    {
		    if (numReadPts < MAX_PTS)
			{
			points[numReadPts+1][0] = xyz[0];
			points[numReadPts+1][1] = xyz[1];
			points[numReadPts+1][2] = xyz[2];
			numReadPts++;
			}
		    else
			{
			fprintf (stderr, "Error: Exceeded maximum number "
			  "of points (%d) at input line %d.\n", 
			  MAX_PTS, lineNum);
			return -1;
			}
		    }
		else
		    {
		    fprintf (stderr, "Error: Unable to parse vertex data "
				"at input line %d.\n", lineNum);
		    }
		}  // end of if vertex line


	    }

	if (numReadPts > 1)
	    {
	    return finishPath (currPrimType, numReadPts, numPts, points);
	    }



	return 0;

	}  // end of getPath

////////////////////////////////////////////////////////////////////////
//
//
// This routine writes a comment to the output extrusion file that
// describes the parameters that were used to create the extrusion.
//
static void
makeTubifyComment (char *tag, int join, int cap, int norm, int divisions, double radius, int format)
	{
	char comment[1000];
	char *joinStr;
	char *capStr;
	char *normStr;
	char *formatStr;


	switch (join)
		{
		case TUBE_JN_RAW:
			joinStr = "Raw";
			break;

		case TUBE_JN_ANGLE:
			joinStr = "Angle";
			break;

		case TUBE_JN_CUT:
			joinStr = "Cut";
			break;

		case TUBE_JN_ROUND:
			joinStr = "Round";
			break;

		default:
			joinStr = "UNKNOWN!";
		}


	capStr = (cap == TUBE_JN_CAP) ? "Yes" : "No";

	switch (norm)
		{
		case TUBE_NORM_FACET:
			normStr = "Facet";
			break;

		case TUBE_NORM_EDGE:
			normStr = "Smooth";
			break;
		default:
			normStr = "UNKNOWN!";
		}

	

	switch (format)
		{
		case OUT_FORMAT_SAVG:
			formatStr = "SAVG";
			break;

		case OUT_FORMAT_OSG:
			formatStr = "OSG";
			break;
		default:
			formatStr = "UNKNOWN!";
		}

		



	sprintf (comment, 
		"  %s extruded tube generated by hev-tubify\n"
		"  with the following parameters:  \n"
		"      Join = %s  \n"
		"      Cap = %s  \n"
		"      Normals = %s  \n"
		"      # Div = %d   \n"
		"      Radius = %f  \n"
		"      Format = %s\n",
		tag,
		joinStr,
		capStr,
		normStr,
		divisions,
		radius,
		formatStr);
		

	writeExtrusionComment (comment);

	}  // end of makeTubifyComment


////////////////////////////////////////////////////////////////////////
//
//
int
main (int argc, char **argv)
	{
	int join;
	int cap;
	int norm;
	int divisions;
	double radius;
	int format;
	int rtn;
	int help;

	int style;
	int numPts;
	double points [PT_ARRAY_SIZE][3];


	rtn = getOptions 
  		(
  		argc, 
  		argv, 
  		&join, 
  		&cap, 
  		&norm, 
  		&divisions, 
  		&radius, 
  		&format,
		&help
  		);

	if (help)
		{
		// the user requested help
		usage ();
		return 0;
		}

	if (rtn)
		{
		// there was some problem parsing the command line.
		usage ();
		return -1;
		}


	// OK, at this point, we have all of the parameters we needd
	// to do the extrusion.

#if 0
	printf ("rtn = %d\n", rtn);

	printf ("join = %d\n", join);
	printf ("cap = %d\n", cap);
	printf ("norm = %d\n", norm);
	printf ("divisions = %d\n", divisions);
	printf ("radius = %f\n", radius);
	printf ("format = %d\n", format);
#endif


	// Tell the output routines how to format the output
	setExtrusionOutputFormat (format);

	// Insert a comment into the output stream
	makeTubifyComment ("BEGIN", join, cap, norm, divisions, radius, format);

	gleSetNumSides (divisions);

	style = norm | join | cap;

	gleSetJoinStyle (style);

	//

	// loop over each path in the input data
	while (getPath (stdin, &numPts, points) == 0)
		{
#if 0
		printPath (stderr, numPts, points);
#endif

		// Generate the extruded cylinder.
		// Note that the NULL in the following call is for
		// the color array.  
		// We are generating tubes without color.
		glePolyCylinder (numPts, points, NULL, radius);
		}

	// Insert a comment into the output stream
	makeTubifyComment ("END", join, cap, norm, divisions, radius, format);

	return 0;
	}  // end of main
