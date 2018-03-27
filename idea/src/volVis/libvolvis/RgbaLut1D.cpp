
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dtk.h>
#include <iris.h>

#include <vtkSmartPointer.h>
#include <vtkColorTransferFunction.h>
// #include <vtkmetaio/metaImage.h>


#include "VolVis.h"
#include "RgbaLut1D.h"
#include "MetaImg.h"



#define MAX(x,y)        (((x)>(y))?(x):(y))
#define MIN(x,y)        (((x)<(y))?(x):(y))

//////////////////////////////////////////////////////////////////////////
//
// Here are the methods for RgbaLut1D:

VolVis::RgbaLut1D::RgbaLut1D ()
        {
        reset ();
        }  // end of VolVis::RgbaLut1D()



VolVis::RgbaLut1D::~RgbaLut1D ()
        {
        // I don't think anything needs to be done here.
        // Do we need to .clear() the vectors?

        }  // end of VolVis::~RgbaLut1D()


int VolVis::RgbaLut1D::reset ()
        {
        // place LUT into default empty state
        funcTypeName = "RgbaLut1D";
        funcName = "noName";

        colInterpType = C_INTERP_RGB;
        alphaInterpType = L_INTERP_LINEAR;
        valueTransform = VALX_NONE;
        combineTables = true;
        tableLength = 256;
        outputTableType = LDT_UCHAR;

        recalcTableRGB = false;
        recalcTableA = false;

        rgbInterpPts.clear ();
        alphaInterpPts.clear ();

        rgbTableRange[0] = 0;
        rgbTableRange[1] = 1;

        rgbTable_uc.clear ();
        rgbTable_f.clear ();

        alphaTableRange[0] = 0;
        alphaTableRange[1] = 1;
        alphaTable_uc.clear ();
        alphaTable_f.clear ();

        doNormalization = true;

#if 0
        lightingModel = LM_EMIT_ONLY;
#endif

        requiredExts.clear();
        requiredExts["volScalar"] = 
                new TFReqExt ("volScalar", "float", "volScalar", "func");

        
        return 0;
        }  // end of int VolVis::RgbaLut1D::reset()

int VolVis::RgbaLut1D::setToDefaultLut ()
        {
        reset ();
        // JGH create a default LUT

        setColorInterp (C_INTERP_DIVERGING);
        float rgb[3];
        rgb[0] = 1;
        rgb[1] = 0;
        rgb[2] = 0;
        setInterpPtRGB (0.0, rgb);
        rgb[0] = 0;
        rgb[1] = 0;
        rgb[2] = 1;
        setInterpPtRGB (1.0, rgb);

        setInterpPtA (0.0, 0.05);
        setInterpPtA (1.0, 0.05);

        funcName = "Dflt";

        return 0;
        }  // end of int VolVis::RgbaLut1D::setToDefaultLut
        

#if 0
int VolVis::RgbaLut1D::setLighting (VolVis::LightingModel lm)
        {
        if ( (lm != lightingModel) && (lm == LM_GRAD_OPAQUE) )
            {
            requiredExts["volPos"] = 
                new TFReqExt ("volPos", "vec3", "volPos", "func");
            requiredExts["volPosPrev"] = 
                new TFReqExt ("volPosPrev", "vec3", "volPosPrev", "func");
            requiredExts["volGradCalc"] =
                new TFReqExt ("volGradCalc", "vec3", "volGradCalc", "func");
            }
        else if ( (lm != lightingModel) && (lightingModel == LM_GRAD_OPAQUE) )
            {
            requiredExts.erase ("volPos");
            requiredExts.erase ("volPosPrev");
            requiredExts.erase ("volGradCalc");
            }

        lightingModel = lm;

        return 0;
        } // end of int VolVis::RgbaLut1D::setLighting (VolVis::LightingModel lm)

VolVis::LightingModel VolVis::RgbaLut1D::getLighting () const
        {
        return lightingModel;
        } // end of VolVis::LightingModel RgbaLut1D::getLighting () const
#endif


int VolVis::RgbaLut1D::setColorInterp (ColorInterpType interpT)
        {
        if (colInterpType != interpT)
                {
                colInterpType = interpT;
                recalcTableRGB = true;
                }

        return 0;
        }  // end of RgbaLut1D::setColorInterp

VolVis::ColorInterpType VolVis::RgbaLut1D::getColorInterp () const
        {
        return colInterpType;
        }  // end of int VolVis::RgbaLut1D::getColorInterp ()
        


int VolVis::RgbaLut1D::setLookupInterp (LookupInterpType interpT)
        {
        alphaInterpType = interpT;
        return 0;
        }  // end of RgbaLut1D::setLookupInterp

VolVis::LookupInterpType VolVis::RgbaLut1D::getLookupInterp () const
        {
        return alphaInterpType;
        }  // end of int VolVis::RgbaLut1D::getLookupInterp ()
        





int VolVis::RgbaLut1D::setValTransform (ValueTransformType vXform)
        {
        if (valueTransform != vXform)
                {
                valueTransform = vXform;
                recalcTableRGB = true;
                recalcTableA = true;
                }
        return 0;
        }  // end of RgbaLut1D::setValTransform

VolVis::ValueTransformType VolVis::RgbaLut1D::getValTransform ()   const
        {
        return valueTransform;
        }  //end of ValueTransformType VolVis::RgbaLut1D::getValTransform ()const




int VolVis::RgbaLut1D::setPreIntegrate (bool preInt)
        {
        preIntegrate = preInt;
        }       // end of VolVis::RgbaLut1D::setPreIntegrate

bool VolVis::RgbaLut1D::getPreIntegrate ()
        {
        return preIntegrate;
        }       // end of VolVis::RgbaLut1D::

int VolVis::RgbaLut1D::setNormalize (bool doNorm)
        {
        doNormalization = doNorm;
        }       // end of VolVis::RgbaLut1D::

bool VolVis::RgbaLut1D::getNormalize ()
        {
        return doNormalization;
        }       // end of VolVis::RgbaLut1D::











int VolVis::RgbaLut1D::setTableLength (unsigned int len)
        {

        if (len == 0)
                {
                return -1;
                }

        if (len != tableLength)
                {
                tableLength = len;
                recalcTableRGB = true;
                recalcTableA = true;
                }

        return 0;
        }  // end of VolVis::RgbaLut1D::setTableLength

unsigned int VolVis::RgbaLut1D::getTableLength () const
        {
        return tableLength;
        }  // end of int VolVis::RgbaLut1D::getTableLength



int VolVis::RgbaLut1D::setTableType (LutDataType type)
        {
        if (outputTableType != type)
                {
                if (outputTableType == LDT_UCHAR)
                        {
                        rgbTable_uc.clear ();
                        alphaTable_uc.clear ();
                        }
                else
                        {
                        rgbTable_f.clear ();
                        alphaTable_f.clear ();
                        }
                recalcTableRGB = true;
                recalcTableA = true;
                outputTableType = type;
                }

        return 0;
        }  // end of int VolVis::RgbaLut1D::setTableType (DataType type)

VolVis::LutDataType VolVis::RgbaLut1D::getTableType () const
        {
        return outputTableType;
        }  // end of LutDataType VolVis::RgbaLut1D::getTableType ()


int VolVis::RgbaLut1D::setCombineTables (bool combine)
        {
        combineTables = combine;
        return 0;
        }  // end of int VolVis::RgbaLut1D::setCombineTables (bool combine);

bool VolVis::RgbaLut1D::getCombineTables () const
        {  
        return combineTables;
        }  // end of bool VolVis::RgbaLut1D::getCombineTables () const



