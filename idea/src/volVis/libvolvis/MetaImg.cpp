
//    ElementByteOrderMSB = True | False
//    //    BinaryDataByteOrderMSB = True | False
//
//
//
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dtk.h>
#include <iris.h>

#include "VolVis.h"
#include "MetaImg.h"



#define MAX(x,y)        (((x)>(y))?(x):(y))
#define MIN(x,y)        (((x)<(y))?(x):(y))


static inline void 
swapBytes (unsigned char * c, int len)
    {
    unsigned cc;

    int hl = len/2;
    int lm1 = len - 1;
    for (int i = 0; i < hl; i++)
        {
        cc = c[i];
        c[i] = c[lm1 - i];
        c[lm1 - i] = c[i];
        }

    } // end of swapBytes

static inline bool 
cpuIsLittleEndian ()
    {
    int i = 1;
    return (*((unsigned char *)(&i)) != 0);
    } // end of cpuIsLittleEndian


// Count the number of elements based on dimensions and levels
// (does not account for multiple channels).
static int
elementCount (const int nDim, const int dims[], const int mipLevels)
    {

    int levelDims[10];   // used when mipLevels > 1
    int nEl = 1;
    for (int i = 0; i < nDim; i++)
        {
        levelDims[i] = dims[i];
        nEl *= dims[i];
        }

    // now add in the number of entries at the other mip levels
    for (int l = 1; l < mipLevels; l++)
        {
        int n = 1;
        for (int i = 0; i < nDim; i++)
            {
            levelDims[i] = ((levelDims[i]-1) >> 1) + 1;
            n *= levelDims[i];
            }

        nEl += n;
        }

    return nEl;
    }  // end of elementCount

