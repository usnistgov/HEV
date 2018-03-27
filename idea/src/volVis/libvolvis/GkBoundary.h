

#ifndef _GK_BOUNDARY_
#define _GK_BOUNDARY_

#include <string>
#include <vector>
#include <map>

#include "VolVis.h"


/*
 * V is data value
 * G is gradient magnitude
 * H is second directional derivative along gradient direction
 *
 * File format:
 * 	HVOL_V_RANGE default f:1
 * 	HVOL_G_RANGE default p:0.005
 * 	HVOL_H_RANGE default p:0.015
 * 	HVOL_DIM  default 256 256 256
 * 	HVOL_K0  value kernel, default "tent"
 * 	HVOL_K1  first deriv kernel, default "cubicd:1,0"
 * 	HVOL_K2  second deriv kernel, default "cubicdd:1,0"
 * 	INFO_PROJECT_H  how to project along second deriv axis: default "mean"
 * 	INFO_DIM  1 or 2, default is 1
 * 	OPAC_BDRY_FUNC  shape width center maxAlpha  default: 1 1 0 1
 * 	OPAC_SIGMA    default is 0 which means automatic
 * 	OPAC_GTHRESH  min significant grad mag; default is 0.04*max(gradMag)
 * 	OPAC_RADIUS   radius of median filtering for opac func; def 0
 *
 *  
 * Then need to specify an RGB LUT:
 * 	VAL_TRANSFORM
 * 	COLOR_INTERP
 * 	RGB_TABLE_LEN  (maybe just make this identical to HVOL_DIM[0])
 * 	RGBLUT
 * 	END_RGBLUT
 *
 *  INPUT_VOL  input volume file on which to base generation of 
 *             RGB and ALPHA tables.  If not specified, then the
 *             output textures cannot be generated.
 *
 * Do we need a superclass method that provides a way of specifying
 * the input volume?  Many classes would not need it, but that would
 * just mean that they could ignore the information.
 *
 * How about this:
 *
 * 		bool inputDataUsed (int i = 0);  // tells the caller whether the
 * 		                                 // class will use input # i
 * 		bool setInputDataFileName (string fileName, int i = 0);
 *
 * And maybe the superclass needs a method that provides
 * a sub-class independent way of changing the contents
 * of the TF.  Perhaps a method like:
 *
 *      modifyParam (string & paramName, string & paramData)
 *
 * paramData could contain multiple items with whitespace delimited tokens.
 *
 */


namespace VolVis {



class GkBoundary: public TransferFunction
        {

    public:
        GkBoundary ();
        ~GkBoundary ();

        int read (FILE * fp, std::string declarationLine = "");
        int write (FILE * fp);
        int read (char *fileName);

        int write (std::string const & fileName);

        int writeGLSL (FILE *fp);
        int writeGLSL (std::string const & fileName);

        int getNumTextures ();
        int writeTexture (int n, std::string const & fileName);

        int getTextureInfo (int n, std::string & glslVarName,
                            int & nChan, int & nDim);
        int getTextureRanges (int n, float *ranges);

        // Set to initial state: all characteristics set to defaults.
        int reset ();

        // For now, I'm just going to handle the setting up of the internal
        // state by reading in a file.  I will implement methods to set
        // the state as they are needed.

        int setInterpPtRGB (double v, const float rgb[3]);

    protected:

        ColorInterpType colorInterpType;
#if 0
        LightingModel lightingModel;
#endif
        std::vector <VAL_RGB> rgbInterpPts;

        std::string hvolRangeV;
        std::string hvolRangeG;
        std::string hvolRangeH;
        int hvolDim[3];
        std::string hvolKernels[3];

        std::string infoProjectH;
        int infoDim;

        float opacBdryFunc[4];
        bool opacSigmaIsAutomatic; // calc from data
        float opacSigma;  // <=0 means automatic
        bool opacGthreshIsRelative; // relative to max gradient mag
        float opacGthresh;
        float opacRadius;

        bool texturesPrepared;


        int parseFuncDecl (std::vector<std::string> tokVec, 
                                        std::string & funcName);
        int parseRGBLUT (FILE *fp);
        int parseFuncBody (FILE * fp);

        int prepForGLSL ();
        int prepTextures ();
        
        int buildTableRGB (float *tableRGB, int tableLength);
        int buildTableRGB (unsigned char *tableRGB, int tableLength);
        void interpRGB (ColorInterpType interpType, float p,
                 float lowerRgb[3], float upperRgb[3], float interpRgb[3]);

        std::string rgbaLutName;
        std::string rgbLutName;
        std::string alphaLutName;
        std::string gradMagVolName;

        } ;  // end of class GkBoundary

}  // end of namespace VolVis




#endif 
// _GK_BOUNDARY_


