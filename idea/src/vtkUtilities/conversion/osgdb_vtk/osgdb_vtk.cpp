//
// This is an OSG file loader for VTK polygonal data files.
//
// This works with the VTK "simple legacy" file formats. as described
// here:
//       http://www.vtk.org/VTK/img/file-formats.pdf
// I believe that these formats are also described in the VTK User's Guide.
//
// The code gets a little messy for several reasons:
//
//    - I've tried (within reason) to preserve the structure
//           of the VTK data as I create the OSG structures.
//    - VTK colors are represented in several ways.
//    - VTK normals, colors, and texture coordinates can
//           be specified at each vertex or at each cell, or both.
//    - I've tried to group all drawable points together, all
//           line segments together, and all triangles together
//           in the OSG structures even when they do not appear
//           together in the VTK data structures. I did this because
//           I believe it will result in faster rendering under OSG.
//           This grouping complicates the handling of per-cell
//           normals and colors.
//
// Should probably use osg::ref_ptr more consistently.  Had some problems
// with this at first, so I minimized its use. Wouldn't be hard to
// redo, but things seem to be working fine so I'm not going to change it.
//
// Similarly, I could probably use vtkSmartPointer more than I do.
//
// Also, the code would probably be a bit less messy if it took a more
// object oriented approach.  Perhaps I will redo the code to be more OO
// at some point in the future.
//

#include <vector> 

#include <osg/Notify>
#include <osg/Node>
#include <osg/Geode>
#include <osg/Array>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>



#include <vtkSmartPointer.h>
#include <vtkErrorCode.h>
#include <vtkDataSet.h>
#include <vtkPolyData.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkGeometryFilter.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkCellArray.h>
#include <vtkLookupTable.h>
#include <vtkPolyDataNormals.h>



#define MIN(x,y)  (((x)<(y))?(x):(y))
#define MAX(x,y)  (((x)>(y))?(x):(y))






//////////////////////////////////////////////////////////////////////

// As far as I can tell, this class definition and the REGISTER_OSGPLUGIN
// represent a minimal template for implementing an OSG file loader.
//
// className is probably not needed.

class ReaderWriterVTK : public osgDB::ReaderWriter
{

public:

    ReaderWriterVTK ();

    virtual const char * className();

    virtual ReadResult readNode ( const std::string & fileName, 
                                  const osgDB::ReaderWriter::Options * ) const;

};  // end of class ReaderWriterVTK

REGISTER_OSGPLUGIN (vtk, ReaderWriterVTK)


//////////////////////////////////////////////////////////////////////

// Here we implement the methods specified above.

// The two easy methods:

ReaderWriterVTK::ReaderWriterVTK ()
    {
    // OK, this use of supportsExtension is all very well, but it does
    // not mean that the resulting osgdb_*.so will be loaded for the
    // files with the indicated extensions.
    //
    // The only way to get it to load osgdb_ext1.so when you're loading
    // a file with extension ext2 is if there has been a call to
    // osgDB::Registry::addFileExtensionAlias which gives ext2 as an 
    // alias for ext1.
    // 
    // Without a call to addFileExtensionAlias, when you try to load
    // a file with extension ext2, it will look for osgdb_ext2.so.
    // So if you want the same loader to handle another extension, 
    // you can simply copy or link osgdb_ext1.so to osgdb_ext2.so.
    //
    // See ReaderWriterQT.cpp and its calls to addFileExtensionAlias;
    // should try this.  JGH

    supportsExtension ("vtk",   "VTK polygon file format");
    // supportsExtension ("vtkp",  "VTK polygon file format");
    // supportsExtension ("vtp",   "VTK polygon file format");


    }  // end of constructor


const char* ReaderWriterVTK::className() 
    {
    return "ReaderWriterVTK"; 
    // Could return a more descriptive string like 
    //                    "VTK polygon file format reader".
    }  // end of className


////////////////////////////////////////////////////////////////////////////////

/*

Here are some notes about some of the less obvious properties of the
VTK "polygonal" data structures.

Color, normal, and texture coords can be specified per vertex or per cell.
When specified both places, per vertex overrides per cell.

Texture coords can be 1, 2, or 3 D.

Color (per vertex or per cell) can be specified:
  - LUT + scalar
        If the "scalar" data field has more than one component, the first 
        component is used for lookup.
  - Unsigned character array
        - 1 component: intensity
        - 2 component: intensity, alpha
        - 3 component: rgb
        - 4 component: rgba

If both LUT and uchar array specified, then LUT overrides uchar.

It seems implicit in the VTK documentation that point data structures are 
all 3D.





Ambiguous situations:

It's not clear what it means to have texture coords per cell.  Apparently 
VTK allows per-cell texture coords, but I believe that OSG does not.
So if they are present, they are ignored. 

If no normals are present, it appears to me that when VTK renders it
calculates polygon face normals;  not sure what happens for vertices 
and lines.  We will add normals for polygon that have no normals?
However I think that this can't be done for the individual triangles
in triangle strips.
JGH: Should we add normals if there aren't any?

What about the material specs?  OK, I'm looking at vtkProperty.cxx
and it looks like these are the defaults:

        Colors: ambient, diff, spec all 1,1,1,1
        Coeff:  amb: 0, diff 1, spec 0
        Spec power 1 
        Gouraud shading
        no back or front face culling
        point size and line width: 1

JGH: Do we need to enforce these defaults?  Or should we use OSG defaults?

*/


////////////////////////////////////////////////////////////////////////////////

// All of the routines below are part of the implementation of readNode,
// which appears at the end.


/////////////////

// Here are some data structures and housekeeping items

// This is the threshold below which something is consider not opaque:
#define TRANSPARENCY_THRESHOLD (0.999)

enum AttribType { NORMAL, COLOR, TEX_COORD };

// 

// Texture coordinates for OSG
typedef struct T_Coords_Arrays
        {
        osg::FloatArray *arr1;
        osg::Vec2Array *arr2;
        osg::Vec3Array *arr3;
        osg::Vec4Array *arr4;
        } TCoordsOSG;


// The OSG DrawElements and associated normal, color, and texture coordinates
typedef struct
        {
        osg::PrimitiveSet::Mode mode; // like osg::PrimitiveSet::POINTS
        osg::DrawElementsUInt *drawEls;
        // osg::Vec3Array *normals;
        // osg::Vec4Array *rgba;
        std::vector <osg::Vec3> *normals;
        std::vector <osg::Vec4> *rgba;
#if 0
        TCoordsOSG tCoords; // unused
#endif
        } AuxElementsOSG;

typedef struct
        {
        bool doAttrib[3];
        AuxElementsOSG ptAuxElO;
        AuxElementsOSG lnAuxElO;
        AuxElementsOSG triAuxElO;
        } AuxElOSGByType;

// VTK color information
typedef struct
        {
        vtkDataArray *scalars;
        vtkLookupTable *lut;
        float *rgba;           // this is the one that's really used
        } ColorVTK;

