
#include <stdio.h>
#include <stdlib.h>
#include <dtk.h>
#include <iris.h>
#include <libgen.h>

#include "VolVis.h"
#include "MetaImg.h"


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
    }  // end of static const std::string dataTypeToUnuStr (VolVis::DataType )





static VolVis::DataType unuStrToDataType (std::string unuTypeStr)
        {

        if (unuTypeStr == "unspecified")
            {
            return VolVis::DT_UNSPECIFIED;
            }
        else if (unuTypeStr == "char")
            {
            return VolVis::DT_CHAR;
            }
        else if (unuTypeStr == "uchar")
            {
            return VolVis::DT_UCHAR;
            }
        else if (unuTypeStr == "short")
            {
            return VolVis::DT_SHORT;
            }
        else if (unuTypeStr == "ushort")
            {
            return VolVis::DT_USHORT;
            }
        else if (unuTypeStr == "int")
            {
            return VolVis::DT_INT;
            }
        else if (unuTypeStr == "uint")
            {
            return VolVis::DT_UINT;
            }
        else if (unuTypeStr == "long")
            {
            return VolVis::DT_LONG;
            }
        else if (unuTypeStr == "ulong")
            {
            return VolVis::DT_ULONG;
            }
        else if (unuTypeStr == "float")
            {
            return VolVis::DT_FLOAT;
            }
        else if (unuTypeStr == "double")
            {
            return VolVis::DT_DOUBLE;
            }

        return VolVis::DT_UNSPECIFIED;
        }  // end of static DataType unuStrToDataType (std::string unuTypeStr)


int
readNrrd
  (
  std::string const & inFN,
  int & nDim,
  int * & dims,
  int & nComp,
  VolVis::DataType & dType,
  std::string & contentStr,
  void * & dataPtr 
  )
    {

    std::string teemBinDir = "/external/teem/teem-1.6.0-src/linux.64/bin/";
    char *hevroot = getenv("HEVROOT");
    if (hevroot == NULL)
        {
        dtkMsg.add (DTKMSG_ERROR,
            "readNrrd: Environment variable HEVROOT is not defined.\n");
        return -1;
        }
    teemBinDir = hevroot + teemBinDir;

    std::string cmd, cmdStdOut;
    int rtn, exitStatus;

    // Transform the input nrrd into a canonical form: 
    //      - raw (binary) data 
    //      - little endian 
    //      - data follows header in same file
    //
    // Need cmd like this: unu save -f nrrd  -e raw -en little -i inFN -o outFN
    char baseName[1000];
    char baseName2[1000];
    strncpy (baseName, inFN.c_str(), sizeof (baseName));
    strncpy (baseName2, basename (baseName), sizeof (baseName));
    if (baseName2[0] == 0)
        {
        strcpy (baseName2, "tmpNrrd");
        }
    std::string tmpNrrdFN = 
            iris::GetTempFileName ("raw", "/var/tmp", baseName2);
    cmd = teemBinDir + 
                "unu save -f nrrd  -e raw -en little -i " +
                    inFN + " -o "  + tmpNrrdFN +
                " 2>&1 ";
    rtn = VolVis::executeCmd (cmd, cmdStdOut, exitStatus);
    if (rtn || exitStatus)
        {
        dtkMsg.add (DTKMSG_ERROR,
        "readNrrd: Error converting nrrd file %s.\n", inFN.c_str() );
        dtkMsg.add (DTKMSG_ERROR,
        "readNrrd: Error executing command:\n        %s\n", cmd.c_str() );
        unlink (tmpNrrdFN.c_str());
        return -1;
        }

    
    FILE *inFP = fopen (tmpNrrdFN.c_str(), "r");
    if (inFP == NULL)
        {
        dtkMsg.add (DTKMSG_ERROR, 
            "readNrrd (%s) : Unable to open file %s for reading.\n", 
            inFN.c_str(), tmpNrrdFN.c_str());
        unlink (tmpNrrdFN.c_str());
        return -1;
        }

    nDim = 0;
    dType = VolVis::DT_UNSPECIFIED;
    dataPtr = NULL;
    dims = NULL;

    char line[1000];
    char tok[1000];

    std::string sizeLine;
    while (fgets (line, sizeof(line), inFP) != NULL)
        {

        if (sscanf (line, "%s", tok) != 1)
            {
            // we're at the end of the header
            break;
            }

        if (strcmp (tok, "content:") == 0)
            {
            contentStr = line;
            }
        else if (strcmp (tok, "type:") == 0)
            {
            char nType[1000];

            if (sscanf (line, "%s %s", tok, nType) != 2)
                {
                dtkMsg.add (DTKMSG_ERROR, 
                    "readNrrd (%s) : Error on type line in nrrd file %s.\n", 
                    inFN.c_str(), tmpNrrdFN.c_str());
                    fclose (inFP);
                    unlink (tmpNrrdFN.c_str());
                    return -1;
                }
            dType = unuStrToDataType (nType);
            }
        else if (strcmp (tok, "dimension:") == 0)
            {
            if (sscanf (line, "%s %d", tok, &nDim) != 2)
                {
                dtkMsg.add (DTKMSG_ERROR, 
                   "readNrrd (%s) : Error on dimension line in nrrd file %s.\n",
                    inFN.c_str(), tmpNrrdFN.c_str());
                    fclose (inFP);
                    unlink (tmpNrrdFN.c_str());
                    return -1;
                }
            }
        else if (strcmp (tok, "sizes:") == 0)
            {
            sizeLine = line;
            }
        }
        
    if (nDim <= 0)
        {
        dtkMsg.add (DTKMSG_ERROR, 
                "readNrrd (%s) : Number of dimensions not properly "
                "specified in nrrd file %s.\n", 
                inFN.c_str(), tmpNrrdFN.c_str());
        fclose (inFP);
        unlink (tmpNrrdFN.c_str());
        return -1;
        }

    dims = new int [nDim];

    std::vector<std::string> sizeVec = iris::ParseString (sizeLine);
        
    if (sizeVec.size() != 1+nDim)
        {
        dtkMsg.add (DTKMSG_ERROR, 
            "readNrrd (%s) : "
            "Bad number of parameters on sizes line in nrrd file %s.\n",
            inFN.c_str(), tmpNrrdFN.c_str());
            fclose (inFP);
            unlink (tmpNrrdFN.c_str());
            return -1;
        }

    int numEl = 1;
    for (int i = 0; i < sizeVec.size()-1; i++)
        {
        if (sscanf (sizeVec[i+1].c_str(), "%d", dims+i) != 1)
            {
            dtkMsg.add (DTKMSG_ERROR, 
                "readNrrd (%s) : Error parsing sizes line in nrrd file %s.\n", 
                inFN.c_str(), tmpNrrdFN.c_str());
            unlink (tmpNrrdFN.c_str());
            return -1;
            }
        numEl *= dims[i];
        }
        

    // OK, we have grabbed all of the information we need from the header
    // and inFP is positioned just after the header at the beginning
    // of the data.

    // So allocate memory and read in the data, which we know is in binary
    // format.
    dataPtr = (void *) ( new char [ numEl * dataTypeLength (dType) ] );
    if (fread (dataPtr, dataTypeLength(dType), numEl, inFP) != numEl)
        {
        dtkMsg.add (DTKMSG_ERROR, 
                "readNrrd (%s) : Error reading data from nrrd file %s.\n", 
                inFN.c_str(), tmpNrrdFN.c_str());
        fclose (inFP);
        unlink (tmpNrrdFN.c_str());
        return -1;
        }

    fclose (inFP);
    unlink (tmpNrrdFN.c_str());
    return 0;

    } // end of readNrrd



