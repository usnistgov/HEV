#include <stdio.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

#include <iris/SceneGraph.h>
#include <iris/Window.h>
#include <iris/Augment.h>


// do this so this DSO's callbacks don't interfere with other DSO's
namespace monoDisplayMod
{
    class monoDisplayMod : public iris::Augment
    {
    public:

	monoDisplayMod():iris::Augment("monoDisplayMod")
	{
	    setDescription("sets all loaded Windows to mono") ;

	    validate(); 

	    if (iris::SceneGraph::instance()->isRealized())
	    {
		dtkMsg.add(DTKMSG_ERROR, "monoDisplayMod: windows have already been realized- too late to set to mono\n") ;
		return ;
	    }
	    const std::vector<osg::ref_ptr<iris::Window> > wl = iris::Window::getWindowList() ;
	    for (unsigned int i=0; i<wl.size(); i++)
	    {
		iris::Window* w = wl[i].get() ;
		//dtkMsg.add(DTKMSG_INFO, "monoDisplayMod: %d %s\n",wl[0]->getName()) ;
		if (w)
		{
		    if (w->getStereo())
		    {
			dtkMsg.add(DTKMSG_INFO, "monoDisplayMod: window %d %s is in stereo, setting to mono\n",i,wl[i].get()->getName()) ;
			w->setStereo(false) ;
		    }
		    else dtkMsg.add(DTKMSG_INFO, "monoDisplayMod: window %d %s is already in mono\n",i,wl[i].get()->getName()) ;
		}
		else dtkMsg.add(DTKMSG_WARNING, "monoDisplayMod: window %d has a null pointer\n") ;
	    }

	}
    };
} ;


/************ DTK C++ dtkAugment loader/unloader functions ***************
 *
 * All DTK dtkAugment DSO files are required to declare these two
 * functions.  These function are called by the loading program to get
 * your C++ objects loaded.
 *
 *************************************************************************/

static dtkAugment* dtkDSO_loader(dtkManager* manager, void* p)
{
    return new monoDisplayMod::monoDisplayMod ;
}

static int dtkDSO_unloader(dtkAugment* augment)
{
#if 0
    delete augment;
#endif
    return DTKDSO_UNLOAD_CONTINUE;
}

