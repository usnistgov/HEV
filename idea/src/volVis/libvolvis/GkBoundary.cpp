#include <iostream>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dtk.h>
#include <iris.h>

#include <vtkSmartPointer.h>
#include <vtkColorTransferFunction.h>
// #include <vtkmetaio/metaImage.h>


#include "VolVis.h"
#include "GkBoundary.h"
#include "MetaImg.h"


#define COMBINE_TABLES 1


#define MAX(x,y)        (((x)>(y))?(x):(y))
#define MIN(x,y)        (((x)<(y))?(x):(y))

//////////////////////////////////////////////////////////////////


VolVis::GkBoundary::GkBoundary ()
    {
    reset ();
    }  // end of constructor


VolVis::GkBoundary::~GkBoundary ()
    {
    // do we need to clear the vectors?
    } // end of VolVis::GkBoundary::~GkBoundary ()


int VolVis::GkBoundary::reset ()
    {
    funcTypeName = "GkBoundary";
    funcName = "noName";
    numInputDataFilesUsed = 1;
    inputDataFNs[0] = "";


    // colorInterpType = C_INTERP_RGB;
    colorInterpType = C_INTERP_CIELAB;

#if 0
    lightingModel = LM_GRAD_OPAQUE;
#endif

    rgbInterpPts.clear ();

    hvolRangeV = "f:1.0";
    hvolRangeG = "p:0.005";
    hvolRangeH = "p:0.015";
    hvolDim[0] = 256;
    hvolDim[1] = 256;
    hvolDim[2] = 256;
    hvolKernels[0] = "tent";
    hvolKernels[1] = "cubicd:1,0";
    hvolKernels[2] = "cubicdd:1,0";

    infoProjectH = "mean";
    infoDim = 1;
    
    opacBdryFunc[0] = 1.0;
    opacBdryFunc[1] = 1.0;
    opacBdryFunc[2] = 0.0;
    opacBdryFunc[3] = 1.0;

    opacSigma = -1;

    opacGthreshIsRelative = true;
    opacGthresh = 0.04;
    opacRadius = 0.0;


    requiredExts.clear();
    requiredExts["volScalar"] =
                new TFReqExt ("volScalar", "float", "volScalar", "func");


    textures.clear();

    texturesPrepared = false;

#if COMBINE_TABLES
    rgbaLutName = funcName + "_GkB";
    rgbLutName = rgbaLutName;
    alphaLutName = rgbaLutName;
    gradMagVolName = funcName + "_GradMag";
#else
    rgbaLutName = funcName + "_GkB";
    rgbLutName = funcName + "_GkB_RGB";
    alphaLutName = funcName + "_GkB_ALPHA";
    gradMagVolName = funcName + "_GradMag";
#endif


    return 0;
    }  // end of reset


int VolVis::GkBoundary::prepForGLSL ()
    {
    

#if 0
    if ( lightingModel == LM_GRAD_OPAQUE )
        {
        requiredExts["volPos"] =
                new TFReqExt ("volPos", "vec3", "volPos", "func");
        requiredExts["volPosPrev"] =
                new TFReqExt ("volPosPrev", "vec3", "volPosPrev", "func");
        requiredExts["volGradCalc"] =
                new TFReqExt ("volGradCalc", "vec3", "volGradCalc", "func");
        }
#endif

    if ( infoDim == 2 )
        {
        requiredExts["volGradMag"] =
                new TFReqExt ("volGradMag", "float", "volGradMag", "func");
        requiredExts["volGradMag"]->infoStrs.push_back ("gkKernels");
        requiredExts["volGradMag"]->infoStrs.push_back (hvolKernels[0]);
        requiredExts["volGradMag"]->infoStrs.push_back (hvolKernels[1]);
        }

#if COMBINE_TABLES
        rgbaLutName = funcName + "_GkB";
        rgbLutName = rgbaLutName;
        alphaLutName = rgbaLutName;
        gradMagVolName = funcName + "_GradMag";
#else
        rgbaLutName = funcName + "_GkB";
        rgbLutName = funcName + "_GkB_RGB";
        alphaLutName = funcName + "_GkB_ALPHA";
        gradMagVolName = funcName + "_GradMag";
#endif


    return 0;
    }  // end of GkBoundary::prepForGLSL






// The following is duplicated from RgbaLut1D.  Should make it a separate utility
void VolVis::GkBoundary::interpRGB (ColorInterpType interpType, float p,
                                float lowerRgb[3],
                                float upperRgb[3],
                                float interpRgb[3])
        {

        if (interpType == C_INTERP_NEAREST)
                {
                if (p < 0.5)
                        {
                        memcpy (interpRgb, lowerRgb, 3*sizeof(float));
                        }
                else
                        {
                        memcpy (interpRgb, upperRgb, 3*sizeof(float));
                        }
                return;
                }
        else if (interpType == C_INTERP_RGB)
                {
                for (int i = 0; i < 3; i++)
                        {
                        interpRgb[i] = (1-p)*lowerRgb[i] + p*upperRgb[i];
                        }
                return;
                }



        // This is inefficient.  Is that a problem?

        vtkSmartPointer<vtkColorTransferFunction> vctf =
                vtkSmartPointer<vtkColorTransferFunction>::New();
        vctf->AddRGBPoint (0.0, lowerRgb[0], lowerRgb[1], lowerRgb[2]);
        vctf->AddRGBPoint (1.0, upperRgb[0], upperRgb[1], upperRgb[2]);

        switch (interpType)
                {
                case C_INTERP_HSV:
                        vctf->SetColorSpaceToHSV();
                        break;

                case C_INTERP_CIELAB:
                        vctf->SetColorSpaceToLab();
                        break;

                case C_INTERP_DIVERGING:
                        vctf->SetColorSpaceToDiverging();
                        break;

                default:
                        dtkMsg.add (DTKMSG_ERROR,
                        "GkBoundary: Invalid color interpolation type: %d;\n"
                        "       using CIE LAB interpolation.\n",
                        interpType);
                        vctf->SetColorSpaceToLab();

                }

        double dRgb[3];
        vctf->GetColor (p, dRgb);

        interpRgb[0] = dRgb[0];
        interpRgb[1] = dRgb[1];
        interpRgb[2] = dRgb[2];


        return;
        }  // end of interpRGB


