
#include <iostream>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dtk.h>
#include <iris.h>


#include "VolVis.h"
#include "RgbaLut1D.h"
#include "GkBoundary.h"
#include "MetaImg.h"



//////////////////////////////////////////////////////////////////////////


int VolVis::dataTypeLength (DataType t)
    {
    int len = 0;

    switch (t)
        {
        case DT_UNSPECIFIED:
            len = 0;
            break;

        case DT_CHAR:
            len = sizeof (char);
            break;

        case DT_UCHAR:
            len = sizeof (unsigned char);
            break;

        case DT_SHORT:
            len = sizeof (short);
            break;

        case DT_USHORT:
            len = sizeof (unsigned short);
            break;

        case DT_INT:
            len = sizeof (int);
            break;

        case DT_UINT:
            len = sizeof (unsigned int);
            break;

        case DT_LONG:
            len = sizeof (long);
            break;

        case DT_ULONG:
            len = sizeof (unsigned long);
            break;

        case DT_FLOAT:
            len = sizeof (float);
            break;

        case DT_DOUBLE:
            len = sizeof (double);
            break;

        }  // end of switch over data type


    return len;
    }  // end of VolVis::dataTypeLength (DataType t)


const std::string VolVis::dataTypeScanfFmt (DataType t)
    {
    const char *fmt;

    fmt = "no format";

    switch (t)
        {
        case DT_UNSPECIFIED:
            fmt = "no format";
            break;

        case DT_CHAR:
            fmt = "%hhd";
            break;

        case DT_UCHAR:
            fmt = "%hhu";
            break;

        case DT_SHORT:
            fmt = "%hd";
            break;

        case DT_USHORT:
            fmt = "%hu";
            break;

        case DT_INT:
            fmt = "%d";
            break;

        case DT_UINT:
            fmt = "%u";
            break;

        case DT_LONG:
            fmt = "%ld";
            break;

        case DT_ULONG:
            fmt = "%lu";
            break;

        case DT_FLOAT:
            fmt = "%f";
            break;

        case DT_DOUBLE:
            fmt = "%lf";
            break;

        }  // end of switch over data type

    return fmt;
    } // end of const std::string & VolVis::dataTypeScanfFmt (DataType t);


const std::string VolVis::dataTypePrintfFmt (DataType t)
    {
    const char *fmt;

    fmt = "no format";

    switch (t)
        {
        case DT_UNSPECIFIED:
            fmt = "no format";
            break;

        case DT_CHAR:
            fmt = "%hhd";
            break;

        case DT_UCHAR:
            fmt = "%hhu";
            break;

        case DT_SHORT:
            fmt = "%hd";
            break;

        case DT_USHORT:
            fmt = "%hu";
            break;

        case DT_INT:
            fmt = "%d";
            break;

        case DT_UINT:
            fmt = "%u";
            break;

        case DT_LONG:
            fmt = "%ld";
            break;

        case DT_ULONG:
            fmt = "%lu";
            break;

        case DT_FLOAT:
            fmt = "%.9g";
            break;

        case DT_DOUBLE:
            fmt = "%.17g";
            break;

        }  // end of switch over data type


    return fmt;
    } // end of const std::string & VolVis::dataTypePrintfFmt (DataType t);





//////////////////////////////////////////////////////////////////////////

VolVis::TransferFunction::TransferFunction ()
        {
        funcTypeName = "NO_TF";
        funcName = "noName";
        numInputDataFilesUsed = 0;
        } // end of TransferFunction::TransferFunction()


VolVis::TransferFunction::~TransferFunction ()
        {
        // nothing needed ???? JGH
        // any possible memory leaks?
        } // end of TransferFunction::~TransferFunction()


const std::string & VolVis::TransferFunction::getTFTypeName () const
        {
        return funcTypeName;
        } // end of const std::string&TransferFunction::getTFTypeName () const

const std::string & VolVis::TransferFunction::getFuncName () const
        {
        return funcName;
        } // end of const std::string&TransferFunction::getFuncName () const

int VolVis::TransferFunction::setFuncName (const std::string name)
        {
        funcName = name;
        return 0;
        }  // end of int hev::transferFunction::setFuncName (const std::string)


#if 0
int VolVis::TransferFunction::getNumTextures () 
        {
        return 0;
        } // end of int VolVis::TransferFunction::getNumTextures () 

