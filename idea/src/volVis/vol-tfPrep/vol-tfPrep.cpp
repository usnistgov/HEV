//
// vol-tfPrep  transferFuncFN  outputFNPrefix
//
// outputFNPrefix: if it ends in / it's assumed to be a directory name
// and the output file names will be placed in that directory.

#include <stdio.h>
#include <stdlib.h>

#include <dtk.h>
#include <VolVis.h>

static void
usage ()
        {
        fprintf (stderr, 
        "Usage:  vol-tfPrep  [--volume volFN] transferFuncFN  [outFnPrefix]\n");
        }
        

static int
parseArgs (int argc, char **argv, 
            std::string & volumeName, 
            std::string & transferFuncFn, 
            std::string & outFnPrefix )
    {
    volumeName = "";
    transferFuncFn = "";
    outFnPrefix = "";

    if ( (argc < 2) || (argc > 5) )
        {
        dtkMsg.add (DTKMSG_ERROR,
                "vol-tfPrep: Bad argument count.\n");
        usage ();
        return -1;
        }

    bool badArgs = false;

    switch (argc)
        {
        case 2:
            transferFuncFn = argv[1];
            break;

        case 3:
            transferFuncFn = argv[1];
            outFnPrefix = argv[2];
            break;

        case 4:
            if ( (strncasecmp ("--volume", argv[1], strlen(argv[1])) == 0) && 
                 (strlen(argv[1]) > 3) )
                {
                volumeName = argv[2];
                transferFuncFn = argv[3];
                }
            else
                {
                badArgs = true;
                }
            break;

        case 5:
            if ( (strncasecmp ("--volume", argv[1], strlen(argv[1])) == 0) && 
                 (strlen(argv[1]) > 3) )
                {
                volumeName = argv[2];
                transferFuncFn = argv[3];
                outFnPrefix = argv[4];
                }
            else
                {
                badArgs = true;
                }
            break;

        }


        if (badArgs)
            {
            dtkMsg.add (DTKMSG_ERROR,
                "vol-tfPrep: Bad arguments.\n");
            usage ();
            return -1;
            }


    if (outFnPrefix != "")
        {
        int len = outFnPrefix.size();
        if ( (outFnPrefix[len-1] != '/') && (outFnPrefix[len-1] != '.') )
            {
            // the idea is that the prefix is separated by what come after with a .
            outFnPrefix += ".";
            }
        }

    return 0;
    }  // end of parseArgs

int
main (int argc, char **argv)
        {


        // send messages to stderr
        dtkMsg.setFile(stderr) ;


#if 0
        if ( (argc < 2) || (argc > 3) )
                {
                dtkMsg.add (DTKMSG_ERROR,
                        "vol-tfPrep: Bad argument count.\n");
                usage ();
                return -1;
                }

        std::string transferFuncFn = argv[1];

        std::string outFnPrefix;
        if (argc == 3)
                {
                outFnPrefix = argv[2];
                int len = outFnPrefix.size();
                if (outFnPrefix[len-1] != '/')
                        {
                        outFnPrefix += ".";
                        }
                }
        else
                {
                outFnPrefix = "";
                }
#else
        std::string volumeFn;
        std::string transferFuncFn;
        std::string outFnPrefix;

        if (parseArgs (argc, argv, volumeFn, transferFuncFn, outFnPrefix ))
            {
            return -1;
            }

#endif
                


        VolVis::TransferFunction * tf = VolVis::readTF (transferFuncFn);

        if (tf == NULL)
                {
                dtkMsg.add (DTKMSG_ERROR,
                    "vol-tfPrep: Error reading transfer function file %s.\n", 
                    transferFuncFn.c_str());
                    return -1;
                }

        if ((volumeFn != "") && (volumeFn != "-none"))
            {
            tf->setInputDataFileName (volumeFn);
            }

        std::string funcName = tf->getFuncName();


        std::string glslName = outFnPrefix + funcName + ".glsl";
        if (tf->writeGLSL (glslName))
                {
                dtkMsg.add (DTKMSG_ERROR,
                        "vol-tfPrep: Error writing GLSL file %s.\n", 
                        glslName.c_str());
                return -1;
                }

        printf ("GLSL %s %s\n", glslName.c_str(), funcName.c_str());

        int numTextures = tf->getNumTextures ();
        if (numTextures < 0)
            {
            dtkMsg.add (DTKMSG_ERROR,
             "vol-tfPrep: Error creating textures for transfer function  %s.\n",
             transferFuncFn.c_str());
            return -1;
            }

        for (int i = 0; i < numTextures; i++)
                {
                char seqstr[10];
                sprintf (seqstr, "%04d", i);
                std::string glslFn = outFnPrefix + funcName + seqstr + ".mha";
                if (tf->writeTexture (i, glslFn))
                        {
                        dtkMsg.add (DTKMSG_ERROR,
                                "vol-tfPrep: Error writing texture file %s.\n", 
                                glslFn.c_str());
                        return -1;
                        }
                std::string glslVarName;
                int nChan;
                int nDim;
                float range[2];

                if (tf->getTextureInfo (i, glslVarName, nChan, nDim))
                        {
                        dtkMsg.add (DTKMSG_ERROR,
                            "vol-tfPrep: Error getting texture info "
                            "for transfer function file %s.\n", 
                                glslFn.c_str());
                        return -1;
                        }


                printf ("TEXTURE %s %s %d %d ", 
                        glslFn.c_str(), glslVarName.c_str(), nChan, nDim);

                
                float *ranges = new float [nDim*2];
                if (tf->getTextureRanges (i, ranges))
                    {
                    dtkMsg.add (DTKMSG_ERROR,
                            "vol-tfPrep: Error getting texture range info "
                            "for transfer function file %s.\n", 
                                glslFn.c_str());
                    return -1;
                    }
                for (int j = 0; j < nDim; j++)
                    {
                    printf ("   %.9g %.9g ", ranges[j*2+0], ranges[j*2+1]);
                    }
                printf ("\n");
                delete ranges;

                }  // end of loop over textures



        // Now print out a list of the required externals
        const std::map <std::string, VolVis::TFReqExt *> & reqs
                        = tf->getRequiredExts ();

        std::map<std::string, VolVis::TFReqExt *>::const_iterator re;
        for (re = reqs.begin(); re != reqs.end(); re++)
                {
                printf ("REQ_EXTERNAL   %-12s  %10s   %-8s  %s",
                    re->second->stdName.c_str(), re->second->objType.c_str(),
                    re->second->dataType.c_str(), re->second->glslName.c_str());
                for (int j = 0; j < re->second->infoStrs.size(); j++)
                    {
                    printf (" %s", re->second->infoStrs[j].c_str());
                    }
                printf ("\n");
                }



        return 0;
        }  // end of main






//
// vol-tfPrep  transferFuncFN  outputFNPrefix
//
// outputFNPrefix: if it ends in / it's assumed to be a directory name
// and the output file names will be placed in that directory.


