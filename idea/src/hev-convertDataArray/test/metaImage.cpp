//
// The header is very strictly formatted.  
// Each line must have the form:
// KeyWord = [value(s)]
//
// The keywords must start at the beginning of the line, followed by a space,
// followed by '=' followed by a space, followed by the value or values.
//
// Extra whitespace before the '=' is not allowed, and the keywords are
// case sensitive.
// 
//
// Keywords handled:
//    ObjectType = Image   	       must be "Image"
//    NDims = N                        number of dimensions; max of 10
//    DimSize = i j k ...              number of elements in each dimension
//    ElementType = MET_XXX            data type of elements.  Must be one of:
//                                       MET_CHAR
//                                       MET_UCHAR
//                                       MET_SHORT
//                                       MET_USHORT
//                                       MET_INT
//                                       MET_UINT
//                                       MET_LONG
//                                       MET_ULONG
//                                       MET_FLOAT
//                                       MET_DOUBLE
//
//    ElementNumberOfChannels = N     number of channels of data, default is 1
//
//    BinaryData = True | False       is data stored in binary or ascii
//
//    ElementByteOrderMSG = True | False
//    BinaryDataByteOrderMSG = True | False  Byte ordering of binary data.
//                                           True means big-endian.
//                                           Default is False.
//                                           Documentation is unclear about the 
//                                           difference between these two fields.
//                                           This software treats these the same on input
//                                           and on output uses BinaryDataByteOrderMSG.
//
//    ElementDataFile = fileName | LOCAL    Must be the last line in header.
//                                          If "LOCAL", then the data follows immediated
//                                          after this line.
//                                          If it's a file name.
//
//    Comment = anything
//    Comment = Levels = N          This is a non-standard extension to specify
//                                  number of MIP levels
//
//
//
// Header lines of any other form are ignored.  No error message is generated 
// when unrecognized header lines are encountered.
//
// In particular, these keywords are ignored:
//    HeaderSize = N # skip N bytes at beginning of data file (-1 not handled)
//	ElelmentSpacing not handled
//	ElementSize
//    SequenceID
//    ElementMin
//    ElementMax
//    ElementSize
//    ObjectSubType
//    TransformType
//    Name
//    ID
//    ParentID
//    COlor
//    Position
//    Orientation
//    AnatomicalOrientation
//    Modality

typedef enum data_type 
          { UCHAR, 
            USHORT, 
            UINT, 
            ULONG, 
            CHAR,   
            SHORT,  
            INT,  
            LONG, 
            FLOAT, 
            DOUBLE }   DataType;

typedef enum file_encoding { ASCII, BINARY } FileEncoding;





int
readMetaImage
  (
  std::string const & inFN,
  int & nDim, 
  int * & dims, 
  int & nComp, 
  DataType & dType,
  void * & dataPtr
  );

int
writeMetaImage 
  (
  std::string const & outFN,
  FileEncoding encoding, 
  const int nDim,
  const int dim[],
  int nComp,
  const DataType dType,
  void *data
  );












#include <stdio.h> 
#include <stdlib.h>
#include <float.h>
#include <math.h>

#include <dtk.h>

#include <iris/Utils.h>

#include <vtkmetaio/metaImage.h>

#define MIN(x,y)        (((x)<(y))?(x):(y))
#define MAX(x,y)        (((x)>(y))?(x):(y))

// http://www.itk.org/Wiki/MetaIO/Documentation

static void
usage ()
        {
        fprintf (stderr, "Usage: hev-convertDataArray [options] inFN outFN\n");
        }  // end of usage 





////////////////////////////////////////////////////////////////////

enum fmt { ASCII, BINARY };
typedef enum fmt Fmt;

enum transform { NO_XFORM, LOG, EXP, LOG10, EXP10 };
typedef enum transform XForm;

enum file_type { F_RAW, F_META };
typedef enum file_type FileType;


#define MAX_DIM (1000)  // max of 1000 dimensions

typedef struct data_file_desc
        {
        int nDim;
        int dim[MAX_DIM]; 
        int nComp;
        vtkmetaio::MET_ValueEnumType type;
        Fmt fmt;  // ascii bin
        std::string fileName;
        FileType fileType;  // mha or raw

        // some derived items
        int totalNumVals;  // product of dimensions and nComp
        int totalDataLen;  // number of bytes needed for data

        void *dataPtr;
        }  DataFileDesc;


typedef struct dataXforms
        {
        bool doClamp;
        double clampVal[2];

        XForm xform;

        bool doNormalize;
        double normVal[2];

        bool doInfSub[2];
        std::string infSub[2];
        float infSubF[2];
        double infSubD[2];

        bool doNanSub;
        std::string nanSub;
        float nanSubF;
        double nanSubD;
        }  DataTransformations;
        

static int 
strToFmt (char const * str, Fmt & f)
        {

        if ( iris::IsSubstring ("ascii", str,  1) )
                {
                f = ASCII;
                return 0;
                }

        if ( iris::IsSubstring ("binary", str, 1) )
                {
                f = BINARY;
                return 0;
                }


        return -1;
        }  // end of strToFmt


        

                d = vtkmetaio::MET_UCHAR;
                d = vtkmetaio::MET_USHORT;
                d = vtkmetaio::MET_UINT;
                d = vtkmetaio::MET_CHAR;
                d = vtkmetaio::MET_SHORT;
                d = vtkmetaio::MET_INT;
                d = vtkmetaio::MET_FLOAT;
                d = vtkmetaio::MET_DOUBLE;







static int 
strToXform (char const * str, XForm & x)
        {

        if ( iris::IsSubstring ("none", str, 0) )
                {
                x = NO_XFORM;
                }
        else if ( iris::IsSubstring ("log", str, 0) )
                {
                x = LOG;
                }
        else if ( iris::IsSubstring ("exp", str, 0) )
                {
                x = EXP;
                }
        else if ( iris::IsSubstring ("log10", str, 0) )
                {
                x = LOG10;
                }
        else if ( iris::IsSubstring ("exp10", str, 0) )
                {
                x = EXP10;
                }
        else
                {
                x = NO_XFORM;
                return -1;
                }

        return 0;

        }  // end of strToXform
        
        
