#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include <iris.h>

void usage()
{
    fprintf(stderr,"Usage: hev-matrixToCoord [--[x][y][z][h][p][r][s]] [--loop] [--every] [--usleep t] shm\n") ;
}

////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{

    bool x = true ;
    bool y = true ;
    bool z = true ;
    bool h = true ;
    bool p = true ;
    bool r = true ;
    bool s = false ;

    bool loop = false ;
    bool every = false ;

    // send dtk messages to stdout
    dtkMsg.setFile(stderr) ;
    
    if (argc < 2 || argc > 7)
    {
	usage() ;
	return 1 ;
    }
    
    int ticks = iris::GetUsleep() ;

    int c = 1 ;
    while (c<argc && argv[c][0] == '-' && argv[c][1] == '-')
    {
	if (iris::IsSubstring("--loop",argv[c],3))
	{
	    loop = true ;
	    c++ ;
	}
	else if (iris::IsSubstring("--usleep",argv[c],3))
	{
	    c++ ;
	    if (c<argc && !iris::StringToInt(argv[c],&ticks)) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-matrixToCoord: invalid usleep value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c++ ;
	}
	else if (iris::IsSubstring("--every",argv[c],3))
	{
	    every = true ;
	    c++ ;
	}
	else if (argv[c][0] == '-' && argv[c][1] == '-')
	{
	    x = y = z = h = p = r = s = false ;
	    for (int i=2; argv[c][i] != '\0'; i++)
	    {
		if (argv[c][i] == 'x') x = true ;
		else if (argv[c][i] == 'y') y = true ;
		else if (argv[c][i] == 'z') z = true ;
		else if (argv[c][i] == 'h') h = true ;
		else if (argv[c][i] == 'p') p = true ;
		else if (argv[c][i] == 'r') r = true ;
		else if (argv[c][i] == 's') s = true ;
		else
		{
		    usage() ;
		    return 1 ;
		}
	    }
	    c++ ;
	}
	else 
	{
	    usage() ;
	    return 1 ;
	}
    }

    if (c+1 != argc)
    {
	usage() ;
	return 1 ;
    }
    osg::Matrix mat ;
    osg::Matrix oldMat ;
    bool first = true ;

    dtkSharedMem *shm = new dtkSharedMem(sizeof(mat),argv[c]) ;
    if (shm->isInvalid()) return 1 ;

    while (1)
    {
	shm->read(mat.ptr()) ;
	if (first || every || mat != oldMat)
	{
	    osg::Vec3d t ;
	    osg::Quat q  ;
	    osg::Vec3d sc ; 
	    iris::Decompose(mat, &t, &q, &sc) ;
	    
	    double hpr[3] ;
	    iris::QuatToEuler(q, hpr, hpr+1, hpr+2) ;

	    if (x) printf("%.17g ",t.x()) ;
	    if (y) printf("%.17g ",t.y()) ;
	    if (z) printf("%.17g ",t.z()) ;
	    if (h) printf("%.17g ",hpr[0]) ;
	    if (p) printf("%.17g ",hpr[1]) ;
	    if (r) printf("%.17g ",hpr[2]) ;
	    if (s) printf("%.17g ",sc.x()) ;
	    printf("\n") ;
	    fflush(stdout) ;
	    if (!every) oldMat = mat ;
	    first = false ;
	}
	if (!loop) break ;
	usleep(ticks) ;
    }
    
    return 0 ;

}