void VolVis::RgbaLut1D::interpAlpha (LookupInterpType interpType, 
                                    float p,
                                    float lowerAlpha,
                                    float upperAlpha,
                                    float & interpAlpha )
        {

        if (interpType == L_INTERP_NEAREST)
                {
                if (p < 0.5)
                        {
                        interpAlpha = lowerAlpha;
                        }
                else
                        {
                        interpAlpha = upperAlpha;
                        }
                return;
                }


        interpAlpha = (1.0 - p)*lowerAlpha + p*upperAlpha;

        return ;
        }  // end of interpAlpha



void VolVis::RgbaLut1D::interpRGB (ColorInterpType interpType, float p,
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
                        "RgbaLut1D: Invalid color interpolation type: %d;\n"
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



int VolVis::RgbaLut1D::buildTableRGB ()
        {
        // use rgbInterpPts, colInterpType, tableLength, and outputTableType 
        //                 to generate 
        //         rgbTableRange and either rgbTable_uc or rgbTable_f 

        if (rgbInterpPts.size() < 1)
                {
                dtkMsg.add (DTKMSG_ERROR, 
                        "RgbaLut1D: Trying to build LUT but "
                        "there are no RGB interpolation points present.\n");
                return -1;
                }

        if (tableLength < 1)
                {
                dtkMsg.add (DTKMSG_ERROR, 
                        "RgbaLut1D: Trying to build LUT but "
                        "RGB table length is zero.\n");
                return -1;
                }

        if (rgbInterpPts.size() == 1)
                {
                // If there's only one interpolation point, we don't need
                // to interpolate.
                rgbTableRange[0] = rgbTableRange[1] = rgbInterpPts[0].val;

                // clear out any existing tables
                rgbTable_uc.clear();
                rgbTable_f.clear();
                if (outputTableType == LDT_UCHAR)
                        {
                        unsigned char urgb[3];
                        RGB_UC uc;
                        uc.rgb[0] = (unsigned char) 
                                        (rgbInterpPts[0].rgb[0]*255.0  +  0.5);
                        uc.rgb[1] = (unsigned char) 
                                        (rgbInterpPts[0].rgb[1]*255.0  +  0.5);
                        uc.rgb[2] = (unsigned char) 
                                        (rgbInterpPts[0].rgb[2]*255.0  +  0.5);
                        rgbTable_uc.reserve (tableLength);
                        for (int i = 0; i < tableLength; i++)
                                {
                                rgbTable_uc.push_back(uc);
                                }
                        }
                else
                        {
                        // do the same thing for rgbTable_f
                        RGB_F f;
                        f.rgb[0] = rgbInterpPts[0].rgb[0];
                        f.rgb[1] = rgbInterpPts[0].rgb[1];
                        f.rgb[2] = rgbInterpPts[0].rgb[2];
                        rgbTable_f.reserve (tableLength);
                        for (int i = 0; i < tableLength; i++)
                                {
                                rgbTable_f.push_back(f);
                                }
                        }

                recalcTableRGB = false;
                return 0;
                }


        // If we get here, it's because we need to really interpolate between
        // multiple points in rgbInterpPts.


        // the entries in rgbInterpPts are sorted by the val field
        int lastPt = rgbInterpPts.size() - 1;

        // first get the range of values:

        rgbTableRange[0] = rgbInterpPts[0].val;
        rgbTableRange[1] = rgbInterpPts[lastPt].val;

        if  (rgbTableRange[1] == rgbTableRange[0])
                {
                dtkMsg.add (DTKMSG_ERROR, 
                        "Error: Range of values has zero length.\n");
                return -1;
                }

        // double valLen = rgbTableRange[1] - rgbTableRange[0];
        double xvalLen = 
                xformVal(rgbTableRange[1]) - xformVal(rgbTableRange[0]);
        int upperIndex = 1;     // in the next loop, the current table entry is 
                                // between this interpPts entry and the one 
                                // below.
        rgbTable_uc.clear();
        rgbTable_f.clear();
        if (outputTableType == LDT_UCHAR)
                {
                rgbTable_uc.reserve (tableLength);
                }
        else
                {
                rgbTable_f.reserve (tableLength);
                }

        for (int i = 0; i < tableLength; i++)
                {
                // entry corresponds to data value iVal
                double iVal = ((double)i)/((double)tableLength);
                iVal = iVal*xvalLen + xformVal(rgbTableRange[0]);

                // See if we're in the same interval
                while (iVal > xformVal(rgbInterpPts[upperIndex].val))
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
                float p = (iVal - xformVal(rgbInterpPts[upperIndex-1].val)) / 
                          (xformVal(rgbInterpPts[upperIndex].val)    - 
                           xformVal(rgbInterpPts[upperIndex-1].val) );

                // Do the interpolation
                float frgb[3];
                interpRGB (     colInterpType, p,
                                rgbInterpPts[upperIndex-1].rgb,
                                rgbInterpPts[upperIndex  ].rgb,
                                frgb );

                // convert to uchar if necessary, put it in the right table.
                if (outputTableType == LDT_UCHAR)
                        {
                        RGB_UC uc;
                        uc.rgb[0] = (unsigned char) (frgb[0]*255.0 + 0.5);
                        uc.rgb[1] = (unsigned char) (frgb[1]*255.0 + 0.5);
                        uc.rgb[2] = (unsigned char) (frgb[2]*255.0 + 0.5);
                        rgbTable_uc.push_back(uc);
                        }
                else
                        {
                        RGB_F f;
                        f.rgb[0] = frgb[0];
                        f.rgb[1] = frgb[1];
                        f.rgb[2] = frgb[2];
                        rgbTable_f.push_back(f);
                        }

                }  // end of loop over table entries

        recalcTableRGB = false;
        return 0;
        }  // end of int VolVis::RgbaLut1D::buildTableRGB ()






int VolVis::RgbaLut1D::buildTableA ()
        {
        // use alphaInterpPts, colInterpType, tableLength, and outputTableType 
        //                 to generate 
        //         alphaTableRange and either alphaTable_uc or alphaTable_f 

        if (alphaInterpPts.size() < 1)
                {
                dtkMsg.add (DTKMSG_ERROR, 
                        "RgbaLut1D: Trying to build LUT but "
                        "there are no RGB interpolation points present.\n");
                return -1;
                }

        if (tableLength < 1)
                {
                dtkMsg.add (DTKMSG_ERROR, 
                        "RgbaLut1D: Trying to build LUT but "
                        "RGB table length is zero.\n");
                return -1;
                }

        if (alphaInterpPts.size() == 1)
                {
                // If there's only one interpolation point, we don't need
                // to interpolate.
                alphaTableRange[0] = alphaTableRange[1] = alphaInterpPts[0].val;

                // clear out any existing tables
                alphaTable_uc.clear();
                alphaTable_f.clear();
                if (outputTableType == LDT_UCHAR)
                        {
                        unsigned char a = 
                          (unsigned char) (255.0*alphaInterpPts[0].alpha + 0.5);
                        alphaTable_uc.reserve (tableLength);
                        for (int i = 0; i < tableLength; i++)
                                {
                                alphaTable_uc.push_back(a);
                                }
                        }
                else
                        {
                        // do the same thing for alphaTable_f
                        float a = alphaInterpPts[0].alpha;
                        alphaTable_f.reserve (tableLength);
                        for (int i = 0; i < tableLength; i++)
                                {
                                alphaTable_f.push_back(a);
                                }
                        }

                recalcTableRGB = false;
                return 0;
                }


        // If we get here, it's because we need to really interpolate between
        // multiple points in alphaInterpPts.


        // the entries in alphaInterpPts are sorted by the val field
        int lastPt = alphaInterpPts.size() - 1;

        // first get the range of values:

        alphaTableRange[0] = alphaInterpPts[0].val;
        alphaTableRange[1] = alphaInterpPts[lastPt].val;

        if  (alphaTableRange[1] == alphaTableRange[0])
                {
                dtkMsg.add (DTKMSG_ERROR, 
                        "Error: Range of values has zero length.\n");
                return -1;
                }

        // double valLen = alphaTableRange[1] - alphaTableRange[0];
        double xvalLen = 
                xformVal(alphaTableRange[1]) - xformVal(alphaTableRange[0]);
        int upperIndex = 1;     // in the next loop, the current table entry is 
                                // between this interpPts entry and the one 
                                // below.
        alphaTable_uc.clear();
        alphaTable_f.clear();
        if (outputTableType == LDT_UCHAR)
                {
                alphaTable_uc.reserve (tableLength);
                }
        else
                {
                alphaTable_f.reserve (tableLength);
                }

        for (int i = 0; i < tableLength; i++)
                {
                // entry corresponds to data value iVal
                double iVal = ((double)i)/((double)tableLength);
                iVal = iVal*xvalLen + xformVal(alphaTableRange[0]);

                // See if we're in the same interval
                while (iVal > xformVal(alphaInterpPts[upperIndex].val))
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
                float p = (iVal - xformVal(alphaInterpPts[upperIndex-1].val)) / 
                          (xformVal(alphaInterpPts[upperIndex].val) - 
                           xformVal(alphaInterpPts[upperIndex-1].val) );

                // Do the interpolation
                float alpha;
                interpAlpha (   alphaInterpType, p,
                                alphaInterpPts[upperIndex-1].alpha,
                                alphaInterpPts[upperIndex  ].alpha,
                                alpha );

                // convert to uchar if necessary, put it in the right table.
                if (outputTableType == LDT_UCHAR)
                        {
                        unsigned char uc;
                        uc = (unsigned char) (alpha*255.0 + 0.5);
                        alphaTable_uc.push_back(uc);
                        }
                else
                        {
                        alphaTable_f.push_back(alpha);
                        }

                }  // end of loop over table entries

        recalcTableA = false;
        return 0;
        }  // end of int VolVis::RgbaLut1D::buildTableA ()




int VolVis::RgbaLut1D::buildTables ()
        {

        int rtnRGB = 0;
        int rtnA = 0;

        if (recalcTableRGB)
                {
                rtnRGB = buildTableRGB();
                }
        
        if (recalcTableA)
                {
                rtnA = buildTableA();
                }
        
        return (rtnRGB != 0) || (rtnA != 0) ;
                        
        }  // end of int VolVis::RgbaLut1D::buildTables ()



int VolVis::RgbaLut1D::setInterpPtRGB (double v, const float rgb[3])
        {
        // insert (v,rgb) into rgbInterpPts such that it's sorted by val

        recalcTableRGB = true;

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

        }  // end of int RgbaLut1D::setInterpPtRGB (double , const float [3])

int VolVis::RgbaLut1D::setInterpPtA (double v, const float a)
        {
        // insert (v,a) into alphaInterpPts such that it's sorted by val

        recalcTableA = true;

        VAL_ALPHA vcol;
        vcol.val = v;
        vcol.alpha = a;

        for ( std::vector<VAL_ALPHA>::iterator ivc = alphaInterpPts.begin();
              ivc != alphaInterpPts.end();
              ivc++ )
                {
                if (v == (*ivc).val)
                        {
                        // just replace the alpha
                        (*ivc).alpha = a;
                        return 0;
                        }
                else if (v < (*ivc).val)
                        {
                        // we've found the insertion point
                        alphaInterpPts.insert (ivc, vcol);
                        return 0;
                        }
                }  // end of loop over entries in alphaInterpPts

        // If we reach here, we didn't find a place to insert it, so
        // put it at the end;
        alphaInterpPts.push_back (vcol);
        return 0;

        }  // end of int RgbaLut1D::setInterpPtA (double , float ])


int VolVis::RgbaLut1D::setInterpPtRGBA (double v, const float rgba[4])
        {
        if ( setInterpPtRGB (v, rgba) || setInterpPtA (v, rgba[3]) )
                {
                return -1;
                }

        return 0;
        }  // end of int RgbaLut1D::setInterpPtRGB (double , const float [3])

int VolVis::RgbaLut1D::getNumInterpPtsRGB () const
        {
        return rgbInterpPts.size ();
        }  // end of int VolVis::RgbaLut1D::getNumInterpPtsRGB () const

int VolVis::RgbaLut1D::getNumInterpPtsA () const
        {
        return alphaInterpPts.size ();
        }  // end of int VolVis::RgbaLut1D::getNumInterpPtsA () const

int VolVis::RgbaLut1D::getNumInterpPtsRGBA () const
        {
        int nrgb = rgbInterpPts.size ();
        int na = alphaInterpPts.size ();
        if (nrgb != na)
                {
                return -1;
                }
        return na;
        }  // end of int VolVis::RgbaLut1D::getNumInterpPtsRGB () const



int VolVis::RgbaLut1D::getInterpPtRGB (
  int iPtIndex, 
  double & v, 
  float rgb[3] 
  ) const
        {
        if ( (iPtIndex < 0) || (iPtIndex >= rgbInterpPts.size()) )
                {
                return -1;
                }

        v = rgbInterpPts[iPtIndex].val;
        memcpy (rgb,  rgbInterpPts[iPtIndex].rgb, 3*sizeof(float) );

        return 0;
        }  // end of getInterpPtRGB (int, double & , float [3])

int VolVis::RgbaLut1D::getInterpPtA (
  int iPtIndex, 
  double & v, 
  float & a 
  ) const
        {
        if ( (iPtIndex < 0) || (iPtIndex >= alphaInterpPts.size()) )
                {
                return -1;
                }

        v = alphaInterpPts[iPtIndex].val;
        a = alphaInterpPts[iPtIndex].alpha;

        return 0;
        }  // end of getInterpPtA (int, double & , float &)

int VolVis::RgbaLut1D::getInterpPtRGBA (
  int iPtIndex, 
  double & v, 
  float rgba[4] 
  ) const
        {
        double v1, v2;

        int r1 = getInterpPtRGB (iPtIndex, v1, rgba);
        int r2 = getInterpPtA (iPtIndex, v2, rgba[3]);
        
        if (r1 || r2)
                {
                return -1;
                }

        if (v1 != v2)
                {
                return -2;
                }

        return 0;
        } // end of  getInterpPtRGBA (int, double &, float[4]) const



int VolVis::RgbaLut1D::removeInterpPtRGBA (int iPtIndex)
        {
        if ( (iPtIndex < 0) || (iPtIndex >= rgbInterpPts.size()) 
                            || (iPtIndex >= alphaInterpPts.size()) )
                {
                return -1;
                }

        int rtn1 = removeInterpPtRGB (iPtIndex);
        int rtn2 = removeInterpPtA (iPtIndex);

        if (rtn1 || rtn2)
                {
                return -2;
                }

        return 0;
        } // end of int VolVis::RgbaLut1D::removeInterpPtRGBA (int iPtIndex)

int VolVis::RgbaLut1D::removeInterpPtRGB (int iPtIndex)
        {
        if ( (iPtIndex < 0) || (iPtIndex >= rgbInterpPts.size())  )
                {
                return -1;
                }

        int i = 0;
        for ( std::vector<VAL_RGB>::iterator ivc = rgbInterpPts.begin();
              ivc != rgbInterpPts.end();
              ivc++ )
                {
                if (i == iPtIndex)
                        {
                        rgbInterpPts.erase (ivc);
                        return 0;
                        }
                }

        return -3;
        } // end of int VolVis::RgbaLut1D::removeInterpPtRGB (int iPtIndex)


int VolVis::RgbaLut1D::removeInterpPtA (int iPtIndex)
        {
        if ( (iPtIndex < 0) || (iPtIndex >= alphaInterpPts.size())  )
                {
                return -1;
                }

        int i = 0;
        for ( std::vector<VAL_ALPHA>::iterator ivc = alphaInterpPts.begin();
              ivc != alphaInterpPts.end();
              ivc++ )
                {
                if (i == iPtIndex)
                        {
                        alphaInterpPts.erase (ivc);
                        return 0;
                        }
                }

        return -3;
        } // end of int VolVis::RgbaLut1D::removeInterpPtA (int iPtIndex)



int VolVis::RgbaLut1D::removeAllInterpPtsA ()
        {
        alphaInterpPts.clear ();
        return 0;
        } // end of int VolVis::RgbaLut1D::removeAllInterpPtsA ()

int VolVis::RgbaLut1D::removeAllInterpPtsRGB ()
        {
        rgbInterpPts.clear ();
        return 0;
        } // end of int VolVis::RgbaLut1D::removeAllInterpPtsRGB ()

int VolVis::RgbaLut1D::removeAllInterpPts ()
        {
        rgbInterpPts.clear ();
        alphaInterpPts.clear ();
        return 0;
        } // end of int VolVis::RgbaLut1D::removeAllInterpPts ()


int VolVis::RgbaLut1D::lookup (double v, float rgba[4])
        {
        // JGH: will implement when it's needed
        return -1;
        }  // end of int VolVis::RgbaLut1D::lookup (double v, float rgba[4])


int VolVis::RgbaLut1D::remapRGBA (const double limits[2])
        {
        // JGH: will implement when it's needed
        return -1;
        }  // end of int VolVis::RgbaLut1D::remapRGBA (const double [2])

int VolVis::RgbaLut1D::remapRGB (const double limits[2])
        {
        // JGH: will implement when it's needed
        return -1;
        }  // end of int VolVis::RgbaLut1D::remapRGB (const double [2])

int VolVis::RgbaLut1D::remapA (const double limits[2])
        {
        // JGH: will implement when it's needed
        return -1;
        }  // end of int VolVis::RgbaLut1D::remapA (const double [2])

int VolVis::RgbaLut1D::getTableElementRGBA (int index, float rgba[4])
        {
        // JGH: will implement when it's needed
        return -1;
        } // end of int getTableElementRGBA (int index, float rgba[4])

int VolVis::RgbaLut1D::getTableElementA (int index, float & a )
        {
        // JGH: will implement when it's needed
        return -1;
        } // end of int getTableElementA (int index, float & a )

int VolVis::RgbaLut1D::getTableElementRGB (int index, float rgb[3])
        {
        // JGH: will implement when it's needed
        return -1;
        } // end of int getTableElementRGB (int index, float rgba[3])

/////////////////////////////////////////////////////////////////////////

// Here are the read and write methods of RgbaLut1D


// First the read methods

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
removeCommas (char *line)
        {
        for (char *c = line; *c; c++)
                {
                if ( *c == ',' )
                        {
                        *c = ' ';
                        }
                }
        }  // end of removeComment



int
VolVis::RgbaLut1D::parseFuncDecl (
  std::vector<std::string> tokVec, 
  std::string & funcName )
        {

        if (parseFuncDeclaration (tokVec, funcTypeName, funcName) )
                {
                return -1;
                }

        if (funcTypeName != "RgbaLut1D")
                {
                dtkMsg.add (DTKMSG_ERROR, 
                        "Error in parsing transfer function description: "
                        "expected function type RgbaLut1D but found type %s.\n",
                        funcTypeName.c_str());
                return -1;
                }


        return 0;
        }  // end of parseLut1DFuncDecl


int
VolVis::RgbaLut1D::parseRGBLUT (FILE *fp)
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
    }  // end of VolVis::RgbaLut1D::parseRGBLUT (FILE *fp)


