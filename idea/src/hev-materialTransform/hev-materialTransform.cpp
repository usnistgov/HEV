
/////////
//
// hev-materialTransform
//
// loads one or more files, modifies materials, and dumps
// them as a single or multiple file
//
//
////////


// #include <string.h>

#include <osg/MatrixTransform>
#include <osg/Vec4>
#include <osg/Material>
#include <osg/StateSet>
#include <osg/Node>
#include <osg/StateAttribute>
// #include <osgAnimation/StatsVisitor>
#include <osgUtil/Statistics>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgUtil/Optimizer>
#include <osgUtil/SmoothingVisitor>

#include <dtk.h> 

using namespace std ;




static int Verbose = 0 ;





////////////////////////////////////////////////////////////////////////
void shortUsage() 
{

fprintf (stderr, 
" \n"
"Usage: \n"
" \n"
"      hev-materialTransform  [ options ]   [infile . . . ]  outfile \n"
" \n"
"  To get an extended usage message:\n"
" \n"
"      hev-materialTransform  --help\n"
" \n"
" \n"
);
}  // end of shortUsage



void usage() 
{



#if 0
Here is a proposed functionality for hev-materialTransform
#endif


fprintf (stderr, 
" \n"
"Usage: \n"
" \n"
"  hev-materialTransform  [ options ]   [infile . . . ]  outfile \n"
" \n"
"Options: \n"
" \n"
"  (All option keywords can be shortened to the shortest  \n"
"   unique string.) \n"
" \n"
"  The usual options: \n"
" \n"
"      --help \n"
"      --verbose \n"
" \n"
" \n"
"  These options specify what to do with the data in the input files  \n"
"  before modifying their materials: \n"
" \n"
"      --rmMaterials    - Remove all materials before adding the\n"
"                            new material.\n"
" \n"
"      --rmGeomColors   - Remove colors attached to points, lines,  \n"
"                            or polygons.\n"
" \n"
"  This option is used only when no input files are specified:\n"
" \n"
"      --noInput        - No input files are specified.\n"
"                         This tells the program to generate a \n"
"                         minimal data structure to receive the \n"
"                         specified material parameters. \n"
" \n"
"  These options specify the material to be used to modify the  \n"
"  input data : \n"
" \n"
"      --sourceMaterial fileName  - Use material from indicated file.\n"
" \n"
"      --ambient front r g b a    - Ambient color of front face.\n"
"      --ambient back  r g b a    - Ambient color of back face.\n"
"      --ambient both  r g b a    - Ambient color of both front and back.\n"
"      --ambient       r g b a    - Ambient color of both front and back.\n"
" \n"
"      --diffuse front r g b a \n"
"      --diffuse back  r g b a \n"
"      --diffuse both  r g b a \n"
"      --diffuse       r g b a \n"
" \n"
"      --specular front r g b a \n"
"      --specular back  r g b a \n"
"      --specular both  r g b a \n"
"      --specular       r g b a \n"
" \n"
"      --emission front r g b a \n"
"      --emission back  r g b a \n"
"      --emission both  r g b a \n"
"      --emission       r g b a \n"
" \n"
"      --shininess front s \n"
"      --shininess back  s \n"
"      --shininess both  s \n"
"      --shininess       s \n"
"           Shininess is used in the calculation of the size of\n"
"           specular highlights. It should be in the range from\n"
"           0.0 to 128.0.\n"
" \n"
#if 0
"      --colorMode front mode \n"
"      --colorMode back  mode \n"
"      --colorMode both  mode \n"
#endif
"      --colorMode       mode \n"
"           The color mode specifies how to use colors that are\n"
"           attached directly to vertices, lines, or polygons. \n"
"           The possible modes are:\n"
"               AMBIENT\n"
"               DIFFUSE\n"
"               AMBIENT_AND_DIFFUSE\n"
"               SPECULAR\n"
"               EMISSION\n"
"               OFF\n"
" \n"
" \n"
"  These options specify how to combine the material specified on the \n"
"  command line with the material(s) already present in the input files: \n"
" \n"
"      --override          - Override all material parameters in the \n"
"                            input file(s). Defaults are used for \n"
"                            unspecified material parameters. \n"
"\n"
"                            If --override  is not specified, then\n"
"                            only the material parameters specified on\n"
"                            the command line are changed. In addition,\n"
"                            nodes and objects without materials are \n"
"                            left unchanged, except for the root node.\n"
"                            A material data structure is created for\n"
"                            the root node if one does not already\n"
"                            exist.\n"

"\n"
"      --rootOnly          - Add or modify materials only at the root\n"
"                            node.\n"
"\n"
"                            If this is not specified then materials \n"
"                            are modified at all nodes and drawable\n"
"                            objects. \n"
"\n"
"       If --override is specified and --rootOnly is not specified, \n"
"       then a new material data structure is created for all nodes and\n"
"       drawable objects that do not already have one.  \n"
"\n"
"       As mentioned above, a material data structure is created \n"
"       for the root node if one does not already exist.\n"
" \n"
" \n"
"  Specification of input files: A list of input file names follows \n"
"  all of the options: \n"
" \n"
"      [infile . . . ]   \n"
" \n"
"  If no input files are specified, then the \"--noInput\" option \n"
"  must also be present.  In this case, a minimal data structure \n"
"  is created to receive the specified material parameters. \n"
" \n"
"       \n"
"  The specification of output file(s):  \n"
" \n"
"      outfile \n"
" \n"
"  If the output file specification is \"osg\" or \"ive\" then one  \n"
"  output file is created for each input file.  The output file  \n"
"  name will be generated from the base file name of the input  \n"
"  file, followed by \".osg\" or \".ive\".  Otherwise, the output \n"
"  file specification is taken to be a complete file name, and \n"
"  data from all of the input files are grouped into that single \n"
"  output file. \n"
" \n"
" \n"
#if 1
"EXAMPLES:\n"
"\n"
"  Change the diffuse material color to red:\n"
"\n"
"    hev-materialTransform --diff 1 0 0 1 cube.osg redCube.osg\n"
"  \n"
"  Change the ambient and diffuse colors for several input files \n"
"  and write a single output file:\n"
"\n"
"    hev-materialTransform --amb 0.8 0.8 0.8 1 --diff 1 0 0 1   \\\n"
"          cube.osg sphere.ive sub.savg composite.ive\n"
"    \n"
"  Change the color mode parameter for several input files and\n"
"  write out one output file (in .ive format) for each input file:\n"
"\n"
"    hev-materialTransform --colormode DIFFUSE  \\\n"
"          cube.osg sphere.ive sub.savg ive\n"
"\n"
"\n"
"NOTES:\n"
"\n"
"  A complete description of the various material parameters can\n"
"  be found in:\n"
"\n"
"    OpenGL Programming Guide, Second Edition\n"
"    By Mason Woo, Jackie Neider, and Tom Davis\n"
"\n"
"  The appearance of an object is determined both by its material\n"
"  properties and by the lighting environment. There is much\n"
"  interaction between materials and lighting. For more information\n"
"  about this, look at the OpenGL Programming Guide.\n"
"\n"
"  The option \"--rmGeomColors\" is (in some sense) not really needed. \n"
"  You can get essentially the same effect with the option \n"
"  \"--colorMode OFF\".\n"
"\n"
"  If you want to nullify all existing surface attributes in a file and \n"
"  force the entire object have the same appearance, you can use this \n"
"  set of options:  \"--rmMaterials --colormode OFF --rootOnly\"\n"
"\n"
"  There is some overlap between this program and the programs\n"
"  hev-applyState and hev-mergeState.\n"
"\n"
"\n"
"SEE ALSO:\n"
"\n"
"  hev-applyState, hev-mergeState\n"
"\n"

#endif

" \n"
);  // end of fprintf 



#if 0

OBSOLETE:


      (NOTE 1: I had not realized that materials can be specified 
       independently for the front and back faces of polygons.  
       In the past, we have made the front and back face material 
       identical, but I can see how it could be useful to give them 
       different appearances.)

      (NOTE 2: There is another material parameter that OpenGL
       defines, which is called GL_COLOR_INDICES.  This has to do
       with lighting when you're in "color index mode".  I don't
       believe that we ever operate in color index mode, and 
       the OSG Material class does not deal with this case.)

  These options specify how to combine the material specified on the
  command line with the material(s) already present in the input files:

      --mergeMaterials    - override only material parmeters specfied 
                            on command line (default)
      --copyMaterials     - override all material parameters; use 
                            defaults for unspecified material parameters

      (NOTE: Im not sure if this use of the terms "merge" and "copy"
       is consistent with the MERGESTATE and COPYSTATE sge commands.
       If not, Ill either change the terms or make their action
       consistent.)

      --allNodes          - add materials at each node (default)
      --topNodeOnly       - add materials only at top node



  OMIT THIS SECTION (combineFiles, separateFiles)
  These options specify how to create the output files:

      --combineFiles      - produces one output file combining
                                all input files (default) 

      --separateFiles     - produce one output file for each 
                                input file 

        If --separateFiles, then each input file is changed separately, 
        and the output file name must contain the string %%BASEFN%%.  
        Each output name is constructed by taking the corresponding 
        inputname, stripping off the extension (that determines the 
        input format) and substituting that stripped name for 
        %%BASEFN%%.  So it a command could look something like this:

        hev-material --separateFiles --ambient 0.1 0.1 0.1 1.0  \
           foo.savg bar.ive  %%BASEFN%%.osg

        This would produce output files foo.osg and bar.osg.






#endif



}  // end of usage()