static FileType
fileNameToType (std::string const & fn)
        {
        int len = fn.length();
        return 
          (fn.substr (len-4,4) == ".mha") || (fn.substr (len-4,4) == ".mhd") ?
          F_META : F_RAW;
        } // end of fnIsMetaImg


static int
parseCmdLine 
  (
  int argc, 
  char **argv,
  DataFileDesc & inDesc,
  DataFileDesc & outDesc,
  DataTransformations & dataXforms,
  int & nInputs,
  char ** & inputFileNameList
  )
        {
        int i;

        nInputs = 0;
        inputFileNameList = (char **) malloc (argc * sizeof (char *));
        bool * argMask = (bool *) malloc (argc * sizeof(bool));
        if ( (inputFileNameList == NULL) || (argMask == NULL) )
                {
                dtkMsg.add (DTKMSG_ERROR, 
                  "Unable to allocate memory for input file name list.\n");
                return -1;
                }
        argMask[0] = true;
        for (i = 1; i < argc; i++)
                {
                argMask[i] = false;
                }


        inDesc.nDim = 1;
        inDesc.dim[0] = -1;
        inDesc.nComp = 1;
        inDesc.fmt = ASCII;
        inDesc.type = vtkmetaio::MET_FLOAT;
        inDesc.fileName = "";
        inDesc.fileType = F_RAW;



        outDesc.fmt = BINARY;
        outDesc.type = vtkmetaio::MET_NONE;
        outDesc.fileName = "";

        dataXforms.xform = NO_XFORM;
        dataXforms.doClamp = false;
        dataXforms.doNormalize = false;
        dataXforms.doNanSub = false;
        dataXforms.nanSub = "none";
        dataXforms.doInfSub[0] = false;
        dataXforms.doInfSub[1] = false;
        dataXforms.infSub[0] = dataXforms.infSub[1] = "";
        dataXforms.infSubD[0] = - DBL_MAX;
        dataXforms.infSubD[1] =   DBL_MAX;
        dataXforms.infSubF[0] = - FLT_MAX;
        dataXforms.infSubF[1] =   FLT_MAX;





        if (argc < 3)
                {
                dtkMsg.add (DTKMSG_ERROR, "Bad argument count.\n");
                return -1;
                }

        inDesc.fileName = argv[argc-2];
        outDesc.fileName = argv[argc-1];
        // argc -= 2;

        inDesc.fileType = fileNameToType (inDesc.fileName);
        outDesc.fileType = fileNameToType (outDesc.fileName);

        bool inSpec = false;
        bool nDimSpecified = false;

        // first we get the specified dimensions
        for (i = 1; i < argc-1; i++)
                {
                if ( iris::IsSubstring ("--nDim", argv[i], 4))
                        {
                        argMask[i] = true;
                        argMask[i+1] = true;
                        inSpec = true;
                        nDimSpecified = true;
                        if (sscanf (argv[i+1], "%d", &inDesc.nDim) != 1)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad value for --nDim (%s).\n", argv[i+1]);
                                return -1;
                                }

                        if (inDesc.nDim < 1)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Value for --nDim must be "
                                  "greater than 0 (%s).\n", argv[i+1]);
                                return -1;
                                }

                        if (inDesc.fileType != F_RAW)
                                {
                                dtkMsg.add (DTKMSG_WARN, 
                                  "Input file type is not raw; "
                                  "arg %s will be ignored.\n", argv[i]);
                                }

                        }
                }

        if (inDesc.nDim < 1)
                {
                dtkMsg.add (DTKMSG_ERROR, 
                                  "No value for --nDim (%s).\n", argv[i+1]);
                return -1;
                }

        for (i = 0; i < inDesc.nDim; i++)
                {
                inDesc.dim[i] = -1;
                }

        bool dimSpecified = false;

        for (i = 1; i < argc; i++)
                {
                // printf ("i = %d  argv[i] = %s\n", i, argv[i]);

                if ( iris::IsSubstring ("--nDim", argv[i], 4))
                        {
                        // we handled it above
                        i++;
                        }

                else if ( iris::IsSubstring ("--dim", argv[i], 3) )
                        {
                        argMask[i] = true;
                        inSpec = true;
                        if ( ! nDimSpecified )
                                {
                                dtkMsg.add (DTKMSG_ERROR,
                                    "Command line argument --dim specified but "
                                    "--nDim not specified.\n");
                                return -1;
                                }

                        dimSpecified = true;
                        if ((i + inDesc.nDim) >= argc)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad arg count for --dim.\n");
                                return -1;
                                }
                        for (int j = 0; j < inDesc.nDim; j++)
                            {
                            argMask[i+j+1] = true;
                            if (sscanf (argv[i+j+1], "%d", inDesc.dim+j) != 1)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad value for --dim %d (%s).\n", 
                                                i, argv[i+1]);
                                return -1;
                                }
                            if (inDesc.dim[j] < 1)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad value for --dim %d (%s).\n", 
                                                i, argv[i+1]);
                                return -1;
                                }
                            }
                        if (inDesc.fileType != F_RAW)
                                {
                                dtkMsg.add (DTKMSG_WARN, 
                                  "Input file type is not raw; "
                                  "arg %s will be ignored.\n", argv[i]);
                                }
                        i += inDesc.nDim;
                        }  // end of --dim

                else if ( iris::IsSubstring ("--nComp", argv[i], 4) )
                        {
                        inSpec = true;
                        if ((i + 1) >= argc)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad arg count for --nComp.\n");
                                return -1;
                                }
                        argMask[i] = true;
                        argMask[i+1] = true;
                        if (sscanf (argv[i+1], "%d", &inDesc.nComp) != 1)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad value for --nComp (%s).\n", argv[i+1]);
                                return -1;
                                }
                        if (inDesc.fileType != F_RAW)
                                {
                                dtkMsg.add (DTKMSG_WARN, 
                                  "Input file type is not raw; "
                                  "arg %s will be ignored.\n", argv[i]);
                                }
                        i++;
                        } // end of --nComp


                else if ( iris::IsSubstring ("--inFmt", argv[i], 5) )
                        {
                        inSpec = true;

                        if ((i + 1) >= argc)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad arg count for --inFmt.\n");
                                return -1;
                                }

                        argMask[i] = true;
                        argMask[i+1] = true;
                        if (strToFmt (argv[i+1], inDesc.fmt))
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad value for --inFmt (%s).\n", argv[i+1]);
                                return -1;
                                }
                        if (inDesc.fileType != F_RAW)
                                {
                                dtkMsg.add (DTKMSG_WARN, 
                                  "Input file type is not raw; "
                                  "arg %s will be ignored.\n", argv[i]);
                                }
                        i++;
                        }  // end of --inFmt


                else if ( iris::IsSubstring ("--inType", argv[i], 5) )
                        {
                        inSpec = true;
                        if ((i + 1) >= argc)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad arg count for --inType.\n");
                                return -1;
                                }

                        argMask[i] = true;
                        argMask[i+1] = true;
                        if (strToType(argv[i+1], inDesc.type))
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad value for --inType (%s).\n", argv[i+1]);
                                return -1;
                                }
                        if (inDesc.fileType != F_RAW)
                                {
                                dtkMsg.add (DTKMSG_WARN, 
                                  "Input file type is not raw; "
                                  "arg %s will be ignored.\n", argv[i]);
                                }
                        i++;
                        }  // end of --inType



                else if ( iris::IsSubstring ("--outFmt", argv[i], 6) )
                        {

                        if ((i + 1) >= argc)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad arg count for --outFmt.\n");
                                return -1;
                                }

                        argMask[i] = true;
                        argMask[i+1] = true;
                        if (strToFmt (argv[i+1], outDesc.fmt))
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad value for --outFmt (%s).\n", argv[i+1]);
                                return -1;
                                }
                        i++;
                        }  // end of --outFmt


                else if ( iris::IsSubstring ("--outType", argv[i], 6) )
                        {
                        if ((i + 1) >= argc)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad arg count for --outType.\n");
                                return -1;
                                }

                        argMask[i] = true;
                        argMask[i+1] = true;
                        if (strToType(argv[i+1], outDesc.type))
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad value for --outType (%s).\n", argv[i+1]);
                                return -1;
                                }
                        i++;
                        } // end of --outType


                else if ( iris::IsSubstring ("--transform", argv[i], 4) )
                        {
                        if ((i + 1) >= argc)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad arg count for --transform.\n");
                                return -1;
                                }
                        argMask[i] = true;
                        argMask[i+1] = true;
                        if (strToXform(argv[i+1], dataXforms.xform))
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                "Bad value for --transform (%s).\n", argv[i+1]);
                                return -1;
                                }
                        i++;
                        }  // end of --transform


                else if ( iris::IsSubstring ("--normalize", argv[i], 4) )
                        {

                        if ((i + 2) >= argc)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad arg count for --normalize.\n");
                                return -1;
                                }

                        argMask[i] = true;
                        argMask[i+1] = true;
                        argMask[i+2] = true;
                        dataXforms.doNormalize = true;
                        if (sscanf (argv[i+1], "%lf", 
                                               &(dataXforms.normVal[0])) != 1)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad value for --normalize (%s).\n", 
                                  argv[i+1]);
                                return -1;
                                }
                        if (sscanf (argv[i+2], "%lf", 
                                               &(dataXforms.normVal[1])) != 1)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad value for --normalize (%s).\n", 
                                  argv[i+1]);
                                return -1;
                                }

                        if (dataXforms.normVal[0] == dataXforms.normVal[1])
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Normalization range is zero length.\n");
                                return -1;
                                }
                        i += 2;
                        }  // end of normalize


                else if ( iris::IsSubstring ("--clamp", argv[i], 4) )
                        {

                        if ((i + 2) >= argc)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad arg count for --clamp.\n");
                                return -1;
                                }
                        argMask[i] = true;
                        argMask[i+1] = true;
                        argMask[i+2] = true;

                        dataXforms.doClamp = true;
                        if (sscanf (argv[i+1], "%lf", 
                                               &(dataXforms.clampVal[0])) != 1)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad value for --clamp (%s).\n", 
                                  argv[i+1]);
                                return -1;
                                }
                        if (sscanf (argv[i+2], "%lf", 
                                               &(dataXforms.clampVal[1])) != 1)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad value for --clamp (%s).\n", 
                                  argv[i+1]);
                                return -1;
                                }

                        if (dataXforms.clampVal[0] == dataXforms.clampVal[1])
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Clamp min and max are identical.\n");
                                return -1;
                                }
                        i += 2;
                        }  // end of clamp


                else if ( iris::IsSubstring ("--infSub", argv[i], 4) )
                        {
                        if ((i + 2) >= argc)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad arg count for --clamp.\n");
                                return -1;
                                }
                        argMask[i] = true;
                        argMask[i+1] = true;
                        argMask[i+2] = true;

                        // no check of validity until we know type of output
                        dataXforms.infSub[0] = argv[i+1];
                        dataXforms.infSub[1] = argv[i+2];

                        i += 2;
                        }  // end of --infSub

                else if ( iris::IsSubstring ("--nanSub", argv[i], 4) )
                        {
                        if ((i + 1) >= argc)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Bad arg count for --nanSub.\n");
                                return -1;
                                }
                        argMask[i] = true;
                        argMask[i+1] = true;

                        // no check of validity until we know type of output
                        dataXforms.nanSub = argv[i+1];

                        i++;
                        } // end of --nanSub