// The VTK representation of normal, color, and texture coordinates
typedef struct
        {
        vtkDataArray *normals;
        vtkDataArray *tCoords;
        ColorVTK color;
        } AuxElementsVTK;

static void 
setAuxOSGNull (AuxElementsOSG & auxElO)
        {
        // init all fields to NULL
        auxElO.drawEls = NULL;
        auxElO.normals = NULL;
        auxElO.rgba = NULL;
#if 0
        auxElO.tCoords.arr1 = NULL;
        auxElO.tCoords.arr2 = NULL;
        auxElO.tCoords.arr3 = NULL;
        auxElO.tCoords.arr4 = NULL;
#endif
        }  // end of setAuxOSGNull

static void
setAuxVTKNull (AuxElementsVTK & auxElV)
        {
        // init all fields to NULL
        auxElV.normals = NULL;
        auxElV.tCoords = NULL;
        auxElV.color.scalars = NULL;
        auxElV.color.lut = NULL;
        auxElV.color.rgba = NULL;
        }  // end of setAuxVTKNull



static void
setupColorVTK (vtkDataArray *scalars, ColorVTK & colorV)
        {

        // Set up the ColorVTK structure based on the scalar field.
        // Note that ColorVTK fields lut and scalars are not used
        // outside of this function. 
        colorV.scalars = scalars;

        if (scalars == NULL)
                {
                // if there are no scalars, there is no color
                colorV.lut = NULL;
                colorV.rgba = NULL;
                return;
                }

        int nComp = colorV.scalars->GetNumberOfComponents();
        if (nComp < 1)
                {
                // no components?  may be an error but treat as
                // if there are no scalars
                return;
                }
        int numTuples = colorV.scalars->GetNumberOfTuples ();

        if (numTuples < 1)
                {
                // no actual data in scalars array?
                return;
                }

        // OK, we're going to allocate an array of rgba and
        // precalculate all of them.
        // delete handled in deallocAuxElV 
        colorV.rgba = new float [4*numTuples];

        colorV.lut = colorV.scalars->GetLookupTable ();
        bool delLUT = false;
        if (colorV.lut == NULL)
                {
                delLUT = true;
                colorV.lut = vtkLookupTable::New();
                colorV.lut->Build();
                // this gets deleted at the bottom of this function.
                }

        int type = colorV.scalars->GetDataType();
        if ( (type == VTK_UNSIGNED_CHAR) && (nComp <= 4) )
                {
                // According to p 287-288 (sec 8.10) of the VTK book, if the 
                // scalar is of this type, then it specifies color directly 
                // not through a LUT.

                // So all we have to do is loop through the data array and
                // convert each specified color to the float [0-1] rgba form.

                for (int i = 0; i < numTuples; i++)
                    {  
                    double *color = colorV.scalars->GetTuple (i);
                    float *rgba = colorV.rgba + 4*i;
                    switch (nComp)
                        {
                        case 1:
                                // *color is intensity
                                rgba[0] = rgba[1] = rgba[2] = (*color)/255.0;
                                rgba[3] = 1.0;
                                break;
                        case 2:
                                // color[0] is intensity, color[1] is opacity
                                rgba[0] = rgba[1] = rgba[2] = color[0]/255.0;
                                rgba[3] = color[1]/255.0;
                                break;
                        case 3:
                                // color[0:2] is rgb
                                rgba[0] = color[0]/255.0;
                                rgba[1] = color[1]/255.0;
                                rgba[2] = color[2]/255.0;
                                rgba[3] = 1.0;
                                break;
                        case 4:
                                // color[0:3] is rgba
                                rgba[0] = color[0]/255.0;
                                rgba[1] = color[1]/255.0;
                                rgba[2] = color[2]/255.0;
                                rgba[3] = color[3]/255.0;
                                break;
                        }  // end of switch
                    }  // end of loop over numTumples

                }  // end of section for direct (non-LUT) color
        else if (colorV.lut != NULL)
                {
                // color isn't specified directly and we have a LUT

                // We're going to run the entire scalar array through the 
                // LUT in one call.  So allocate the unsigned char array
                // to receive the LUT output.

                unsigned char * rgbaUchar = new unsigned char [4*numTuples];

                // run the scalars through the LUT
                colorV.lut->MapScalarsThroughTable (colorV.scalars, rgbaUchar);
        
                // convert to float rgba form
                for (int i = 0; i < 4*numTuples; i++)
                        {
                        colorV.rgba[i] = rgbaUchar[i] / 255.0;
                        }

                delete [] rgbaUchar;
                }


        if ( (colorV.lut != NULL) && delLUT )
                {
                colorV.lut->Delete();
                colorV.lut = NULL;
                }

        return;
        }  // setupColorVTK



#if 1
static void
setupTCoordsO ( vtkDataArray *tCoordsV, TCoordsOSG & tCoordsO )
        {
        // set up the OSG texture coords struct based on the VTK texture 
        // coord struct


        tCoordsO.arr1 = NULL;
        tCoordsO.arr2 = NULL;
        tCoordsO.arr3 = NULL;
        tCoordsO.arr4 = NULL;

        if (tCoordsV != NULL)
                {
                // If there is a VTK tex coord array, create OSG tex coord array
        
                int nCoords = tCoordsV->GetNumberOfComponents();

                if (nCoords < 1) 
                        {
                        OSG_WARN << 
                          "VTK file loader: "
                          "Bad number of components for texture coords: " << 
                          nCoords << "." << std::endl;
                        tCoordsV = NULL;
                        }
                else
                        {
                        // Note that the array allocated by new here will get
                        // hooked into the OSG data structures in 
                        // addPtTexCoordsToGeom.
                        // So we don't have to delete it.
                        switch (nCoords)
                                {
                                case 1:
                                        tCoordsO.arr1 = new osg::FloatArray;
                                        break;

                                case 2:
                                        tCoordsO.arr2 = new osg::Vec2Array;
                                        break;

                                case 3:
                                        tCoordsO.arr3 = new osg::Vec3Array;
                                        break;

                                default:
                                        tCoordsO.arr4 = new osg::Vec4Array;
                                        if (nCoords > 4)
                                                {
                                                OSG_WARN << 
                                                  "VTK file loader: "
                                                  "Bad number of components "
                                                  "for texture coords: " << 
                                                  nCoords << "." << std::endl <<
                                                  "    Will use first "
                                                  "four components." << 
                                                  std::endl;
                                                }
                                        break;
                                }  // end of switch
                        }  // end of else
                }  // end of if tCoordsV != NULL

        }  // end of setupTCoordsO
#endif


static void
addPtColor (int i, ColorVTK &colorV, osg::Vec4Array *colorO)
        {
        // add the indicated VTK color to the end of the OSG color array.
        if ( (colorV.rgba != NULL) && (colorO != NULL) )
                {
                colorO->push_back  (
                                   osg::Vec4 
                                        (
                                        colorV.rgba[i*4 + 0],
                                        colorV.rgba[i*4 + 1],
                                        colorV.rgba[i*4 + 2],
                                        colorV.rgba[i*4 + 3]
                                        )
                                   );
                }

        return;
        } // end of addPtColor