#if 1
int VolVis::TransferFunction::writeTexture (int n, std::string const & fileName) 
        {
        dtkMsg.add (DTKMSG_ERROR,
           "TransferFunction::writeTexture: "
           "No textures defined; cannot write file %s.\n", fileName.c_str());
        return -1;
        }  // end of writeTexture
#endif
#endif

int VolVis::TransferFunction::read (char *fileName)
        {
        FILE *fp = fopen (fileName, "r");

        if (fp == NULL)
                {
                dtkMsg.add (DTKMSG_ERROR,
                                "Error reading transfer function definition file: "
                                "Unable to open file %s.\n",
                                fileName);
                return -1;
                }

        int rtn = read (fp);
        fclose (fp);

        return rtn;
        } // end of int VolVis::TransferFunction::read (char *fileName)

#if 1
int VolVis::TransferFunction::write (std::string const & fileName)
        {
        FILE *fp = fopen (fileName.c_str(), "w");

        if (fp == NULL)
                {
                dtkMsg.add (DTKMSG_ERROR,
                                "Error writing transfer function definition file: "
                                "Unable to open file %s.\n",
                                fileName.c_str());
                return -1;
                }

        int rtn = write (fp);
        fclose (fp);

        return rtn;
        } // end of int VolVis::TransferFunction::write (char *fileName)

int VolVis::TransferFunction::writeGLSL (std::string const & fileName)
        {
        FILE *fp = fopen (fileName.c_str(), "w");

        if (fp == NULL)
                {
                dtkMsg.add (DTKMSG_ERROR,
                                "Error writing transfer function definition file: "
                                "Unable to open file %s.\n",
                                fileName.c_str());
                return -1;
                }

        int rtn = write (fp);
        fclose (fp);

        return rtn;
        } // end of int VolVis::TransferFunction::writeGLSL (std::string )
#endif

std::map <std::string, VolVis::TFReqExt *> const & 
VolVis::TransferFunction::getRequiredExts () const
        {
        return requiredExts;
        } // end of getRequiredExts



// Reports whether input data is used by this TF
bool VolVis::TransferFunction::inputDataUsed (int i)
    {
    return (i >= 0) && (i < numInputDataFilesUsed);
    }  // end of bool VolVis::TransferFunction::inputDataUsed (int i = 0)
  
// set the file name of input data; returns true for success
bool VolVis::TransferFunction::setInputDataFileName (std::string fileName, int i)
    {
    bool rtn;
    if ( (i >= 0) && (i < numInputDataFilesUsed) )
        {
        inputDataFNs[i] = fileName;
        rtn = true;
        }
    else
        {
        rtn = false;
        }

    return rtn;
    } // end of bool VolVis::TransferFunction::setInputDataFileName(string ,int)


VolVis::TransferFunction * VolVis::readTF (FILE *fp)
        {
        char line[1000];
        std::vector<std::string> tokVec;

        // line with first set of tokens
        if (getNextLineTokens (fp, tokVec, line, sizeof(line)))
                {
                // there are no tokens left in file
                return NULL;
                }

        if (tokVec[0] != "FUNC")
            {
            dtkMsg.add (DTKMSG_ERROR,
              "Error reading transfer function file; bad declaration line:\n"
              "      <%s>\n", 
              line);
            return NULL;
            }

        if (tokVec.size() < 3)
            {
            dtkMsg.add (DTKMSG_ERROR,
              "Error reading transfer function file; bad declaration line:\n"
              "      <%s>\n", 
              line);
            return NULL;
            }

        VolVis::TransferFunction *tf;

        if (tokVec[1] == "RgbaLut1D")
            {
            tf = new VolVis::RgbaLut1D;
            }
        else if (tokVec[1] == "GkBoundary")
            {
            tf = new VolVis::GkBoundary;
            }
        else
            {
            dtkMsg.add (DTKMSG_ERROR,
              "Unrecognized transfer function type %s found in file.\n", 
              tokVec[1].c_str());
            return NULL;
            }


        if (tf->read (fp, line))
            {
            dtkMsg.add (DTKMSG_ERROR,
                        "Error reading body of transfer function file.\n");
            delete tf;
            tf = NULL;
            }

        return tf;
        } // end of VolVis::TransferFunction * VolVis::readTF (FILE *fp)