#if 0
                else
                        {
                        dtkMsg.add (DTKMSG_ERROR, 
                                "Unrecognized command line argument (%s).\n",
                                argv[i]);
                        return -1;
                        }
#endif

                }  // end of loop over arguments



        if ( (inDesc.fileType == F_RAW) && 
                                (inDesc.nDim != 1) && (!dimSpecified) )
            {
            // the args say that we have more than one dimension, but
            // not what the dimensions are.
            dtkMsg.add (DTKMSG_ERROR, 
               "Raw file specified as having %d dimensions, but the dimension "
               "            lengths (--dim) have not been specified.\n",
                            inDesc.nDim);
            return -1;
            }
        else if (dimSpecified)
            {
            inDesc.totalNumVals = inDesc.nComp;
            for (int i; i < inDesc.nDim; i++)
                {
                inDesc.totalNumVals *= inDesc.dim[i];
                }
            }

        // now we figure out how many input file names we have

        int firstFN = -1;
        for (i = argc-1; i > 0; i--)
                {
                if (argMask[i])
                        {
                        break;
                        }
                firstFN = i;
                }

        if (firstFN == -1)
                {
                dtkMsg.add (DTKMSG_ERROR, "No input files specified.\n");
                return -1;
                }

        if (firstFN == argc-1)
                {
                dtkMsg.add (DTKMSG_ERROR, "No output file specified.\n");
                return -1;
                }

        

        nInputs = 0;
        for (i = firstFN; i < argc-1; i++)
                {
                if ( iris::IsSubstring (argv[i], "--", 2) )
                        {
                        dtkMsg.add (DTKMSG_ERROR, 
                           "Unrecognized command line argument: %s.\n", 
                           argv[i]);
                        return -1;
                        }
                inputFileNameList[nInputs] = argv[i];
                nInputs++;
                }

        for (i = 0; i < firstFN; i++)
                {
                if ( ! argMask[i] )
                        {
                        dtkMsg.add (DTKMSG_ERROR, 
                           "Unrecognized command line argument: %s.\n", 
                           argv[i]);
                        return -1;
                        }
                }

        return 0;
        }  // end of parseCmdLine