int VolVis::GkBoundary::buildTableRGB (unsigned char *tableRGB, int tableLength)
        {
        bool rgbUnspecified = (rgbInterpPts.size() == 0);

        if (rgbUnspecified)
                {
                float rgb[3];
                rgb[0] = 0.2;
                rgb[1] = 0.8;
                rgb[2] = 1.0;
                setInterpPtRGB (0.0, rgb);
                rgb[0] = 1.0;
                rgb[1] = 0.5;
                rgb[2] = 0.2;
                setInterpPtRGB (1.0, rgb);
                }

        if (tableLength < 1)
                {
                dtkMsg.add (DTKMSG_ERROR,
                        "GkBoundary: Trying to build LUT but "
                        "RGB table length is zero.\n");
                return -1;
                }

        if (rgbInterpPts.size() == 1)
                {
                // If there's only one interpolation point, we don't need
                // to interpolate.
                // clear out any existing tables
                unsigned char urgb[3];
                urgb[0] = (unsigned char)
                                        (rgbInterpPts[0].rgb[0]*255.0  +  0.5);
                urgb[1] = (unsigned char)
                                        (rgbInterpPts[0].rgb[1]*255.0  +  0.5);
                urgb[2] = (unsigned char)
                                        (rgbInterpPts[0].rgb[2]*255.0  +  0.5);
                for (int i = 0; i < tableLength; i++)
                        {
                        (tableRGB+i*3)[0] = urgb[0];
                        (tableRGB+i*3)[1] = urgb[1];
                        (tableRGB+i*3)[2] = urgb[2];
                        }
                return 0;
                }


        // If we get here, it's because we need to really interpolate between
        // multiple points in rgbInterpPts.

        // the entries in rgbInterpPts are sorted by the val field
        int lastPt = rgbInterpPts.size() - 1;

        // first get the range of values:

        double rgbTableRange[2];
        rgbTableRange[0] = rgbInterpPts[0].val;
        rgbTableRange[1] = rgbInterpPts[lastPt].val;

        if  (rgbTableRange[1] == rgbTableRange[0])
                {
                dtkMsg.add (DTKMSG_ERROR,
                      "GkBoundary: Error: Range of values has zero length.\n");
                return -1;
                }

        double valLen = rgbTableRange[1] - rgbTableRange[0];
        int upperIndex = 1;     // in the next loop, the current table entry is
                                // between this interpPts entry and the one
                                // below.

        for (int i = 0; i < tableLength; i++)
                {
                // entry corresponds to data value iVal
                double iVal = ((double)i)/((double)(tableLength-1));
                iVal = iVal*valLen + rgbTableRange[0];

                // See if we're in the same interval
                while (iVal > rgbInterpPts[upperIndex].val)
                        {
                        if (upperIndex < lastPt)
                                {
                                upperIndex++;
                                }
                        else
                                {
                                break;
                                }
                        }

                // Figure out where in the interpPts interval we are
                float p = (iVal - rgbInterpPts[upperIndex-1].val) /
                   (rgbInterpPts[upperIndex].val - rgbInterpPts[upperIndex-1].val);

                // Do the interpolation
                float frgb[3];
                interpRGB (     colorInterpType, p,
                                rgbInterpPts[upperIndex-1].rgb,
                                rgbInterpPts[upperIndex  ].rgb,
                                frgb );

                (tableRGB+i*3)[0] = (unsigned char) (frgb[0]*255.0 + 0.5);
                (tableRGB+i*3)[1] = (unsigned char) (frgb[1]*255.0 + 0.5);
                (tableRGB+i*3)[2] = (unsigned char) (frgb[2]*255.0 + 0.5);
                }  // end of loop over table entries


        if (rgbUnspecified)
                {
                rgbInterpPts.clear ();
                }

        return 0;
        } // end of int GkBoundary::buildTableRGB (uchar *tableRGB, int tableLen)






int VolVis::GkBoundary::buildTableRGB (float *tableRGB, int tableLength)
        {
        bool rgbUnspecified = (rgbInterpPts.size() == 0);

        if (rgbUnspecified)
                {
                float rgb[3];
                rgb[0] = 0.2;
                rgb[1] = 0.8;
                rgb[2] = 1.0;
                setInterpPtRGB (0.0, rgb);
                rgb[0] = 1.0;
                rgb[1] = 0.5;
                rgb[2] = 0.2;
                setInterpPtRGB (1.0, rgb);
                }

        if (tableLength < 1)
                {
                dtkMsg.add (DTKMSG_ERROR,
                        "GkBoundary: Trying to build LUT but "
                        "RGB table length is zero.\n");
                return -1;
                }

        if (rgbInterpPts.size() == 1)
                {
                // If there's only one interpolation point, we don't need
                // to interpolate.
                for (int i = 0; i < tableLength; i++)
                        {
                        (tableRGB+i*3)[0] = rgbInterpPts[0].rgb[0];
                        (tableRGB+i*3)[1] = rgbInterpPts[0].rgb[1];
                        (tableRGB+i*3)[2] = rgbInterpPts[0].rgb[2];
                        }
                return 0;
                }


        // If we get here, it's because we need to really interpolate between
        // multiple points in rgbInterpPts.

        // the entries in rgbInterpPts are sorted by the val field
        int lastPt = rgbInterpPts.size() - 1;

        // first get the range of values:

        double rgbTableRange[2];
        rgbTableRange[0] = rgbInterpPts[0].val;
        rgbTableRange[1] = rgbInterpPts[lastPt].val;

        if  (rgbTableRange[1] == rgbTableRange[0])
                {
                dtkMsg.add (DTKMSG_ERROR,
                      "GkBoundary: Error: Range of values has zero length.\n");
                return -1;
                }

        double valLen = rgbTableRange[1] - rgbTableRange[0];
        int upperIndex = 1;     // in the next loop, the current table entry is
                                // between this interpPts entry and the one
                                // below.

        for (int i = 0; i < tableLength; i++)
                {
                // entry corresponds to data value iVal
                double iVal = ((double)i)/((double)(tableLength-1));
                iVal = iVal*valLen + rgbTableRange[0];

                // See if we're in the same interval
                while (iVal > rgbInterpPts[upperIndex].val)
                        {
                        if (upperIndex < lastPt)
                                {
                                upperIndex++;
                                }
                        else
                                {
                                break;
                                }
                        }

                // Figure out where in the interpPts interval we are
                float p = (iVal - rgbInterpPts[upperIndex-1].val) /
                   (rgbInterpPts[upperIndex].val - rgbInterpPts[upperIndex-1].val);

                // Do the interpolation
                float frgb[3];
                interpRGB (     colorInterpType, p,
                                rgbInterpPts[upperIndex-1].rgb,
                                rgbInterpPts[upperIndex  ].rgb,
                                frgb );

                (tableRGB+i*3)[0] = frgb[0];
                (tableRGB+i*3)[1] = frgb[1];
                (tableRGB+i*3)[2] = frgb[2];
                }  // end of loop over table entries


        if (rgbUnspecified)
                {
                rgbInterpPts.clear ();
                }

        return 0;
        } // end of int GkBoundary::buildTableRGB (float *tableRGB, int tableLen)







static const std::string dataTypeToUnuStr (VolVis::DataType dType)
    {

    switch (dType)
        {
        case VolVis::DT_UNSPECIFIED:
            return "unspecified";

        case VolVis::DT_CHAR:
            return "char";

        case VolVis::DT_UCHAR:
            return "uchar";

        case VolVis::DT_SHORT:
            return "short";

        case VolVis::DT_USHORT:
            return "ushort";

        case VolVis::DT_INT:
            return "int";

        case VolVis::DT_UINT:
            return "uint";

        case VolVis::DT_LONG:
            return "long";

        case VolVis::DT_ULONG:
            return "ulong";

        case VolVis::DT_FLOAT:
            return "float";

        case VolVis::DT_DOUBLE:
            return "double";

        }  // end of switch over data type


    return "unspecified";
    }  // end of






static void
getCalcOpacVals (std::string opacStdOut, 
                    std::string & gthreshCalc, std::string & sigmaCalc)
    {
    gthreshCalc = "";
    sigmaCalc = "";

    std::stringstream opacSS (opacStdOut);
    std::string line;
    while(std::getline (opacSS, line))
        {
        char sig[1000];
        char gth[1000];
        if (sscanf (line.c_str(), "gkms opac: calculated gthresh = %s", gth) == 1)
            {
            gthreshCalc = gth;
            }
        else if (sscanf(line.c_str(), "gkms opac: calculated sigma = %s", sig) == 1)
            {
            sigmaCalc = sig;
            }
        }

    }  // end of getCalcOpacVals



static void  removeFiles (std::vector<std::string> & toBeDeleted)
    {
    for (int i = 0; i < toBeDeleted.size(); i++)
        {
        // unlink (toBeDeleted[i].c_str());
        }
    toBeDeleted.clear();
    } // end of removeFiles


