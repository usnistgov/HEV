
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <dtk.h>
#include <iris.h>

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
    }  // end of






static void
usage ()
    {
    fprintf (stderr, "Usage:  hev-mhaToNrrd  inFN  outFN\n");
    } // end of usage

int
main (int argc, char **argv)
    {

     // send messages to stderr
     dtkMsg.setFile(stderr) ;


    if (argc != 3)
        {
        dtkMsg.add (DTKMSG_ERROR, "hev-mhaToNrrd: Bad argument count.\n");
        usage ();
        return -1;
        }

    std::string inMhaFN = argv[1];
    std::string outNrrdFN = argv[2];



    std::string teemBinDir = "/external/teem/teem-1.6.0-src/linux.64/bin/";
    char *hevroot = getenv("HEVROOT");
    if (hevroot == NULL)
        {
        dtkMsg.add (DTKMSG_ERROR,
            "GkBoundary: Environment variable HEVROOT is not defined.\n");
        }
    teemBinDir = hevroot + teemBinDir;






    int nDim;
    int *dims;
    int nChan;
    int mipLevels;
    VolVis::DataType dType;
    void *dataPtr;
    int rtn, exitStatus;
    std::string cmdStdOut;

    // fprintf (stderr, "about to read meta image %s.\n", inMhaFN.c_str());

    if (readMetaImage (inMhaFN, nDim, dims, nChan, mipLevels, dType, dataPtr))
        {
        dtkMsg.add (DTKMSG_ERROR,
            "hev-mhaToNrrd: Error reading MetaImage file %s.\n",
            inMhaFN.c_str() );
        return -1;
        }


    int fullLen = 1;
    std::string dimStr = "";
    std::string spStr = "";
    if (nChan > 1)
        {
        char ds[100];
        sprintf (ds, " %d ", nChan);
        dimStr += ds;
        spStr += " NaN ";
        }

    for (int i = 0; i < nDim; i++)
        {
        fullLen *= dims[i];
        char ds[100];
        sprintf (ds, " %d ", dims[i]);
        dimStr += ds;
        spStr += " 1 ";
        }
    fullLen *= nChan;

    char inBaseName[1000];
    char inName[1000];
    strncpy (inName, inMhaFN.c_str(), sizeof(inName));
    strncpy (inBaseName, basename (inName), sizeof(inBaseName));
    std::string rawFN = 
        iris::GetTempFileName ("raw", "/var/tmp", inBaseName);

    // fprintf (stderr, "about to open raw file %s.\n", rawFN.c_str());

    FILE *rawFP = fopen (rawFN.c_str(), "w");
    if (rawFP == NULL)
        {
        dtkMsg.add (DTKMSG_ERROR,
          "hev-mhaToNrrd: Unable to open temporary file %s.\n", rawFN.c_str() );
        return -1;
        }


    if (fwrite (dataPtr, VolVis::dataTypeLength (dType), 
                                    fullLen, rawFP) != fullLen)
        {
        dtkMsg.add (DTKMSG_ERROR,
            "hev-mhaToNrrd: Error writing raw file %s.\n", rawFN.c_str() );
        return -1;
        }

    fclose (rawFP);

    std::string cmd;
    cmd = teemBinDir + 
                "unu make " +
                " -i " + rawFN +
                " -t " + dataTypeToUnuStr (dType) +
                " -sp " + spStr +
                " -e raw " +
                " -s " + dimStr +
                " -c " + inMhaFN +
                " -o " + outNrrdFN +
                " 2>&1 ";

    // fprintf (stderr, "about to execute cmd %s.\n", cmd.c_str());

    rtn = VolVis::executeCmd (cmd, cmdStdOut, exitStatus);

    // fprintf (stderr, "cmd exitStatus = %d\n", exitStatus);
    // fprintf (stderr, "executeCmd rtn = %d\n", rtn);
    // fprintf (stderr, "cmd out:\n%s\n", cmdStdOut.c_str());

    if (rtn || exitStatus)
        {
        dtkMsg.add (DTKMSG_ERROR,
            "hev-mhaToNrrd: Error executing command:\n        %s\n",
            cmd.c_str() );
        unlink (rawFN.c_str());
        return -1;
        }

    unlink (rawFN.c_str());
    // printf ("about to return 0\n");
    return 0;

    }  // end of main


