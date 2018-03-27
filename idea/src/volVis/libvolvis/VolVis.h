
#ifndef _VOLVIS_H_
#define _VOLVIS_H_ 1

#include <string>
#include <vector>

namespace VolVis {


enum DataType 
  {
  DT_UNSPECIFIED, 
  DT_CHAR, 
  DT_UCHAR, 
  DT_SHORT, 
  DT_USHORT, 
  DT_INT, 
  DT_UINT, 
  DT_LONG, 
  DT_ULONG, 
  DT_FLOAT, 
  DT_DOUBLE
  };


enum LightingModel
  {
  LM_UNKNOWN,
  LM_EMIT_ONLY,
  LM_GRAD_OPAQUE
  };



int dataTypeLength (DataType t);
const std::string dataTypeScanfFmt (DataType t);
const std::string dataTypePrintfFmt (DataType t);

enum DataEncoding { ASCII, BINARY };

class TFTexture 
        {

    public:
        TFTexture ( const std::string & name, 
                    int nDim, int nChan, int dimSizes[], 
                    DataType t, void *dataPtr );

        ~TFTexture ();


        int getNDim ();
        int getNChan ();
        int getDimSize (int dim);
        int getDimSizes (int dimSizes[]);
        DataType getDataType ();
        void *getDataPtr ();
        std::string getName ();

        // for normalization of lookup coords
        int setNormRange (int dim, const double range[2]); 
        int getNormRange (int dim, double range[2]); 

        int writeMha (const std::string & fileName);
        const std::string & getMhaFileName ();
        int addInfo (std::string info);

    protected:
        std::string name;
        int nDim;
        int nChan;
        int * dimSizes;
        DataType dataType;
        int dataLength;
        void * dataPtr;
        

        typedef double NormRange[2];
        NormRange * normRanges;

        std::vector<std::string> infoStrs;

        std::string mhaFileName;
        };  // end of class TFTexture





// This class describes an external reference that is required by a
// transfer function.  Everything is encoded in a string.
class TFReqExt
{
    public:
        const std::string objType;    // either "globalVar" or "func"
        const std::string dataType;   // the type of the req
        const std::string stdName;    // the standard name of the req
        std::string glslName;         // the name to be used in GLSL
        std::vector<std::string> infoStrs;  // additional associated info

        // Note that all of the members are const except for 
        // the glslName. So the glslName is the only member that can
        // be changed after construction.

        TFReqExt (      std::string stdNm, 
                        std::string dataT, 
                        std::string glslNm, 
                        std::string objT    ) : stdName(stdNm), 
                                                dataType (dataT), 
                                                objType(objT)
                {
                glslName = glslNm;
                }
};  // end of class TFReqExt





class TransferFunction 
        {

    public:
        TransferFunction ();
        ~TransferFunction ();

        virtual const std::string & getTFTypeName () const;
        virtual const std::string & getFuncName () const;
        virtual int setFuncName (const std::string name);

        virtual int read (FILE * fp, std::string declarationLine="") = 0;
        virtual int read (char * fileName);
        virtual int write (FILE * fp) = 0;
        // virtual int write (char * fileName);
        virtual int write (std::string const & fileName);

        virtual int writeGLSL (FILE * fp) = 0;
        // virtual int writeGLSL (char * fileName);
        virtual int writeGLSL (std::string const & fileName);

        virtual int getNumTextures () = 0;
        // virtual int writeTexture (int n, char *fileName);
        virtual int writeTexture (int n, std::string const & fileName) = 0;
#if 0
        virtual int getTextureInfo (int n, std::string & glslVarName, 
                                    int & nChan, int & nDim, float range[2]) =0;
#endif
        virtual int getTextureInfo (int n, std::string & glslVarName, 
                                                  int & nChan, int & nDim) =0;
        virtual int getTextureRanges (int nTxtr, float *ranges) = 0;

        virtual  std::map <std::string, TFReqExt *> const & 
                                                      getRequiredExts () const;


