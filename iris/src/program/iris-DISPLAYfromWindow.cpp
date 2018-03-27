#include <osg/GraphicsContext>

#include <iris/SceneGraph.h>
#include <iris/Utils.h>
#include <iris/Window.h>

////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{

    // send messages to stderr
    dtkMsg.setFile(stderr) ;

    // load DSOs
    iris::LoadFile(argc, argv) ;

    // find the first window
    std::vector<osg::ref_ptr<iris::Window> > wl = iris::Window::getWindowList() ;
    if (wl.size()==0)
    {
	dtkMsg.add(DTKMSG_ERROR,"iris-DISPLAYfromWindow: no Windows defined\n") ;
	return 1 ;
    }
    else if (wl.size()>1)
    {
	dtkMsg.add(DTKMSG_ERROR,"iris-DISPLAYfromWindow: %d Windows defined\n",wl.size()) ;
	return 1 ;
    }

    osg::GraphicsContext::Traits* traits = wl[0]->getTraits() ;
    printf("%s\n",traits->displayName().c_str()) ;
    dtkMsg.add(DTKMSG_INFO,"iris-DISPLAYfromWindow: DISPLAY=%s\n",traits->displayName().c_str()) ;
    return 0 ;
}