static void
addPtNormal (int i, vtkDataArray *normalsV, osg::Vec3Array *normalsO)
        {
        // add the indicated VTK normal to the end of the OSG normal array
        if (normalsV != NULL)
                {
                // If we have normals, transfer normal vector from 
                // VTK to OSG array.
                double * norm = normalsV->GetTuple3 (i);
                normalsO->push_back (osg::Vec3 (norm[0], norm[1], norm[2]));
                }

        } // end of addPtNormal



static void
addPtTexCoord (int i, vtkDataArray *tCoordsV, TCoordsOSG & tCoordsO)
        {
        // add the indicated VTK texture coord to the end of the 
        // OSG texture coord array
        if (tCoordsV == NULL)
                {
                return;
                }

        double *coord = tCoordsV->GetTuple (i);

        if (tCoordsO.arr1 != NULL)
            {
            tCoordsO.arr1->push_back ((float)coord[0]);
            }
        else if (tCoordsO.arr2 != NULL)
            {
            tCoordsO.arr2->push_back (osg::Vec2 (coord[0], coord[1]));
            }
        else if (tCoordsO.arr3 != NULL)
            {
            tCoordsO.arr3->
                push_back (osg::Vec3 (coord[0], coord[1], coord[2]));
            }
        else if (tCoordsO.arr4 != NULL)
            {
            tCoordsO.arr4->
                push_back (osg::Vec4 (coord[0], coord[1], coord[2], coord[3]));
            }

        }  // end of addPtTexCoord


static void
getVDataTuple (vtkDataArray * vArray, int indx, int rNumComp, float result[])
        {
        // accesses the VTK data array, pulls out the indicated element, and
        // stores the result in a float array.  Unused slots in the result
        // array are set to zero.

        // first, set result all to zero
        for (int i = 0; i < rNumComp; i++)
                {
                result[i] = 0;
                }

        if (vArray != NULL)
            {
            int vNumComp = vArray->GetNumberOfComponents ();
            double * v = vArray->GetTuple (indx);

            for (int i = 0; i < MIN (vNumComp, rNumComp); i++)
                {
                result[i] = v[i];
                }

            }  // end of if (vArray != NULL)


        return;
        }  // end of getVDataTuple


static void
getVColor (ColorVTK & colorV, int iCell, float rgba[4])
        {
        // extract the indicated VTK color

        if (colorV.rgba == NULL)
                {
                // default color
                rgba[0] = rgba[1] = rgba[2] = 0.9;
                rgba[3] = 1.0;
                }
        else
                {
                rgba[0] = colorV.rgba[iCell*4 + 0];
                rgba[1] = colorV.rgba[iCell*4 + 1];
                rgba[2] = colorV.rgba[iCell*4 + 2];
                rgba[3] = colorV.rgba[iCell*4 + 3];
                }

        return;
        }  // end of getVColor


//////////////////////////////////////////////////////

// Here we have some functions that manipulate OSG data structures

static int
addToArray3 (std::vector<osg::Vec3> *arr, float *vec)
        {
        // add the 3-component vec to the array
        if  ( ((arr == NULL) && (vec != NULL)) || 
              ((arr != NULL) && (vec == NULL))    )
                {
                OSG_WARN << "VTK file loader: " <<
                  "Internal inconsitency in auxiliary array allocation (1)." << 
                  std::endl;
                return -1;
                }

        if (arr != NULL)
                {
                arr->push_back ( osg::Vec3 (vec[0], vec[1], vec[2]) );
                }

        return 0;
        }  // end of addToArray3

static int
addToArray4 (std::vector<osg::Vec4> *arr, float *vec)
        {
        // add the 4-component vec to the array
        if  ( ((arr == NULL) && (vec != NULL)) || 
              ((arr != NULL) && (vec == NULL))    )
                {
                OSG_WARN << "VTK file loader: " <<
                  "Internal inconsitency in auxiliary array allocation (2)." << 
                  std::endl;
                return -1;
                }

        if (arr != NULL)
                {
                arr->push_back ( osg::Vec4 (vec[0], vec[1], vec[2], vec[3]) );
                }

        return 0;
        }  // end of addToArray4



static void
addPtNormalsToGeom 
  (
  osg::Vec3Array *normalsO, 
#if NO_OSG_DEPRECATED
  osg::Geometry * geom, 
#else
  deprecated_osg::Geometry * geom, 
#endif
  bool & normalsDone
  )
        {
        // add the indicated vertex normals to the geom

        normalsDone = false;

        if (normalsO != NULL)
                {
                // if normals are present, add normal array to geom
                geom->setNormalArray (normalsO);
                geom->setNormalBinding(deprecated_osg::Geometry::BIND_PER_VERTEX);
#if NO_OSG_DEPRECATED
                geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
#else
                geom->setNormalBinding(deprecated_osg::Geometry::BIND_PER_VERTEX);
#endif
                normalsDone = true;
                }

        } // end of addPtNormalsToGeom

static void
addPtColorToGeom 
  (
  //osg::Vec4Array *colorO, 
  osg::Vec4Array *colorO, 
#if NO_OSG_DEPRECATED
  osg::Geometry * geom, 
#else
  deprecated_osg::Geometry * geom, 
#endif
  bool & colorDone
  )
        {
        // add the indicated vertex colors to the geom

        colorDone = false;

        if (colorO != NULL)
                {
                // if clor present, add array to geom
                geom->setColorArray (colorO);
#if NO_OSG_DEPRECATED
                geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
#else
                geom->setColorBinding(deprecated_osg::Geometry::BIND_PER_VERTEX);
#endif
                colorDone = true;
                }

        } // end of addPtColorToGeom


static void
addPtTexCoordsToGeom 
  (
  TCoordsOSG & tCoords,
#if NO_OSG_DEPRECATED
  osg::Geometry * geom, 
#else
  deprecated_osg::Geometry * geom, 
#endif
  bool & tCoordsDone
  )
        {
        // add the indicated vertex texture coords to the geom

        tCoordsDone = false;

        if (tCoords.arr1 != NULL)
                {
                geom->setTexCoordArray (0, tCoords.arr1);
                tCoordsDone = true;
                }
        else if (tCoords.arr2 != NULL)
                {
                geom->setTexCoordArray (0, tCoords.arr2);
                tCoordsDone = true;
                }
        else if (tCoords.arr3 != NULL)
                {
                geom->setTexCoordArray (0, tCoords.arr3);
                tCoordsDone = true;
                }
        else if (tCoords.arr4 != NULL)
                {
                geom->setTexCoordArray (0, tCoords.arr4);
                tCoordsDone = true;
                }

        } // end of addPtTexCoordsToGeom



/////////////////////////





/////////////////////////////////////////////////////////////////////////



// In this section of code, we will traverse and process the cells within
// a vtkPolyData object.