typedef struct output_spec
	{
	bool singleOutput;
	char fileName[1000];
	} OutputSpec;



enum MaterialPropertyType { NO_PT, AMB_PT, DIF_PT, SPEC_PT, EM_PT, SH_PT, CM_PT, NUM_PROPERTY_TYPES };

enum MaterialPropertyFace { FRONT_FACE, BACK_FACE, BOTH_FACES };

typedef struct material_property
	{
	MaterialPropertyType type;
	bool facePresent[3];  // the 3 are: front, back, both
	// double rgba[3][4];

	osg::Vec4 rgba[3];   // rgba[][0] is shininess
			// rgba[0][0] is mode (converted from enum integer)

	// osg::Material::ColorMode mode;
	} MaterialProperty;
	
 
typedef struct material_spec
	{
	bool merge;
	bool rootOnly;
	bool materialsSpecified;
	char sourceFN[1000];
	MaterialProperty property[NUM_PROPERTY_TYPES];
	} MaterialSpec;


typedef struct preprocess_input_spec
	{
	bool removeMat;
	bool removeGeomCol;
	} PreprocessInputSpec;



static char *
fltCmToName (float fltCm)
	{
	osg::Material::ColorMode cm = 
		(osg::Material::ColorMode) ((int) (fltCm + 0.5));

	switch (cm)
		{
		case osg::Material::AMBIENT:
		  return const_cast<char*>("AMBIENT");

		case osg::Material::DIFFUSE:
		  return const_cast<char*>("DIFFUSE");

		case osg::Material::SPECULAR:
		  return const_cast<char*>("SPECULAR");

		case osg::Material::EMISSION:
		  return const_cast<char*>("EMISSION");

		case osg::Material::AMBIENT_AND_DIFFUSE:
		  return const_cast<char*>("AMBIENT_AND_DIFFUSE");

		case osg::Material::OFF:
		  return const_cast<char*>("OFF");

		}

	return const_cast<char*>("UNKNOWN");

	}  // end of cmToName

static char *
propTypeName (MaterialPropertyType type)
	{

	switch (type)
		{
		case AMB_PT: return const_cast<char*>("ambient");

		case DIF_PT: return const_cast<char*>("diffuse");

		case SPEC_PT: return const_cast<char*>("specular");

		case EM_PT: return const_cast<char*>("emission");

		case SH_PT: return const_cast<char*>("shininess");

		case CM_PT: return const_cast<char*>("colorMode");

		case NO_PT: return const_cast<char*>("no property");

		}

	return const_cast<char*>("unknown property");
	}  // end of propTypeName

static char *
intToFaceName (int iface)
	{

	if (iface == BACK_FACE)
		{
		  return const_cast<char*>("back face ");
		}

	if (iface == FRONT_FACE)
		{
		  return const_cast<char*>("front face");
		}

	if (iface == BOTH_FACES)
		{
		  return const_cast<char*>("both faces");
		}


	return const_cast<char*>("unrecognized face");

	}  // end of intToFaceName



static void
printMaterialProperty (MaterialProperty &prop)
	{

	printf ("       material property for %s:\n", propTypeName(prop.type));

	for (int i = 0; i < 3; i++)
		{
		printf ("            %s = %s ",
		  intToFaceName(i), prop.facePresent[i] ? "present: " : "none");

		if (prop.facePresent[i])
		    {
		    switch (prop.type)
			{
			case SH_PT:
				// shininess
				printf ("  shininess = %f\n", 
					prop.rgba[i][0]);
				break;


			case CM_PT:
				// color mode
				printf ("  color mode = %s\n", 
					fltCmToName (prop.rgba[0][0]));
				break;


			default:
				// one of the colors: rgba
				printf ("  rgba = %g %g %g %g\n",
					prop.rgba[i][0], prop.rgba[i][1],
					prop.rgba[i][2], prop.rgba[i][3]);
			}
		    }
		else
		    {
		    printf ("\n");
		    }
		}


	}  // end of printMaterialProperty


static bool TransparencyPresent = false;