int VolVis::RgbaLut1D::parseALPHALUT (FILE *fp)
   {
   char line[1000];
    std::vector<std::string> tokVec;

    // read v-alpha lines until we get to END_ALPHALUT
    while (getNextLineTokens (fp, tokVec, line, sizeof(line)) == 0)
        {
        if (tokVec.size() == 2)
                {
                double value;
                float alpha;
                if ( ! iris::StringToDouble (tokVec[0], &value))
                        {
                        dtkMsg.add (DTKMSG_ERROR, 
                                        "Error parsing ALPHA LUT entry: "
                                        "Expected double, found %s\n"
                                        "    on line: %s.\n", 
                                        tokVec[0].c_str(), line);
                        return -1;
                        }
                if ( ! iris::StringToFloat (tokVec[1], &alpha))
                        {
                        dtkMsg.add (DTKMSG_ERROR, 
                                        "Error parsing ALPHA LUT entry: "
                                        "Expected float, found %s.\n"
                                        "    on line: %s.\n",
                                        tokVec[1].c_str(), line);
                        return -1; 
                        }

                if (setInterpPtA (value, alpha))
                        {
                        return -1;
                        }
                        
                }
        else if (tokVec.size() == 1)
                {
                if (iris::IsSubstring ("END_ALPHALUT", tokVec[0]))
                        {
                        return 0;
                        }
                else
                        {
                        dtkMsg.add (DTKMSG_ERROR, 
                                        "Error parsing ALPHA LUT: "
                                        "Expected END_ALPHALUT, found %s.\n",
                                        tokVec[0].c_str());
                        }
                }
        else
                {
                dtkMsg.add (DTKMSG_ERROR, 
                            "Error parsing ALPHA LUT: "
                            "Bad number of tokens: %d\n"
                            "    on line: %s.\n",
                            tokVec.size(), line);
                return -1;
                }

        }  // end of while

    return 0;
    }  // end of VolVis::RgbaLut1D::parseALPHALUT (FILE *fp)