////////////////////////////////////////////////////////////////////////////


static int
readChars (FILE *inFP, int numVal, 
                vtkmetaio::MET_ValueEnumType type, void *inDataPtr)
        {
        
        if (type == vtkmetaio::MET_UCHAR)
                {
                unsigned char *uPtr = (unsigned char *) inDataPtr;
                for (int i = 0; i < numVal; i++)
                        {
                        unsigned int uVal;
                        if (fscanf (inFP, "%u", &uVal) != 1)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Unable to read unsigned char "
                                  "value at position %d.\n", i);
                                return -1;
                                }



                        if (uVal > 255)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Value (%u) out of range for "
                                  "unsigned char at "
                                  "position %d.\n", uVal, i);
                                return -1;
                                }
                        *uPtr = uVal;
                        uPtr++;
                        }
                }
        else if (type == vtkmetaio::MET_CHAR)
                {
                char *cPtr = (char *) inDataPtr;
                for (int i = 0; i < numVal; i++)
                        {
                        int iVal;
                        if (fscanf (inFP, "%i", &iVal) != 1)
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Unable to read signed char "
                                  "value at position %d.\n", i);
                                return -1;
                                }

                        if ( (iVal > 127) || (iVal < -128) )
                                {
                                dtkMsg.add (DTKMSG_ERROR, 
                                  "Value (%u) out of range for "
                                  "signed char at "
                                  "position %d.\n", iVal, i);
                                return -1;
                                }
                        *cPtr = iVal;
                        cPtr++;
                        }
                }
        else
                {
                dtkMsg.add (DTKMSG_ERROR, 
                   "Internal error in readChars: bad type specifier.\n");
                return -1;
                }

        return 0;
        } // end of readChars

static int
getTypeLen (vtkmetaio::MET_ValueEnumType type)
        {
        int len;
        

        switch (type)
                {
                case vtkmetaio::MET_CHAR:
                        len = sizeof (char);
                        break;

                case vtkmetaio::MET_UCHAR:
                        len = sizeof (unsigned char);
                        break;

                case vtkmetaio::MET_SHORT:
                        len = sizeof (short);
                        break;

                case vtkmetaio::MET_USHORT:
                        len = sizeof (unsigned short);
                        break;

                case vtkmetaio::MET_INT:
                        len = sizeof (int);
                        break;

                case vtkmetaio::MET_UINT:
                        len = sizeof (unsigned int);
                        break;

                case vtkmetaio::MET_FLOAT:
                        len = sizeof (float);
                        break;

                case vtkmetaio::MET_DOUBLE:
                        len = sizeof (double);
                        break;

                default:
                        len = -1;
                }

        return len;

        } // end of getTypeLen

static char *
getFmtStr (vtkmetaio::MET_ValueEnumType type)
        {
        char *fmtStr;

        switch (type)
                {
                case vtkmetaio::MET_CHAR:
                        fmtStr = "%i";
                        break;

                case vtkmetaio::MET_UCHAR:
                        fmtStr = "%u";
                        break;

                case vtkmetaio::MET_SHORT:
                        fmtStr = "%hi";
                        break;

                case vtkmetaio::MET_USHORT:
                        fmtStr = "%hu";
                        break;

                case vtkmetaio::MET_INT:
                        fmtStr = "%i";
                        break;

                case vtkmetaio::MET_UINT:
                        fmtStr = "%u";
                        break;

                case vtkmetaio::MET_FLOAT:
                        fmtStr = "%f";
                        break;

                case vtkmetaio::MET_DOUBLE:
                        fmtStr = "%lf";
                        break;

                default:
                        fmtStr = "";
                }

        return fmtStr;
        }  // end of getFmtStr