int
VolVis::readMetaImage
  (
  std::string const & inFN,
  int & nDim,
  int * & dims,
  int & nChannels,
  int & mipLevels,
  VolVis::DataType & elementType,
  void * & dataPtr ,
  std::vector<std::string> *comments
  )
    {

#if 1
    if (comments != NULL)
        {
        comments->clear();
        }
#endif
    
    FILE *inFP = fopen (inFN.c_str(), "r");
    if (inFP == NULL)
        {
        dtkMsg.add (DTKMSG_ERROR,
                  "readMetaImage: Unable to open file %s.\n", inFN.c_str());
        return -1;
        }

    nDim = 1;
    dims = NULL;
    nChannels = 1;
    mipLevels = 1;
    elementType = DT_UCHAR;
    bool binary = true;
    bool littleEndian = true;
    elementType = DT_UNSPECIFIED;
    int headerSize = 0;
    char objType[1000];
    char elFN[1000];
    char elTypeStr[1000];
    char tf[1000];

    char hdrLine[1000];
    
    dims = new int [10];

    // Read header lines
    while (fgets (hdrLine, sizeof(hdrLine), inFP) != NULL)
        {

        // The following parsing is styled after Marc's code in the the
        // original version of the .mha osg loader.
        // Note that the expressions inside the if tests are comma-separated
        // expressions.  The idea is that it does the first one, then the
        // second one, and the value of the whole expression is the second one.
        // Also, the %n conversion gives the number of characters of the input
        // that have been processed to that point.

        int pos = 0;


        if (sscanf(hdrLine, "ObjectType = %s%n", objType, &pos), pos)
            {
            if (strcmp (objType, "Image") != 0)
                {
                dtkMsg.add (DTKMSG_ERROR,
                  "readMetaImage: File %s had ObjectType = %s; "
                  "can only handle \"Image\".\n", 
                  inFN.c_str(), objType);
                fclose (inFP);
                return -1;
                }
            }

        else if (sscanf(hdrLine, "NDims = %d%n", &nDim, &pos), pos)
            {
            if ((nDim > 10) || (nDim < 1) )
                {
                dtkMsg.add (DTKMSG_ERROR,
                  "readMetaImage: File %s had NDims = %d; "
                  "must be 1 to 10.\n", 
                inFN.c_str(), nDim);
                fclose (inFP);
                return -1;
                }
            }

        else if (sscanf(hdrLine, "DimSize = %n"
                                      "%d %d %d %d %d %d %d %d %d %d",
                        &pos, 
                        &dims[0], &dims[1], &dims[2], &dims[3], &dims[4], 
                        &dims[5], &dims[6], &dims[7], &dims[8], &dims[9]), pos)
            continue;

        else if (sscanf(hdrLine, 
                           "Comment = Levels = %d%n", &mipLevels, &pos), pos)
            continue;

        else if (sscanf(hdrLine, 
                           "Comment = %n", &pos), pos)
            {
            if (comments != NULL)
                {
                comments->push_back (hdrLine+pos);
                }
            }

        else if (sscanf(hdrLine, "ElementNumberOfChannels = %d%n",
                        &nChannels, &pos), pos)
            continue;

        else if (sscanf(hdrLine, "HeaderSize = %d%n", &headerSize, &pos), pos)
            continue;

        else if (sscanf(hdrLine, "BinaryData = %s%n", &tf, &pos), pos)
            {
            binary = (strncasecmp (tf, "true", 4) == 0);
            }

        else if (
             (sscanf(hdrLine,"BinaryDataByteOrderMSB = %s%n", tf, &pos), pos) ||
             (sscanf(hdrLine,"ElementByteOrderMSB = %s%n",    tf, &pos), pos)  )
            {
            littleEndian = (strncasecmp(tf, "false", 4) == 0) ;
            continue;
            }

        else if (sscanf(hdrLine, "ElementType = %s%n", elTypeStr, &pos), pos) 
            {
            // decode elTypeStr to elementType
            if (strcmp (elTypeStr, "MET_CHAR") == 0)
                {
                elementType = DT_CHAR;
                }
            else if (strcmp (elTypeStr, "MET_UCHAR") == 0)
                {
                elementType = DT_UCHAR;
                }
            else if (strcmp (elTypeStr, "MET_SHORT") == 0)
                {
                elementType = DT_SHORT;
                }
            else if (strcmp (elTypeStr, "MET_USHORT") == 0)
                {
                elementType = DT_USHORT;
                }
            else if (strcmp (elTypeStr, "MET_INT") == 0)
                {
                elementType = DT_INT;
                }
            else if (strcmp (elTypeStr, "MET_UINT") == 0)
                {
                elementType = DT_UINT;
                }
            else if (strcmp (elTypeStr, "MET_LONG") == 0)
                {
                elementType = DT_LONG;
                }
            else if (strcmp (elTypeStr, "MET_ULONG") == 0)
                {
                elementType = DT_ULONG;
                }
            else if (strcmp (elTypeStr, "MET_FLOAT") == 0)
                {
                elementType = DT_FLOAT;
                }
            else if (strcmp (elTypeStr, "MET_DOUBLE") == 0)
                {
                elementType = DT_DOUBLE;
                }
            else
                {
                dtkMsg.add (DTKMSG_ERROR,
                  "readMetaImage: Unrecognized ElementType = %s.\n", elTypeStr);
                fclose (inFP);
                return -1;
                }
            }

        else if (sscanf(hdrLine, "ElementDataFile = %s%n", elFN, &pos), pos) 
            {
            // this is the last line of header
            break;
            }

        }  // end of loop over header lines
            
    if (elementType == DT_UNSPECIFIED)
        {
        dtkMsg.add (DTKMSG_ERROR,
                  "readMetaImage: ElementType not specified in file %s.\n",
                inFN.c_str());
        fclose (inFP);
        return -1;
        }

    // Need to figure out the size of the buffer needed for the entire
    // data array.
    //

    int nEl = elementCount (nDim, dims, mipLevels);
    nEl *= nChannels;  // total number of entries in array 

    int elSize = VolVis::dataTypeLength (elementType);
    // Here's the entire size of the array in bytes
    int dataSize = nEl * elSize;


    unsigned char *data = new unsigned char [ dataSize ];
    if (data == NULL)
        {
        dtkMsg.add (DTKMSG_ERROR,
                  "readMetaImage: Unable to allocate memory for data array.\n");
        fclose (inFP);
        return -1;
        }

    dataPtr = data;

    if ( strcmp (elFN, "LOCAL") != 0 )
        {
        fclose (inFP);
        inFP = fopen (elFN, "r");
        if (inFP == NULL)
            {
            dtkMsg.add (DTKMSG_ERROR,
                  "readMetaImage: Unable to open data file %s.\n",
                  elFN);
            fclose (inFP);
            return -1;
            }
        }

    if (headerSize > 0)
        {
        if ( fseek (inFP, headerSize, SEEK_SET) )
            {
            dtkMsg.add (DTKMSG_ERROR,
                  "readMetaImage: Error while seeking past header.n");
            fclose (inFP);
            return -1;
            }
        }
    else if (headerSize < 0)
        {
        if ( ! binary )
            {
            dtkMsg.add (DTKMSG_ERROR,
                  "readMetaImage: Header size specified as -1 in ASCII file.\n"
                  "     This is an illegal combination of parameters.\n");
            fclose (inFP);
            return -1;
            }
        else if ( fseek (inFP, dataSize, SEEK_END) )
            {
            dtkMsg.add (DTKMSG_ERROR,
               "readMetaImage: Error while seeking relative to end of file.\n");
            fclose (inFP);
            return -1;
            }

        }


    if (binary)
        {
        if (fread (data, 1, dataSize, inFP) != dataSize)
            {
            dtkMsg.add (DTKMSG_ERROR,
               "readMetaImage: Error while reading binary data array.\n");
            fclose (inFP);
            return -1;
            }
        if (littleEndian != cpuIsLittleEndian())
            {
            unsigned char *dataPtr = data;
            for (int i = 0; i < nEl; i++)
                {
                swapBytes (dataPtr, elSize);
                dataPtr += elSize;
                }
            }
        }
    else
        {
        std::string fmt = VolVis::dataTypeScanfFmt (elementType);

        unsigned char *dataPtr = data;
        for (int i = 0; i< nEl; i++)
            {
            if (fscanf (inFP, fmt.c_str(), dataPtr) != 1)
                {
                dtkMsg.add (DTKMSG_ERROR,
                    "readMetaImage: Error while reading ASCII data array "
                    "at position %d.\n", i);
                fclose (inFP);
                return -1;
                }
            dataPtr += elSize;
            }
        }

    // success
    
    fclose (inFP);
    return 0;
    }  // end of readMetaImage