#if 0
std::string VolVis::RgbaLut1D::toStr (VolVis::LightingModel lm)
        {
        switch (lm)
                {
                case LM_EMIT_ONLY:
                        return "EMIT_ONLY";

                case LM_GRAD_OPAQUE:
                        return "GRAD_OPAQUE";

                default:
                        dtkMsg.add (DTKMSG_ERROR,
                            "Invalid value for lighting found (%d); "
                            "using EMIT_ONLY.\n",
                            lm);
                        return "EMIT_ONLY";
                }

        }  // end of std::string VolVis::RgbaLut1D::toStr (VolVis::LightingModel lm)


VolVis::LightingModel 
VolVis::RgbaLut1D::strToLightingModel (std::string & str)
        {
        VolVis::LightingModel lm;

        if (iris::IsSubstring ("EMIT_ONLY", str))
                {
                lm = LM_EMIT_ONLY;
                }
        else if (iris::IsSubstring ("GRAD_OPAQUE", str))
                {
                lm = LM_GRAD_OPAQUE;
                }
        else if (iris::IsSubstring ("UNKNOWN", str))
                {
                dtkMsg.add (DTKMSG_ERROR,
                        "Parsing Error: "
                        "Bad value for lighting model: %s; using EMIT_ONLY.\n",
                        str.c_str());
                lm = LM_EMIT_ONLY;
                }
        else
                {
                dtkMsg.add (DTKMSG_ERROR,
                            "Parsing Error: "
                            "Bad value for lighting model: %s.\n",
                            str.c_str());
                lm = LM_UNKNOWN;
                }

        return lm;
        } // end of VolVis::LightingModel strToLightingModel (std::string & str)