static void
usage ()
    {
    fprintf (stderr, "Usage:  hev-nrrdToMha [--vector]  inFN  outFN\n");
    } // end of usage


int
main (int argc, char **argv)
    {

    if ((argc < 3) || (argc > 4))
        {
        dtkMsg.add (DTKMSG_ERROR, "hev-mhaToNrrd: Bad argument count.\n");
        usage ();
        return -1;
        }

    bool vectorData = false;
    if (argc == 4)
        {
        if (argv[1] != std::string("--vector"))
            {
            dtkMsg.add (DTKMSG_ERROR, 
                "hev-mhaToNrrd: Error parsing command line arguments.\n");
            usage ();
            return -1;
            }
        vectorData = true;
        }

    std::string inNrrdFN = argv[argc-2];
    std::string outMhaFN = argv[argc-1];

    int nDim;
    int *dims;
    int nChan;
    VolVis::DataType dType;
    void *dataPtr;
    std::string contentStr;

    // read in that file into memory based on the header
    if (readNrrd (inNrrdFN, nDim, dims, nChan, dType, contentStr, dataPtr))
        {
        dtkMsg.add (DTKMSG_ERROR, 
             "hev-mhaToNrrd: Error reading nrrd file %s.\n", inNrrdFN.c_str());
        usage ();
        return -1;
        }

    if (vectorData && (nDim == 1))
        {
        dtkMsg.add (DTKMSG_ERROR,
            "hev-mhaToNrrd: Option --vector was specified but "
            "input data is insufficient.\n");
        return -1;
        }
        
    nChan = 1;
    if (vectorData)
        {
        nChan = dims[0];
        for (int i = 1; i < nDim; i++)
            {
            dims[i-1] = dims[i];
            }
        nDim--;
        }

    int mipLevels = 1;
    std::vector<std::string> sv;
    sv.push_back(contentStr);
    if (writeMetaImage (outMhaFN, VolVis::BINARY, nDim, dims, nChan, mipLevels, 
                        dType, dataPtr, &sv))
        {
        dtkMsg.add (DTKMSG_ERROR,
            "hev-mhaToNrrd: Error writing MetaImage file %s.\n",
            outMhaFN.c_str() );
        return -1;
        }

    return 0;
    }  // end of main