static int
 printFormatted (FILE *outFP, unsigned char *dataPtr, VolVis::DataType dType)
    {
    long l;
    unsigned long u;
    float f;
    double d;


    // not the most efficient code, but I don't think that this is time critical

    switch (dType)
        {
        case VolVis::DT_CHAR:
            l = *((char *)dataPtr);
            break;

        case VolVis::DT_UCHAR:
            u = *((unsigned char *)dataPtr);
            break;

        case VolVis::DT_SHORT:
            l = *((short *)dataPtr);
            break;

        case VolVis::DT_USHORT:
            u = *((short *)dataPtr);
            break;

        case VolVis::DT_INT:
            l = *((int *)dataPtr);
            break;

        case VolVis::DT_UINT:
            u = *((unsigned int *)dataPtr);
            break;

        case VolVis::DT_LONG:
            l = *((long *)dataPtr);
            break;

        case VolVis::DT_ULONG:
            u = *((unsigned long *)dataPtr);
            break;

        case VolVis::DT_FLOAT:
            f = *((float *)dataPtr);
            break;

        case VolVis::DT_DOUBLE:
            d = *((double *)dataPtr);
            break;

        default:
            dtkMsg.add (DTKMSG_ERROR,
                    "writeMetaImage: Bad data type encountered (%d).\n",
                    dType);
            return -1;
        }

    switch (dType)
        {
        case VolVis::DT_CHAR:
        case VolVis::DT_SHORT:
        case VolVis::DT_INT:
        case VolVis::DT_LONG:
            fprintf (outFP, "%ld", l);
            break;

        case VolVis::DT_UCHAR:
        case VolVis::DT_USHORT:
        case VolVis::DT_UINT:
        case VolVis::DT_ULONG:
            fprintf (outFP, "%lu", u);
            break;

        case VolVis::DT_FLOAT:
            fprintf (outFP, "%.9g", f);
            break;

        case VolVis::DT_DOUBLE:
            fprintf (outFP, "%.17g", d);
            break;
        }


    return 0;
    } // end of printFormatted


#if 1