int VolVis::GkBoundary::prepTextures ()
    {


    std::vector<std::string> toBeDeleted;

    if (texturesPrepared)
        {
        return 0;
        }

    // Note that the RGB and the Alpha textures are always prepared separately.

    // need to know nDim, dims[nDim], nComp, dataType, dataPtr, 
    // range[nDim][2], for each
    // where range[][2] is min and max of input data for each dimension (to 
    // be used for normalization 

    // create a 1D RGB texture


    float * rgbPtr = new float [hvolDim[0]*3];
    if (rgbPtr == NULL)
        {
        dtkMsg.add (DTKMSG_ERROR,
                    "GkBoundary: Unable to allocate memory for RGB table.\n");
        return -1;
        }

    if (buildTableRGB (rgbPtr, hvolDim[0]))
        {
        return -1;
        }


    TFTexture * rgbTxtr = 
        new TFTexture (rgbLutName, 1, 3, hvolDim, DT_FLOAT, rgbPtr);
    if (rgbTxtr == NULL)
        {
        dtkMsg.add (DTKMSG_ERROR,
                    "GkBoundary: Unable to allocate memory for RGB texture.\n");
        }

#if ! COMBINE_TABLES
    textures.push_back (rgbTxtr);
#endif

    // Now make alpha texture

    if (inputDataFNs[0] == "")
        {
        dtkMsg.add (DTKMSG_ERROR,
          "GkBoundary: Input volume data file name has not been specified..\n");
        return -1;
        }

    //
    // Here is the scenario for generating the opacity LUT:
    //
    // convert input mha volume file to raw data file
    //
    // use unu to create a nrrd header for the raw data file (inVol.nhdr) 
    //
    // gkms hvol <options> -i inVol.nhdr  -i hvol.nrrd
    // 
    // one dimesional:
    //   gkms info <options> -i hvol.nrrd -one -o info.nrrd
    // two dimesional:
    //   gkms info <options> -i hvol.nrrd -o info.nrrd
    // 
    // gkms opac <options> -i info.nrrd -o opac.nrrd
    //
    // read the data normalization (axis mins, axis maxs) info from header of 
    //      opac.nrrd (or is it info.nrrd?)
    //
    // use unu to convert opac.nrrd to opac.raw
    // 
    // read the raw data from opac.raw into memory 
    //
    // Use these to construct the TFTexture
    //


    std::string inVolRawFN = 
                iris::GetTempFileName ("raw", "/var/tmp", "inVol");
    std::string inVolNrrdFN = 
                iris::GetTempFileName ("nhdr", "/var/tmp", "inVol");
    toBeDeleted.push_back (inVolRawFN);
    toBeDeleted.push_back (inVolNrrdFN);

    int nDim, nChan, mipLevels;
    int * dims;
    DataType dType;
    void *inDataPtr;
    // Read the input volume .mha into memory
    if (readMetaImage (inputDataFNs[0], nDim, dims, nChan, 
                                mipLevels, dType, inDataPtr))
        {
        dtkMsg.add (DTKMSG_ERROR, "GkBoundary: Error reading volume file %s\n",
            inputDataFNs[0].c_str() );
        return -1;
        }

    if ( (nDim != 3) || (nChan != 1) )
        {
        dtkMsg.add (DTKMSG_ERROR, 
            "GkBoundary: Volume file %s has bad number of "
            "dimesions or channels.\n", inputDataFNs[0].c_str() );
        return -1;
        }


    // write the input volume data to a raw file
    FILE *rawFP = fopen (inVolRawFN.c_str(), "w");
    if (rawFP == NULL)
        {
        dtkMsg.add (DTKMSG_ERROR, 
            "GkBoundary: Unable to open temporary output file %s.\n", 
            inVolRawFN.c_str());
        return -1;
        }

    if (fwrite (inDataPtr, dataTypeLength (dType), 
            dims[0]*dims[1]*dims[2], rawFP) != dims[0]*dims[1]*dims[2])
        {
        dtkMsg.add (DTKMSG_ERROR, 
            "GkBoundary: Error writing to temporary output file %s.\n",
            inVolRawFN.c_str());
        return -1;
        }

    fclose (rawFP);

    std::string cmd;
    std::string cmdStdOut;
    int exitStatus;
    std::string binDir = "/external/teem/teem-1.6.0-src/linux.64/bin/";
    char *hevroot = getenv("HEVROOT");
    if (hevroot == NULL)
        {
        dtkMsg.add (DTKMSG_ERROR, 
            "GkBoundary: Environment variable HEVROOT is not defined.\n");
        }
    binDir = hevroot + binDir;

    int rtn; 

    // Use unu to write a nrrd header for the raw file
    char dimsStr[1000];
    sprintf (dimsStr, " %d %d %d ", dims[0], dims[1], dims[2]);
    cmd = binDir + "unu make -h " + 
                " -i " + inVolRawFN + 
                " -t " + dataTypeToUnuStr (dType) + 
                " -sp 1 1 1 " +
                " -e raw " +
                " -s " + dimsStr +
                " -c " + inVolRawFN + 
                " -o " + inVolNrrdFN +
                " 2>&1 ";
    rtn = VolVis::executeCmd (cmd, cmdStdOut, exitStatus);
    if (rtn || exitStatus)
        {
        dtkMsg.add (DTKMSG_ERROR, 
            "GkBoundary: Error executing command:\n        %s\n", 
            cmd.c_str() );
        if (cmdStdOut != "")
            {
            fprintf (stderr, "\n");
            dtkMsg.add (DTKMSG_ERROR, 
              "GkBoundary: Output from failed command : \n\n%s\n\n********\n\n",
                cmdStdOut.c_str());
            }
        removeFiles (toBeDeleted);
        return -1;
        }

    sprintf (dimsStr, " %d %d %d ", hvolDim[0], hvolDim[1], hvolDim[2]);

    std::string options;
    // Use gkms to generate the histogram volume from the input volume
    std::string hvolFN = iris::GetTempFileName ("nrrd", "/var/tmp", "hvol");
    toBeDeleted.push_back (hvolFN);
    options = " -s " + hvolRangeV + " " + hvolRangeG + " " + hvolRangeH +
              " -d " + dimsStr +
              " -k00 " + hvolKernels[0] +
              " -k11 " + hvolKernels[1] +
              " -k22 " + hvolKernels[2] +
              " ";
    cmd = binDir + "gkms hvol " + options + 
                " -i " + inVolNrrdFN + " -o " + hvolFN +
                " 2>&1 ";
    rtn = VolVis::executeCmd (cmd, cmdStdOut, exitStatus);
    if (rtn || exitStatus)
        {
        dtkMsg.add (DTKMSG_ERROR, 
            "GkBoundary: Error executing command:\n        %s\n", 
            cmd.c_str() );
        if (cmdStdOut != "")
            {
            fprintf (stderr, "\n");
            dtkMsg.add (DTKMSG_ERROR, 
              "GkBoundary: Output from failed command : \n\n%s\n\n********\n\n",
                cmdStdOut.c_str());
            }
        removeFiles (toBeDeleted);
        return -1;
        }


    // Use gkms to generate the "info" data from the histogram volume
    std::string infoFN = iris::GetTempFileName ("nrrd", "/var/tmp", "info");
    toBeDeleted.push_back (infoFN);
    options =   " -m " + infoProjectH +
                ((infoDim == 1) ? " -one " : " ") +
                " ";
    if (infoDim == 1)
        {
        cmd = binDir + "gkms info " + options + 
                " -i " + hvolFN + " -one -o "+ infoFN;
        }
    else
        {
        cmd = binDir + "gkms info " + options + 
                " -i " + hvolFN + " -o "+ infoFN;
        }

    cmd += " 2>&1";
    rtn = VolVis::executeCmd (cmd, cmdStdOut, exitStatus);
    if (rtn || exitStatus)
        {
        dtkMsg.add (DTKMSG_ERROR, 
            "GkBoundary: Error executing command:\n        %s\n", 
            cmd.c_str() );
        if (cmdStdOut != "")
            {
            fprintf (stderr, "\n");
            dtkMsg.add (DTKMSG_ERROR, 
              "GkBoundary: Output from failed command : \n\n%s\n\n********\n\n",
                cmdStdOut.c_str());
            }
        removeFiles (toBeDeleted);
        return -1;
        }


    // Use gkms to generate the opacity LUT from the "info" data 
    std::string opacFN = iris::GetTempFileName ("nrrd", "/var/tmp", "opac");
    toBeDeleted.push_back (opacFN);
    char bdryStr[1000];
    sprintf (bdryStr, "%.9g,%.9g,%.9g,%.9g", 
        opacBdryFunc[0], opacBdryFunc[1], opacBdryFunc[2], opacBdryFunc[3]);
    char sigmaStr[1000];
    if (opacSigma <= 0)
        {
        sigmaStr[0] = 0;
        }
    else
        {
        sprintf (sigmaStr, " -s %.9g ", opacSigma);
        }

    char gthreshStr[1000];
    sprintf (gthreshStr, "%.9g", opacGthresh);
    char radiusStr[1000];
    sprintf (radiusStr, "%.9g", opacRadius);

    options =   std::string(" -b ") + 
                bdryStr +
                sigmaStr +
                " -g " + (opacGthreshIsRelative ? "x":"") + gthreshStr + " " +
                " -r " + radiusStr +
                " ";
    cmd = binDir + "gkms opac " + options + 
                " -i " + infoFN + " -o " + opacFN;
    cmd += " 2>&1";

    rtn = VolVis::executeCmd (cmd, cmdStdOut, exitStatus);
    if (rtn || exitStatus)
        {
        dtkMsg.add (DTKMSG_ERROR, 
            "GkBoundary: Error executing command:\n        %s\n", 
            cmd.c_str() );
        if (cmdStdOut != "")
            {
            fprintf (stderr, "\n");
            dtkMsg.add (DTKMSG_ERROR, 
              "GkBoundary: Output from failed command : \n\n%s\n\n********\n\n",
                cmdStdOut.c_str());
            }
        removeFiles (toBeDeleted);
        return -1;
        }

    std::string gthreshCalc;
    std::string sigmaCalc;
    getCalcOpacVals (cmdStdOut, gthreshCalc, sigmaCalc);

    // Now read the opacity file to get the axis maxs and mins
    // Along the way, we'll check for consistency in some of the other
    // fields of the header.
    float axMin[3];
    float axMax[3];
    FILE *opacFP = fopen (opacFN.c_str(), "r");
    if (opacFP == NULL)
        {
        dtkMsg.add ( DTKMSG_ERROR, 
            "GkBoundary: Error openning temporary opacity file %s.\n", 
            opacFN.c_str() );
        removeFiles (toBeDeleted);
        return -1;
        }


    char line[1000];
    char tok[1000];
    int d, d0, d1;

    // loop through the lines of the opac file's nrrd header
    while (fgets (line, sizeof(line), opacFP) != NULL)
        {

        if (line[0] == '\n')
            {
            break;
            }

        if (sscanf (line, "type: %s", tok) == 1)
            {
            if (strcasecmp (tok, "float") != 0)
                {
                dtkMsg.add (DTKMSG_ERROR, "GkBoundary: "
                    "Bad data type (%s) in temporary opacity file %s.\n",
                    tok, opacFN.c_str());
                fclose (opacFP);
                removeFiles (toBeDeleted);
                return -1;
                }
            }
        else if (sscanf (line, "dimension: %d", &d) == 1)
            {
            if (d != infoDim)
                {
                dtkMsg.add (DTKMSG_ERROR, "GkBoundary: "
                    "Bad dimension (%d) in temporary opacity file %s.\n",
                    d, opacFN.c_str());
                fclose (opacFP);
                removeFiles (toBeDeleted);
                return -1;
                }
            }
        else if (sscanf (line, "sizes: %d", &d0) == 1)
            {
            d1 = hvolDim[1];
            if (infoDim != 1)
                {
                if (sscanf (line, "sizes: %d %d", &d0, &d1) != 2)
                    {
                    dtkMsg.add (DTKMSG_ERROR, "GkBoundary: "
                        "Bad sizes in temporary opacity file %s.\n",
                        opacFN.c_str());
                    fclose (opacFP);
                    removeFiles (toBeDeleted);
                    return -1;
                    }
                }
            if ( (d0 != hvolDim[0]) || (d1 != hvolDim[1]) )
                {
                dtkMsg.add (DTKMSG_ERROR, "GkBoundary: "
                        "Bad sizes (%d, %d) in temporary opacity file %s.\n",
                        d0, d1, 
                        opacFN.c_str());
                fclose (opacFP);
                removeFiles (toBeDeleted);
                return -1;
                }
            }

        else if (sscanf (line, "axis mins: %f", axMin+0) == 1)
            {
            if (infoDim != 1)
                {
                if (sscanf (line, "axis mins: %f %f", axMin+0, axMin+1) != 2)
                    {
                    dtkMsg.add (DTKMSG_ERROR, "GkBoundary: "
                        "Bad axis mins in temporary opacity file %s.\n",
                        opacFN.c_str());
                    fclose (opacFP);
                    removeFiles (toBeDeleted);
                    return -1;
                    }
                }
            }
        else if (sscanf (line, "axis maxs: %f", axMax+0) == 1)
            {
            if (infoDim != 1)
                {
                if (sscanf (line, "axis maxs: %f %f", axMax+0, axMax+1) != 2)
                    {
                    dtkMsg.add (DTKMSG_ERROR, "GkBoundary: "
                        "Bad axis maxs in temporary opacity file %s.\n",
                        opacFN.c_str());
                    fclose (opacFP);
                    removeFiles (toBeDeleted);
                    return -1;
                    }
                }
            }

        }  // end of loop over lines in the opac nrrd header
    fclose (opacFP);



    // use unu to write out the opac data without the nrrd header

    std::string opacRawFN = iris::GetTempFileName ("raw", "/var/tmp", "opac");
    toBeDeleted.push_back (opacRawFN);
    cmd = binDir + "unu data " + opacFN + " > " + opacRawFN;
    cmd += " 2>&1";
    rtn = VolVis::executeCmd (cmd, cmdStdOut, exitStatus);
    if (rtn || exitStatus)
        {
        dtkMsg.add (DTKMSG_ERROR, 
            "GkBoundary: Error executing command:\n        %s\n", 
            cmd.c_str() );
        if (cmdStdOut != "")
            {
            fprintf (stderr, "\n");
            dtkMsg.add (DTKMSG_ERROR, 
              "GkBoundary: Output from failed command : \n\n%s\n\n********\n\n",
                cmdStdOut.c_str());
            }
        removeFiles (toBeDeleted);
        return -1;
        }



    // Now read opacRawFN into memory
    int nOpac = hvolDim[0];
    if (infoDim == 2)
        {
        nOpac *= hvolDim[1];
        }

    float *opacDataPtr = new float[nOpac];

    rawFP = fopen (opacRawFN.c_str(), "r");
    if (rawFP == NULL)
        {
        dtkMsg.add (DTKMSG_ERROR, 
           "GkBoundary: Unable to open temporary raw opac file %s.\n", 
            opacRawFN.c_str() );
        }

    if (fread (opacDataPtr, dataTypeLength (DT_FLOAT), nOpac, rawFP) != nOpac)
        {
        fclose (rawFP);
        dtkMsg.add (DTKMSG_ERROR, 
           "GkBoundary: Unable to read temporary raw opac file %s.\n", 
            opacRawFN.c_str() );
        removeFiles (toBeDeleted);
        return -1;
        }
    fclose (rawFP);



    // use opacDataPtr, axMin, axMax  in TFTexture constructor below
    

    TFTexture * alphaTxtr = 
        new TFTexture (alphaLutName, infoDim, 1, hvolDim, DT_FLOAT, opacDataPtr);

    double normRange[2];
    normRange[0] = axMin[0];
    normRange[1] = axMax[0];
    alphaTxtr->setNormRange (0, normRange);
    if (infoDim == 2)
        {
        normRange[0] = axMin[1];
        normRange[1] = axMax[1];
        alphaTxtr->setNormRange (1, normRange);
        }
#if ! COMBINE_TABLES
    textures.push_back (alphaTxtr);
#endif

#if 1

#if COMBINE_TABLES

    float *combinedTable = new float [nOpac * 4];



    int jmax = (infoDim==1)?1:hvolDim[1];
    for (int j = 0; j < jmax; j++)
        {
        for (int i = 0; i < hvolDim[0]; i++)
            {
            combinedTable[j*hvolDim[0]*4 + i*4 + 0] = rgbPtr[i*3 + 0];
            combinedTable[j*hvolDim[0]*4 + i*4 + 1] = rgbPtr[i*3 + 1];
            combinedTable[j*hvolDim[0]*4 + i*4 + 2] = rgbPtr[i*3 + 2];
            combinedTable[j*hvolDim[0]*4 + i*4 + 3] = 
                                            opacDataPtr[j*hvolDim[0] +i];
            }
        }
    TFTexture * combinedTxtr = 
        new TFTexture (rgbaLutName, infoDim, 4, hvolDim, DT_FLOAT, combinedTable);

    normRange[0] = axMin[0];
    normRange[1] = axMax[0];
    combinedTxtr->setNormRange (0, normRange);
    if (infoDim == 2)
        {
        normRange[0] = axMin[1];
        normRange[1] = axMax[1];
        combinedTxtr->setNormRange (1, normRange);
        }
    textures.push_back (combinedTxtr);

#endif


#endif

    if (gthreshCalc != "")
        {
        rgbTxtr->addInfo ("calculated gthresh = " + gthreshCalc);
        alphaTxtr->addInfo ("calculated gthresh = " + gthreshCalc);
#if COMBINE_TABLES
        combinedTxtr->addInfo ("calculated gthresh = " + gthreshCalc);
#endif
        }

    if (sigmaCalc != "")
        {
        rgbTxtr->addInfo ("calculated sigma = " + sigmaCalc);
        alphaTxtr->addInfo ("calculated sigma = " + sigmaCalc);
#if COMBINE_TABLES
        combinedTxtr->addInfo ("calculated sigma = " + sigmaCalc);
#endif
        }



#if 0

    if (infoDim == 2)
        {
        // then create the gradient magnitude texture

        // Here's the outline:
        //   vprobe -k scalar -q gm -i original.nhdr -o gradMag.nrrd
        //   unu data gradMag.nrrd > gradMag.bin.raw
        //   read the raw data into memory 


        std::string gmNrrdFN = iris::GetTempFileName ("nrrd", "/var/tmp", "gm");
        toBeDeleted.push_back (gmNrrdFN);
        cmd = binDir + "vprobe -k scalar -q gm -i " + inVolNrrdFN + " -o " + gmNrrdFN;
        cmd += " 2>&1";
        rtn = VolVis::executeCmd (cmd, cmdStdOut, exitStatus);
        if (rtn || exitStatus)
            {
            dtkMsg.add (DTKMSG_ERROR, 
                "GkBoundary: Error executing command:\n        %s\n", cmd.c_str() );
        if (cmdStdOut != "")
            {
            fprintf (stderr, "\n");
            dtkMsg.add (DTKMSG_ERROR, 
              "GkBoundary: Output from failed command : \n\n%s\n\n********\n\n",
                cmdStdOut.c_str());
            }
            removeFiles (toBeDeleted);
            return -1;
            }

        std::string gmRawFN = iris::GetTempFileName ("raw", "/var/tmp", "gm");
        toBeDeleted.push_back (gmNrrdFN);
        cmd = binDir + "unu data " + gmNrrdFN + " > " + gmRawFN;
        cmd += " 2>&1";
        rtn = VolVis::executeCmd (cmd, cmdStdOut, exitStatus);
        if (rtn || exitStatus)
            {
            dtkMsg.add (DTKMSG_ERROR, 
                "GkBoundary: Error executing command:\n        %s\n", cmd.c_str() );
            if (cmdStdOut != "")
                {
                fprintf (stderr, "\n");
                dtkMsg.add (DTKMSG_ERROR, 
              "GkBoundary: Output from failed command : \n\n%s\n\n********\n\n",
                    cmdStdOut.c_str());
                }
            removeFiles (toBeDeleted);
            return -1;
            }

        float * gmDataPtr = new float [dims[0]*dims[1]*dims[2]];

        // write the input volume data to a raw file
        rawFP = fopen (gmRawFN.c_str(), "r");
        if (rawFP == NULL)
            {
            dtkMsg.add (DTKMSG_ERROR, 
                "GkBoundary: Unable to open temporary output file %s.\n", 
                gmRawFN.c_str());
            return -1;
            }

        if (fread (gmDataPtr, sizeof(float),
                dims[0]*dims[1]*dims[2], rawFP) != dims[0]*dims[1]*dims[2])
            {
            dtkMsg.add (DTKMSG_ERROR, 
                "GkBoundary: Error reading to temporary output file %s.\n",
                gmRawFN.c_str());
            return -1;
            }

        fclose (rawFP);


        TFTexture * gmTxtr = 
            new TFTexture (gradMagVolName, 3, 1, dims, DT_FLOAT, gmDataPtr);
        textures.push_back (gmTxtr);

        }
#endif

    texturesPrepared = true;
    // if combine tables
    //      textures[0] is combined rgb & alpha
    //      textures[1] if present is gradmag (infoDim == 2)
    // else
    //      textures[0] is rgb
    //      textures[1] is alpha
    //      textures[2] if present is gradmag (infoDim == 2)

    removeFiles (toBeDeleted);
    return 0;

    }  // end of int VolVis::GkBoundary::prepTextures ()