// The big issue here is how to deal with the auxiliary data: colors, 
// normals, and texture coords. These are each specified independently.
// Any of these data fields can be specified per-point and/or per-cell.
// In vtk, per-point data take precedence over per-cell data, so if it
// is speficied per-point then we can ignore corresponding per-cell data.
// 
// If a data field is specified per-cell and not per-point, then 
// then we can specify it in the OSG Geometry object with a binding of 
// BIND_OVERALL, BIND_PER_PRIMITIVE or BIND_PER_PRIMITIVE_SET depending
// on the data. We are going to use BIND_PER_PRIMITIVE as that seems
// (to me) to strike a balance between simplicity of the conversion and
// simplicity of the resulting OSG data structures.
//
// It should be noted however that it is pretty messy to handle all of
// the cases and alternatives with per-point / per-case and the three
// auxiliary data fields.  (And the varieties of color representations.)
// I think that I've covered all of the cases but it's hard to tell given 
// the VTK documentation.




// AuxElementsVTK AuxElV;

static void
deallocAuxElV ( AuxElementsVTK & auxElV )
    {

    if ( auxElV.color.rgba != NULL)
        {
        delete [] auxElV.color.rgba;
        }

    if (auxElV.color.lut != NULL)
        {
        auxElV.color.lut->Delete();
        }

    }  // end of deallocAuxElV

static void
createCellAuxElLists 
  (
  AuxElementsVTK & auxElV,
  AuxElOSGByType & aux, 
  bool attribDone[], 
  vtkCellData *cellData
  )
        {

        // this functions sets things up to handle the auxiliary data:
        // normals, texture coordinates, and colors.  It is intended to
        // be used for vtkCellData rather than vtkPointData.
        //
        // It has come to my attention that texture coordinates probably
        // are not used for cells.  But I'm leaving in some of the code
        // and data structures just in case they are needed in the future.
        //

        // This function sets up the global data in the following four 
        // structures
        setAuxOSGNull (aux.ptAuxElO);
        setAuxOSGNull (aux.lnAuxElO);
        setAuxOSGNull (aux.triAuxElO);
        aux.doAttrib[NORMAL] = 
        aux.doAttrib[TEX_COORD] = 
        aux.doAttrib[COLOR] = false;

        setAuxVTKNull (auxElV);

        if (cellData == NULL)
                {
                return;
                }

        // First set up for accessing the VTK data
        if ( ! attribDone[NORMAL] )
                {
                auxElV.normals = cellData->GetNormals ();
                if (auxElV.normals != NULL)
                        {
                        aux.doAttrib[NORMAL] = true;
                        }
                }

#if 0
        if ( ! attribDone[TEX_COORD] )
                {
                auxElV.tCoords = cellData->GetTCoords ();
                if (auxElV.tCoords != NULL)
                        {
                        aux.doAttrib[TEX_COORD] = true;
                        }
                }
#endif

        if ( ! attribDone[COLOR] )
                {
                setupColorVTK (cellData->GetScalars (), auxElV.color);
                if (auxElV.color.rgba != NULL)
                        {
                        aux.doAttrib[COLOR] = true;
                        }
                }

        // Now set up for the OSG structures for each of the cell types

        aux.ptAuxElO.mode  = osg::PrimitiveSet::POINTS;
        aux.lnAuxElO.mode  = osg::PrimitiveSet::LINES;
        aux.triAuxElO.mode = osg::PrimitiveSet::TRIANGLES;

        // the members drawEls, normals, and rgba are only created
        // when and if necessary




        return;
        }  // end of createCellAuxElLists






static int
#if NO_OSG_DEPRECATED
addDelAuxElCell (AuxElementsOSG & de, osg::Geometry *geom)
#else
addDelAuxElCell (AuxElementsOSG & de, deprecated_osg::Geometry *geom)
#endif
    {

    if (de.drawEls == NULL)
        {
        setAuxOSGNull (de);
        return 0;
        }


    if (de.drawEls->getNumPrimitives() != 0)
        {
        // if the draw element is non-empty add it to geom
        geom->addPrimitiveSet (de.drawEls);
        if (de.normals != NULL)
                {
                osg::Array * arr = geom->getNormalArray ();
                // If geom doesn't have a normal array, we create one
                osg::Vec3Array * normArr;
                if (arr == NULL)
                        {
                        normArr = new osg::Vec3Array ();
                        geom->setNormalArray (normArr);
                        }
                else if (arr->getType() != osg::Array::Vec3ArrayType)
                        {
                        OSG_WARN << 
                          "VTK file loader: "
                          "Internal Error: Inconsistent normal array types." << 
                          std::endl;
                        return -1;
                        }
                else
                        {
                        normArr = static_cast<osg::Vec3Array*> (arr);
                        }

                // loop over all normals and push_back onto normArr:
                // int n = de.normals->getNumElements ();
                int n = de.normals->size ();
                for (int i = 0; i < n; i++)
                        {
                        normArr->push_back ((*(de.normals))[i]);
                        }
#if NO_DEPRECATED_OSG
                geom->setNormalBinding (osg::Geometry::BIND_PER_PRIMITIVE_SET);
#else
                geom->setNormalBinding (deprecated_osg::Geometry::BIND_PER_PRIMITIVE);
#endif
                }  // end of section for normals


        if (de.rgba != NULL)
                {
                // similarly for color
                osg::Vec4Array * colorArr;
                osg::Array * arr = geom->getColorArray ();
                if (arr == NULL)
                        {
                        colorArr = new osg::Vec4Array ();
                        geom->setColorArray (colorArr);
                        }
                else if (arr->getType() != osg::Array::Vec4ArrayType)
                        {
                        OSG_WARN << 
                          "VTK file loader: "
                          "Internal Error: Inconsistent color array types." << 
                          std::endl;
                        return -1;
                        }
                else
                        {
                        colorArr = static_cast<osg::Vec4Array*> (arr);
                        }


                // loop over all colorsand push_back onto normArr:
                // int n = de.rgba->getNumElements ();
                int n = de.rgba->size ();
                for (int i = 0; i < n; i++)
                        {
                        colorArr->push_back ( (*(de.rgba)) [i] );
                        }
#if NO_DEPRECATED_OSG
                geom->setColorBinding (osg::Geometry::BIND_PER_PRIMITIVE_SET);
#else
                geom->setColorBinding (deprecated_osg::Geometry::BIND_PER_PRIMITIVE);
#endif
                }

        // at present, we don't do anything with texture coords for cells

        }
    else
        {
        // we shouldn't get here; if we do, we have to take a memory leak
        // because we can't do this : delete de.drawEls;
        de.drawEls = NULL;
        }

    if (de.rgba != NULL) delete de.rgba;
    if (de.normals != NULL) delete de.normals;


    setAuxOSGNull (de);

    return 0;
    }  // end of addDelAuxElCell