int VolVis::RgbaLut1D::strToLightingModelRead (std::string & str)
        {

        VolVis::LightingModel lm = strToLightingModel (str);

        if (lm == LM_UNKNOWN)
                {
                return -1;
                }

        if (setLighting (lm))
                {
                return -1;
                }

        return 0;
        }  // end int VolVis::RgbaLut1D::strToLightingModelRead (std::string & )
#endif



int VolVis::RgbaLut1D::strToValXform (std::string & str)
        {
        ValueTransformType vx = VolVis::strToValXform (str);
        if (vx == VALX_UNKNOWN)
                {
                return -1;
                }

        if (setValTransform (vx))
                {
                return -1;
                }

        return 0;
        }  // end of RgbaLut1D::strToValXform



int VolVis::RgbaLut1D::strToColorInterp (std::string & str)
        {


        ColorInterpType ci = VolVis::strToColorInterp (str);
        if (ci == C_INTERP_UNKNOWN)
                {
                return -1;
                }

        if (setColorInterp (ci))
                {
                return -1;
                }

        return 0;

        }  // end of RgbaLut1D::strToColorInterp


int VolVis::RgbaLut1D::strToLookupInterp (std::string & str)
        {


        LookupInterpType li = VolVis::strToLookupInterp (str);
        if (li == L_INTERP_UNKNOWN)
                {
                return -1;
                }

        if (setLookupInterp (li))
                {
                return -1;
                }

        return 0;
        }  // end of RgbaLut1D::strToLookupInterp


int VolVis::RgbaLut1D::strToCombineRGBA (std::string & str)
        {

        bool combine;
        if (VolVis::strToBool (str, combine))
                {
                dtkMsg.add (DTKMSG_ERROR, 
                            "Error parsing transfer function: "
                            "Bad combine RGBA specifier: %s.\n",
                            str.c_str());
                return -1;
                }

        setCombineTables (combine);
        return 0;
        }  // end of RgbaLut1D::strToCombineRGBA


int VolVis::RgbaLut1D::strToPreIntegrate (std::string & str)
        {
        bool preInt;
        if (VolVis::strToBool (str, preInt))
                {
                dtkMsg.add (DTKMSG_ERROR, 
                            "Error parsing transfer function: "
                            "Bad PRE_INTEGRATE specifier: %s.\n",
                            str.c_str());
                return -1;
                }

        setPreIntegrate (preInt);
        return 0;
        }  // end of RgbaLut1D::strToPreIntegrate


int VolVis::RgbaLut1D::strToDoNorm (std::string & str)
        {
        bool doNorm;
        if (VolVis::strToBool (str, doNorm))
                {
                dtkMsg.add (DTKMSG_ERROR, 
                            "Error parsing transfer function: "
                            "Bad NORMALIZE specifier: %s.\n",
                            str.c_str());
                return -1;
                }

        setNormalize (doNorm);
        return 0;
        }  // end of RgbaLut1D::strToDoNorm







int VolVis::RgbaLut1D::strToTableLen (std::string & str)
        {
        unsigned int len;
        if ( ! iris::StringToUInt (str, &len) )
                {
                dtkMsg.add (DTKMSG_ERROR, 
                                "Error parsing RGBA LUT table length: "
                                "Expected integer, found %s.\n",
                                str.c_str());
                return -1;
                }

        if (len < 1)
                {
                dtkMsg.add (DTKMSG_ERROR, 
                                "Error parsing RGBA LUT table length: "
                                "Bad value (%d); must be greater than zero.\n",
                                len);
                return -1;
                }

        if (setTableLength (len))
                {
                return -1;
                }

        return 0;
        }  // end of RgbaLut1D::strToTableLen


int VolVis::RgbaLut1D::strToTableType (std::string & str)
        {

        LutDataType t = VolVis::strToLDType (str);
        if (t == LDT_UNKNOWN)
                {
                return -1;
                }

        if (setTableType (t))
                {
                return -1;
                }

        return 0;
        }  // end of RgbaLut1D::strTableToType


int VolVis::RgbaLut1D::parseFuncBody (FILE *fp)
    {
    char line[1000];
    std::vector<std::string> tokVec;


    while (getNextLineTokens (fp, tokVec, line, sizeof(line)) == 0)
        {
        int nTok = tokVec.size();

        if (iris::IsSubstring ("VAL_TRANSFORM", tokVec[0]))
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }

            if (strToValXform (tokVec[1]))
                {
                return -1;
                }

            }
        else if (iris::IsSubstring ("COLOR_INTERP", tokVec[0]))
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }

            if (strToColorInterp (tokVec[1]))
                {
                return -1;
                }
            }


        else if (iris::IsSubstring ("PRE_INTEGRATE", tokVec[0]) ||
                 iris::IsSubstring ("PREINTEGRATE", tokVec[0])      )
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }

            if (strToPreIntegrate (tokVec[1]))
                {
                return -1;
                }
            }

        else if (iris::IsSubstring ("NORMALIZE", tokVec[0]))
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }

            if (strToDoNorm (tokVec[1]))
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

            if (strToLightingModelRead (tokVec[1]))
                {
                return -1;
                }
            }
#endif





        else if (iris::IsSubstring ("ALPHA_INTERP", tokVec[0]))
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }

            if (strToLookupInterp (tokVec[1]))
                {
                return -1;
                }
            }
        else if (iris::IsSubstring ("COMBINE_RGBA", tokVec[0]))
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }

            if (strToCombineRGBA (tokVec[1]))
                {
                return -1;
                }
            }
        else if (iris::IsSubstring ("TABLE_LEN", tokVec[0]))
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }

            if (strToTableLen (tokVec[1]))
                {
                return -1;
                }
            }
        else if (iris::IsSubstring ("TABLE_TYPE", tokVec[0]))
            {
            if (nTok != 2)
                {
                badArgCountMsg (line);
                return -1;
                }

            if (strToTableType (tokVec[1]))
                {
                return -1;
                }
            }
        else if (iris::IsSubstring ("RGBLUT", tokVec[0]))
            {
            if (parseRGBLUT (fp))
                {
                return -1;
                }
            }
        else if (iris::IsSubstring ("ALPHALUT", tokVec[0]))
            {
            if (parseALPHALUT (fp))
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
                    "Error parsing RgbaLut1D: "
                    "Function name at beginning (%s) does not "
                    "match function name at end (%s).\n", 
                    funcName.c_str(), tokVec[1].c_str() );
                }
            return 0;
            }

        }  // end of while

    // if we get here, we got to EOF without seeing END_FUNC
    return -1;

    }  // end of parseFuncBody

int VolVis::RgbaLut1D::read (FILE * fp, std::string declarationLine)
        {
        // assume that we are positioned at the beginning of the RgbaLut1D

        char line[1000];
        std::vector<std::string> tokVec;


        // If declarationLine has any tokens, we assume that it is the
        // declaration of a RgbaLut1D function.
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
                                "Error parsing RgbaLut1D: "
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

        return 0;
        }  // end of VolVis::RgbaLut1D::read (FILE * fp)


int VolVis::RgbaLut1D::read (char *fileName)
        {
        return TransferFunction::read (fileName);
        }  // end of VolVis::RgbaLut1D::read (FILE * fp)

///////////////////////////////////////////////////
//

// the write methods