        // Reports whether input data is used by this TF
        virtual bool inputDataUsed (int i = 0);
        // set the file name of input data; returns true for success
        virtual bool setInputDataFileName (std::string fileName, int i = 0);

        // virtual const TFTexture & getTexture (int i) const = 0;


    protected:

        std::string funcName;
        std::string funcTypeName;

        std::map <std::string, TFReqExt *> requiredExts;
        std::vector <TFTexture *> textures;

        int numInputDataFilesUsed;
        std::map <int, std::string> inputDataFNs;


        };  // end of class TransferFunction

enum LutDataType
  {
  LDT_UNKNOWN, LDT_UCHAR, LDT_FLOAT
  };




class RGB_UC
        {
        public:
        RGB_UC(){};
        ~RGB_UC(){};
        unsigned char rgb[3];
        } ;

class RGB_F
        {
        public:
        RGB_F(){};
        ~RGB_F(){};
        float rgb[3];
        } ;

class VAL_RGB 
        {
        public:
        VAL_RGB(){};
        ~VAL_RGB(){};
        double val;
        float rgb[3];
        } ;

class VAL_ALPHA  
        {
        public:
        VAL_ALPHA(){};
        ~VAL_ALPHA(){};
        double val;
        float alpha;
        } ;




enum ColorInterpType 
  {
  C_INTERP_UNKNOWN,
  C_INTERP_NEAREST,
  C_INTERP_RGB,
  C_INTERP_HSV,
  C_INTERP_CIELAB,
  C_INTERP_DIVERGING
  };

enum LookupInterpType
  {
  L_INTERP_UNKNOWN,
  L_INTERP_NEAREST,
  L_INTERP_LINEAR
  };




enum ValueTransformType
  {
  VALX_UNKNOWN,
  VALX_NONE,
  VALX_LOG,
  VALX_LOG2,
  VALX_LOG10,
  VALX_EXP,
  VALX_EXP2,
  VALX_EXP10
  // others??
  };  





////////////////////////////
//
// Utility routines

int executeCmd (const std::string & cmd, 
                std::string & cmdStdOut, int & exitStatus);


// Some read utilities

// These are generic readers that will read any kind of transfer function
TransferFunction * readTF (std::string const fileName);
TransferFunction * readTF (FILE *fp);

// Some utilites used by the reader functions
std::vector <std::string> getTokens (char *line);

int
getNextLineTokens (
  FILE *fp,
  std::vector<std::string> & tokVec,
  char *fullLine = NULL,
  int maxFullLen = 80);

int getVar (
  const std::string & token, 
  std::string &varName, 
  unsigned int & varLen );

int parseFuncDeclaration (
  std::vector<std::string> tokVec,
  std::string & lfuncTypeName, 
  std::string & lfuncName );



// Reading various keywords
ValueTransformType strToValXform (std::string & str);
ColorInterpType strToColorInterp (std::string & str);
LookupInterpType strToLookupInterp (std::string & str);
LutDataType strToLDType (std::string & str);
LightingModel strToLightingModel (std::string & str);

// return true on success
bool strToValXform (std::string & str, ValueTransformType & t);
bool strToColorInterp (std::string & str, ColorInterpType & t);
bool strToLookupInterp (std::string & str, LookupInterpType & t);
bool strToLDType (std::string & str, LutDataType & t);
bool strToLightingModel (std::string & str, LightingModel  & t);



// Wrting various keywords
std::string toStr (ValueTransformType t);
std::string toStr (ColorInterpType t);
std::string toStr (LookupInterpType t);
std::string toStr (LutDataType t);
std::string toStr (LightingModel lm);
int strToBool (std::string & str, bool & rtn);


// some write utilities
void badArgCountMsg (char *line);
void resetIndent ();
void incrementIndent ();
void decrementIndent ();
void printIndent (FILE *fp);

////////////////////////////


double transformValue (double val, ValueTransformType xform);


}  // end of namespace VolVis





#endif

// _VOLVIS_H_