static int
readRawInput (
  FILE *inFP, 
  int & numVal, 
  Fmt & fmt, 
  vtkmetaio::MET_ValueEnumType & type, 
  void * & inDataPtr)
        {
        int len;
        char *fmtStr;

        len = getTypeLen (type);
        fmtStr = getFmtStr (type);

        inDataPtr = malloc (len * numVal);
        if ( inDataPtr == NULL)
                {
                dtkMsg.add (DTKMSG_ERROR, 
                   "Unable to allocate memory for input data.\n");
                return -1;
                }


        if (fmt == BINARY)
                {
                if (fread (inDataPtr, len, numVal, inFP) != numVal)
                        {
                        dtkMsg.add (DTKMSG_ERROR, 
                                "Error reading input data.\n");
                        return -1;
                        }
                }  // end of if binary
        else
                {
                if ( (type == vtkmetaio::MET_CHAR) || 
                     (type == vtkmetaio::MET_UCHAR)          )
                        {
                        if (readChars (inFP, numVal, type, inDataPtr))
                                {
                                return -1;
                                }
                        }
                else
                        {
                        unsigned char *currPtr = (unsigned char *) inDataPtr;
                        for (int i = 0; i < numVal; i++)
                                {
                                if (fscanf (inFP, fmtStr, currPtr) != 1)
                                    {
                                    dtkMsg.add (DTKMSG_ERROR, 
                                    "Error reading input data at position %d.\n"
                                    , i);
                                    return -1;
                                    }
                                currPtr += len;
                                }
                        }
                }

        return 0;

        }  // end of readRawInput (FILE *inFP

static int
readRawInput ( DataFileDesc & inDesc )
        {
        FILE *inFP;

        inFP = fopen (inDesc.fileName.c_str(), "r");
        if (inFP == NULL)
                {
                dtkMsg.add (DTKMSG_ERROR, 
                   "Error openning input data file %s.\n", 
                        inDesc.fileName.c_str());
                }

        
        int numElements = 0;
        if (inDesc.fmt == ASCII)
                {
                char dummy[1000];
                while (fscanf (inFP, "%100s", dummy) == 1)
                        {
                        numElements++;
                        }
                rewind (inFP);
                }
        else
                {
                fseek (inFP, 0L, SEEK_END);
                long fileLen = ftell (inFP);
                rewind (inFP);
                int l = getTypeLen (inDesc.type);
                numElements = fileLen / l;
                }

        if (inDesc.dim[0] == -1)
                {
                if (inDesc.nDim != 1)
                    {
                    dtkMsg.add (DTKMSG_ERROR, 
                     "Internal error: No dimensions present, but nDim = %d.\n", 
                     inDesc.nDim);
                    return -1;
                    }
                inDesc.totalNumVals = inDesc.dim[0] = numElements;
                }
        else
                {
                // check numElements against inDesc.totalNumVals
                inDesc.totalNumVals = inDesc.nComp;
                for (int i = 0; i < inDesc.nDim; i++)
                        {
                        inDesc.totalNumVals *= inDesc.dim[i];
                        }
                inDesc.totalDataLen = inDesc.totalNumVals * 
                                        getTypeLen (inDesc.type);
                if (numElements < inDesc.totalNumVals)
                    {
                    dtkMsg.add (DTKMSG_ERROR, 
                     "Number of elements in file %s (%d) is \nless than "
                      "the number of elements derived from arguments (%d).\n",
                      inDesc.fileName.c_str(), numElements, 
                      inDesc.totalNumVals);
                    return -1;
                    }
                else if (numElements > inDesc.totalNumVals)
                    {
                    dtkMsg.add (DTKMSG_WARN, 
                     "Number of elements in file %s (%d) is \ngreater than "
                      "the number of elements derived from arguments (%d).\n",
                      inDesc.fileName.c_str(), numElements, 
                      inDesc.totalNumVals);
                    }

                }
                
                

        int rtn = readRawInput (inFP, inDesc.totalNumVals, 
                        inDesc.fmt, inDesc.type, inDesc.dataPtr);



        fclose (inFP);

        return rtn;
        }  // end of readRawInput (string inDesc




static int
cvtVals 
  (
  int numVal, 
  vtkmetaio::MET_ValueEnumType inType, 
  void *inDataPtr, 
  vtkmetaio::MET_ValueEnumType outType, 
  void * outDataPtr
  )
        {
        int inLen = getTypeLen (inType);
        int outLen = getTypeLen (outType);


        if (inType == outType)
                {
                memcpy (outDataPtr, inDataPtr, numVal*inLen);
                return 0;
                }

        unsigned char *inPtr = (unsigned char *) inDataPtr;
        unsigned char *outPtr = (unsigned char *) outDataPtr;

        long l;
        double d;
        bool useLong;

        for (int i = 0; i < numVal; i++)
            {
                
            // do the conversion

            switch (inType)
                {
                case vtkmetaio::MET_CHAR:
                        useLong = true;
                        l = *((char *)inPtr);
                        break;

                case vtkmetaio::MET_UCHAR:
                        useLong = true;
                        l = *((unsigned char *)inPtr);
                        break;

                case vtkmetaio::MET_SHORT:
                        useLong = true;
                        l = *((short *)inPtr);
                        break;

                case vtkmetaio::MET_USHORT:
                        useLong = true;
                        l = *((unsigned short *)inPtr);
                        break;

                case vtkmetaio::MET_INT:
                        useLong = true;
                        l = *((int *)inPtr);
                        break;

                case vtkmetaio::MET_UINT:
                        useLong = true;
                        l = *((unsigned int *)inPtr);
                        break;

                case vtkmetaio::MET_FLOAT:
                        useLong = false;
                        d = *((float *)inPtr);
                        break;
                        
                case vtkmetaio::MET_DOUBLE:
                        useLong = false;
                        d = *((double *)inPtr);
                        break;

                }


            switch (outType)
                {
                case vtkmetaio::MET_CHAR:
                    *((char *)outPtr) = (char) (useLong ? l : round (d));
                    break;

                case vtkmetaio::MET_UCHAR:
                    *((unsigned char *)outPtr) = 
                                (unsigned char) (useLong ? l : round (d));
                    break;

                case vtkmetaio::MET_SHORT:
                    *((short *)outPtr) = 
                                (short) (useLong ? l : round (d));
                    break;

                case vtkmetaio::MET_USHORT:
                    *((unsigned short *)outPtr) = 
                                (unsigned short) (useLong ? l : round (d));
                    break;

                case vtkmetaio::MET_INT:
                    *((int *)outPtr) = 
                                (int ) (useLong ? l : round (d));
                    break;

                case vtkmetaio::MET_UINT:
                    *((unsigned int *)outPtr) = 
                                (unsigned int) (useLong ? l : round (d));
                    break;

                case vtkmetaio::MET_FLOAT:
                    *((float *)outPtr) = useLong ? l : d;
                    break;
                        
                case vtkmetaio::MET_DOUBLE:
                    *((double *)outPtr) = useLong ? l : d;
                    break;

                }

            inPtr += inLen;
            outPtr += outLen;
            }  // end of loop over values

        return 0;
        }  // end of cvtVals;