int VolVis::GkBoundary::write (std::string const & fileName)
        {

        resetIndent ();
        int rtn = TransferFunction::write (fileName);
        resetIndent ();
        return rtn;
        }  // end of VolVis::GkBoundary::write (char *filename)




int VolVis::GkBoundary::write (FILE *fp)
        {

        printIndent (fp);
        fprintf (fp, "FUNC GkBoundary %s\n", funcName.c_str());

        incrementIndent ();


        // first do the gkms stuff

        printIndent (fp);
        fprintf (fp, "HVOL_V_RANGE %s\n", hvolRangeV.c_str());
        printIndent (fp);
        fprintf (fp, "HVOL_G_RANGE %s\n", hvolRangeG.c_str());
        printIndent (fp);
        fprintf (fp, "HVOL_H_RANGE %s\n", hvolRangeH.c_str());

        printIndent (fp);
        fprintf (fp, "HVOL_DIM %d %d %d\n", hvolDim[0], hvolDim[1], hvolDim[2]);

        printIndent (fp);
        fprintf (fp, "HVOL_K0 %s\n", hvolKernels[0].c_str());

        printIndent (fp);
        fprintf (fp, "HVOL_K1 %s\n", hvolKernels[1].c_str());

        printIndent (fp);
        fprintf (fp, "HVOL_K2 %s\n", hvolKernels[2].c_str());

        printIndent (fp);
        fprintf (fp, "INFO_PROJECT_H %s\n", infoProjectH.c_str());

        printIndent (fp);
        fprintf (fp, "INFO_DIM %d\n", infoDim);

        printIndent (fp);
        fprintf (fp, "OPAC_BDRY_FUNC %.9g %.9g %.9g %.9g\n", 
          opacBdryFunc[0], opacBdryFunc[1], opacBdryFunc[2], opacBdryFunc[3] );

        
        printIndent (fp);
        fprintf (fp, "OPAC_SIGMA ");
        if (opacSigma < 0)
            {
            fprintf (fp, "AUTO\n");
            }
        else
            {
            fprintf (fp, "%.9g\n", opacSigma);
            }
        

        printIndent (fp);
        fprintf (fp, "OPAC_GTHRESH ");
        if (opacGthreshIsRelative)
            {
            fprintf (fp, "x");
            }
        fprintf (fp, "%.9g\n", opacGthresh);
            

        printIndent (fp);
        fprintf (fp, "OPAC_RADIUS %.9g\n", opacRadius);




        // Now we do the RGB stuff
        printIndent (fp);
        fprintf (fp, "COLOR_INTERP %s\n", VolVis::toStr(colorInterpType).c_str());

#if 0
        printIndent (fp);
        fprintf (fp, "LIGHTING %s\n", toStr (lightingModel).c_str());
#endif

        printIndent (fp);
        fprintf (fp, "RGBLUT\n");

        incrementIndent ();

        for (int i = 0; i < rgbInterpPts.size(); i++)
                {
                printIndent (fp);
                fprintf ( fp, "%g   %g %g %g\n",
                                rgbInterpPts[i].val,
                                rgbInterpPts[i].rgb[0],
                                rgbInterpPts[i].rgb[1],
                                rgbInterpPts[i].rgb[2] );
                }

        decrementIndent ();

        printIndent (fp);
        fprintf (fp, "END_RGBLUT\n");
        decrementIndent ();


        printIndent (fp);
        fprintf (fp, "END_FUNC %s\n", funcName.c_str());

        return 0;
        }  // end of int VolVis::GkBoundary::write (FILE *FP)