VolVis::TransferFunction * VolVis::readTF (std::string const fileName)
        {
        FILE *fp = fopen (fileName.c_str(), "r");

        if (fp == NULL)
                {
                dtkMsg.add (DTKMSG_ERROR,
                                "Error reading transfer function: "
                                "Unable to open TF file %s.\n",
                                fileName.c_str());
                return NULL;
                }

        VolVis::TransferFunction * tf = VolVis::readTF (fp);
        fclose (fp);

        return tf;
        } // end of VolVis::TransferFunction * VolVis::readTF (std::string const )



///////////////////////////////////////////////////////////////
//
// Here are some utility routines

// First the read utilities

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

std::vector <std::string> 
VolVis::getTokens (char *line)
        {
        removeComment (line);
        //removeCommas (line);

        return iris::ParseString (line);

        } // end of getTokens

int
VolVis::getNextLineTokens (
  FILE *fp,
  std::vector<std::string> & tokVec,
  char *fullLine,
  int maxFullLen)
        {
        char line[1000];

        while (fgets (line, sizeof(line), fp) != NULL)
                {
                tokVec = VolVis::getTokens (line);
                if (tokVec.size() > 0)
                        {
                        if (fullLine != NULL)
                                {
                                strncpy (fullLine, line, maxFullLen-1);
                                fullLine[maxFullLen-1] = 0;
                                }
                        return 0;
                        }
                }

        return -1;
        } // end of getNextLineTokens

int
VolVis::getVar (
  const std::string & token, std::string &varName, 
  unsigned int & varLen
  )
        {

        // parsing "varName[varLen]"

        varName.clear ();

        std::string lenStr;


        int iPos = 0;

        for (iPos = 0; iPos < token.size(); iPos++)
                {
                if (token[iPos] == '[')
                        {
                        break;
                        }
                // varName[iPos] = token[iPos];
                varName += token[iPos];
                }


        iPos++;
        int lPos = 0;
        for ( ; iPos < token.size(); iPos++)
                {
                if (token[iPos] == ']')
                        {
                        break;
                        }
                lenStr += token[iPos];
                lPos++;
                }
                

        if ( (varName.length() == 0) || (lenStr.length() == 0) )
                {
                dtkMsg.add (DTKMSG_ERROR, 
                        "Parsing Error: "
                        "bad format for variable: %s.\n", token.c_str());
                return -1;
                }


        if ( ! iris::StringToUInt (lenStr, & varLen) )
                {
                dtkMsg.add (DTKMSG_ERROR, 
                        "Parsing Error: "
                        "Unable to interpret variable length: %s.\n", 
                        token.c_str());
                return -1;
                }

        return 0;
        }  // end of getVar(const std::string &, std::string &, unsigned int & )


int
VolVis::parseFuncDeclaration (
  std::vector<std::string> tokVec,
  std::string & lfuncTypeName, 
  std::string & lfuncName )
        {


        if ( (tokVec.size() < 3) || (tokVec.size() > 3) )
                {
                dtkMsg.add (DTKMSG_ERROR, 
                        "Error in parsing transfer function description: "
                        "bad format for function declaration.\n");
                return -1;
                }

        if (tokVec[0] != "FUNC")
                {
                dtkMsg.add (DTKMSG_ERROR, 
                        "Error in parsing transfer function description: "
                        "function declaration does not begin with FUNC.\n");
                return -1;
                }

        lfuncTypeName = tokVec[1];

        lfuncName = tokVec[2];

        return 0;
        }  // end of parseFuncDeclaration


/////////////////