static void
addNewAuxEl
  (
  osg::PrimitiveSet::Mode mode, 
  vtkIdType nVerts, 
  vtkIdList *vertIndices, 
  float *rgba,
  float *normal,
  float *tCoord,
#if NO_OSG_DEPRECATED
  osg::Geometry * geom
#else
  deprecated_osg::Geometry * geom
#endif
  )
        {

        // Note that we are ignoring texture coords at cells

        // create a draw element of the indicated type and add the vertices
        osg::DrawElementsUInt *drawEl = new osg::DrawElementsUInt(mode, 0);
        for (int iVert = 0; iVert < nVerts; iVert++)
                {
                drawEl->push_back ( vertIndices->GetId (iVert) );
                }


        // add the draw el to geom
        geom->addPrimitiveSet (drawEl);

        if (normal != NULL)
                {
                osg::Vec3Array * normArr =
                   static_cast<osg::Vec3Array*> (geom->getNormalArray());
                // If geom doesn't have a normal array, we create one
                if (normArr == NULL)
                        {
                        normArr = new osg::Vec3Array ();
                        geom->setNormalArray (normArr);
                        }
                normArr->
                  push_back (osg::Vec3 (normal[0], normal[1], normal[2]));
#if NO_DEPRECATED_OSG
                geom->setNormalBinding (osg::Geometry::BIND_PER_PRIMITIVE_SET);
#else
                geom->setNormalBinding (deprecated_osg::Geometry::BIND_PER_PRIMITIVE);
#endif
                }


        if (rgba != NULL)
                {
                osg::Vec4Array * colorArr =
                   static_cast<osg::Vec4Array*> (geom->getColorArray());
                // If geom doesn't have a color array, we create one
                if (colorArr == NULL)
                        {
                        colorArr = new osg::Vec4Array ();
                        geom->setColorArray (colorArr);
                        }
                colorArr->
                  push_back (osg::Vec4 (rgba[0], rgba[1], rgba[2], rgba[3]));
#if NO_DEPRECATED_OSG
                geom->setColorBinding (osg::Geometry::BIND_PER_PRIMITIVE_SET);
#else
                geom->setColorBinding (deprecated_osg::Geometry::BIND_PER_PRIMITIVE);
#endif
                }

        }  // end of addNewAuxEl


static int addCellData 
  (
  AuxElementsOSG & auxEl, 
  int numVerts,
  vtkIdList *vertIndices, 
  float *rgba, 
  float *normal, 
  float *tcoord,
  bool repeat = false
  )
        {

        if (numVerts == 0)
                {
                return 0;
                }

        // for now, we're not doing anything with texture coordinates on cells


        // We test drawEls, normals and rgb for null and do a new if necessary
        // These newed objects are disposed of in addDelAuxElCell:
        //   if drawEls is newed, it is inserted into OSG data structures 
        //   if normals or rgba are newed, they are deleted
        if (numVerts > 0)
                {

                if (auxEl.drawEls == NULL)
                        {
                        auxEl.drawEls = new osg::DrawElementsUInt(auxEl.mode, 0);
                        }
                for (int i = 0; i < numVerts; i++)
                        {
                        auxEl.drawEls->push_back ( vertIndices->GetId (i) );
                        }

                if (normal != NULL)
                        {
                        if (auxEl.normals == NULL)
                                {
                                auxEl.normals  = new std::vector <osg::Vec3>;
                                }
                        if (repeat)
                                {
                                for (int i = 0; i < numVerts; i++)
                                        {
                                        auxEl.normals->
                                          push_back ( 
                                            osg::Vec3 (normal[0], normal[1], normal[2]) );
                                        }
                                }
                        else
                                {
                                auxEl.normals->
                                  push_back ( osg::Vec3 (normal[0], normal[1], normal[2]) );
                                }
                        }


                if (rgba != NULL)
                        {
                        if (auxEl.rgba == NULL)
                                {
                                auxEl.rgba  = new std::vector <osg::Vec4>;
                                }
                        if (repeat)
                                {
                                for (int i = 0; i < numVerts; i++)
                                        {
                                        auxEl.rgba->
                                          push_back (
                                            osg::Vec4 (rgba[0], rgba[1], rgba[2], rgba[3]));
                                        }
                                }
                        else
                                {
                                auxEl.rgba->
                                 push_back (osg::Vec4 (rgba[0], rgba[1], rgba[2], rgba[3]));
                                }
                        }

                }

        return 0;
        }  // end of addCellData

static void
printCellTypeName (int cellType)
        {
        printf ("cell type %d is ", cellType);
        switch (cellType)
            {
            case VTK_VERTEX:
                printf ("VTK_VERTEX");
                break;
            case VTK_POLY_VERTEX:
                printf ("VTK_POLY_VERTEX");
                break;
            case VTK_LINE:
                printf ("VTK_LINE");
                break;
            case VTK_POLY_LINE:
                printf ("VTK_POLY_LINE");
                break;
            case VTK_TRIANGLE:
                printf ("VTK_TRIANGLE");
                break;
            case VTK_QUAD:
                printf ("VTK_QUAD");
                break;
            case VTK_POLYGON:
                printf ("VTK_POLYGON");
                break;
            case VTK_TRIANGLE_STRIP:
                printf ("VTK_TRIANGLE_STRIP");
                break;
            default:
                printf ("unknown");
                break;
            }  // end of switch on cellType
        printf (".\n");
        return;
        }  // end of printCellTypeName

static int 
processCell 
  (
  bool attribDone[],
  AuxElOSGByType & aux, 
  int cellType, 
  vtkCell *cell, 
  float *normal, 
  float *tCoord,
  float *rgba,
#if NO_OSG_DEPRECATED
  osg::Geometry *polyGeom,
  osg::Geometry *ptLnGeom
#else
  deprecated_osg::Geometry *polyGeom,
  deprecated_osg::Geometry *ptLnGeom
#endif
  )
        {

        if (attribDone[NORMAL] || (!aux.doAttrib[NORMAL]))
                {
                normal = NULL;
                }

        if (attribDone[TEX_COORD] || (!aux.doAttrib[TEX_COORD]))
                {
                tCoord = NULL;
                }

        if (attribDone[COLOR] || (!aux.doAttrib[COLOR]))
                {
                rgba = NULL;
                }


        int nVerts = cell->GetNumberOfPoints ();
        vtkIdList * vertIndices = cell->GetPointIds ();

        // vtkPolyData doc says that the class only supports cell types
        //   vtkVertex, vtkPolyVertex, vtkLine, vtkPolyLine, vtkTriangle,
        //   vtkQuad, vtkPolygon, vtkTriangleStrip
        // for cell type enum, see vtkCellType.h

        // printCellTypeName (cellType);

        switch (cellType)
            {
            case VTK_VERTEX:
            case VTK_POLY_VERTEX:
                addCellData (aux.ptAuxElO, nVerts, vertIndices, 
                                                rgba, normal, tCoord, true);
                break;

            case VTK_LINE:
            case VTK_POLY_LINE:
                if (nVerts == 2)
                    {
                    // if only two verts, then put them on the oLines list
                    addCellData (aux.lnAuxElO, 2, vertIndices, 
                                                rgba, normal, tCoord);
                    }
                else
                    {
                    // if more than two verts, make line strip and add to geom
                    addNewAuxEl (osg::PrimitiveSet::LINE_STRIP, 
                          nVerts, vertIndices, rgba, normal, tCoord, ptLnGeom);
                    }
                break;

            case VTK_TRIANGLE:
            case VTK_QUAD:
            case VTK_POLYGON:
                if (nVerts == 3)
                    {
                    // if it's a triangle, put it on the triangle list
                    addCellData(aux.triAuxElO, 3, vertIndices, 
                                                rgba, normal, tCoord);
                    }
                else
                    {
                    // if it's not a triangle, create a polygon and add to geom
                    addNewAuxEl (osg::PrimitiveSet::POLYGON, 
                          nVerts, vertIndices, rgba, normal, tCoord, polyGeom);
                    }
                break;

            case VTK_TRIANGLE_STRIP:
                // each vtk tristrip is an osg tristrip and is added to geom
                addNewAuxEl (osg::PrimitiveSet::TRIANGLE_STRIP, 
                          nVerts, vertIndices, rgba, normal, tCoord, polyGeom);
                break;


            default:
                OSG_WARN << "VTK file loader: " <<
                  "Internal error: Bad cell type (" <<
                  cellType <<
                  ")." <<
                  std::endl;


            }  // end of switch on cellType


        return 0;
        }  // end of processCell