////////////////////////////////////////////////////////////////////



int
VolVis::GkBoundary::parseFuncDecl (
  std::vector<std::string> tokVec,
  std::string & funcName )
        {

        if (parseFuncDeclaration (tokVec, funcTypeName, funcName) )
                {
                return -1;
                }

        if (funcTypeName != "GkBoundary")
                {
                dtkMsg.add (DTKMSG_ERROR,
                        "Error in parsing transfer function description: "
                        "expected function type GkBoundary but found type %s.\n",
                        funcTypeName.c_str());
                return -1;
                }


        return 0;
        }  // end of parseLut1DFuncDecl
 



int VolVis::GkBoundary::setInterpPtRGB (double v, const float rgb[3])
        {
        // insert (v,rgb) into rgbInterpPts such that it's sorted by val


        VAL_RGB vcol;
        vcol.val = v;
        memcpy (vcol.rgb, rgb, 3*sizeof(float));

        for ( std::vector<VAL_RGB>::iterator ivc = rgbInterpPts.begin();
              ivc != rgbInterpPts.end();
              ivc++ )
                {
                if (v == (*ivc).val)
                        {
                        // just replace the color
                        memcpy ((*ivc).rgb, rgb, 3*sizeof(float));
                        return 0;
                        }
                else if (v < (*ivc).val)
                        {
                        // we've found the insertion point
                        rgbInterpPts.insert (ivc, vcol);
                        return 0;
                        }
                }  // end of loop over entries in rgbInterpPts

        // If we reach here, we didn't find a place to insert it, so
        // put it at the end;
        rgbInterpPts.push_back (vcol);
        return 0;

        }  // end of int GkBoundary::setInterpPtRGB (double , const float [3])