VolVis::LightingModel VolVis::strToLightingModel (std::string & str)
        {
        LightingModel lm;

        if (iris::IsSubstring ("EMIT_ONLY", str))
                {
                lm = LM_EMIT_ONLY;
                }
        else if (iris::IsSubstring ("GRAD_OPAQUE", str))
                {
                lm = LM_GRAD_OPAQUE;
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

        } // end of VolVis::LightingModel VolVis::strToLightingModel (std::string & )

VolVis::ValueTransformType VolVis::strToValXform (std::string & str)
        {
        ValueTransformType vx;
        if (iris::IsSubstring ("none", str))
                {
                vx = VALX_NONE;
                }
        else if (iris::IsSubstring ("LOG", str))
                {
                vx = VALX_LOG;
                }
        else if (iris::IsSubstring ("LOG2", str))
                {
                vx = VALX_LOG2;
                }
        else if (iris::IsSubstring ("LOG10", str))
                {
                vx = VALX_LOG10;
                }
        else if (iris::IsSubstring ("EXP", str))
                {
                vx = VALX_EXP;
                }
        else if (iris::IsSubstring ("EXP2", str))
                {
                vx = VALX_EXP2;
                }
        else if (iris::IsSubstring ("EXP10", str))
                {
                vx = VALX_EXP10;
                }
        else
                {
                dtkMsg.add (DTKMSG_ERROR, 
                            "Parsing Error: "
                            "Bad value transform: %s.\n",
                            str.c_str());
                vx = VALX_UNKNOWN;
                }


        return vx;
        }  // end of strToValXform

VolVis::ColorInterpType VolVis::strToColorInterp (std::string & str)
        {
        ColorInterpType ci;

        if (iris::IsSubstring ("NEAREST", str))
                {
                ci = C_INTERP_NEAREST;
                }
        else if (iris::IsSubstring ("RGB", str))
                {
                ci = C_INTERP_RGB;
                }
        else if (iris::IsSubstring ("HSV", str))
                {
                ci = C_INTERP_HSV;
                }
        else if (iris::IsSubstring ("CIELAB", str))
                {
                ci = C_INTERP_CIELAB;
                }
        else if (iris::IsSubstring ("LAB", str))
                {
                ci = C_INTERP_CIELAB;
                }
        else if (iris::IsSubstring ("DIVERGING", str))
                {
                ci = C_INTERP_DIVERGING;
                }
        else
                {
                dtkMsg.add (DTKMSG_ERROR, 
                            "Parsing Error: "
                            "Bad color interpolation specifier: %s.\n",
                            str.c_str());
                ci = C_INTERP_UNKNOWN;
                }

        return ci;
        }  // end of strToColorInterp


VolVis::LookupInterpType VolVis::strToLookupInterp (std::string & str)
        {
        LookupInterpType li;
        if (iris::IsSubstring ("NEAREST", str))
                {
                li = L_INTERP_NEAREST;
                }
        else if (iris::IsSubstring ("LINEAR", str))
                {
                li = L_INTERP_LINEAR;
                }
        else 
                {
                li = L_INTERP_UNKNOWN;
                }

        return li;
        }  // end of strToLookupInterp


int VolVis::strToBool (std::string & str, bool & rtn)
        {
        if (iris::IsSubstring ("true", str))
                {
                rtn = true;
                }
        else if (iris::IsSubstring ("false", str))
                {
                rtn = false;
                }
        else 
                {
                dtkMsg.add (DTKMSG_ERROR, 
                            "Parsing Error: "
                            "Bad boolean specifier: %s.\n",
                            str.c_str());
                return -1;
                }


        return 0;

        }  // end of strToBool



VolVis::LutDataType VolVis::strToLDType (std::string & str)
        {
        LutDataType t;
        if (iris::IsSubstring ("UCHAR", str))
                {
                t = LDT_UCHAR;
                }
        else if (iris::IsSubstring ("FLOAT", str))
                {
                t = LDT_FLOAT;
                }
        else 
                {
                dtkMsg.add (DTKMSG_ERROR, 
                                "Parsing Error: "
                                "Bad LUT data type; %s.\n",
                                str.c_str());
                t = LDT_UNKNOWN;
                }

        return t;
        }  // end of strToLDType


// return true on success
bool VolVis::strToValXform (std::string & str, VolVis::ValueTransformType & t)
    {
    return ( (t = strToValXform (str)) != VALX_UNKNOWN );
    } // end of bool strToValXform (std::string & str, ValueTransformType & t)


bool VolVis::strToColorInterp (std::string & str, VolVis::ColorInterpType & t)
    {
    return ( (t = strToColorInterp (str)) != C_INTERP_UNKNOWN );
    } // end of bool strToColorInterp (std::string & str, ColorInterpType & t)


bool VolVis::strToLookupInterp (std::string & str, VolVis::LookupInterpType & t)
    {
    return ( (t = strToLookupInterp (str)) != L_INTERP_UNKNOWN );
    } // end of bool strToLookupInterp (std::string & str, LookupInterpType & t)


bool VolVis::strToLDType (std::string & str, VolVis::LutDataType & t)
    {
    return ( (t = strToLDType (str)) != LDT_UNKNOWN );
    } // end of bool strToLDType (std::string & str, LutDataType & t)


bool VolVis::strToLightingModel (std::string & str, VolVis::LightingModel  & t)
    {
    return ( (t = strToLightingModel (str)) != LM_UNKNOWN );
    } // end of bool strToLightingModel (std::string & str, LightingModel  & t)




void VolVis::badArgCountMsg (char *line)
        {
        dtkMsg.add (DTKMSG_ERROR, 
            "Parsing Error: Bad token count on line:\n     %s\n",
            line);
        }  // end of badArgCountMsg


////////////////////////////////////////////////////////////////////
//
// the write methods

static int indent = 0;

void VolVis::resetIndent ()
        {
        indent = 0;
        }  // end of resetIndent ();

void VolVis::incrementIndent ()
        {
        indent += 2;
        }  // end of incrementIndent

void VolVis::decrementIndent ()
        {
        indent -= 2;
        if (indent < 0)
                {
                indent = 0;
                }
        }

void VolVis::printIndent (FILE *fp)
        {
        for (int i = 0; i < indent; i++)
                {
                fprintf (fp, " ");
                }
        }
                

std::string VolVis::toStr (ValueTransformType t)
        {
        switch (t)
                {
                case VALX_NONE:
                        return "NONE";

                case VALX_LOG:
                        return "LOG";

                case VALX_LOG2:
                        return "LOG2";

                case VALX_LOG10:
                        return "LOG10";

                case VALX_EXP:
                        return "EXP";

                case VALX_EXP2:
                        return "EXP2";

                case VALX_EXP10:
                        return "EXP10";

                default:
                        dtkMsg.add (DTKMSG_ERROR, 
                          "Invalid value transform type found (%d).\n", t);
                        return "NONE";

                }


        }  // end of std::string VolVis::toStr (ValueTransformType)

std::string VolVis::toStr (ColorInterpType t)
        {
        switch (t)
                {
                case C_INTERP_NEAREST:
                        return "NEAREST";

                case C_INTERP_RGB:
                        return "RGB";

                case C_INTERP_HSV:
                        return "HSV";

                case C_INTERP_CIELAB:
                        return "CIELAB";

                case C_INTERP_DIVERGING:
                        return "DIVERGING";

                default:
                        dtkMsg.add (DTKMSG_ERROR, 
                          "Invalid color interpolation type found (%d).\n", t);
                        return "UNKNOWN";

                }


        }  // end of std::string VolVis::toStr (ColorInterpType)

std::string VolVis::toStr (LookupInterpType t)
        {
        switch (t)
                {
                case L_INTERP_NEAREST:
                        return "NEAREST";

                case L_INTERP_LINEAR:
                        return "LINEAR";

                default:
                        return "UNKNOWN";

                }


        }  // end of std::string VolVis::toStr (LookupInterpType)

std::string VolVis::toStr (LutDataType t)
        {
        switch (t)
                {
                case LDT_UCHAR:
                        return "UCHAR";

                case LDT_FLOAT:
                        return "FLOAT";

                default:
                        dtkMsg.add (DTKMSG_ERROR, 
                          "Invalid lookup table data type found (%d).\n", t);
                        return "UNKNOWN";

                }


        }  // end of std::string VolVis::toStr (LutDataType )

std::string VolVis::toStr (VolVis::LightingModel lm)
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

        }  // end of std::string VolVis::toStr (VolVis::LightingModel lm)