int VolVis::RgbaLut1D::write (FILE *fp)
        {

        buildTables ();

        printIndent (fp);
        fprintf (fp, "FUNC RgbaLut1D %s  inArr[1] outArr[4]\n", 
                                                        funcName.c_str());
        incrementIndent ();

        printIndent (fp);
        fprintf (fp, "VAL_TRANSFORM %s\n", VolVis::toStr(valueTransform).c_str());

        printIndent (fp);
        fprintf (fp, "COLOR_INTERP %s\n", VolVis::toStr(colInterpType).c_str());

#if 0
        printIndent (fp);
        fprintf (fp, "LIGHTING %s\n", toStr (lightingModel).c_str());
#endif

        printIndent (fp);
        fprintf (fp, "NORMALIZE %s\n", doNormalization ? "true" : "false");
        
        printIndent (fp);
        fprintf (fp, "PRE_INTEGRATE %s\n", preIntegrate ? "true" : "false");
        
        printIndent (fp);
        fprintf (fp,"ALPHA_INTERP %s\n",VolVis::toStr(alphaInterpType).c_str());

        printIndent (fp);
        fprintf (fp, "TABLE_TYPE %s\n", VolVis::toStr(outputTableType).c_str());

        printIndent (fp);
        fprintf (fp, "TABLE_LEN %d\n", tableLength);

        printIndent (fp);
        fprintf (fp, "COMBINE_RGBA %s\n", combineTables ? "true" : "false");


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


        printIndent (fp);
        fprintf (fp, "ALPHALUT\n");

        incrementIndent ();
        for (int i = 0; i < alphaInterpPts.size(); i++)
                {
                printIndent (fp);
                fprintf ( fp, "%g   %g\n", 
                                alphaInterpPts[i].val,
                                alphaInterpPts[i].alpha);
                }
        decrementIndent ();

        printIndent (fp);
        fprintf (fp, "END_ALPHALUT\n");

        
        decrementIndent ();
        printIndent (fp);

        fprintf (fp, "END_FUNC %s\n", funcName.c_str());

        return 0;

        }  // end of int VolVis::RgbaLut1D::write (FILE *fp)


int VolVis::RgbaLut1D::write (std::string const & fileName)
        {

        resetIndent ();
        int rtn = TransferFunction::write (fileName);
        resetIndent ();
        return rtn;
        }  // end of VolVis::RgbaLut1D::write (char *filename)

int VolVis::RgbaLut1D::prepForGLSL ()
        {

        if (buildTables () )
                {
                return -1;
                }

        if (rgbInterpPts.size() == 0)
                {
                dtkMsg.add (DTKMSG_ERROR, "Error: No rgb specified.\n" );
                return -1;
                }

        if (alphaInterpPts.size() == 0)
                {
                dtkMsg.add (DTKMSG_ERROR, "Error: No alpha specified.\n" );
                return -1;
                }

        // Determine if we really need a table lookup:
        rgbLU = (rgbInterpPts.size() > 1);
        alphaLU = (alphaInterpPts.size() > 1);

        // OK, we are going to force a table lookup even if there is only
        // one interp point specified.
        rgbLU = true;
        alphaLU = true;
        // However we will keep in the code that handles the non-lookup cases.


        if (combineTables)
                {
                if ( rgbLU && alphaLU )
                    {
                    if ( (rgbTableRange[0] != alphaTableRange[0]) || 
                         (rgbTableRange[1] != alphaTableRange[1])     )
                        {
                        dtkMsg.add (DTKMSG_ERROR, 
                                "Error: RGB and Alpha data ranges cannot "
                                "disagree for combined tables.\n" );
                        return -1;
                        }
                    }
                combinedRange[0] = rgbTableRange[0];
                combinedRange[1] = rgbTableRange[1];
                }


        numTextures = 0;
        if (rgbLU && alphaLU)
                {
                numTextures = combineTables ? 1 : 2;
                }
        else if (rgbLU || alphaLU)
                {
                numTextures = 1;
                }


        // These are the GLSL variable name of the samplers
        // The associated normalization ranges will simpy have "Range" appended
        rgbaLutName = funcName+"_RgbaLut1D";
        rgbLutName = funcName+"_RgbLut1D";
        alphaLutName= funcName+"_AlphaLut1D";
        
        return 0;
        }  // end of prepForGLSL

int VolVis::RgbaLut1D::writeGLSL (FILE *fp)
        {

        if (prepForGLSL())
                {
                return -1;
                }

        fprintf (fp, "\n");
        fprintf (fp, "//////////////////\n");
        fprintf (fp, "// Code for RgbaLut1D transfer function %s.\n", 
                        funcName.c_str());
        fprintf (fp, "\n");

        if (combineTables)
            {
            // create uniforms for rgba LUT and normalization
            fprintf (fp, "uniform sampler1D %s;\n", rgbaLutName.c_str());
            if (doNormalization)
                {
                fprintf (fp, 
                         "uniform float %s_Range0[4];\n", rgbaLutName.c_str());
                }
            }
        else
            {
            if (rgbLU)
                {
                // create uniforms for rgb LUT and normalization
                fprintf (fp, "uniform sampler1D %s;\n", rgbLutName.c_str());
                if (doNormalization)
                    {
                    fprintf (fp, 
                        "uniform float %s_Range0[4];\n", rgbLutName.c_str());
                    }
                }

            if (alphaLU)
                {
                // create uniforms for alpha LUT and normalization
                fprintf (fp, "uniform sampler1D %s;\n", alphaLutName.c_str());
                if (doNormalization)
                    {
                    fprintf (fp, "uniform float %s_Range0[4];\n", 
                                                alphaLutName.c_str());
                    }
                }
            }

        fprintf (fp, "\n");
        fprintf (fp, "vec4 %s ()\n", funcName.c_str());
        fprintf (fp, "{\n");

        if (requiredExts.size() > 0)
            {
            fprintf (fp, "    // Required Externals:\n");
            std::map<std::string, TFReqExt *>::iterator re;
            for (re = requiredExts.begin(); re != requiredExts.end(); re++)
                {
                fprintf (fp, "    //    %-12s %10s:   %-8s %s\n",
                    re->second->stdName.c_str(), re->second->objType.c_str(), 
                    re->second->dataType.c_str(), re->second->glslName.c_str());
                }
            fprintf (fp, "\n");
            }

        fprintf (fp, "    vec4 color;\n");

        if ( (!rgbLU) && (!alphaLU) )
            {
            fprintf (fp, "    color = vec4 (%g, %g, %g, %g);\n",
                                rgbInterpPts[0].rgb[0],
                                rgbInterpPts[0].rgb[1],
                                rgbInterpPts[0].rgb[2],
                                alphaInterpPts[0].alpha
                                );
            }
        else   // we do at least one lookup
            {
            if ( ! combineTables )
                {
                if ( ! rgbLU )
                        {
                        fprintf (fp, "    color.rgb = vec3 (%g, %g, %g);\n",
                                rgbInterpPts[0].rgb[0],
                                rgbInterpPts[0].rgb[1],
                                rgbInterpPts[0].rgb[2]
                                );
                        }
                else if ( ! alphaLU )
                        {
                        fprintf (fp, "    color.a = %g;\n", 
                                alphaInterpPts[0].alpha);
                        }
                }
                

            fprintf (fp, "    float val = %s ();\n", 
                                requiredExts["volScalar"]->glslName.c_str());

            if (combineTables)
                {
                if (doNormalization)
                    {
                    fprintf (fp, "    color = texture1D (%s, "
                                       "(val + %s_Range0[2]) * %s_Range0[3]);\n",
                             rgbaLutName.c_str(), rgbaLutName.c_str(), 
                             rgbaLutName.c_str() );
                    }
                else
                    {
                    fprintf (fp, 
                      "    color = texture1D (%s, val);\n", rgbaLutName.c_str());
                    }
                }
            else
                {
                if (rgbLU)
                    {
                    if (doNormalization)
                        {
                        fprintf (fp, "    color.rgb = texture1D (%s, "
                                    "(val + %s_Range0[2]) * %s_Range0[3]).rgb;\n",
                          rgbLutName.c_str(), rgbLutName.c_str(), 
                          rgbLutName.c_str() );
                        }
                    else
                        {
                        fprintf (fp, 
                          "    color.rgb = texture1D (%s, val).rgb;\n",
                          rgbLutName.c_str());
                        }
                    }

                if (alphaLU)
                    {
                    if (doNormalization)
                        {
                        fprintf (fp, "    color.a = texture1D (%s, "
                                     "(val + %s_Range0[2]) * %s_Range0[3]).x;\n",
                          alphaLutName.c_str(), alphaLutName.c_str(), 
                          alphaLutName.c_str() );
                        }
                    else
                        {
                        fprintf (fp, 
                          "    color.a = texture1D (%s, val).x;\n",
                          alphaLutName.c_str());
                        }
                    }


                }  // end of section for doing separate RGB vs A


            }  // end of clause for doing at least one lookup


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
                fprintf (fp, 
                    "#include \"vol.lightOpaque.glsl\"\n");
                fprintf (fp, "// end of lighting\n\n");

                }
