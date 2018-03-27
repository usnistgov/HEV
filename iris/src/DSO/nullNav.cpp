// a navigation that does nothing!

#include <stdio.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

#include <iris/Augment.h>
#include <iris/Nav.h>
#include <iris/Utils.h>
#include <iris/SceneGraph.h>


class nullNav : public iris::Augment
{
public:

    nullNav():iris::Augment("nullNav")
    {
	setDescription("a DSO that navigates that does nothing!") ;

	osg::ref_ptr<iris::Nav> nav = new iris::Nav("nullNav") ;

	validate(); 
    }

    // every navigation DSO should implement a control method, even if
    // it only passes it along to the base class
    virtual bool control(const std::string& line, const std::vector<std::string>& vec)
    {
	// pass to Nav class
	return iris::Nav::getCurrentNav()->control(line, vec) ;
    }

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
    return new nullNav ;
}

static int dtkDSO_unloader(dtkAugment* augment)
{
    return DTKDSO_UNLOAD_CONTINUE;
}