static void
addDelAuxElCellLists 
  (
  AuxElOSGByType & aux, 
#if NO_OSG_DEPRECATED
  osg::Geometry * polyGeom, 
  osg::Geometry * ptLnGeom
#else
  deprecated_osg::Geometry * polyGeom, 
  deprecated_osg::Geometry * ptLnGeom
#endif
  )
        {
        addDelAuxElCell (aux.ptAuxElO, ptLnGeom);
        addDelAuxElCell (aux.lnAuxElO, ptLnGeom);
        addDelAuxElCell (aux.triAuxElO, polyGeom);
        } // end of addDelAuxElCellLists

static void
getAuxData 
  (
  bool attribDone[3],
  AuxElementsVTK & auxElV,
  int iCell, 
  float normal[3], 
  float tcoord[4],
  float rgba[4],
  bool & allOpaque   // change only to falsify
  )
        {
        if ( ! attribDone[NORMAL] )
                {
                getVDataTuple (auxElV.normals, iCell, 3, normal);
                }

        // if ( ! attribDone[TEX_COORD] )
                // {
                // getVDataTuple (auxElV.tCoords, iCell, 4, tcoord);
                // }

        if ( ! attribDone [COLOR] )
                {
                getVColor (auxElV.color, iCell, rgba);
                if (rgba[3] < TRANSPARENCY_THRESHOLD)
                        {
                        allOpaque = false;
                        }
                }
        }  // getAuxData


static int
processVtkCells (
  vtkPolyData *pData, 
#if NO_OSG_DEPRECATED
  osg::Geometry *polyGeom, 
  osg::Geometry *ptLnGeom, 
#else
  deprecated_osg::Geometry *polyGeom, 
  deprecated_osg::Geometry *ptLnGeom, 
#endif
  int & numCells, 
  bool attribDone[],
  bool & allOpaque
  )
    {
    allOpaque = true;

    AuxElementsVTK auxElV;
    AuxElOSGByType auxLists;


    createCellAuxElLists (auxElV, auxLists, attribDone, pData->GetCellData());

    int nCells = pData->GetNumberOfCells ();
    numCells = nCells;


    for (int i = 0; i < nCells; i++)
        {
        int cellType = pData->GetCellType (i);
        vtkCell *cell = pData->GetCell (i);

        // get rgba, normal, tcoord
        float normal[3], tcoord[4], rgba[4];
        getAuxData (attribDone, auxElV, i, normal, tcoord, rgba, allOpaque);
        processCell (attribDone, auxLists, cellType, cell, 
                                    normal, tcoord, rgba, polyGeom, ptLnGeom);
        }

    // now add lists to geom
    addDelAuxElCellLists (auxLists, polyGeom, ptLnGeom);

    deallocAuxElV (auxElV);

    return 0;
    }  // end of processVtkCellsB


// end of section that traverses and processes the cells within 
// a vtkPolyData object.

///////////////////////////////////////////////////////////////////////


static void
testPtOpacity (float *rgba, int numPts, bool & allOpaque)
        {
        allOpaque = true;
        if (rgba != NULL)
            {
            for (int i = 0; i < numPts; i++)
                {
                if (rgba[i*4+3] < TRANSPARENCY_THRESHOLD)
                        {
                        allOpaque = false;
                        return;
                        }
                }
            }

        return;
        }  // end of testPtOpacity

static int
processVtkPoints 
  (
  vtkPolyData *pData, 
#if NO_OSG_DEPRECATED
  osg::Geometry * polyGeom, 
  osg::Geometry * ptLnGeom, 
#else
  deprecated_osg::Geometry * polyGeom, 
  deprecated_osg::Geometry * ptLnGeom, 
#endif
  int & numPts, 
  bool attribDone[10],
  bool & allOpaque
  )
    {
    // Extracts the basic vertex coordinate data from the vtkPolyData and
    // adds it to the Geometry.  Also handles the per-vertex auxiliary
    // data: normals, texture coords, color


    allOpaque = true;

    attribDone[NORMAL] = attribDone[COLOR] = attribDone[TEX_COORD] = false;

    // everything that is newly created in this function gets hooked into
    // the OSG data structure that we're building.  
    // See the routines add*ToGeom that are called below.

    // Create the OSG Array to receive the points.
    osg::Vec3Array *verticesO = new osg::Vec3Array;


    // Here's the VTK array of normals attached to each point.
    vtkDataArray *normalsV = pData->GetPointData()->GetNormals();
    osg::Vec3Array *normalsO = NULL;
    if (normalsV != NULL)
        {
        // If there is a VTK normal array, create an OSG normal array.
        normalsO = new osg::Vec3Array;
        }

    // Here's the VTK array of texture coords attached to each point.
    vtkDataArray *tCoordsV = pData->GetPointData()->GetTCoords();
    // and corresponding OSG texture coords struct 
    TCoordsOSG tCoordsO;
    setupTCoordsO ( tCoordsV, tCoordsO );
        
    // Here's my structure that describes VTK colors
    ColorVTK colorV;
    setupColorVTK (pData->GetPointData()->GetScalars (), colorV);
    osg::Vec4Array *colorO = NULL;
    if (colorV.rgba != NULL)
        {
        // if there are vtk colors, create an OSG color array
        colorO = new osg::Vec4Array;
        }

    // We're done with setup; now traverse the list of points


    numPts = pData->GetNumberOfPoints ();

    // for each point, get VTK point XYZ coord and place on OSG array
    for (int i = 0; i < numPts; i++)
        {
        double * xyz = pData -> GetPoint (i);
        verticesO->push_back ( osg::Vec3 (xyz[0], xyz[1], xyz[2]) );
        addPtNormal (i, normalsV, normalsO);
        addPtTexCoord (i, tCoordsV, tCoordsO);
        addPtColor (i, colorV, colorO);
        }


    // add coordinate array to geoms
    polyGeom->setVertexArray (verticesO);
    ptLnGeom->setVertexArray (verticesO);

    // add the newly created normals, tex coords, and colors to the
    // OSG data structures.

    addPtNormalsToGeom (normalsO, polyGeom, attribDone[NORMAL]);
    addPtTexCoordsToGeom (tCoordsO, polyGeom, attribDone[TEX_COORD]);
    addPtColorToGeom (colorO, polyGeom, attribDone[COLOR]);

    addPtNormalsToGeom (normalsO, ptLnGeom, attribDone[NORMAL]);
    addPtTexCoordsToGeom (tCoordsO, ptLnGeom, attribDone[TEX_COORD]);
    addPtColorToGeom (colorO, ptLnGeom, attribDone[COLOR]);

    testPtOpacity (colorV.rgba, numPts, allOpaque);

    if (colorV.rgba != NULL)
        {
        delete [] colorV.rgba;
        colorV.rgba = NULL;
        }

    return 0;

    }  // end of processVtkPoints

