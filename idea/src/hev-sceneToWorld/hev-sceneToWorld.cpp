#include <stdio.h>
#include <string>
#include <unistd.h>
#include <dtk.h>

#include <iris.h>

// evil global variables
bool running = true ;

std::string sceneShmName ;
dtkSharedMem *sceneShm ;
osg::Matrix scene ;

std::string worldShmName ;
dtkSharedMem *worldShm ;
osg::Matrix world ;

// shared memory that's needed to convert from scene to world
// irisfly sets this up
std::string worldToSceneShmName = "idea/worldToScene" ;
dtkSharedMem *worldToSceneShm ;
osg::Matrix worldToScene; 
osg::Matrix sceneToWorld; 

void usage()
{
    fprintf(stderr,"Usage: hev-sceneToWorld [--usleep t] sceneShm worldShm\n") ;
}

void update()
{
    osg::Matrix mat ;
    sceneShm->read(mat.ptr()) ;	
	
    // update scenegraph transformation
    worldToSceneShm->read(worldToScene.ptr()) ;
    sceneToWorld.invert(worldToScene) ;

    // transform by the worldToScene matrix
    mat.postMult(sceneToWorld) ;

    // write out if it changed or first time
    static bool first = true ;
    if (first || mat != world)
    {
	first = false ;
	world = mat ;
	//iris::PrintMatrix(world) ;
	worldShm->write(world.ptr()) ;	
    }
}

int main(int argc, char **argv)
{

    // send messages to stderr
    dtkMsg.setFile(stderr) ;

    if (argc!=3 && argc!=5)
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
		dtkMsg.add(DTKMSG_ERROR, "hev-sceneToWorld: invalid usleep value\n") ;
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

    sceneShmName = argv[c] ;
    worldShmName = argv[c+1] ;

    // get shared memory
    worldToSceneShm = new dtkSharedMem(sizeof(worldToScene), worldToSceneShmName.c_str(),0) ;
    if (worldToSceneShm->isInvalid())
    {
	dtkMsg.add(DTKMSG_ERROR, "hev-sceneToWorld: can't open shared memory file %s\n",worldToSceneShmName.c_str()) ;
	return 1 ;
    }

    sceneShm = new dtkSharedMem(sizeof(double)*16,sceneShmName.c_str(),0) ;
    if (sceneShm->isInvalid()) return 1 ;
    
    worldShm = new dtkSharedMem(sizeof(double)*16,worldShmName.c_str(),0) ;
    if (worldShm->isInvalid()) return 1 ;
    
    while (running)
    {
	update() ;
	usleep(ticks) ;
    }

    return 0 ;
}