static int
resolveInfNanSubs (
  DataFileDesc & outDesc,
  DataTransformations & dataXforms
  )

        {
        // Check that the infSubs and nanSub are OK given the output type
        // We do this here because values that might work for double might 
        // not work for float.
        if (dataXforms.nanSub != "none")
                {
                if (outDesc.type == vtkmetaio::MET_FLOAT)
                        {
                        if (sscanf (dataXforms.nanSub.c_str(), "%f", 
                                                &dataXforms.nanSubF) != 1)
                            {
                            dtkMsg.add (DTKMSG_ERROR, 
                              "Bad value for float Nan substitution (%s) .\n",
                              dataXforms.nanSub.c_str());
                            usage ();
                            return -1;
                            }
                        dataXforms.nanSubD = dataXforms.nanSubF;
                        }
                else // if (outDesc.type == vtkmetaio::MET_DOUBLE)
                        {
                        // might need this even if output isn't float or double
                        if (sscanf (dataXforms.nanSub.c_str(), "%lf", 
                                                &dataXforms.nanSubD) != 1)
                            {
                            dtkMsg.add (DTKMSG_ERROR, 
                              "Bad value for double Nan substitution (%s) .\n",
                              dataXforms.nanSub.c_str());
                            usage ();
                            return -1;
                            }
                        }
                dataXforms.doNanSub = true;
                }
        
        for (int i = 0; i < 2; i++)
            {
            if (dataXforms.infSub[i] != "")
                {
                if (dataXforms.infSub[i] == "none")
                        {
                        dataXforms.doInfSub[i] = false;
                        }
                else if (outDesc.type == vtkmetaio::MET_FLOAT)
                        {
                        if (sscanf (dataXforms.infSub[i].c_str(), "%f", 
                                                &(dataXforms.infSubF[i]) ) != 1)
                            {
                            dtkMsg.add (DTKMSG_ERROR, 
                              "Bad value for float inf substitution (%s) .\n",
                              dataXforms.infSub[i].c_str());
                            usage ();
                            return -1;
                            }
                        dataXforms.doInfSub[i] = true;
                        dataXforms.infSubD[i] = dataXforms.infSubF[i];
                        }
                else if (outDesc.type == vtkmetaio::MET_DOUBLE)
                        {
                        if (sscanf (dataXforms.infSub[i].c_str(), "%lf", 
                                                &(dataXforms.infSubD[i]) ) != 1)
                            {
                            dtkMsg.add (DTKMSG_ERROR, 
                              "Bad value for float inf substitution (%s) .\n",
                              dataXforms.infSub[i].c_str());
                            usage ();
                            return -1;
                            }
                        dataXforms.doInfSub[i] = true;
                        }
                }
            }
                
        return 0;
        }  // resolveInfNanSubs 

static void
nanInfSub (DataTransformations & xforms, double & val)
        {
        if (xforms.doNanSub && isnan(val))
                {
                val = xforms.nanSubD;
                }

        int isInfVal = isinf(val); 
        if ( (isInfVal < 0) && xforms.doInfSub[0])
                {
                val = xforms.infSubD[0];
                }
        else if ( (isInfVal > 0) && xforms.doInfSub[1])
                {
                val = xforms.infSubD[1];
                }
        }  // end of nanSub double



static void
nanInfSub (DataTransformations & xforms, float & val)
        {
        if (xforms.doNanSub && isnan(val))
                {
                val = xforms.nanSubF;
                }

        int isInfVal = isinf(val); 
        if ( (isInfVal < 0) && xforms.doInfSub[0])
                {
                val = xforms.infSubF[0];
                }
        else if ( (isInfVal > 0) && xforms.doInfSub[1])
                {
                val = xforms.infSubF[1];
                }
        }  // end of nanSub double



static void
clamp (DataTransformations & xforms, double & val)
        {
        if (xforms.doClamp)
            {
            if ( isless (val, xforms.clampVal[0]) )
                {
                val = xforms.clampVal[0];
                }
            else if ( isgreater (val, xforms.clampVal[1]) )
                {
                val = xforms.clampVal[1];
                }
            }
        } // end of clamp


static void
normalize (DataTransformations & xforms, double & val)
        {

        bool forward =  isless (xforms.normVal[0], xforms.normVal[1]);

        if (xforms.doNormalize)
            {
            if ( isless (val, xforms.normVal[0]) )
                {
                val = forward ? 0 : 1;
                }
            else if ( isgreater (val, xforms.normVal[1]) )
                {
                val = forward ? 1 : 0;
                }
            else
                {
                val = (val-xforms.normVal[0]) / 
                        (xforms.normVal[1]-xforms.normVal[0]);
                }
            }

        }  // end of normalize

static int
transform (DataTransformations & xforms, double & val)
        {
        switch (xforms.xform)
            {
            case NO_XFORM:
                break;
            case LOG:
                val = log (val);
                break;
            case EXP:
                val = exp (val);
                break;
            case LOG10:
                val = log (val) / log(10.0) ;
                break;
            case EXP10:
                val = pow (10.0, val);
                break;

            default:
                dtkMsg.add (DTKMSG_ERROR, 
                  "Internal error: bad transform specification.\n");
                return -1;
            }

        return 0;
        } // end of transform