int
VolVis::GkBoundary::parseRGBLUT (FILE *fp)
    {
    char line[1000];
    std::vector<std::string> tokVec;

    // read v-rgb lines until we get to END_RGBLUT
    while (getNextLineTokens (fp, tokVec, line, sizeof(line)) == 0)
        {
        if (tokVec.size() == 4)
                {
                double value;
                float rgb[3];
                if ( ! iris::StringToDouble (tokVec[0], &value))
                        {
                        dtkMsg.add (DTKMSG_ERROR,
                                "Error parsing RGBA LUT entry: "
                                "Expected double, found %s\n"
                                "    on line: %s\n",
                                tokVec[0].c_str(), line);
                        return -1;
                        }
                for (int i = 0; i < 3; i++)
                        {
                        if ( ! iris::StringToFloat (tokVec[1+i], rgb+i))
                                {
                                dtkMsg.add (DTKMSG_ERROR,
                                        "Error parsing RGBA LUT entry: "
                                        "Expected double, found %s\n"
                                        "    on line: %s\n",
                                        tokVec[1+i].c_str(), line);
                                return -1;
                                }
                        }

                if (setInterpPtRGB (value, rgb))
                        {
                        return -1;
                        }

                }
        else if (tokVec.size() == 1)
                {
                if (iris::IsSubstring ("END_RGBLUT", tokVec[0]))
                        {
                        return 0;
                        }
                else
                        {
                        dtkMsg.add (DTKMSG_ERROR,
                                        "Error parsing RGB LUT: "
                                        "Expected END_RGBLUT, found %s\n"
                                        "    on line: %s\n",
                                        tokVec[0].c_str(), line);
                        }
                }
        else
                {
                dtkMsg.add (DTKMSG_ERROR,
                            "Error parsing RGB LUT: "
                            "Bad number of tokens: %d\n",
                            "    on line: %s.\n",
                            tokVec.size(), line);
                return -1;
                }

        }  // end of while

    return 0;
    }  // end of VolVis::GkBoundary::parseRGBLUT (FILE *fp)



