
#ifndef _RGBA_LUT_1D_
#define _RGBA_LUT_1D_ 1

#include <string>
#include <vector>
#include <map>

#include "VolVis.h"


namespace VolVis {



class RgbaLut1D: public TransferFunction 
        {

    public:

        RgbaLut1D();
        ~RgbaLut1D ();

        // read from file and write to file
        int read (FILE * fp, std::string declarationLine = "");
        int write (FILE * fp);
        int read (char *fileName);
        // int write (char *fileName);
        int write (std::string const & fileName);

        int writeGLSL (FILE *fp);
        // int writeGLSL (char *fileName);
        int writeGLSL (std::string const & fileName);

        int getNumTextures ();
        // int writeTexture (int n, char *fileName);
        int writeTexture (int n, std::string const & fileName);

        int getTextureInfo (int n, std::string & glslVarName,
                            int & nChan, int & nDim);
        int getTextureRanges (int n, float *ranges);


        // Set characteristics of LUT

        // Set to initial state: no LUT, all characteristics set to defaults.
        int reset ();           

        int setToDefaultLut ();


#if 0
        int setLighting (LightingModel lm);
        LightingModel getLighting () const;
#endif

        int setColorInterp (ColorInterpType interpT);
        ColorInterpType getColorInterp () const;

        int setLookupInterp (LookupInterpType interpT);
        LookupInterpType getLookupInterp () const;

        int setValTransform (ValueTransformType vXform);
        ValueTransformType getValTransform ()   const;

        int setTableLength (unsigned int len);
        unsigned int getTableLength () const;

        int setTableType (LutDataType type);
        LutDataType getTableType () const;

        int setCombineTables (bool combine);
        bool getCombineTables () const;

        int setPreIntegrate (bool preIntegrate);
        bool getPreIntegrate ();

        int setNormalize (bool doNormalize);
        bool getNormalize ();

        int buildTables ();

        int setInterpPtRGB (double v, const float rgb[3]);
        int setInterpPtA (double v, const float a);
        int setInterpPtRGBA (double v, const float rgba[4]);

        int getNumInterpPtsRGB () const;
        int getNumInterpPtsA () const;
        int getNumInterpPtsRGBA () const;

        int getInterpPtRGB ( int iPtIndex, double & v, float rgb[3]) const;
        int getInterpPtA ( int iPtIndex, double & v, float & a) const;
        int getInterpPtRGBA ( int iPtIndex, double & v, float rgba[4]) const;

        int removeInterpPtRGBA (int iPtIndex);
        int removeInterpPtRGB (int iPtIndex);
        int removeInterpPtA (int iPtIndex);

        int removeAllInterpPtsA ();
        int removeAllInterpPtsRGB ();
        int removeAllInterpPts ();

        int lookup (double v, float rgba[4]);

        int remapRGBA (const double limits[2]);
        int remapRGB (const double limits[2]);
        int remapA (const double limits[2]);

        int getTableElementRGBA (int index, float rgba[4]);
        int getTableElementA (int index, float & a );
        int getTableElementRGB (int index, float rgb[3]);

        // const TFTexture & getTexture (int i) const;

    protected:

        ColorInterpType colInterpType;
        LookupInterpType alphaInterpType;
        ValueTransformType valueTransform;
#if 0
        LightingModel lightingModel;
#endif
        bool combineTables;
        unsigned int tableLength;
        
        LutDataType outputTableType;
        bool recalcTableRGB;
        bool recalcTableA;

        std::vector <VAL_RGB> rgbInterpPts;
        std::vector <VAL_ALPHA> alphaInterpPts;
        

        float rgbTableRange[2];
        std::vector <RGB_UC> rgbTable_uc;
        std::vector <RGB_F> rgbTable_f;

        float alphaTableRange[2];
        std::vector <unsigned char> alphaTable_uc;
        std::vector <float> alphaTable_f;

        bool doNormalization;

        bool preIntegrate;

        // Some auxiliary protected methods:

        void interpRGB (ColorInterpType interpType, float p,
             float lowerRgb[3], float upperRgb[3], float interpRgb[3]);

        void interpAlpha (LookupInterpType interpType, float p,
             float lowerA, float upperA, float & interpA);

        int buildTableRGB ();
        int buildTableA ();

        double xformVal(double v);

        // these are for reading
        int parseFuncDecl ( std::vector<std::string> tokVec,
                            std::string & funcName );
        int parseFuncBody (FILE *fp);

        int parseRGBLUT (FILE *fp);
        int parseALPHALUT (FILE *fp);

        int strToValXform (std::string & str);
        int strToColorInterp (std::string & str);
        int strToLookupInterp (std::string & str);
        int strToCombineRGBA (std::string & str);
        int strToTableType (std::string & str);
        int strToTableLen (std::string & str);
        int strToType (std::string & str);
        int strToDoNorm (std::string & str);
        int strToNormRange (std::string & s0, std::string & s1);
        int strToPreIntegrate (std::string & str);
#if 0
        LightingModel strToLightingModel (std::string & str);
        int strToLightingModelRead (std::string & str);

        // these are for writing
        std::string toStr (LightingModel lm);
#endif

        int prepForGLSL ();   // prepare for outputting GLSL and .mhas
        int getTextureInfo (int n, std::string & glslVarName,
                            int & nChan, int & nDim, float range[2]);
        // the following data items are used only by prepForGLSL and writeGLSL
        bool rgbLU;     // are we doing an rgb lookup?
        bool alphaLU;     // are we doing an alpha lookup?
        // if combineTables is true, then we have one LUT, unless there is
        // only one element in each (rgb and a) list of interp points.
        std::string rgbaLutName;
        std::string rgbLutName;
        std::string alphaLutName;
        double combinedRange[2];

        int numTextures;

        int writeRGBTexture (std::string const & fileName);
        int writeRGBATexture (std::string const & fileName);
        int writeAlphaTexture (std::string const & fileName);


        } ;  // end of class RgbaLut1D

}  // end of namespace VolVis


#endif
//_RGBA_LUT_1D_ 