#endif
                


        fprintf (fp, "    return color;\n");
        fprintf (fp, "} // %s\n", funcName.c_str());
        fprintf (fp, "\n");
        fprintf (fp, "// end of code for RgbaLut1D transfer function %s.\n",
                                funcName.c_str());
        fprintf (fp, "//////////////////\n");
        return 0;
        }   // int VolVis::RgbaLut1D::writeGLSL (FILE *fp)

int VolVis::RgbaLut1D::writeGLSL (std::string const & fileName)
        {


        FILE *fp = fopen (fileName.c_str(), "w");

        if (fp == NULL)
                {
                dtkMsg.add (DTKMSG_ERROR, 
                                "Error writing RgbaLut1D GLSL file: "
                                "Unable to open file %s.\n",
                                fileName.c_str());
                return -1;
                }

        int rtn = writeGLSL (fp);
        fclose (fp);

        return rtn;
        }  // int VolVis::RgbaLut1D::writeGLSL (std::string const & filename)

/////////////////////////////////////////////////////////////////////
//
// The following code deals with the texture files

int VolVis::RgbaLut1D::getNumTextures ()
        {
        if (prepForGLSL())
                {
                return -1;
                }

        return numTextures;

        return 0;
        
        } // end of int VolVis::RgbaLut1D::getNumTextures (int n, char *fileName)



int VolVis::RgbaLut1D::writeRGBTexture (std::string const & fileName)
        {
        if (buildTableRGB())
                {
                return -1;
                }

        int itemLen = (outputTableType == LDT_FLOAT) ? 
                                sizeof (float) : sizeof (unsigned char) ;
        void * dataPtr = malloc ( 3 * itemLen * tableLength);
        if (dataPtr == NULL)
                {
                dtkMsg.add (DTKMSG_ERROR, 
                  "RgbaLut1D: "
                  "Unable to allocate space for output RGB texture.\n");
                return -1;
                }

        float *currF = (float *) dataPtr;
        unsigned char *currU = (unsigned char *) dataPtr;
        for (int i = 0; i < tableLength; i++)
                {
                if (outputTableType == LDT_FLOAT)
                        {
                        memcpy (currF, rgbTable_f[i].rgb, 3*itemLen);
                        currF += 3;
                        }
                else
                        {
                        memcpy (currU, rgbTable_uc[i].rgb, 3*itemLen);
                        currU += 3;
                        }

                }
                

////////////////////////////////////////////////////////////
// JGH change this section:

#if 0
        // MetaIO stuff here
        float elSpacing = 1.0;
        vtkmetaio::MetaImage *metaIm = 
            new vtkmetaio:: MetaImage 
                        (
                        1, 
                        (int *) &tableLength, 
                        &elSpacing, 
                        (outputTableType == LDT_FLOAT) ? 
                            vtkmetaio::MET_FLOAT : vtkmetaio::MET_UCHAR,
                        3, 
                        dataPtr 
                        );
        // metaIm->BinaryData (false);
        metaIm->BinaryData (true);
        int rtn = 0;
        if ( ! metaIm->Write (fileName.c_str()) )
                {
                dtkMsg.add (DTKMSG_ERROR, 
                  "RgbaLut1D: "
                  "Error in MetaIO library while writing file %s.\n", 
                  fileName.c_str());
                rtn = -1;
                }


        delete metaIm;
#else

        int rtn = 0;
        if (writeMetaImage 
                    (    
                        fileName, 
                        BINARY, 
                        1, 
                        (int *) &tableLength, 
                        3, 
                        1, 
                        (outputTableType == LDT_FLOAT) ? DT_FLOAT : DT_UCHAR, 
                        dataPtr
                    )           )
            {
            dtkMsg.add (DTKMSG_ERROR, 
                  "RgbaLut1D: Error while writing file %s.\n", 
                  fileName.c_str());
            rtn = -1;
            }


#endif
////////////////////////////////////////////////////////////

        free (dataPtr);

        return rtn;
        }  // end of int VolVis::RgbaLut1D::writeRGBTexture (std::string const &)


int VolVis::RgbaLut1D::writeAlphaTexture (std::string const & fileName)
        {
        if (buildTableA ())
                {
                return -1;
                }

        int itemLen = (outputTableType == LDT_FLOAT) ? 
                                sizeof (float) : sizeof (unsigned char) ;
        void * dataPtr = malloc ( itemLen * tableLength);
        if (dataPtr == NULL)
                {
                dtkMsg.add (DTKMSG_ERROR, 
                  "RgbaLut1D: "
                  "Unable to allocate space for output Alpha texture.\n");
                return -1;
                }

        float *currF = (float *) dataPtr;
        unsigned char *currU = (unsigned char *) dataPtr;
        for (int i = 0; i < tableLength; i++)
                {
                if (outputTableType == LDT_FLOAT)
                        {
                        memcpy (currF, &(alphaTable_f[i]), itemLen);
                        currF += 1;
                        }
                else
                        {
                        memcpy (currU, &(alphaTable_uc[i]), itemLen);
                        currU += 1;
                        }

                }
                

///////////////////////////////////////////////////////////////////
//JGH: change this:
//
//
#if 0
        // MetaIO stuff here
        float elSpacing = 1.0;
        vtkmetaio::MetaImage *metaIm = 
            new vtkmetaio:: MetaImage 
                        (
                        1, 
                        (int *) &tableLength, 
                        &elSpacing, 
                        (outputTableType == LDT_FLOAT) ? 
                            vtkmetaio::MET_FLOAT : vtkmetaio::MET_UCHAR,
                        1, 
                        dataPtr 
                        );
        // metaIm->BinaryData (false);
        metaIm->BinaryData (true);
        int rtn = 0;
        if ( ! metaIm->Write (fileName.c_str()) )
                {
                dtkMsg.add (DTKMSG_ERROR, 
                  "RgbaLut1D: "
                  "Error in MetaIO library while writing file %s.\n", 
                  fileName.c_str());
                rtn = -1;
                }


        delete metaIm;
#else
        int rtn = 0;
        if (writeMetaImage 
                    (    
                        fileName, 
                        BINARY, 
                        1, 
                        (int *) &tableLength, 
                        1, 
                        1, 
                        (outputTableType == LDT_FLOAT) ? DT_FLOAT : DT_UCHAR, 
                        dataPtr
                    )           )
            {
            dtkMsg.add (DTKMSG_ERROR, 
                  "RgbaLut1D: Error while writing file %s.\n", 
                  fileName.c_str());
            rtn = -1;
            }
#endif

/////////////////////////////////////////////////////////////////

        free (dataPtr);

        return rtn;
        }  // end of int writeAlphaTexture (std::string const & )