static void 
setTransparency(osg::StateSet *ss)
        {
        ss->setRenderingHint (osg::StateSet::TRANSPARENT_BIN);
        ss->setMode (GL_BLEND, 
                     osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
        }  // end of setTransparency


static int
setFaceProperty (
  MaterialPropertyType type, 
  osg::Vec4 &rgba, 
  osg::Material::Face face, 
  osg::Material *material)
	{


	switch (type)
		{
		case AMB_PT:
                        if (rgba[3] < 1.0)
                                {
                                TransparencyPresent = true;
                                }
			material->setAmbient (face, rgba);
			break;

		case DIF_PT:
                        if (rgba[3] < 1.0)
                                {
                                TransparencyPresent = true;
                                }
			material->setDiffuse (face, rgba);
			break;

		case SPEC_PT:
                        if (rgba[3] < 1.0)
                                {
                                TransparencyPresent = true;
                                }
			material->setSpecular (face, rgba);
			break;

		case EM_PT:
                        if (rgba[3] < 1.0)
                                {
                                TransparencyPresent = true;
                                }
			material->setEmission (face, rgba);
			break;

		case SH_PT:
			material->setShininess (face, rgba[0]);
			break;

		case CM_PT:
			if (face != osg::Material::FRONT_AND_BACK)
				{
    				dtkMsg.add(DTKMSG_ERROR,
	          			"hev-materialTransform: "
					"Bad color mode face %d "
					"(internal error).\n", face);
				return -1;
				}

			material->setColorMode (
			  (osg::Material::ColorMode) ((int) (rgba[0] + 0.5)) );
			break;

		default:
    			dtkMsg.add(DTKMSG_ERROR,
	          		"hev-materialTransform: "
				"Bad property type %d (internal error).\n",
				type) ;
			return -1;

		}  // end of switch

	return 0;
	} // end of setFaceProperty



static int
modifyMaterial (
  MaterialProperty property[NUM_PROPERTY_TYPES], 
  osg::Material *material
  )
	{

	if ( ! material )
		{
		// If the material is NULL, then don't do anything.
		return 0;
		}

	for (int i = AMB_PT; i < NUM_PROPERTY_TYPES; i++)
		{

		if (property[i].facePresent[FRONT_FACE])
			{
			setFaceProperty ((MaterialPropertyType)i, 
					property[i].rgba[FRONT_FACE], 
					osg::Material::FRONT, material);
			}
		
		if (property[i].facePresent[BACK_FACE])
			{
			setFaceProperty ((MaterialPropertyType)i, 
					property[i].rgba[BACK_FACE], 
					osg::Material::BACK, material);
			}
		
		if (property[i].facePresent[BOTH_FACES])
			{
			setFaceProperty ((MaterialPropertyType)i, 
					property[i].rgba[BOTH_FACES], 
					osg::Material::FRONT_AND_BACK, material);
			}
					
		}  // end of loop over property types


	return 0;
	}  // end of modifyMaterial


static int
modifyStateSetMaterial (
  bool merge, 
  MaterialProperty property[NUM_PROPERTY_TYPES], 
  osg::StateSet *ss)
	{
	osg::Material *material;

	material = (osg::Material *) ss->getAttribute (
					osg::StateAttribute::MATERIAL);

	if ( material )
		{
		if ( ! merge )
			{
			// if we're supposed to override the current material
			// then we just remove the current material and
			// pretend it never existed.
			ss->removeAttribute (material);
			material = NULL;
			}
		}

	// if there is no material and we are supposed to merge, then
	// we should not create a new material.

	// However, if there is no material and we are supposed to override,
	// then we need to create a default material, and then modify it
	// based on the incoming property parameters.

	if ( ( ! material ) && ( ! merge ) )
		{
		material = new osg::Material;
		ss->setAttribute (material);
		}

        
        TransparencyPresent = false;

	modifyMaterial (property, material);

	// ss->setAttribute (material);

        if (TransparencyPresent)
                {
                setTransparency (ss);
                }

	return 0;
	}  // modifyStateSetMaterial


static int
modifyNodeMaterial (
  bool merge, 
  MaterialProperty property[NUM_PROPERTY_TYPES], 
  osg::Node *node)
	{
	osg::StateSet *ss;

	if ( ! merge )
		{
		ss = node->getOrCreateStateSet ();

		if ( ! ss )
			{
			// error
    			dtkMsg.add(DTKMSG_ERROR,
	          	"Unable to get StateSet for Node (internal error).\n");
			return -1;
			}
		}
	else
		{
		ss = node->getStateSet ();
		if ( ! ss )
			{
			return 0;
			}
		}




	return modifyStateSetMaterial (merge, property, ss);
	}  // modifyNodeMaterial
	

static int
modifyDrawableMaterial (
  bool merge, 
  MaterialProperty property[NUM_PROPERTY_TYPES], 
  osg::Drawable *drawable)
	{
	osg::Material *material;

#if 0
	osg::StateSet *ss = drawable->getOrCreateStateSet ();
	if ( ! ss )
		{
		// error
    		dtkMsg.add(DTKMSG_ERROR,
	          "Unable to get StateSet for Drawable (internal error).\n");
		return -1;
		}
#else

	osg::StateSet *ss;
	if ( ! merge )
		{
		ss = drawable->getOrCreateStateSet ();

		if ( ! ss )
			{
			// error
    			dtkMsg.add(DTKMSG_ERROR,
	          	"Unable to get StateSet for Node (internal error).\n");
			return -1;
			}
		}
	else
		{
		ss = drawable->getStateSet ();
		if ( ! ss )
			{
			return 0;
			}
		}

#endif




	return modifyStateSetMaterial (merge, property, ss);
	}  // modifyDrawableMaterial



static void
ensureNodeMaterial (osg::Node *node)
	{
	// We want to make sure that this node has a material

	osg::StateSet *ss = node->getOrCreateStateSet ();
	if ( ! ss )
		{
		// error
    		dtkMsg.add(DTKMSG_ERROR,
	          "Unable to get StateSet for Node (internal error).\n");
		return;
		}

	osg::Material *material = (osg::Material *) ss->getAttribute (
					osg::StateAttribute::MATERIAL);

	// If there is no material in the StateSet, make one and put it in.
	if ( ! material )
		{
		material = new osg::Material;
		ss->setAttribute (material);
		}

	}  // end of ensureNodeMaterial

static void
modifyMaterials (MaterialSpec &matSpec, osg::Node *node)
	{

	// Here (and below) is where the action is.

	if ( ! matSpec.materialsSpecified )
		{
		if (Verbose)
			{
			printf ("   No material modifications specified.\n");
			}
		return;
		}

	if (Verbose)
		{
		printf ("   Modifying materials under current node.\n");
		}


	// We have to make sure that the root node already has a Material.
	ensureNodeMaterial (node);

	if (matSpec.rootOnly)
		{
		// Only do the root node
		modifyNodeMaterial ( matSpec.merge, matSpec.property, node);
		if (Verbose)
			{
			printf ("   Done modifying materials at root node.\n");
			}
		return;
		}

	// We get here if only if we are to modify materials throughout
	// the tree.

	// OK, we want to traverse the entire structure modifying or adding
	// the material at every node and every drawable


	// The StatsVisitor gives us an easy way to find all drawables
	osgUtil::StatsVisitor stats;
	node->accept (stats);
	stats.totalUpStats ();
	// stats.print(std::cout);


	// loop over all drawables
	for( osgUtil::StatsVisitor::DrawableSet::iterator 
             itr  = stats._drawableSet.begin();
	     itr != stats._drawableSet.end();
	     ++itr)
    		{
		modifyDrawableMaterial ( matSpec.merge, 
					 matSpec.property,  
					 *itr);
		}  // end of loop over drawables

	stats.reset ();


	// OK, now we use a Node visitor and call modifyNodeMaterial
	// at each Node.
	class modMatVisitor : public osg::NodeVisitor
		{
		public:
		bool merge;
		MaterialProperty *property;

		modMatVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
			{
			};

		virtual void apply (osg::Node &node)
			{
			modifyNodeMaterial ( merge, property, &(node));
			traverse (node);
			}

		} ;


	modMatVisitor visitor;
	visitor.merge = matSpec.merge;
	visitor.property = matSpec.property;
	node->accept (visitor);

	if (Verbose)
		{
		printf (
		    "   Done modifying materials throughout the input data.\n");
		}


	}  // end of modifyMaterials



static void
removeMaterials (osg::Node *node)
	{
	if (Verbose)
		{
		printf ("   Removing all materials under current node.\n");
		}

	// Materials appear in StateSets.  StateSets appear only in 
	// Nodes and in Drawables.

	// We use a StatsVisitor to deal with the Drawables and a 
	// NodeVisitor to handle nodes.


	// First the Drawables:
	int numDrawables = 0;
	int numDrawableMatRemovals = 0;

	// The StatsVisitor gives us an easy way to find all drawables
	osgUtil::StatsVisitor stats;
	node->accept (stats);
	stats.totalUpStats ();
	// stats.print(std::cout);


	// loop over all drawables
	for( osgUtil::StatsVisitor::DrawableSet::iterator 
             itr  = stats._drawableSet.begin();
	     itr != stats._drawableSet.end();
	     ++itr)
    		{

		numDrawables++;
		// get the StateSet if there is one
		osg::StateSet *ss = (*itr)->getStateSet();
		if (ss)
			{
			// now get the material
			osg::Material *mat = (osg::Material *) ss->getAttribute (
					osg::StateAttribute::MATERIAL);
			if (mat)
				{
				// remove the material if there is one
				numDrawableMatRemovals++;
				ss->removeAttribute (mat);
				}
			}
		}  // end of loop over drawables

	stats.reset ();


	// OK, now we use a Node visitor to look for StateSets and Materials
	// at each Node.
	class rmMatVisitor : public osg::NodeVisitor
		{
		public:

		int numNodes;
		int numNodeMatRemovals;

		rmMatVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
			{
			numNodes = 0;
			numNodeMatRemovals = 0;
			};

		virtual void apply (osg::Node &node)
			{
			// This is called for each Node.
			numNodes++;

			// first get the StateSet, if there is one
			osg::StateSet *ss = node.getStateSet ();
			if (ss)
				{
				// now get the material
				osg::Material *mat = (osg::Material *) ss->getAttribute (
					osg::StateAttribute::MATERIAL);
				if (mat)
					{
					// remove the material if there is one
					numNodeMatRemovals++;
					ss->removeAttribute (mat);
					}
				}
			traverse (node);
			}

		} ;


	rmMatVisitor visitor;
	node->accept (visitor);

	if (Verbose)
		{
		printf ("   Removed %d materials from %d nodes.\n",
			visitor.numNodeMatRemovals, visitor.numNodes);
		printf ("   Removed %d materials from %d drawables.\n",
			numDrawableMatRemovals, numDrawables);
		}

	visitor.reset();
	}  // end of removeMaterials



static void
removeGeomColors (osg::Node *node)
	{
	if (Verbose)
		{
		printf ("   Removing all geometry color under current node.\n");
		}


	// Color associated with geometry can only occur in Geometry objects.

	// The osgUtil::StatsVisitor gives us an easy way get a list of Geometries
	osgUtil::StatsVisitor stats;
	node->accept (stats);
	stats.totalUpStats ();
	// stats.print(std::cout);


	int numGeoms = 0;
	int numGeomColorRemovals = 0;

	for( osgUtil::StatsVisitor::DrawableSet::iterator 
	     itr  = stats._geometrySet.begin();
	     itr != stats._geometrySet.end();
	     ++itr)
    		{
        	osg::Geometry *geom = (osg::Geometry *)(*itr);

		// For each geometry, we look for each way that
		// color can be attached.

		osg::Array *array;

// wng2: NO_DEPRECATED_OSG is used below to indicate that the *Indices members 
// of osg::Geometry have been deprecated in osg >= 3.2 and thus remove the
// code below (since NO_DEPRECATED_OSG is not declared anywhere).
// This is more for documentation purposes, incase we need to figure out how
// to deal with osg files that have data in the *Indices arrays.
#if NO_DEPRECATED_OSG
		osg::IndexArray *indexArray;
#endif

		// look for a color Array
		array = geom->getColorArray ();
		if (array)
			{
			numGeomColorRemovals++;
			array = NULL;
			geom->setColorArray (array);
			}

#if NO_DEPRECATED_OSG
		// look for color indices
		indexArray = geom->getColorIndices();
		if (indexArray)
			{
			numGeomColorRemovals++;
			indexArray = NULL;
			geom->setColorIndices (indexArray);
			}
#endif

		// look for a secondary color Array
		array = geom->getSecondaryColorArray ();
		if (array)
			{
			numGeomColorRemovals++;
			array = NULL;
			geom->setSecondaryColorArray (array);
			}

#if NO_DEPRECATED_OSG
		// look for secondary color indices
		indexArray = geom->getColorIndices();
		if (indexArray)
			{
			numGeomColorRemovals++;
			indexArray = NULL;
			geom->setSecondaryColorIndices 
						(indexArray);
			}
#endif

		numGeoms ++;

    		}  // end of loop over geometries

	if (Verbose)
		{
		printf (
		"   Removed %d color arrays from %d geometries in %d geodes.\n",
			numGeomColorRemovals, 
			numGeoms , 
			stats._numInstancedGeode);
		}

	stats.reset ();
	}  // end of removeGeomColors


static int
writeNode (osg::Node *node, char *outFN)
	{

	// if (osgDB::writeObjectFile(*node, string(outFN)))  
	if (osgDB::writeNodeFile(*node, string(outFN)))  
		{
    		// normal exit
    		return 0;
		}
	else
		{
    		dtkMsg.add(DTKMSG_ERROR,
	          "Unable to write file \"%s\", exiting\n",outFN) ;
    		return 1 ;
		}

	}  // end of writeNode

static osg::Node *
readNode (string &inFN, osg::Group *parent)
	{
	osg::Node *fileNode = osgDB::readNodeFile(inFN) ;
	if (fileNode)
		{
		if (parent)
			{
    			parent->addChild(fileNode) ;
			}
		} 
	else
		{
    		dtkMsg.add(DTKMSG_ERROR,
       		  "Unable to load file \"%s\".\n", inFN.c_str()) ;
		}

	return fileNode;
	}  // end of readNode
		

static void
preprocessInputData (PreprocessInputSpec &preProc, osg::Node *node)
	{
	if (preProc.removeMat)
		{
		removeMaterials (node);
		}

	if (preProc.removeGeomCol)
		{
		removeGeomColors (node);
		}
	}  // end of preprocessInputData

static void
mkOutputFileName (const char *inFN, char *outType, char *outFN)
	{

	strcpy (outFN, inFN);
	char *dot = strrchr (outFN, '.');

	if (dot != NULL)
		{
		*dot = 0;
		}

	strcat (outFN, ".");
	strcat (outFN, outType);
	}  // end of mkOutputFileName

static int
inchByInch (
	vector<string> &inFN, 
	PreprocessInputSpec &preProc,
	MaterialSpec &matSpec,
	char *outFileType)
	{

	if (Verbose)
		{
		printf ("Processing each input file into "
				"a separate output file.\n");
		printf ("Reading %d input file(s).\n", inFN.size());
		}

	// read, process, write each input file individually
	for (int i = 0; i < inFN.size(); i++)
		{

		if (Verbose)
			{
			printf ("   reading file %d : \"%s\"\n", 
						i+1, inFN[i].c_str());
			}

		// read file
		osg::Node *fileNode = readNode (inFN[i], NULL);

		if (fileNode)
			{
			// if read was successful

			// remove materials and geom colors if needed
			preprocessInputData (preProc, fileNode);

			modifyMaterials (matSpec, fileNode);

			char outFN[1000];
			mkOutputFileName (inFN[i].c_str(), outFileType, outFN);

			if (Verbose)
				{
				printf ("   %d writing file \"%s\"\n", 
							i, outFN);
				}

			if (writeNode (fileNode, outFN))
				{
				return 1;
				}

			}

		}

	return 0;
	}  // end of inchByInch







static int
allTogetherNow (
	vector<string> &inFN, 
	PreprocessInputSpec &preproc,
	MaterialSpec &matSpec,
	char *outFN)
	{

    	osg::Node *node;

	if (Verbose)
		{
		printf ("Processing all inputs into a single output file.\n");
		}

	if (inFN.size() == 0)
	    {
	    if (Verbose)
		{
		printf ("No Input files specified, "
			   "so we're creating a minimal Node.\n");
		}

	    // create a single minimal node; I make it a Group node
	    // so that the user could (conceivably) load it into the
	    // the scene graph and then hang some other nodes under it,
	    // which would get the Materials specified here.
	    node = new osg::Group;

	    }
	else
	    {

	    if (Verbose)
		{
		printf ("Reading %d input file%s.\n", inFN.size(),
			(inFN.size()==1)?"":"s");
		}

	    // If there is more than one input file, read all input files, 
	    // put them under a new group node

	    // If there is only one input file, then don't introduce
	    // a new group node to hold it.

	    osg::Group *groupNode;
	    if (inFN.size() > 1)
		{
	        groupNode = new osg::Group ;
	        node = groupNode;
		}
	    else
		{
		groupNode = NULL;
		}

	    for (int i = 0; i < inFN.size(); i++)
		{
		if (Verbose)
			{
			printf ("   reading file %d : \"%s\"\n", 
					i+1, inFN[i].c_str());
			}

		osg::Node *fileNode = readNode (inFN[i], groupNode);
		if ( ! fileNode )
			{
    			dtkMsg.add(DTKMSG_ERROR, "hev-materialTransform: "
				"Error reading input file \"%s\".\n", 
				inFN[i].c_str());
			return -1;
			}
		else if (groupNode == NULL)
			{
			node = fileNode;
			}
		}
	    }

	// remove materials and geom colors if needed
	preprocessInputData (preproc, node);

	modifyMaterials (matSpec, node);

	if (Verbose)
		{
		printf ("Writing file \"%s\"\n", outFN);
		}

	return writeNode (node, outFN);
	}  // end of allTogetherNow
		








////////////////////////////////////////////////////////////////////////
//
// Here's where we start parsing the input
//



static int
getMaterialProperty (
   int &iarg, 
   MaterialPropertyType type,
   int argc, 
   char **argv, 
   MaterialProperty &matProp )
	{

	matProp.type = type;

	int cStart;
	int len;

	int ii = (iarg) + 1;


	MaterialPropertyFace face;

	// Figure out whether back, front, or both have been specified

	len = strlen (argv[ii]);

	if ( (len > 1) && (! strncasecmp ("back", argv[ii], len)) )
		{
		face = BACK_FACE;
		cStart = ii+1;
		}
	else if ( ! strncasecmp ("front", argv[ii], len) )
		{
		face = FRONT_FACE;
		cStart = ii+1;
		}
	else if ( (len > 1) && (! strncasecmp ("both", argv[ii], len)) )
		{
		face = BOTH_FACES;
		cStart = ii+1;
		}
	else
		{
		face = BOTH_FACES;
		cStart = ii;
		}


	double d;

	len = strlen (argv[iarg]);
	// Handle colorMode and shininess differently.
	if ( ! strncasecmp (argv[iarg], "--colorMode", len) )
		{

		if (face != BOTH_FACES)
			{
    			dtkMsg.add(DTKMSG_ERROR, "hev-materialTransform: "
				"Color mode cannot be applied separately "
				"to front or back face.\n");
			return -1;
			}

		if (cStart >= argc)
			{
    			dtkMsg.add(DTKMSG_ERROR, "hev-materialTransform: "
			    "Color mode value is missing.\n");
			return -1;
			}

		len = strlen (argv[cStart]);

		if ( (len <= 7) && (! strncasecmp(argv[cStart],"ambient", len)))
			{
			matProp.rgba[2][0] = 
			matProp.rgba[1][0] = 
			matProp.rgba[0][0] = osg::Material::AMBIENT;
			}
		else if ( ! strncasecmp (argv[cStart], "diffuse", len) )
			{
			matProp.rgba[2][0] = 
			matProp.rgba[1][0] = 
			matProp.rgba[0][0] = osg::Material::DIFFUSE;
			}
		else if ( ! strncasecmp (argv[cStart], "specular", len) )
			{
			matProp.rgba[2][0] = 
			matProp.rgba[1][0] = 
			matProp.rgba[0][0] = osg::Material::SPECULAR;
			}
		else if ( ! strncasecmp (argv[cStart], "emission", len) )
			{
			matProp.rgba[2][0] = 
			matProp.rgba[1][0] = 
			matProp.rgba[0][0] = osg::Material::EMISSION;
			}
		else if ( !strncasecmp (argv[cStart],"ambient_and_diffuse",len))
			{
			matProp.rgba[2][0] = 
			matProp.rgba[1][0] = 
			matProp.rgba[0][0] = osg::Material::AMBIENT_AND_DIFFUSE;
			}
		else if ( ! strncasecmp (argv[cStart], "off", len) )
			{
			matProp.rgba[2][0] = 
			matProp.rgba[1][0] = 
			matProp.rgba[0][0] = osg::Material::OFF;
			}
		else
			{
    			dtkMsg.add(DTKMSG_ERROR, "hev-materialTransform: "
			    "Unrecognized color mode \"%s\".\n", argv[cStart]);

			return -1;
			}

		iarg = cStart+1;

		}
	else if ( ! strncasecmp (argv[iarg], "--shininess", len) )
		{

		if (cStart >= argc)
			{
    			dtkMsg.add(DTKMSG_ERROR, "hev-materialTransform: "
			    "Shininess value is missing.\n");
			return -1;
			}

		if ( sscanf (argv[cStart], "%lf",  &d) != 1)
			{
    			dtkMsg.add(DTKMSG_ERROR, "hev-materialTransform: "
			   "Unable to parse shininess \"%s\".\n", argv[cStart]);
			return -1;
			}

		matProp.rgba[face][0] = d;
		iarg = cStart+1;
		}
	else
		{

		// handle ambient, diffuse, specular, and emission all the same

		if ((cStart+3) >= argc)
			{
    			dtkMsg.add(DTKMSG_ERROR, "hev-materialTransform: "
			    "%s RGBA values are missing.\n", argv[iarg]);
			return -1;
			}

		for (int i = 0; i < 4; i++)
			{

			if (sscanf (argv[cStart+i], "%lf", &d) != 1)
				{
    				dtkMsg.add(DTKMSG_ERROR, 
					"hev-materialTransform: "
			    		"Unable to parse %s value \"%s\".\n", 
					argv[iarg],
					argv[cStart+i]);
				return -1;
				}
			matProp.rgba[face][i] = d;

			}
		iarg = cStart+4;
		}

	matProp.facePresent[face] = true;

	return 0;
	}  // end of getMaterialProperty


// The following function determines if two faces are identical for
// the given property
static bool
samePropVal (
  MaterialProperty &prop, 
  MaterialPropertyFace face1,
  MaterialPropertyFace face2)
	{

	if (prop.type == SH_PT)
		{
		return prop.rgba[face1][0] == prop.rgba[face2][0];
		}

	if (prop.type == CM_PT)
		{
		// front and back are always the same
		return true;
		}


	// if we get here, we have one of the color types


	for (int i = 0; i < 4; i++)
		{
		if (prop.rgba[face1][i] != prop.rgba[face2][i])
			{
			return false;
			}
		}

	return true;
	}  // end of samePropVal



static int
simplifyProperty (MaterialProperty &property)
	{




	if (property.facePresent[BOTH_FACES])
	    {
	    // If we have a spec for BOTH_FACES and for either front or back
	    // then we need to check that they agree.  
	    // If they don't agree it's an error; if they do agree, then
	    // we should keep BOTH and throw away the front and/or back.
	

	    if (property.facePresent[FRONT_FACE])
		{
		if ( ! samePropVal (property, BOTH_FACES, FRONT_FACE))
			{
			dtkMsg.add(DTKMSG_ERROR, "hev-materialTransform: "
				"Conflicting %s specifications.\n", 
				propTypeName (property.type)  );
			return -1;
			}
		property.facePresent[FRONT_FACE] = false;
		}

	    if (property.facePresent[BACK_FACE])
		{
		if ( ! samePropVal (property, BOTH_FACES, BACK_FACE))
			{
			dtkMsg.add(DTKMSG_ERROR, "hev-materialTransform: "
				"Conflicting %s specifications.\n", 
				propTypeName (property.type)  );
			return -1;
			}
		property.facePresent[BACK_FACE] = false;
		}
	    }
	else if ( (property.facePresent[FRONT_FACE]) && 
			(property.facePresent[BACK_FACE]) )
	    {

	    // if front and back are the same, then convert them to BOTH
	    if ( samePropVal (property, FRONT_FACE, BACK_FACE))
		{

		// Should only transfer the data relevant to the property
		// type, but there's no harm in moving everything.
#if 0
		property.rgba[BOTH_FACES][0] = property.rgba[FRONT_FACE][0];
		property.rgba[BOTH_FACES][1] = property.rgba[FRONT_FACE][1];
		property.rgba[BOTH_FACES][2] = property.rgba[FRONT_FACE][2];
		property.rgba[BOTH_FACES][3] = property.rgba[FRONT_FACE][3];
#else
		property.rgba[BOTH_FACES] = property.rgba[FRONT_FACE];
#endif
		// property.mode[BOTH_FACES] = property.mode[FRONT_FACE];
		// property.shininess[BOTH_FACES] = property.shininess[FRONT_FACE];

		property.facePresent[BACK_FACE] = false;
		property.facePresent[FRONT_FACE] = false;
		property.facePresent[BOTH_FACES] = true;

		}
	    }

	return 0;
	}  // end of simplifyProperty

static int
simplifyProperties (
  MaterialProperty property[NUM_PROPERTY_TYPES])
	{

	for (int i = 0; i < NUM_PROPERTY_TYPES; i++)
		{
		if (property[i].type != NO_PT)
			{
			if (simplifyProperty (property[i]))
				{
				return -1;
				}
			}
		}

	return 0;
	}  // end of simplifyProperties

static osg::Material * getMaterialFromFile (char *fn, int *err)
	{

	*err = 0;

	if (strlen (fn) == 0)
		{
		return NULL;
		}

	
	string *str = new string (fn);
	static osg::Node *node = readNode (*str, NULL);
	if (node == NULL)
		{
		*err = -1;
		return NULL;
		}

	// First we look for a Material at the root node
	osg::StateSet *ss = node->getStateSet ();
	if (ss)
		{
		// now get the material
		osg::Material *mat = (osg::Material *) ss->getAttribute (
					osg::StateAttribute::MATERIAL);
		if (mat)
			{
			// We've found our material
			return mat;
			}
		}


	// If we don't find a Material at the root node, then we're
	// going to do a NodeVisitor to look for one at each Node.
	// If we don't find one on a node, then we'll use a osgUtil::StatsVisitor
	// to look at each Drawable.

	class findMatVisitor : public osg::NodeVisitor
		{
		public:

		osg::Material *foundMaterial;

		findMatVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
			{
			foundMaterial = NULL;
			};

		virtual void apply (osg::Node &node)
			{
			// first get the StateSet, if there is one
			osg::StateSet *ss = node.getStateSet ();
			if (ss)
				{
				// now get the material
				osg::Material *mat = (osg::Material *) ss->getAttribute (
					osg::StateAttribute::MATERIAL);
				if (mat)
					{
					if (foundMaterial == NULL)
						{
						foundMaterial = mat;
						}
					}
				else
					{
					// only continue traversal if we
					// don't find a material
					traverse (node);
					}
				}
			}

		} ;  // end of findMatVisitor class


	findMatVisitor visitor;
	node->accept (visitor);

	if (visitor.foundMaterial != NULL)
		{
		return visitor.foundMaterial;
		}


	// OK we didn't find a Material on a node, so we'll try to find
	// one on a Drawable


	// The osgUtil::StatsVisitor gives us an easy way to find all drawables
	osgUtil::StatsVisitor stats;
	node->accept (stats);
	stats.totalUpStats ();
	// stats.print(std::cout);


	// loop over all drawables
	for( osgUtil::StatsVisitor::DrawableSet::iterator 
             itr  = stats._drawableSet.begin();
	     itr != stats._drawableSet.end();
	     ++itr)
    		{

		// get the StateSet if there is one
		osg::StateSet *ss = (*itr)->getStateSet();
		if (ss)
			{
			// now get the material
			osg::Material *mat = (osg::Material *) ss->getAttribute (
					osg::StateAttribute::MATERIAL);
			if (mat)
				{
				// Yes! We've found a Material, so return!
				return mat;
				}
			}
		}  // end of loop over drawables

	// If we've reached this point, there are no Materials in the
	// file that we read in.  We regard this as an error.

	dtkMsg.add (DTKMSG_ERROR, 
		"hev-materialTransform: No materials found in file \"%s\".\n", 
		fn);

	*err = -1;
	return NULL;
	}  // end of getMaterialFromFile

static void
reconcileProperty (
  MaterialProperty &property, 
  bool fb, 
  const osg::Vec4 &fRgba, 
  const osg::Vec4 &bRgba)
	{

	// The data present in property overrides the data in fb, fRGBA, bRgba

	// So we only use the stuff in fb, fRGBA, bRgba, when it's not present
	// in property.

	if (property.facePresent[BOTH_FACES])
		{
		// both faces specified in property so don't do anything
		return;
		}

	if ( ! property.facePresent[FRONT_FACE] )
		{
	        property.facePresent[FRONT_FACE] = true;



#if 1
		property.rgba[FRONT_FACE] = fRgba;
#else
		if (property.type == SH_PT)
			{
			property.shininess[FRONT_FACE] = fRgba[0];
			}
		else
			{
#if 0
			property.rgba[FRONT_FACE][0] = fRgba[0];
			property.rgba[FRONT_FACE][1] = fRgba[1];
			property.rgba[FRONT_FACE][2] = fRgba[2];
			property.rgba[FRONT_FACE][3] = fRgba[3];
#else
			property.rgba[FRONT_FACE] = fRgba;
#endif
			}
#endif



		}

	if ( ! property.facePresent[BACK_FACE] )
		{
	        property.facePresent[BACK_FACE] = true;
#if 1

		property.rgba[BACK_FACE] = bRgba;

#else
		if (property.type == SH_PT)
			{
			property.shininess[BACK_FACE] = bRgba[0];
			}
		else
			{
#if 0
			property.rgba[BACK_FACE][0] = bRgba[0];
			property.rgba[BACK_FACE][1] = bRgba[1];
			property.rgba[BACK_FACE][2] = bRgba[2];
			property.rgba[BACK_FACE][3] = bRgba[3];
#else
			property.rgba[BACK_FACE] = bRgba;
#endif
			}
#endif




		}


	}  // end of reconcileProperty


static int
regularizeMaterialSpec (MaterialSpec &matSpec)
	{

	if (simplifyProperties (matSpec.property))
		{
		shortUsage ();
		return -1;
		}

	if (matSpec.sourceFN[0])
		{
		int err;
		osg::Material *mat = getMaterialFromFile (matSpec.sourceFN, &err);

		if (err)
			{
			return -1;
			}

		if ( mat )
			{
			// if there was no Material in the specified file
	
			// Query out each of the data items in the Material 
			// and reconcile
			// with the corresponding fields in matSpec

			osg::Material::ColorMode cm = mat->getColorMode ();
			if (! matSpec.property[CM_PT].facePresent[BOTH_FACES] )
				{
				matSpec.property[CM_PT].
					facePresent[BOTH_FACES] = true;
				matSpec.property[CM_PT].rgba[0][0] = cm;
				}

			bool fb;
			osg::Vec4 fRgba, bRgba;
	
			fb = mat->getShininessFrontAndBack ();
			fRgba[0] = mat->getShininess (osg::Material::FRONT);
			bRgba[0] = mat->getShininess (osg::Material::BACK);
			reconcileProperty (matSpec.property[SH_PT], 
							fb, fRgba, bRgba);
		
			fb = mat->getAmbientFrontAndBack ();
			fRgba = mat->getAmbient (osg::Material::FRONT);
			bRgba = mat->getAmbient (osg::Material::BACK);
			reconcileProperty (matSpec.property[AMB_PT], 
							fb, fRgba, bRgba);
		
			fb = mat->getDiffuseFrontAndBack ();
			fRgba = mat->getDiffuse (osg::Material::FRONT);
			bRgba = mat->getDiffuse (osg::Material::BACK);
			reconcileProperty (matSpec.property[DIF_PT], 
							fb, fRgba, bRgba);
	
			fb = mat->getSpecularFrontAndBack ();
			fRgba = mat->getSpecular (osg::Material::FRONT);
			bRgba = mat->getSpecular (osg::Material::BACK);
			reconcileProperty (matSpec.property[SPEC_PT], 
							fb, fRgba, bRgba);
		
			fb = mat->getEmissionFrontAndBack ();
			fRgba = mat->getEmission (osg::Material::FRONT);
			bRgba = mat->getEmission (osg::Material::BACK);
			reconcileProperty (matSpec.property[EM_PT], 
							fb, fRgba, bRgba);
			}
		}

	if (simplifyProperties (matSpec.property))
		{
		return -1;
		}


	matSpec.materialsSpecified = (matSpec.sourceFN[0] != 0) || 
					( ! matSpec.merge ) ;

	for (int i = 0; i < NUM_PROPERTY_TYPES; i++)
		{
		if (matSpec.property[i].type != NO_PT)
			{
			matSpec.materialsSpecified |= 
				matSpec.property[i].facePresent[FRONT_FACE];
			matSpec.materialsSpecified |= 
				matSpec.property[i].facePresent[BACK_FACE];
			matSpec.materialsSpecified |= 
				matSpec.property[i].facePresent[BOTH_FACES];
			}
		}
			
	return 0;
	}  // end of regularizeMaterialSpec




static void
setDefaultMaterialProperties (MaterialPropertyType pt, MaterialProperty &prop)
	{

	prop.type = pt;

	for (int i = 0; i < 3; i++)
		{
		prop.facePresent[i] = false;
		prop.rgba[i][0] = 0;
		prop.rgba[i][1] = 0;
		prop.rgba[i][2] = 0;
		prop.rgba[i][3] = 0;
		// prop.shininess[i] = 0.0;
		}

	if (pt == CM_PT)
		{
		prop.rgba[0][0] = osg::Material::OFF;
		}


	}  // end of setDefaultMaterialProperties



static void
setDefaults
   (
   vector<string> &inFN,
   PreprocessInputSpec &preproc,
   MaterialSpec &matSpec,
   bool &singleOutput,
   char *outFN
   ) 
	{
	inFN.clear ();

	preproc.removeMat = false;
	preproc.removeGeomCol = false;


	matSpec.merge = true;
	matSpec.rootOnly = false;
	matSpec.materialsSpecified = false;
	matSpec.sourceFN[0] = 0;

	for (int i = 0; i < NUM_PROPERTY_TYPES; i++)
		{
		MaterialPropertyType pt = (MaterialPropertyType) i;
		setDefaultMaterialProperties (pt, matSpec.property[i]);
		}

	matSpec.property[CM_PT].rgba[0][0] = osg::Material::OFF;

	singleOutput = true;
	outFN[0] = 0;
	}  // end of setDefaults

static int processArgs
   (
   int argc, 
   char **argv, 
   vector<string> &inFN,
   PreprocessInputSpec &preproc,
   MaterialSpec &matSpec,
   bool &singleOutput,
   char *outFN
   ) 
	{
	bool noInput = false;

	setDefaults (inFN, preproc, matSpec, singleOutput, outFN);
		// JGH (should do inFN->erase () in setDefaults)

  
	if (argc == 1)
		{
		usage ();
		return 1;
		}


	Verbose = 0;

	// process remaining arguments
	int iarg = 1 ;
  
	// this loop is only for the "--" options
    	while (iarg < argc) 
		{
    
		int len = strlen(argv[iarg]);


		if ( strncmp ("--", argv[iarg], 2) )
			{
			// if the next arg does not start with --, then we're
			// then we're done with the options
			break;
			}

		else if ( ! strncasecmp ("--verbose", argv[iarg], len) )
			{
			Verbose = 1;
			printf ("\n");
			printf ("\n");
			printf ("### BEGIN hev-materialTransform ###\n");
			printf ("\n");
			iarg++;
			}

		else if ( ! strncasecmp ("--help", argv[iarg], len) )
			{
			usage ();
			return 1;
			}

		else if ( ! strncasecmp ("--noInput", argv[iarg], len) )
			{
			noInput = true;
			iarg++;
			}

		else if ( (len > 4) &&
                          ( ! strncasecmp ("--rmMaterials", argv[iarg], len) ) )
			{
			preproc.removeMat = 1;
			iarg++;
			}

		else if ( (len > 4) &&
                          ( ! strncasecmp ("--rmGeomColors", argv[iarg], len) ))
			{
			preproc.removeGeomCol = 1;
			iarg++;
			}

		else if ( (len > 3) &&
                          ( ! strncasecmp ("--sourceMaterial", argv[iarg],len)))
			{
			iarg++;
			strcpy (matSpec.sourceFN, argv[iarg]);
			iarg++;
			}

		else if ( ! strncasecmp ("--ambient", argv[iarg], len) )
			{
			if (getMaterialProperty (iarg, AMB_PT, argc, argv, 
						matSpec.property[AMB_PT]))
				{
				shortUsage ();
				return -1;
				}
			}

		else if ( ! strncasecmp ("--diffuse", argv[iarg], len) )
			{
			if (getMaterialProperty (iarg, DIF_PT, argc, argv, 
						matSpec.property[DIF_PT]))
				{
				shortUsage ();
				return -1;
				}
			}

		else if ( (len > 3) &&
                          ( ! strncasecmp ("--specular", argv[iarg], len) ) )
			{
			if (getMaterialProperty (iarg, SPEC_PT, argc, argv, 
						matSpec.property[SPEC_PT]))
				{
				shortUsage ();
				return -1;
				}
			}


		else if ( ! strncasecmp ("--emission", argv[iarg], len) )
			{
			if (getMaterialProperty (iarg, EM_PT, argc, argv, 
						matSpec.property[EM_PT]))
				{
				shortUsage ();
				return -1;
				}
			}



		else if ( (len > 3) &&
                          ( ! strncasecmp ("--shininess", argv[iarg], len) ) )
			{
			if (getMaterialProperty (iarg, SH_PT, argc, argv, 
						matSpec.property[SH_PT]))
				{
				shortUsage ();
				return -1;
				}
			}



		else if ( (len > 4) &&
                          ( ! strncasecmp ("--colorMode", argv[iarg], len) ) )
			{
			if (getMaterialProperty (iarg, CM_PT, argc, argv, 
						matSpec.property[CM_PT]))
				{
				shortUsage ();
				return -1;
				}
			}


		else if ( (len > 4) &&
                          ( ! strncasecmp ("--override", argv[iarg], len)))
			{
			matSpec.merge = 0;
			iarg++;
			}

		else if ( (len > 4) &&
                          ( ! strncasecmp ("--rootOnly", argv[iarg], len) ) )
			{
			matSpec.rootOnly = 1;
			iarg++;
			}
		else 
			{
    			dtkMsg.add(DTKMSG_ERROR, "hev-materialTransform: "
				"Unrecognized command line option: \"%s\"\n",
				argv[iarg]);
			shortUsage ();
			return -1;
			}

		}  // end of loop over "--" options

    	if (argc < 3) 
		{
		dtkMsg.add(DTKMSG_ERROR, 
			"hev-materialTransform: Bad argument count.\n");
		shortUsage () ;
		return -1 ;
    		}


	if (iarg == argc)
		{
		dtkMsg.add(DTKMSG_ERROR, 
		"hev-materialTransform: No input or output files specified.\n");
		shortUsage () ;
		return -1 ;
		}



	// first we're going to get the output FN from the end of argv
	strcpy (outFN, argv[argc-1]);

	singleOutput = strcmp (outFN, "osg") && strcmp (outFN, "ive");


#if 0
	if (simplifyProperties (matSpec.property))
		{
		shortUsage ();
		return -1;
		}
#endif

	// We've picked off all of the "--" options from the beginning
	// of argv, and we've picked off the output file name from the
	// end of argv, so the rest of the args are input file names
	// iarg is the first of the args that don't begin with "--"
	for (  ; iarg < argc-1; iarg++ )
		{
		inFN.push_back ( string(argv[iarg]) );
		}


	if (inFN.size() == 0)
		{

		// if no inputs, then it just produces an output file
		// with the minimal structure to hold a Material

		// There is an error if there are no input
		// files and the output is for one output per input file
		// (i.e. if singleOutput is false)

		if ( ! noInput )
			{
    			dtkMsg.add(DTKMSG_ERROR, "hev-materialTransform: "
				"No input files have been specified, but"
				" the \"--noInput\" option is not present.\n");
			shortUsage ();
			return -1;
			}

		if ( ! singleOutput )
			{
    			dtkMsg.add(DTKMSG_ERROR, "hev-materialTransform: "
				"Must specify explicit output file name "
				"when there are input files specified.\n");
			shortUsage ();
			return -1;
			}
		}
	else
		{
		if ( noInput )
		    {
		    if (inFN.size() == 1)
			{
    			dtkMsg.add(DTKMSG_ERROR, "hev-materialTransform: "
				"An input file has been specified, but"
				" the \"--noInput\" option is present.\n");
			shortUsage ();
			return -1;
			}
		    else
			{
    			dtkMsg.add(DTKMSG_ERROR, "hev-materialTransform: "
				"Input files have been specified, but"
				" the \"--noInput\" option is present.\n");
			shortUsage ();
			return -1;
			}
		    }
		}



	if (Verbose)
		{
		// dump out results of parsing arguments
		printf ("\n");
		printf ("Results of command line argument parsing:\n");
		printf ("   preproc spec:\n");
		printf ("       remove materials: %s\n", 
				preproc.removeMat ? "true":"false");
		printf ("       remove geometry color: %s\n", 
				preproc.removeGeomCol ? "true":"false");

		
		printf ("   material spec: \n");
		printf ("       merge: %s\n", matSpec.merge ? "true":"false");
		printf ("       root node only: %s\n",
				    matSpec.rootOnly ? "true":"false");
		printf ("       source file: \"%s\"\n", 
		    matSpec.sourceFN[0] ? matSpec.sourceFN : "none specified");

		for (int i = 0; i < NUM_PROPERTY_TYPES; i++)
			{
			if (matSpec.property[i].type != NO_PT)
				{
				printMaterialProperty (matSpec.property[i]);
				}
			}

		printf ("   # input files: %d\n", inFN.size());
		for (int i = 0; i < inFN.size(); i++)
			{
			printf ("       file %d  \"%s\"\n", i, inFN[i].c_str());
			}

		printf ("   single output: %s\n", 
				singleOutput ? "true":"false");
		printf ("   output file spec: \"%s\"\n", outFN);
		printf ("End of results of command line argument parsing.\n");
		printf ("\n\n");
		}

	return 0;
	}  // end of processArgs


////////////////////////////////////////////////////////////////////////
static void
exitProgram (int e)
	{

	if (Verbose)
		{
		printf ("\n");
		printf ("### END hev-materialTransform ###\n");
		printf ("\n\n");
		}

	exit (e);
	}  // end of exitProgram

int main(int argc, char **argv) 
	{
  

	vector<string> inFN;
	PreprocessInputSpec preproc;
	MaterialSpec matSpec;
	bool singleOutput;
	char outFN[1000];

	if (processArgs (
   			argc, 
   			argv, 
   			inFN,
   			preproc,
   			matSpec,
   			singleOutput,
   			outFN
   			)  )
		{
		exitProgram (-1);
		}


	if (regularizeMaterialSpec (matSpec))
		{
		exitProgram (-1);
		}

	int rtn;

	if (singleOutput)
		{
		rtn = allTogetherNow (inFN, preproc, matSpec, outFN);
		}
	else
		{
		rtn = inchByInch (inFN, preproc, matSpec, outFN);
		}


	exitProgram (rtn);
	}  // end of main
		