int VolVis::GkBoundary::parseFuncBody ( FILE * fp )
    {

    char line[1000];
    std::vector<std::string> tokVec;


    hvolRangeV = "f:1.0";
    hvolRangeG = "p:0.005";
    hvolRangeH = "p:0.015";
    hvolDim[0] = 256;
    hvolDim[1] = 256;
    hvolDim[2] = 256;
    hvolKernels[0] = "tent";
    hvolKernels[1] = "cubicd:1,0";
    hvolKernels[2] = "cubicdd:1,0";

    infoProjectH = "mean";
    infoDim = 1;
    
    opacBdryFunc[0] = 1.0;
    opacBdryFunc[1] = 1.0;
    opacBdryFunc[2] = 0.0;
    opacBdryFunc[3] = 1.0;

    opacSigma = -1;

    opacGthreshIsRelative = true;
    opacGthresh = 0.04;
    opacRadius = 0.0;






    while (getNextLineTokens (fp, tokVec, line, sizeof(line)) == 0)
        {
        int nTok = tokVec.size();

        if (iris::IsSubstring ("INPUT_VOLUME", tokVec[0]))
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }
            inputDataFNs[0] = tokVec[1];
            }

        else if (iris::IsSubstring ("HVOL_V_RANGE", tokVec[0]))
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }
            hvolRangeV = tokVec[1];
            }

        else if (iris::IsSubstring ("HVOL_G_RANGE", tokVec[0]))
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }
            hvolRangeG = tokVec[1];
            }

        else if (iris::IsSubstring ("HVOL_H_RANGE", tokVec[0]))
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }
            hvolRangeH = tokVec[1];
            }

        else if (iris::IsSubstring ("HVOL_DIM", tokVec[0]))
            {
            if (nTok != 4)
                {
                badArgCountMsg (line);
                return -1;
                }
            for (int i = 0; i < 3; i++)
                {
                if ( ! iris::StringToInt (tokVec[i+1], hvolDim+i) )
                    {
                    dtkMsg.add (DTKMSG_ERROR, "Error parsing HVOL_DIM entry: "
                        "Expected integer, found %s\n"
                        "     on line: %s\n", tokVec[i+1].c_str(), line);
                    return -1;
                    }
                }
                
            }

        else if (iris::IsSubstring ("HVOL_K0", tokVec[0]))
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }
            hvolKernels[0] = tokVec[1];
            }

        else if (iris::IsSubstring ("HVOL_K1", tokVec[0]))
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }
            hvolKernels[1] = tokVec[1];
            }

        else if (iris::IsSubstring ("HVOL_K2", tokVec[0]))
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }
            hvolKernels[2] = tokVec[1];
            }

        else if (iris::IsSubstring ("INFO_PROJECT_H", tokVec[0]))
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }
            infoProjectH = tokVec[1];
            }

        else if (iris::IsSubstring ("INFO_DIM", tokVec[0]))
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }
            if ( ! iris::StringToInt (tokVec[1], &infoDim) )
                {
                dtkMsg.add (DTKMSG_ERROR, "Error parsing INFO_DIM entry: "
                        "Expected integer, found %s\n"
                        "     on line: %s\n", tokVec[1].c_str(), line);
                    return -1;
                }
            }

        else if (iris::IsSubstring ("OPAC_BDRY_FUNC", tokVec[0]))
            {
            if (nTok != 5)
                {
                badArgCountMsg (line);
                return -1;
                }
            for (int i = 0; i < 4; i++)
                {
                if ( ! iris::StringToFloat (tokVec[i+1], opacBdryFunc+i) )
                    {
                    dtkMsg.add (DTKMSG_ERROR, "Error parsing OPAC_BDRY_FUNC entry: "
                        "Expected float, found %s\n"
                        "     on line: %s\n", tokVec[i+1].c_str(), line);
                    return -1;
                    }
                }
                
            }

        else if (iris::IsSubstring ("OPAC_SIGMA", tokVec[0]))
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }

            if (! iris::StringToFloat (tokVec[1], &opacSigma) )
                {
                dtkMsg.add (DTKMSG_ERROR, "Error parsing OPAC_SIGMA entry: "
                        "Expected float, found %s\n"
                        "     on line: %s\n", tokVec[1].c_str(), line);
                return -1;
                }
            opacSigmaIsAutomatic = (opacSigma <= 0);
            }

        else if (iris::IsSubstring ("OPAC_GTHRESH", tokVec[0]))
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }

            if (! iris::StringToFloat (tokVec[1], &opacGthresh) )
                {
                dtkMsg.add (DTKMSG_ERROR, "Error parsing OPAC_GTHRESH entry: "
                        "Expected float, found %s\n"
                        "     on line: %s\n", tokVec[1].c_str(), line);
                return -1;
                }

            if ( opacGthreshIsRelative = (opacGthresh < 0) )
                {
                opacGthresh = - opacGthresh;
                }
                
            }


        else if (iris::IsSubstring ("COLOR_INTERP", tokVec[0]))
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }

            if ( ! strToColorInterp (tokVec[1], colorInterpType) )
                {
                return -1;
                }
            }

#if 0
        else if (iris::IsSubstring ("LIGHTING", tokVec[0]))
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }

            if ( ! strToLightingModel (tokVec[1], lightingModel) )
                {
                return -1;
                }
            }
#endif

        else if (iris::IsSubstring ("RGBLUT", tokVec[0]))
            {
            if (parseRGBLUT (fp))
                {
                return -1;
                }
            }
        else if (iris::IsSubstring ("END_FUNC", tokVec[0]))
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }

            if (funcName != tokVec[1])
                {
                dtkMsg.add (DTKMSG_ERROR,
                    "Error parsing GkBoundary: "
                    "Function name at beginning (%s) does not "
                    "match function name at end (%s).\n",
                    funcName.c_str(), tokVec[1].c_str() );
                }
            return 0;
            }
        else
            {
            dtkMsg.add (DTKMSG_ERROR, 
                "Error parsing GkBoundary: Unrecognized keyword: %s.\n", 
                tokVec[0].c_str());
            return -1;
            }

        }  // end of while

    // if we get here, we got to EOF without seeing END_FUNC
    return -1;
    }  // end of parseFuncBody


int VolVis::GkBoundary::read (char *fileName)
        {
        return TransferFunction::read (fileName);
        }  // end of VolVis::GkBoundary::read (FILE * fp)



int VolVis::GkBoundary::read (FILE * fp, std::string declarationLine)
        {
        // assume that we are positioned at the beginning of the GkBoundary

        char line[1000];
        std::vector<std::string> tokVec;


        // If declarationLine has any tokens, we assume that it is the
        // declaration of a GkBoundary function.
        // So we copy the declaration line and parse into tokens
        strncpy (line, declarationLine.c_str(), sizeof(line)-1);
        line[sizeof(line)-1] = 0;
        tokVec = VolVis::getTokens (line);


        if (tokVec.size() <= 0)
                {
                // if there are no tokens on the supplied declarationLine,
                // then read the file until we get some active tokens
                if ( getNextLineTokens (fp, tokVec, line, sizeof(line) ) )
                        {
                        dtkMsg.add (DTKMSG_ERROR,
                                "Error parsing GkBoundary: "
                                "No transfer function specification found.\n");
                        return -1;
                        }
                }

        if ( parseFuncDecl (tokVec, funcName) )
                {
                return -1;
                }


        if (parseFuncBody (fp))
                {
                return -2;
                }

#if COMBINE_TABLES
        rgbaLutName = funcName + "_GkB";
        rgbLutName = rgbaLutName;
        alphaLutName = rgbaLutName;
        gradMagVolName = funcName + "_GradMag";
#else
        rgbaLutName = funcName + "_GkB";
        rgbLutName = funcName + "_GkB_RGB";
        alphaLutName = funcName + "_GkB_ALPHA";
        gradMagVolName = funcName + "_GradMag";
#endif

        return 0;
        }  // end of VolVis::GkBoundary::read (FILE * fp)


/////////////////////////////////////////////////////////////////////////////////