int VolVis::RgbaLut1D::writeRGBATexture (std::string const & fileName)
        {

        if (buildTables())
                {
                return -1;
                }

        int itemLen = (outputTableType == LDT_FLOAT) ? 
                                sizeof (float) : sizeof (unsigned char) ;
        void * dataPtr = malloc ( 4 * itemLen * tableLength);
        if (dataPtr == NULL)
                {
                dtkMsg.add (DTKMSG_ERROR, 
                  "RgbaLut1D: "
                  "Unable to allocate space for output RGB texture.\n");
                return -1;
                }

        float *currF = (float *) dataPtr;
        unsigned char *currU = (unsigned char *) dataPtr;
        for (int i = 0; i < tableLength; i++)
                {
                if (outputTableType == LDT_FLOAT)
                        {
                        memcpy (currF, rgbTable_f[i].rgb, 3*itemLen);
                        currF += 3;
                        memcpy (currF, &(alphaTable_f[i]), itemLen);
                        currF += 1;
                        }
                else
                        {
                        memcpy (currU, rgbTable_uc[i].rgb, 3*itemLen);
                        currU += 3;
                        memcpy (currU, &(alphaTable_uc[i]), itemLen);
                        currU += 1;
                        }

                }
                
///////////////////////////////////////////////////////////////
// JGH change this

#if 0
        // MetaIO stuff here
        float elSpacing = 1.0;
        vtkmetaio::MetaImage *metaIm = 
            new vtkmetaio:: MetaImage 
                        (
                        1, 
                        (int *) &tableLength, 
                        &elSpacing, 
                        (outputTableType == LDT_FLOAT) ? 
                            vtkmetaio::MET_FLOAT : vtkmetaio::MET_UCHAR,
                        4, 
                        dataPtr 
                        );
        // metaIm->BinaryData (false);
        metaIm->BinaryData (true);
        int rtn = 0;
        if ( ! metaIm->Write (fileName.c_str()) )
                {
                dtkMsg.add (DTKMSG_ERROR, 
                  "RgbaLut1D: "
                  "Error in MetaIO library while writing file %s.\n", 
                  fileName.c_str());
                rtn = -1;
                }


        delete metaIm;
#else
        int rtn = 0;
        if (writeMetaImage 
                    (    
                        fileName, 
                        BINARY, 
                        1, 
                        (int *) &tableLength, 
                        4, 
                        1, 
                        (outputTableType == LDT_FLOAT) ? DT_FLOAT : DT_UCHAR, 
                        dataPtr
                    )           )
            {
            dtkMsg.add (DTKMSG_ERROR, 
                  "RgbaLut1D: Error while writing file %s.\n", 
                  fileName.c_str());
            rtn = -1;
            }
#endif
/////////////////////////////////////////////////////////

        free (dataPtr);

        return rtn;
        }  // end of int writeRGBATexture (std::string const & )


int VolVis::RgbaLut1D::getTextureRanges (
  int n, 
  float *range
  )
        {
        int nChan;
        int nDim;
        std::string glslVarName;
        return getTextureInfo (n, glslVarName, nChan, nDim, range);
        } // end of getTextureRanges

int VolVis::RgbaLut1D::getTextureInfo (
  int n, 
  std::string & glslVarName,
  int & nChan, 
  int & nDim
  )
        {
        float range[2];

        return getTextureInfo (n, glslVarName, nChan, nDim, range);

        }  // end of getTextureInfo (int, string, int &, int &);




int VolVis::RgbaLut1D::getTextureInfo (
  int n, 
  std::string & glslVarName,
  int & nChan, 
  int & nDim, 
  float range[2]
  )
        {
        if (prepForGLSL())
                {
                return -1;
                }

        if ((n < 0) || (n >= numTextures))
                {
                dtkMsg.add (DTKMSG_ERROR, 
                  "RgbaLut1D::getTextureInfo: "
                  "Texture index %d is out of bounds.\n", n);
                return -1;
                }

        nDim = 1;

        if (rgbLU && alphaLU && combineTables)
                {
                nChan = 4;
                range[0] = xformVal(combinedRange[0]);
                range[1] = xformVal(combinedRange[1]);
                glslVarName = rgbaLutName;
                }
        else if (rgbLU && alphaLU)
                {
                if (n == 0)
                        {
                        nChan = 3;
                        range[0] = xformVal(rgbTableRange[0]);
                        range[1] = xformVal(rgbTableRange[1]);
                        glslVarName = rgbLutName;
                        }
                else if (n == 1)
                        {
                        nChan = 1;
                        range[0] = xformVal(alphaTableRange[0]);
                        range[1] = xformVal(alphaTableRange[1]);
                        glslVarName = alphaLutName;
                        }
                else
                        {
                        dtkMsg.add (DTKMSG_ERROR, 
                          "RgbaLut1D::getTextureInfo: "
                          "Internal error in texture info generation.\n");
                        return -1;
                        }
                }
        
        else if (rgbLU)
                {
                nChan = 3;
                range[0] = xformVal(rgbTableRange[0]);
                range[1] = xformVal(rgbTableRange[1]);
                glslVarName = rgbLutName;
                }
        else
                {
                nChan = 1;
                range[0] = xformVal(alphaTableRange[0]);
                range[1] = xformVal(alphaTableRange[1]);
                glslVarName = alphaLutName;
                }

        if (doNormalization && (range[0] == range[1]) )
                {

                // dtkMsg.add (DTKMSG_WARN, 
                //        "Error: Range of values has zero length.\n");

                // This should only occur if there is a table lookup but
                // only one interp point.
                if (range[0] == 0)
                        {
                        range[1] = 1;
                        }
                else 
                        {
                        range[1] = - range[0];
                        }
                }

        return 0;
        }  // end int getTextureInfo(int, std::string &, int &, int &, float[2])


int VolVis::RgbaLut1D::writeTexture (int n, std::string const & fileName)
        {
        if (prepForGLSL())
                {
                return -1;
                }

        if ((n < 0) || (n >= numTextures))
                {
                dtkMsg.add (DTKMSG_ERROR, 
                  "RgbaLut1D::writeTexture: "
                  "Texture index %d is out of bounds.\n", n);
                return -1;
                }

        if (rgbLU && alphaLU && combineTables)
                {
                return writeRGBATexture (fileName);
                }
        else if (rgbLU && alphaLU)
                {
                if (n == 0)
                        {
                        return writeRGBTexture (fileName);
                        }
                else if (n == 1)
                        {
                        return writeAlphaTexture (fileName);
                        }
                }
        
        else if (rgbLU)
                {
                return writeRGBTexture (fileName);
                }
        else
                {
                return writeAlphaTexture (fileName);
                }


        // shouldn't get here
        return 0;

        } // end of int writeTexture (int n, std::string const & fileName)


double VolVis::RgbaLut1D::xformVal (double v)
        {
        return VolVis::transformValue (v, valueTransform);
        }


///////////////////////////////////////////////////