////////////////////////////
double VolVis::transformValue (double val, ValueTransformType t)
        {
        switch (t)
                {
                case VALX_NONE:
                        return val;

                case VALX_LOG:
                        return log(val);

                case VALX_LOG2:
                        return log(val)/log(2.0);

                case VALX_LOG10:
                        return log(val)/log(10.0);

                case VALX_EXP:
                        return exp (val);

                case VALX_EXP2:
                        return pow (2.0, val);

                case VALX_EXP10:
                        return pow (10.0, val);

                default:
                        dtkMsg.add (DTKMSG_ERROR, 
                          "Invalid value transform type found (%d).\n", t);
                        return val;

                }


        }  // end of double VolVis::transformValue (double val, ValueTransformType t)






////////////////////////////

// Implementation of TFTexture

VolVis::TFTexture::TFTexture (
  const std::string & nm, 
  int nD, 
  int nC, 
  int dimSzs[], 
  DataType t, 
  void *dataP
  )
    {
    name = nm;
    nDim = nD;
    nChan = nC;
    dataType = t;
    dataPtr = dataP;
    // mhaFileName get constructed as an empty string

    dimSizes = new int[nDim];
    normRanges = new NormRange[nDim];

    dataLength = dataTypeLength (dataType) * nChan;
    for (int i = 0; i < nDim; i++)
        {
        dimSizes[i] = dimSzs[i];
        normRanges[i][0] = 0;
        normRanges[i][1] = 0;
        dataLength *= dimSizes[i];
        }
    } // end of TFTexture constructor