static int
cvtToOutputForm 
  (
  DataFileDesc & inDesc, 
  DataTransformations & dataXforms,
  DataFileDesc & outDesc
  )
        {


        // make output conform to input
        outDesc.nDim = inDesc.nDim;
        for (int i = 0; i < inDesc.nDim; i++)
                {
                outDesc.dim[i] = inDesc.dim[i];
                }
        outDesc.nComp = inDesc.nComp;
        outDesc.totalNumVals = inDesc.totalNumVals;
        if (outDesc.type == vtkmetaio::MET_NONE)
                {
                // output has same type only if it had not been specified
                outDesc.type = inDesc.type;
                }
        
        int dLen = getTypeLen (outDesc.type);
        outDesc.totalDataLen = outDesc.totalNumVals*dLen;
        outDesc.dataPtr = new unsigned char [outDesc.totalDataLen];

        if (resolveInfNanSubs (outDesc, dataXforms))
                {
                return -1;
                }



        if ( ( dataXforms.xform == NO_XFORM ) &&
             ( ! dataXforms.doClamp ) &&
             ( ! dataXforms.doNormalize ) &&
             ( ! dataXforms.doInfSub[0] ) &&
             ( ! dataXforms.doInfSub[1] ) &&
             ( ! dataXforms.doNanSub )            )
                {
                // no transformation is needed
                if (inDesc.type == outDesc.type) 
                        {
                        // no type conversion is needed
                        // there's nothing to do
                        outDesc.dataPtr = inDesc.dataPtr;
                        return 0;
                        }
                else
                        {
                        // only type conversion is needed
                        return cvtVals (inDesc.totalNumVals, inDesc.type, 
                             inDesc.dataPtr, outDesc.type, outDesc.dataPtr);
                        }
                }


        // Some data manipulation is needed.

        // Data manipulations will be done in double, then moved back to 
        // final output format.

        // convert to double
        double *xformData = new double [inDesc.totalNumVals];

        if (cvtVals (inDesc.totalNumVals, inDesc.type, inDesc.dataPtr, 
                                        vtkmetaio::MET_DOUBLE, xformData))
                {
                return -1;
                }

        // do the data transformations
        for (int i = 0; i < inDesc.totalNumVals; i++)
                {
                nanInfSub (dataXforms, xformData[i]);
                clamp (dataXforms, xformData[i]);
                transform (dataXforms, xformData[i]);
                nanInfSub (dataXforms, xformData[i]);
                normalize (dataXforms, xformData[i]);
                nanInfSub (dataXforms, xformData[i]);
                }


        // convert back to final output
        if (cvtVals (inDesc.totalNumVals, vtkmetaio::MET_DOUBLE, xformData, 
                        outDesc.type, outDesc.dataPtr ))
                {
                return -1;
                }

        // If the final output is float, then do the final nan/inf substitutions
        // on the float output.
        if (outDesc.type == vtkmetaio::MET_FLOAT)
            {
            for (int i = 0; i < inDesc.totalNumVals; i++)
                {
                nanInfSub (dataXforms, ((float*)outDesc.dataPtr)[i]);
                }
            }
        return 0;
        }  // end of cvtToOutputForm




static int
readMetaImage
  (
  std::string const & inFN,
  int & nDim, 
  int * dims, 
  int & nComp, 
  vtkmetaio::MET_ValueEnumType & type,
  void * & dataPtr
  )





        {
        char typeName[1000];

        vtkmetaio::MetaImage *metaIm = new vtkmetaio::MetaImage ();
        if ( ! metaIm->Read(inFN.c_str()) )
                {
                // does it fail on true or false
                dtkMsg.add (DTKMSG_ERROR, 
                        "Error reading MetaIO Image from file %s.\n", 
                        inFN.c_str() );
                return -1;
                }


        type = metaIm->ElementType();

        // useful from metaio lib:
        // bool MET_TypeToString(MET_ValueEnumType _type, char *_str);

        switch (type)
                {
                case vtkmetaio::MET_CHAR:
                case vtkmetaio::MET_UCHAR:
                case vtkmetaio::MET_SHORT:
                case vtkmetaio::MET_USHORT:
                case vtkmetaio::MET_INT:
                case vtkmetaio::MET_UINT:
                case vtkmetaio::MET_FLOAT:
                case vtkmetaio::MET_DOUBLE:
                        // all is well ; do nothing
                        break;



                case vtkmetaio::MET_ASCII_CHAR:
                case vtkmetaio::MET_LONG:
                        metaIm->ConvertElementDataTo (vtkmetaio::MET_INT);
                        break;

                case vtkmetaio::MET_ULONG:
                        metaIm->ConvertElementDataTo (vtkmetaio::MET_UINT);
                        break;


                case vtkmetaio::MET_LONG_LONG:
                case vtkmetaio::MET_ULONG_LONG:
                        MET_TypeToString(type, typeName);
                        dtkMsg.add (DTKMSG_WARN, 
                            "Input MetaImage file %s has data type %s.\n",
                            inFN.c_str(), typeName);
                        dtkMsg.add (DTKMSG_WARN, 
                            "Converting data to double which may cause "
                            "loss of precision.\n");
                        metaIm->ConvertElementDataTo (vtkmetaio::MET_DOUBLE);
                        break;


                default:
                        MET_TypeToString(type, typeName);
                        dtkMsg.add (DTKMSG_ERROR, 
                            "Input MetaImage file %s has data type %s, "
                            "which cannot be handled.\n", 
                            inFN.c_str(), typeName);
                        return -1;
        
                }  // end of case
                

        type = metaIm->ElementType();


        dataPtr = metaIm->ElementData ();
        nComp = metaIm->ElementNumberOfChannels ();

        nDim = 0;
        for (int i = 0; i < 20; i++)
                {
                if (metaIm->DimSize(i) == 0)
                        {
                        break;
                        }
                nDim++;
                }

        
        for (int i = 0; i < nDim; i++)
                {
                dims[i] = metaIm->DimSize(i);
                }

        return 0;
        }  // end of getMetaImageData




                
static int
writeArrayMeta (DataFileDesc & outDesc)
        {
        // write it out as a MetaImage
        float * elSpacing = new float [outDesc.nDim];
        for (int i = 0; i < outDesc.nDim; i++)
                {
                elSpacing[i] = 1;
                }

for (int i = 0; i < 1000; i++)
{
printf ("%p outDesc.dataPtr[%d] = %f\n",
outDesc.dataPtr,
i,
((float *)outDesc.dataPtr)[i] );
}

        vtkmetaio::MetaImage *metaIm = new vtkmetaio::




writeMetaImage (
fileName
fmt, // bin vs ascii
int nDim,
int dim[],
type,
int nComp,
void *data
)




                MetaImage (outDesc.nDim, outDesc.dim, elSpacing,
                           outDesc.type, outDesc.nComp, outDesc.dataPtr );

        metaIm->BinaryData (outDesc.fmt == BINARY);




        metaIm->Write (outDesc.fileName.c_str());

        return 0;
        }  // end of writeArrayMeta