int
VolVis::writeMetaImage
  (
  std::string const & outFN,
  const VolVis::DataEncoding encoding,
  const int nDim,
  const int dim[],
  const int nChannels,
  const int mipLevels,
  const VolVis::DataType dType,
  void *data ,
  const std::vector<std::string> *comments
  )
    {



    if ((nDim < 1) || (nDim > 10))
        {
        dtkMsg.add (DTKMSG_ERROR,
                    "writeMetaImage: Number of dimensions is %d; "
                    "must be between 1 and 10.\n", nDim);
        return -1;
        }

    if (dType == DT_UNSPECIFIED)
        {
        dtkMsg.add (DTKMSG_ERROR,
                    "writeMetaImage: Data type is unspecified.\n");
        return -1;
        }


    FILE *outFP = fopen (outFN.c_str(), "w");
    if (outFP == NULL)
        {
        dtkMsg.add (DTKMSG_ERROR,
                    "writeMetaImage: Error openning output data file %s.\n", 
                    outFN.c_str());
        return -1;
        }

    if (mipLevels > 1)
        {
        fprintf (outFP, "Comment = Levels = %d\n", mipLevels);
        }
    fprintf (outFP, "ObjectType = Image\n");
    fprintf (outFP, "NDims = %d\n", nDim);

    fprintf (outFP, "DimSize =");
    for (int i = 0; i < nDim; i++)
        {
        fprintf (outFP, " %d", dim[i]);
        }
    fprintf (outFP, "\n");

    fprintf (outFP, "ElementNumberOfChannels = %d\n", nChannels);
    
    const char *metType = "MET_UCHAR";
    switch (dType)
        {
        case DT_CHAR:
            metType = "MET_CHAR";
            break;

        case DT_UCHAR:
            metType = "MET_UCHAR";
            break;

        case DT_SHORT:
            metType = "MET_SHORT";
            break;

        case DT_USHORT:
            metType = "MET_USHORT";
            break;

        case DT_INT:
            metType = "MET_INT";
            break;

        case DT_UINT:
            metType = "MET_UINT";
            break;

        case DT_LONG:
            metType = "MET_LONG";
            break;

        case DT_ULONG:
            metType = "MET_ULONG";
            break;

        case DT_FLOAT:
            metType = "MET_FLOAT";
            break;

        case DT_DOUBLE:
            metType = "MET_DOUBLE";
            break;

        default:
            dtkMsg.add (DTKMSG_ERROR,
                    "writeMetaImage: Unrecognized data type (%d).\n", 
                    dType);
            fclose (outFP);
            return -1;
        }

    fprintf (outFP, "ElementType = %s\n", metType);

    fprintf (outFP, "BinaryData = %s\n", 
                        (encoding == BINARY) ? "True" : "False");


    if (encoding == BINARY)
        {
        fprintf (outFP, "BinaryDataByteOrderMSB = %s\n",
                            cpuIsLittleEndian () ? "False" : "True");
        fprintf (outFP, "ElementByteOrderMSB = %s\n",
                            cpuIsLittleEndian () ? "False" : "True");
        }

    // HeaderSize = 0

    if (comments != NULL)
        {
        int nComments = comments->size();
        for (int i = 0; i < nComments; i++)
            {
            fprintf (outFP, "Comment = %s\n", (*comments)[i].c_str());
            }
        }
        
    fprintf (outFP, "ElementDataFile = LOCAL\n");


    int nEl = elementCount (nDim, dim, mipLevels);
    int elSize = VolVis::dataTypeLength (dType);


    if (encoding == BINARY)
        {
        nEl *= nChannels;  // total number of entries in array 

        if (fwrite (data, elSize, nEl, outFP) != nEl)
            {
            dtkMsg.add (DTKMSG_ERROR,
                    "writeMetaImage: Error writing data array.\n" );
            fclose (outFP);
            return -1;
            }
        }
    else
        {
        unsigned char *dataPtr = (unsigned char *) data;
        for (int i = 0; i < nEl; i++)
            {
            for (int j = 0; j < nChannels; j++)
                {
                if (j != 0)
                    {
                    fprintf (outFP, " ");
                    }
                printFormatted (outFP, dataPtr, dType);
                dataPtr += elSize;
                }
            fprintf (outFP, "\n");
            }
        }
    return 0;
    }  // end of writeMetaImage

#endif
