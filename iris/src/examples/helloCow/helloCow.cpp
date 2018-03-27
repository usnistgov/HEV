// simple iris example

// ./helloCow [--foo] cow.osg
// env IRIS_FILES=cow.osg:raveSimDisplay ./helloCow --foo
// ./helloCow helloCow.iris
// env IRIS_FILES=helloCow.iris ./helloCow 

#include <osgDB/ReadFile>

#include <iris.h>

////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{

    // creates the scenegraph and viewer. loads DSOs in IRIS_FILES envvar
    iris::SceneGraph isg ;

    // use an ArgumentParser object to manage the program arguments.
    iris::ArgumentParser args(&argc,argv);

    // load DSOs and model files and put model files under the world node
    iris::LoadFile(&args, isg.getWorldNode()) ;

    // put the model files front and center
    isg.examine() ;

#if 0
    // run for 500 frames or until escape pressed in graphics, or ^C (or other signal) is caught
    while (!isg.done() && isg.getViewer()->getFrameStamp()->getFrameNumber()<500)
    {
	isg.frame() ;
    }
    return 1 ;
#else    
    // run until escape pressed in graphics, or ^C (or other signal) is caught
    return isg.run() ;
#endif
}
