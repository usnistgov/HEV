#include <stdio.h>
#include <string>
#include <unistd.h>
#include <dtk.h>

#include <iris.h>

// evil global variables
bool running = true ;

std::string worldShmName ;
dtkSharedMem *worldShm ;
osg::Matrix world ;

std::string sceneShmName ;
dtkSharedMem *sceneShm ;
osg::Matrix scene ;

// shared memory that's needed to convert from world to scene
// irisfly sets this up
std::string worldToSceneShmName = "idea/worldToScene" ;
dtkSharedMem *worldToSceneShm ;
osg::Matrix worldToScene; 

void usage()
{
    fprintf(stderr,"Usage: hev-worldToScene [--usleep t] worldShm sceneShm\n") ;
}

void update()
{
    osg::Matrix mat ;
    worldShm->read(mat.ptr()) ;	
	
    // update scenegraph transformation
    worldToSceneShm->read(worldToScene.ptr()) ;

    // transform by the sceneToWorld matrix
    mat.postMult(worldToScene) ;

    // write out if it changed or first time
    static bool first = true ;
    if (first || mat != scene)
    {
	first = false ;
	scene = mat ;
	//iris::PrintMatrix(scene) ;
	sceneShm->write(scene.ptr()) ;	
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
		dtkMsg.add(DTKMSG_ERROR, "hev-worldToScene: invalid usleep value\n") ;
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

    worldShmName = argv[c] ;
    sceneShmName = argv[c+1] ;

    // get shared memory
    worldToSceneShm = new dtkSharedMem(sizeof(worldToScene), worldToSceneShmName.c_str(),0) ;
    if (worldToSceneShm->isInvalid())
    {
	dtkMsg.add(DTKMSG_ERROR, "hev-worldToScene: can't open shared memory file %s\n",worldToSceneShmName.c_str()) ;
	return 1 ;
    }

    worldShm = new dtkSharedMem(sizeof(double)*16,worldShmName.c_str(),0) ;
    if (worldShm->isInvalid()) return 1 ;
    
    sceneShm = new dtkSharedMem(sizeof(double)*16,sceneShmName.c_str(),0) ;
    if (sceneShm->isInvalid()) return 1 ;
    
    while (running)
    {
	update() ;
	usleep(ticks) ;
    }

    return 0 ;
}