VolVis::TFTexture::~TFTexture ()
    {
    delete dimSizes;
    delete normRanges;

    // the caller is responsible for dataPtr delete
    // possible memory leak ??

    }  // end of destructor


std::string VolVis::TFTexture::getName ()
    {
    return name;
    }  // end of std::string VolVis::TFTexture::getName ()

int VolVis::TFTexture::getNDim ()
    {
    return nDim;
    }  // end of int VolVis::TFTexture::getNDim ()

int VolVis::TFTexture::getNChan ()
    {
    return nChan;
    }  // end of int VolVis::TFTexture::getNChan ()

int VolVis::TFTexture::getDimSize (int dim)
    {
    if ((dim < 0) || (dim >= nDim))
        {
        return 0;
        }
    return dimSizes[dim];
    }  // end of int VolVis::TFTexture::getDimSize (int dim)

int VolVis::TFTexture::getDimSizes (int dimSzs[])
    {
    for (int i = 0; i < nDim; i++)
        {
        dimSzs[i] = dimSizes[i];
        }
    return 0;
    }  // end of int VolVis::TFTexture::getDimSizes (int dimSzs[])

VolVis::DataType VolVis::TFTexture::getDataType ()
    {
    return dataType;
    }  // end of DataType VolVis::TFTexture::getDataType ()


int VolVis::TFTexture::getNormRange (int dim, double range[2])
    {
    if ((dim < 0) || (dim >= nDim))
        {
        return -1;
        }
    range[0] = normRanges[dim][0];
    range[1] = normRanges[dim][1];
    return 0;
    }  // end of int VolVis::TFTexture::getNormRange (int dim, double *range)

int VolVis::TFTexture::setNormRange (int dim, const double range[2])
    {
    if ((dim < 0) || (dim >= nDim))
        {
        return -1;
        }
    normRanges[dim][0] = range[0];
    normRanges[dim][1] = range[1];
    return 0;
    }  // end of int VolVis::TFTexture::getNormRange (int dim, const double *range)


int VolVis::TFTexture::writeMha (const std::string & fileName)
    {
    
    if (writeMetaImage (fileName, BINARY, nDim, dimSizes, nChan, 
                        1, // mipLevel
                        dataType, dataPtr, &infoStrs))
        {
        return -1;
        }

    mhaFileName = fileName;
    return 0;
    } // end of int writeMha (const std::string & fileName)


const std::string & VolVis::TFTexture::getMhaFileName ()
    {
    return mhaFileName;
    } // end of const std::string & getMhaFileName ()


int VolVis::TFTexture::addInfo (std::string info)
    {
    std::stringstream ss (info);
    std::string line;
    while(std::getline (ss, line))
        {
        infoStrs.push_back(line);
        }

    return 0;
    } // end of const std::string & VolVis::TFTexture::addInfo (std::string info)

////////////////////////////

int VolVis::executeCmd (const std::string & cmd, 
                        std::string & cmdStdOut, int & exitStatus)
    {
    FILE *fp;

    // printf ("cmd = %s\n", cmd.c_str());

    if ( (fp = popen (cmd.c_str(), "r")) == NULL)
        {
        return -1;
        }

    cmdStdOut = "";
    char line[1000];
    while (fgets(line, sizeof(line), fp) != NULL)
        {
        cmdStdOut += line;
        }

    exitStatus = pclose (fp);
    return 0;
    } // end of executeCmd


