// run with iris-viewer tim examples/timTest.iris

#include <signal.h>
#include <dtk.h>
#include <osg/Matrix>
#include <osg/Vec3>

#include <iris.h>

struct Object
{
    osg::Matrix* mat ;
    char base[255] ;
} ;
Object objects[6] ;
bool done = false ;

////////////////////////////////////////////////////////////////////////
// for catching the KILL, QUIT, ABRT, TERM and INT signals
static void signal_catcher(int sig)
{
    dtkMsg.add(DTKMSG_INFO,"timTest:: PID %d, caught signal %d, starting exit sequence ...\n", getpid(), sig);
#if 0
    // the SceneGraph class will do this
    for (int i=0; i<6; i++)
    {
	printf("unload %s.matrix\nunload %s\n",objects[i].base,objects[i].base) ;
    }
    fflush(stdout) ;
#endif
    done = true ;
}

////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    // send messages to stderr
    dtkMsg.setFile(stderr) ;
    
    // catch signals that kill us off
    iris::Signal(signal_catcher); 

    const float bound = 10.f ;

    printf("dso tim begin timTest\n") ;

    // create scenegraph
    osg::Matrix positions[6] ;
    int i = 0 ;
    objects[i].mat = &positions[i] ;
    objects[i].mat->makeTranslate(osg::Vec3(bound,0,0)) ;
    strcpy(objects[i].base,"001") ;

    i++ ;
    objects[i].mat = &positions[i] ;
    objects[i].mat->makeTranslate(osg::Vec3(-bound,0,0)) ;
    strcpy(objects[i].base,"110") ;

    i++ ;
    objects[i].mat = &positions[i] ;
    objects[i].mat->makeTranslate(osg::Vec3(0,bound,0)) ;
    strcpy(objects[i].base,"010") ;

    i++ ;
    objects[i].mat = &positions[i] ;
    objects[i].mat->makeTranslate(osg::Vec3(0,-bound,0)) ;
    strcpy(objects[i].base,"101") ;


    i++ ;
    objects[i].mat = &positions[i] ;
    objects[i].mat->makeTranslate(osg::Vec3(0,0,bound)) ;
    strcpy(objects[i].base,"011") ;

    i++ ;
    objects[i].mat = &positions[i] ;
    objects[i].mat->makeTranslate(osg::Vec3(0,0,-bound)) ;
    strcpy(objects[i].base,"100") ;

    for (i=0; i<6; i++)
    {
	printf("matrix %s.matrix\naddchild %s.matrix world\n",objects[i].base,objects[i].base) ;
	printf("load %s examples/%s.osg\naddchild %s %s.matrix\n",objects[i].base,objects[i].base,objects[i].base,objects[i].base) ;
	printf("dso tim %s.matrix\n",objects[i].base) ;
    }

    // open the shared memory
    dtkSharedMem* timTest = new dtkSharedMem(sizeof(positions),"timTest") ;
    timTest->write(positions) ;

    // tell tim all done. it then opens the shared memory
    printf("dso tim end timTest\n") ;

    // do once for for examine mode
    printf("dso tim update timTest\nexamine\nframe\n") ; fflush(stdout) ;

    printf("dso tim start timTest\nframe\n\n") ; fflush(stdout) ;
    // move them around
    while (!done)
    {
	objects[0].mat->preMultRotate(iris::EulerToQuat(.1f, 0.f, 0.f)) ;
	objects[1].mat->preMultRotate(iris::EulerToQuat(-.2f, 0.f, 0.f)) ;
	objects[2].mat->preMultRotate(iris::EulerToQuat(0.f, .3f, 0.f)) ;
	objects[3].mat->preMultRotate(iris::EulerToQuat(0.f, -.4f, 0.f)) ;
	objects[4].mat->preMultRotate(iris::EulerToQuat(0.f, 0.f, .5f)) ;
	objects[5].mat->preMultRotate(iris::EulerToQuat(0.f, 0.f, -.6f)) ;
	timTest->write(positions) ;
	usleep(iris::GetUsleep()) ;
    }


    return 0 ;
}
