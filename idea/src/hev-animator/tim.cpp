#include <dtk.h>
#include <osg/Matrix>
#include <osg/Vec3>

#include <iris.h>

// names of the 6 objects to tim around
std::string objects[6] ;

// howMany frames of time data- hev-animator goes 0 - (howMany - 1)
const int howMany = 360 ;

// holds al the frames for all of the object
// the subscript order is important!
osg::Matrix frames[howMany][6] ;

bool done = false ;
////////////////////////////////////////////////////////////////////////
// for catching the KILL, QUIT, ABRT, TERM and INT signals
static void signal_catcher(int sig)
{
    dtkMsg.add(DTKMSG_INFO,"tim:: PID %d, caught signal %d, starting exit sequence ...\n", getpid(), sig);
    for (int i=0; i<6; i++)
    {
	printf("unload %s.matrix\nunload %s\n",objects[i].c_str(),objects[i].c_str()) ;
    }
    fflush(stdout) ;
    done = true ;
}

////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    // send messages to stderr
    dtkMsg.setFile(stderr) ;
    
    // catch signals that kill us off
    iris::Signal(signal_catcher); 

    // need to set up dtk shared memory for reading frame number set by animator
    int frameNum ;

    int i = 0 ;
    const float bound = 10.f ;

    frames[0][i].makeTranslate(osg::Vec3(bound,0,0)) ;
    // compute frames to move them around- build on frame 0
    for (int j=1; j<howMany; j++)
    {
	frames[j][i] = frames[j-1][i] ;
	frames[j][i].preMultRotate(iris::EulerToQuat(1.f, 0.f, 0.f)) ;
    }
    objects[i] = "001" ;

    i++ ;
    frames[0][i].makeTranslate(osg::Vec3(-bound,0,0)) ;
    for (int j=1; j<howMany; j++)
    {
	frames[j][i] = frames[j-1][i] ;
	frames[j][i].preMultRotate(iris::EulerToQuat(-2.f, 0.f, 0.f)) ;
    }
    objects[i] = "110" ;

    i++ ;
    frames[0][i].makeTranslate(osg::Vec3(0,bound,0)) ;
    for (int j=1; j<howMany; j++)
    {
	frames[j][i] = frames[j-1][i] ;
	frames[j][i].preMultRotate(iris::EulerToQuat(0.f, 3.f, 0.f)) ;
    }
    objects[i] = "010" ;

    i++ ;
    frames[0][i].makeTranslate(osg::Vec3(0,-bound,0)) ;
    for (int j=1; j<howMany; j++)
    {
	frames[j][i] = frames[j-1][i] ;
	frames[j][i].preMultRotate(iris::EulerToQuat(0.f, -4.f, 0.f)) ;
    }
    objects[i] = "101" ;

    i++ ;
    frames[0][i].makeTranslate(osg::Vec3(0,0,bound)) ;
    for (int j=1; j<howMany; j++)
    {
	frames[j][i] = frames[j-1][i] ;
	frames[j][i].preMultRotate(iris::EulerToQuat(0.f, 0.f, 5.f)) ;
    }
    objects[i] = "011" ;

    i++ ;
    frames[0][i].makeTranslate(osg::Vec3(0,0,-bound)) ;
    for (int j=1; j<howMany; j++)
    {
	frames[j][i] = frames[j-1][i] ;
	frames[j][i].preMultRotate(iris::EulerToQuat(0.f, 0.f, -6.f)) ;
    }
    objects[i] = "100" ;

    // create scenegraph
    printf("dso tim begin tim\n") ;

    for (i=0; i<6; i++)
    {
	printf("matrix %s.matrix\naddchild %s.matrix world\n",objects[i].c_str(),objects[i].c_str()) ;
	printf("load %s examples/%s.osg\naddchild %s %s.matrix\n",objects[i].c_str(),objects[i].c_str(),objects[i].c_str(),objects[i].c_str()) ;
	printf("dso tim %s.matrix\n",objects[i].c_str()) ;
    }

    // open the shared memory
    dtkSharedMem* tim = new dtkSharedMem(sizeof(double)*16*6,"tim") ;
    tim->write(&(frames[0][0])) ;

    // tell tim all done.
    printf("dso tim end tim\n") ;

    // do once for for examine mode
    printf("dso tim update tim\nexamine\nframe\n") ; fflush(stdout) ;

    printf("dso tim start tim\nframe\n\n") ; fflush(stdout) ;

    int frame ;
    int oldFrame = -1 ;
    dtkSharedMem* frameShm = new dtkSharedMem(sizeof(int),"tmp/hev-animator") ;
    while (!done)
    {
	// read the shared memory from the animator
	frameShm->read(&frame) ;
	if (frame != oldFrame)
	{
	    // if it's different, write a different block of shared memory
	    //fprintf(stderr,"frame %d\n",frame) ;
	    oldFrame = frame ;
	    tim->write(&(frames[frame][0])) ;
	}
	usleep(iris::GetUsleep()) ;
    }
    return 0 ;
}