static void
cvtNumToStr (DataFileDesc outDesc, int i, char *numStr)
        {
        // convert the i-th value from the outDesc data to a string

        switch (outDesc.type)
                {
                case vtkmetaio::MET_CHAR:
                        sprintf (numStr, "%d", 
                              (int) ((char *)outDesc.dataPtr)[i]);
                        break;

                case vtkmetaio::MET_UCHAR:
                        sprintf (numStr, "%d", 
                              (int) ((unsigned char *)outDesc.dataPtr)[i]);
                        break;

                case vtkmetaio::MET_SHORT:
                        sprintf (numStr, "%d", 
                              (int) ((short *)outDesc.dataPtr)[i]);
                        break;

                case vtkmetaio::MET_USHORT:
                        sprintf (numStr, "%d", 
                              (int) ((unsigned short *)outDesc.dataPtr)[i]);
                        break;

                case vtkmetaio::MET_INT:
                        sprintf (numStr, "%d", 
                              ((int *)outDesc.dataPtr)[i]);
                        break;

                case vtkmetaio::MET_UINT:
                        sprintf (numStr, "%u", 
                              ((unsigned int *)outDesc.dataPtr)[i]);
                        break;

                case vtkmetaio::MET_FLOAT:
                        sprintf (numStr, "%.8g", 
                              ((float *)outDesc.dataPtr)[i]);
                        break;

                case vtkmetaio::MET_DOUBLE:
                        sprintf (numStr, "%.17g", 
                              ((float *)outDesc.dataPtr)[i]);
                        break;

                default:
                        // I don't think that this can happen
                        strcpy (numStr, "data type error");
                }

        } // end of cvtNumToStr

static int
writeArrayRaw (DataFileDesc & outDesc)
        {
        FILE *fp = fopen (outDesc.fileName.c_str(), "w");
        if (fp == NULL)
                {
                dtkMsg.add (DTKMSG_ERROR, 
                        "Unable to open output file %s.\n", 
                                        outDesc.fileName.c_str());
                return -1;
                }

        if (outDesc.fmt == BINARY)
                {
                if (fwrite (outDesc.dataPtr, outDesc.totalDataLen, 1, fp) != 1)
                        {
                        dtkMsg.add (DTKMSG_ERROR, 
                                "Error writing output file %s.\n", 
                                outDesc.fileName.c_str());
                        return -1;
                        }
                }
        else
                {
                char numStr[100];
                for (int i = 0; i < outDesc.totalNumVals; i++)
                        {
                        cvtNumToStr (outDesc, i, numStr);
                        fprintf (fp, "%s\n", numStr);
                        }
                }

        fclose (fp);

        return 0;
        }  // end of writeArrayRaw


static int
writeArray (DataFileDesc & outDesc)
        {
        int rtn;

        if (outDesc.fileType == F_RAW)
                {
                rtn = writeArrayRaw (outDesc);
                }
        else if (outDesc.fileType == F_META)
                {
                rtn = writeArrayMeta (outDesc);
                }
        else
                {
                dtkMsg.add (DTKMSG_ERROR, 
                         "Internal error: bad file type for output file %s.\n", 
                                outDesc.fileName.c_str());
                rtn = -1;
                }

        return rtn;
        }
                


static int
readArray (DataFileDesc & inDesc)
        {

        // Get the input data based on file type
        if (inDesc.fileType == F_META)
                {
                if (getMetaImageData(inDesc.fileName, inDesc.nDim, inDesc.dim,
                                     inDesc.nComp, inDesc.type, inDesc.dataPtr))
                        {
                        return (-1);
                        }
                }
        else
                {
                if (readRawInput (inDesc))
                        {
                        return (-1);
                        }
                }

        inDesc.totalNumVals = inDesc.nComp;
        for (int i = 0; i < inDesc.nDim; i++)
                {
                inDesc.totalNumVals *= inDesc.dim[i];
                }
        inDesc.totalDataLen = inDesc.totalNumVals * getTypeLen (inDesc.type);

        return 0;

        }  // end of readArray


 

main (int argc, char **argv) 
        {

        dtkMsg.setPreMessage ("hev-convertDataArray: ");

        DataFileDesc inDesc, outDesc;
        DataTransformations dataXforms;

        int nInputs;
        char ** inFNList;

        if (parseCmdLine ( argc, argv, inDesc, outDesc, dataXforms, 
                                                        nInputs, inFNList ) )
                {
                usage ();
                exit (-1);
                }


        std::string outFnTemplate = outDesc.fileName;
        std::string prevOutFN = "";

        for (int i = 0; i < nInputs; i++)
                {
                // set fileName in both inDesc and outDesc
                inDesc.fileName = inFNList[i];
                if (nInputs != 1)
                        { 
                        char newFN[10000];
                        snprintf(newFN, sizeof(newFN), outFnTemplate.c_str(),i);
                        if (std::string(newFN) == prevOutFN)
                            {
                            dtkMsg.add (DTKMSG_ERROR, 
                              "Generated same output file name (%s) for "
                              "outputs %d and %d.\n", newFN, i-1, i);
                            dtkMsg.add (DTKMSG_ERROR, 
                              "Probable error in output file name template.\n");
                            exit (-1);
                            }
                        prevOutFN = newFN;
                        outDesc.fileName = newFN;
                        }

                if (readArray (inDesc))
                        {
                        exit (-1);
                        }

                // transform and convert the data
                if ( cvtToOutputForm (inDesc, dataXforms, outDesc) )
                        {
                        exit (-1);
                        }


                // write it out
                if (writeArray (outDesc))
                        {
                        exit (-1);
                        }
                }

        exit (0);
        }  // end of main
