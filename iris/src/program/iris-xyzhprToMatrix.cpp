#include <stdio.h>
#include <string>
#include <unistd.h>
#include <dtk.h>

#include <iris.h>

// evil global variables
bool running = true ;

std::string xyzhprShmName ;
dtkSharedMem *xyzhprShm ;
dtkCoord xyzhpr ;

std::string matrixShmName ;
dtkSharedMem *matrixShm ;
osg::Matrix matrix ;

void usage()
{
    fprintf(stderr,"Usage: iris-xyzhprToMatrix [--usleep t] xyzhprShm matrixShm\n") ;
}

void update()
{
    
    xyzhprShm->read(xyzhpr.d) ;
    dtkCoord xyzhprCoord ;
    osg::Matrix mat = iris::CoordToMatrix(xyzhpr) ;
	
    // write out if it changed or first time
    static bool first = true ;
    if (first || mat != matrix)
    {
	first = false ;
	matrix = mat ;
	matrixShm->write(matrix.ptr()) ;	
    }
}

int main(int argc, char **argv)
{

    // send messages to stderr
    dtkMsg.setFile(stderr) ;

    if (argc<3 || argc>5)
    {
	usage() ;
	return 1 ;
    }

    int ticks = iris::GetUsleep() ;
    
    int c = 1 ;
    while (c<argc && argv[c][0] == '-' && argv[c][1] == '-')
    {
	if (iris::IsSubstring("--usleep",argv[c],3))
	{
	    c++ ;
	    if (c<argc && !iris::StringToInt(argv[c],&ticks)) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-xyzhprToMatrix: invalid usleep value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c++ ;
	}
	else 
	{
	    usage() ;
	    return 1 ;
	}
    }

    xyzhprShmName = argv[c] ;
    matrixShmName = argv[c+1] ;

    xyzhprShm = new dtkSharedMem(6*sizeof(float), xyzhprShmName.c_str(),0) ;
    if (xyzhprShm->isInvalid())
    {
	dtkMsg.add(DTKMSG_ERROR, "iris-xyzhprToMatrix: unable to open xyzhpr shared memory with size %d\n", 6*sizeof(float)) ;
	return 1 ;
    }
    
    matrixShm = new dtkSharedMem(sizeof(double)*16,matrixShmName.c_str(),0) ;
    if (matrixShm->isInvalid())
    {
	dtkMsg.add(DTKMSG_ERROR, "iris-xyzhprToMatrix: unable to open matrix shared memory with size %d\n", 16*sizeof(double)) ;
	return 1 ;
    }

    while (running)
    {
	update() ;
	usleep(ticks) ;
    }

    return 0 ;
}