int VolVis::GkBoundary::writeGLSL (FILE *fp)
    {

        prepForGLSL ();


        fprintf (fp, "\n");
        fprintf (fp, "//////////////////\n");
        fprintf (fp, "// Code for GkBoundary transfer function %s.\n",
                        funcName.c_str());
        fprintf (fp, "//\n");
        fprintf (fp, "\n");



        if (requiredExts.size() > 0)
            {
            fprintf (fp, "// Required Externals:\n");
            std::map<std::string, TFReqExt *>::iterator re;
            for (re = requiredExts.begin(); re != requiredExts.end(); re++)
                {
                fprintf (fp, "//    %-12s %10s:   %-8s %s\n",
                    re->second->stdName.c_str(), re->second->objType.c_str(),
                    re->second->dataType.c_str(), re->second->glslName.c_str());
                }
            fprintf (fp, "\n");
            }



        if (infoDim == 1)
            {
            // code for 1D alpha lookup

            // create uniforms for rgba LUT and normalization
#if COMBINE_TABLES
            fprintf (fp, "uniform sampler1D %s;\n", rgbaLutName.c_str());
#else
            fprintf (fp, "uniform sampler1D %s;\n", rgbLutName.c_str());
            fprintf (fp, "uniform sampler1D %s;\n", alphaLutName.c_str());
#endif
            fprintf (fp, "uniform float %s_Range0[4];\n", rgbaLutName.c_str());

            fprintf (fp, "\n");
            fprintf (fp, "vec4 %s ()\n", funcName.c_str());
            fprintf (fp, "{\n");
            fprintf (fp, "    float val = %s ();\n",
                                requiredExts["volScalar"]->glslName.c_str());
            fprintf (fp, "    float p = (val+%s_Range0[2])*%s_Range0[3];\n",
                rgbaLutName.c_str(), rgbaLutName.c_str() );


#if COMBINE_TABLES
            fprintf (fp, "    vec4 color = texture1D (%s, p);\n", rgbaLutName.c_str());
#else
            fprintf (fp, 
              "    vec4 color = vec4 ( texture1D (%s, p).rgb, texture1D (%s, p).x);\n", 
              rgbLutName.c_str(), alphaLutName.c_str());
#endif



            }
        else
            {
            // code for 2D alpha lookup


#if COMBINE_TABLES
            // code for alpha/rgb combined in 2D LUT
            // code for 1D alpha lookup
            fprintf (fp, "uniform sampler2D %s;\n", rgbaLutName.c_str());
#else
            // code for alpha & rgb in separate LUTs
            fprintf (fp, "uniform sampler1D %s;\n", rgbLutName.c_str());
            fprintf (fp, "uniform sampler2D %s;\n", alphaLutName.c_str());
#endif

            fprintf (fp, "uniform float %s_Range0[4];\n", 
                                                        rgbaLutName.c_str());
            fprintf (fp, "uniform float %s_Range1[4];\n", 
                                                        rgbaLutName.c_str());
            
            fprintf (fp, "\n");


#if 0
            fprintf (fp, "uniform sampler3D %s;\n", gradMagVolName.c_str());
            fprintf (fp, "\n");
            fprintf (fp, "float volGradMag()\n");
            fprintf (fp, "{\n");
            fprintf (fp, "    return texture3D (%s, 0.5+CurrPosition/VolumeSize).x;\n",
                            gradMagVolName.c_str());
            fprintf (fp, "} // end of volGradMag \n");
            fprintf (fp, "{\n");
#endif


            fprintf (fp, "vec4 %s ()\n", funcName.c_str());
            fprintf (fp, "{\n");
            fprintf (fp, "    float val = %s ();\n",
                                requiredExts["volScalar"]->glslName.c_str());
            fprintf (fp, "    float gm = %s ();\n",
                                requiredExts["volGradMag"]->glslName.c_str());
            fprintf (fp, 
              "    vec2 p2 = vec2 (\n"
              "          (val+%s_Range0[2])*%s_Range0[3],\n"
              "          (gm +%s_Range1[2])*%s_Range1[3] );\n",
              rgbaLutName.c_str(), rgbaLutName.c_str(),
              rgbaLutName.c_str(), rgbaLutName.c_str()  );

#if COMBINE_TABLES
            // code for alpha/rgb combined in 2D LUT
            // code for 1D alpha lookup
            fprintf (fp, "    vec4 color = texture2D (%s, p2);\n", rgbaLutName.c_str());
#else
            // code for alpha & rgb in separate LUTs
            fprintf (fp,
              "    vec4 color = "
                     "vec4 ( texture1D (%s, p2.x).rgb , texture2D (%s, p2).x );\n",
              rgbLutName.c_str(), alphaLutName.c_str() );
#endif
            }  // end of section for 2D alpha lookup




        // JGH: Note that the alpha adjustments and the lighting are
        //      now being handled in the common code that is independent
        //      of any specific transfer function.
#if 0
        fprintf (fp, "    color.a = (color.a+alphaOffset()) * alphaScale();\n");

        if (lightingModel == LM_GRAD_OPAQUE)
                {
                fprintf (fp, "\n// lighting:\n");
                fprintf (fp, "#define VOL_POS %s\n",
                                requiredExts["volPos"]->glslName.c_str());
                fprintf (fp, "#define VOL_POS_PREV %s\n",
                                requiredExts["volPosPrev"]->glslName.c_str());
                fprintf (fp, "#define VOL_GRAD_CALC %s\n",
                                requiredExts["volGradCalc"]->glslName.c_str());
                if (infoDim == 2)
                    {
                    fprintf (fp, "#define VOL_GRAD_MAG %s\n",
                                requiredExts["volGradMag"]->glslName.c_str());
                    }
                fprintf (fp,
                    "#include \"vol.lightOpaque.glsl\"\n");
                fprintf (fp, "// end of lighting\n\n");

                }
#endif

        fprintf (fp, "    return color;\n");
        fprintf (fp, "} // end of %s\n", funcName.c_str());
        fprintf (fp, "\n");

        fprintf (fp, "//\n");
        fprintf (fp, "// end of code for GkBoundary transfer function %s.\n",
                                funcName.c_str());
        fprintf (fp, "//////////////////\n");
        return 0;






    }  // end of int VolVis::GkBoundary::writeGLSL (FILE *fp)



int VolVis::GkBoundary::writeGLSL (std::string const & fileName)
    {
    FILE *fp = fopen (fileName.c_str(), "w");

    if (fp == NULL)
        {
        dtkMsg.add (DTKMSG_ERROR,
                                "Error writing GkBoundary GLSL file: "
                                "Unable to open file %s.\n",
                                fileName.c_str());
        return -1;
        }

    int rtn = writeGLSL (fp);
    fclose (fp);

    return rtn;
    }  // end of int VolVis::GkBoundary::writeGLSL (std::string const & fileName)


/////////////////////////////////////////////////////////

int VolVis::GkBoundary::getNumTextures ()
    {
    if (prepTextures ())
        {
        return -1;
        }

    return textures.size();

#if COMBINE_TABLES
    return 1;
#else
    return 2;
#endif
    } // end of int VolVis::GkBoundary::getNumTextures ()


int VolVis::GkBoundary::writeTexture (int n, std::string const & fileName)
    {

    if (prepTextures ())
        {
        return -1;
        }

#if 0
#if COMBINE_TABLES
    if (n != 0)
        {
        dtkMsg.add (DTKMSG_ERROR,
                  "GkBoundary::writeTexture: "
                  "Texture index %d is out of bounds.\n", n);
        return -1;
        }
    int nTxtr = 2;
#else
    if (n < 0 || n > 1)
        {
        dtkMsg.add (DTKMSG_ERROR,
                  "GkBoundary::writeTexture: "
                  "Texture index %d is out of bounds.\n", n);
        return -1;
        }
    int nTxtr = n;
#endif
#endif
    if (n < 0 || n >= textures.size())
        {
        dtkMsg.add (DTKMSG_ERROR,
                  "GkBoundary::writeTexture: "
                  "Texture index %d is out of bounds.\n", n);
        return -1;
        }

    return textures[n]->writeMha (fileName);
    }  // end of int GkBoundary::writeTexture (int n, string const & fileName)

int VolVis::GkBoundary::getTextureInfo (
  int n, 
  std::string & glslVarName,
  int & nChan, 
  int & nDim
  )
    {

    if (prepTextures ())
        {
        return -1;
        }

    if (n < 0 || n >= textures.size())
        {
        dtkMsg.add (DTKMSG_ERROR,
                  "GkBoundary::getTextureInfo : "
                  "Texture index %d is out of bounds.\n", n);
        return -1;
        }

    glslVarName = textures[n]->getName ();
    nChan = textures[n]->getNChan ();
    nDim = textures[n]->getNDim ();

    return 0;
    }  // end of getTextureInfo


int VolVis::GkBoundary::getTextureRanges (
  int n,
  float *ranges )
    {
    if (prepTextures ())
        {
        return -1;
        }

    if (n < 0 || n >= textures.size())
        {
        dtkMsg.add (DTKMSG_ERROR,
                  "GkBoundary::getTextureRanges: "
                  "Texture index %d is out of bounds.\n", n);
        return -1;
        }



    for (int i = 0; i < textures[n]->getNDim(); i++)
        {
        double drange[2];
        if (textures[n]->getNormRange (i, drange))
            {
            return -1;
            }
        ranges[2*i + 0] = drange[0];
        ranges[2*i + 1] = drange[1];
        }  

    return 0;
    } // end of getTextureRanges



/////////////////////////////////////////////////////////