static void
#if NO_OSG_DEPRECATED
addColorIfNone (osg::ref_ptr<osg::Geometry> & geom)
#else
addColorIfNone (osg::ref_ptr<deprecated_osg::Geometry> & geom)
#endif
        {

        // If no color is specified in this geometry, make everything white.

        if (geom->getColorArray() == NULL)
                {
                osg::Vec4Array *white = new osg::Vec4Array;
                white->push_back (osg::Vec4(1,1,1,1));
                geom->setColorArray (white);
#if NO_OSG_DEPRECATED
                geom->setColorBinding (osg::Geometry::BIND_OVERALL);
#else
                geom->setColorBinding (deprecated_osg::Geometry::BIND_OVERALL);
#endif
                }

        }  // end of addColorIfNone

static osg::Node * 
vtkPolyToOsgNode (vtkPolyData *pData)
{


    //
    //  JGH: 
    //      Recall that if we have non-opaque primitives, then we need
    //      to use the opaque and transparent Group and Geode scheme that is
    //      used in osgdb_savg.cpp.




    // First we create the Geometry to hold the relevant data
#if 0
    osg::Geometry * polyGeom = new osg::Geometry;
    osg::Geometry * ptLnGeom = new osg::Geometry;
#else
    // By using ref_ptr, these Geometries are destroyed when they go out 
    // of scope unless they've been hooked into the osg data structures.
#if NO_OSG_DEPRECATED
    osg::ref_ptr<osg::Geometry> polyGeom = new osg::Geometry;
    osg::ref_ptr<osg::Geometry> ptLnGeom = new osg::Geometry;
#else
    osg::ref_ptr<deprecated_osg::Geometry> polyGeom = new deprecated_osg::Geometry;
    osg::ref_ptr<deprecated_osg::Geometry> ptLnGeom = new deprecated_osg::Geometry;
#endif
#endif

    int numPts, numVCells, numLCells, numPCells, numSCells;
    bool attribDone[10];
    bool allPtsOpaque = true;
    bool allCellsOpaque = true;

    // Note: Should probably process cells first, then the vertex
    // colors and other attribs would more naturally override the
    // cell attribs.  Wouldn't have to keep track of "attribsDone".

    // First we handle the basic VTK point coordinate data that are
    // referenced by all of the primitive types.  
    // This is inserted into a Geometry structure. 
    if (processVtkPoints (pData, polyGeom, ptLnGeom, 
                                        numPts, attribDone, allPtsOpaque))
        {
        return NULL;
        }

    int numCells;

    // Now we handle all of the VTK cells.  The cells specify renderable 
    // geometry with references to the list of coordinates.
    if (processVtkCells (pData, polyGeom, ptLnGeom, 
                                numCells, attribDone, allCellsOpaque))
        {
        return NULL;
        }

    OSG_INFO << "VTK file loader: A total of " <<
        numPts << " points and " <<
        numCells << " cells were processed." << std::endl;

    // If no points were found then something might be wrong.
    if (numPts == 0)
        {
        if (numCells != 0)
                {
                // How can we have cells without any points?
                // Something is probably wrong here.
                OSG_WARN << "VTK file loader: " <<
                  "Warning: No coordinates were found, but " <<
                  numCells <<
                  "geometric primitives were processed." <<
                  std::endl;
                OSG_WARN << "VTK file loader: " <<
                  "Output OSG structures may be invalid." <<
                  std::endl;
                }
        else
                {
                // No points and no cells.  This may be OK, but should notify 
                // the user.
                OSG_WARN << "VTK file loader: " <<
                  "Warning: No coordinates or geometric primitives "
                  "were processed." <<
                  std::endl;
                }
        }
    else if (numCells == 0)
        {
        // We have points but no cells.  This may be OK, but should notify
        // the user.
        OSG_WARN << "VTK file loader: " <<
                "Warning: " <<
                numPts <<
                " coordinates were found, " 
                "but no geometric primitives were processed." <<
                std::endl;
        }



    // Now hook the geometry into a Geode and return the Geode.
    osg::Geode *geode = new osg::Geode;


    if (polyGeom->getNumPrimitiveSets() > 0)
        {
        addColorIfNone (polyGeom);
        geode->addDrawable (polyGeom);
        }


    if (ptLnGeom->getNumPrimitiveSets() > 0)
        {
        addColorIfNone (ptLnGeom);
        ptLnGeom->getOrCreateStateSet()->
           setMode (GL_LIGHTING, 
                    osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
        geode->addDrawable (ptLnGeom);
        }

    if ( ! (allPtsOpaque && allCellsOpaque) )
        {
        osg::StateSet * ss = geode->getOrCreateStateSet();
        ss->setRenderingHint( osg::StateSet::TRANSPARENT_BIN);
        ss->setMode (GL_BLEND, 
                     osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
        }

    return geode;

} // end of vtkPolyToOsgNode


static int
getPolyDataFromLegacyFile 
  (
  std::string & fileName,
  vtkSmartPointer<vtkGenericDataObjectReader> & pReader,
  vtkSmartPointer<vtkGeometryFilter> & geom ,
  vtkSmartPointer<vtkPolyDataNormals> & pdNorm ,
  vtkPolyData * & pData
  )
    {
    vtkPolyData * pDataTmp = NULL;
    pData = NULL;

    int errCode = pReader->GetErrorCode();

    switch (errCode)
        {
        case vtkErrorCode::NoError:

            // if the read was successful, we check to see if the file
            // produced poly data.  If so, then we're done.  If not, then
            // then we use a vtkGeometryFilter to convert to poly data.
            if ( pReader->IsFilePolyData () )
                {
                OSG_INFO << "VTK file loader: File " << 
                        fileName.data() << 
                        ": Successfully read VTK polygon data." << 
                        std::endl;
                // we're done.
                pData = pReader->GetPolyDataOutput();
                if (pData != NULL)
                        {
                        pData->Update();
                        }
                return 0;
                }
            else if (pReader->IsFileStructuredPoints())
                {
                OSG_ALWAYS << "VTK file loader: Warning for file " << 
                        fileName.data() << 
                        ": File contains VTK structured point data." << 
                        std::endl;
                }
            else if (pReader->IsFileStructuredGrid())
                {
                OSG_ALWAYS << "VTK file loader: Warning for file " << 
                        fileName.data() << 
                        ": File contains VTK structured grid data." << 
                        std::endl;
                }
            else if (pReader->IsFileUnstructuredGrid())
                {
                OSG_ALWAYS << "VTK file loader: Warning for file " << 
                        fileName.data() << 
                        ": File contains VTK unstructured grid data." << 
                        std::endl;
                }
            else if (pReader->IsFileRectilinearGrid())
                {
                OSG_ALWAYS << "VTK file loader: Warning for file " << 
                        fileName.data() << 
                        ": File contains VTK rectilinear grid data." << 
                        std::endl;
                }
            else
                {
                OSG_ALWAYS << "VTK file loader: Error reading file " << 
                        fileName.data() << 
                        ": File contains data of unrecognized type." << 
                        std::endl;
                }

            OSG_ALWAYS << 
             "VTK file loader: Will try to transform to geometric primitives." 
             << std::endl;

            // if we get here it's because we've successfully read the file, but
            // it produced data that was not poly data.  So we're going to use
            // a geometry filter to produce poly data.
            geom->SetInput (pReader->GetOutput());
            geom->Update();
            pDataTmp = geom->GetOutput();

            if ( (pDataTmp->GetCellData()->GetNormals() == NULL) &&
                 (pDataTmp->GetPointData()->GetNormals() == NULL)   )
                {
                pdNorm->SetFeatureAngle (0.0);
                pdNorm->SplittingOn();
                pdNorm->ConsistencyOn();
                pdNorm->ComputeCellNormalsOn();
                pdNorm->SetInput (pDataTmp);
                pdNorm->Update();
                pData = pdNorm->GetOutput();
                }
            else
                {
                pData = pDataTmp;
                }



            if (pData != NULL)
                {
                pData->Update ();
                }
            return 0;
            break;

        case vtkErrorCode::FileNotFoundError:
            OSG_WARN << "VTK file loader: Error reading file " << 
                        fileName.data() << 
                        ": File not found." <<
                        std::endl;
            return -1;
            break;


        case vtkErrorCode::CannotOpenFileError:
            OSG_WARN << "VTK file loader: Error reading file " << 
                        fileName.data() << 
                        ": Cannot open file." <<
                        std::endl;
            return -1;
            break;


        case vtkErrorCode::PrematureEndOfFileError:
            OSG_WARN << "VTK file loader: Error reading file " << 
                        fileName.data() << 
                        ": Premature end of file." <<
                        std::endl;
            return -1;
            break;


        case vtkErrorCode::UnrecognizedFileTypeError:
            OSG_WARN << "VTK file loader: Error reading file " << 
                        fileName.data() << 
                        ": Unrecognized file type." <<
                        std::endl;
            return -1;
            break;

        case vtkErrorCode::FileFormatError:
            OSG_WARN << "VTK file loader: Error reading file " << 
                        fileName.data() << 
                        ": Error in file format." <<
                        std::endl;
            return -1;
            break;

        case vtkErrorCode::UnknownError:
        default:
            OSG_WARN << "VTK file loader: Error reading file " << 
                        fileName.data() << 
                        ": Unknown file error encountered." <<
                        std::endl;
            return -1;
            break;

        }  // end of switch on errCode

    // can't get here.
    return -1;
    }  // end of getPolyDataFromLegacyFile

// Read VTK polygon file and convert to an OSG Node
osgDB::ReaderWriter::ReadResult
ReaderWriterVTK::readNode (const std::string& file,
                            const osgDB::ReaderWriter::Options* options) const
{


    // The use of this loader the first time is triggered when OSG is
    // asked to load a file with extension .vtk.  It checks all of the
    // loaders that are already loaded to see if they accept .vtk.
    // If they don't, then it looks for osgdb_vtk.so.  If it finds it
    // it loads it, and uses it to load the file.  
    //
    // But in subsequent file loads, OSG will check with this loader to
    // see if it handles the file.  This is done by calling one of the
    // read methods.  
    //
    // So we need a test here that checks whether this readNode handles
    // the file.

    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    // Find the file
    std::string fileName = osgDB::findDataFile( file, options );
    if (fileName.empty()) return 
                           osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND;

    // Notify
    OSG_INFO << "osgDB::ReaderWriterVTK::readNode() Reading file "
                             << fileName.data() << std::endl;

    osgDB::ReaderWriter::ReadResult result;




    // See http://www.vtk.org/Wiki/VTK/Tutorials/SmartPointers for 
    // a discussion of SmartPointers.  I could probably manage these
    // myself, but it's no big deal either way.

    vtkSmartPointer<vtkGenericDataObjectReader> pReader =
                        vtkSmartPointer<vtkGenericDataObjectReader>::New();
    pReader->SetFileName (fileName.c_str());
    pReader->Update();
    vtkSmartPointer<vtkGeometryFilter> geom =
                        vtkSmartPointer<vtkGeometryFilter>::New();
    vtkSmartPointer<vtkPolyDataNormals> pdNorm =
                        vtkSmartPointer<vtkPolyDataNormals>::New();

    // Note that I create the reader, the geom filter, and the 
    // polyDataNormals objects outside of the function that actually 
    // uses them.  I implemented it this way before I really understood
    // some of the memory management issues.  It's working now, so I'm
    // not inclined to change it even though it's a bit messy.


    vtkPolyData *pData = NULL;
    if (getPolyDataFromLegacyFile (fileName, pReader, geom, pdNorm, pData))
        {
        return osgDB::ReaderWriter::ReadResult::ERROR_IN_READING_FILE;
        }


    if ( pData == NULL )
        {
        OSG_WARN << "VTK file loader: " <<
            "File " <<
            fileName.data() << 
            " does not contain any data." <<
            std::endl;
        return osgDB::ReaderWriter::ReadResult::ERROR_IN_READING_FILE;
        }


    osg::Node * node = vtkPolyToOsgNode (pData);

    if (node == NULL)
        {
        OSG_WARN << "VTK file loader: " <<
            "Unable to convert polygonal data in file " <<
            fileName.c_str() <<
            " to osg::Node." <<
            std::endl;
        return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;
        }

    OSG_INFO << "VTK file loader: File " << 
        fileName.data() << " successfully converted to osg::Node." << std::endl;

    // For some reason we can return a node as a ReadResult.
    return node;

}  // end of ReaderWriterVTK::readNode (fn, options)



//////////////////////////////////////////////////////////////



