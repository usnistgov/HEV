#ifndef __IRIS_AUGMENT__
#define __IRIS_AUGMENT__

#include <dtk.h>
#include <osg/Referenced>

namespace iris
{
    /**
       \brief The %Augment class is sub-classed from the dtkAugment class.  All IRIS DSOs
       are based on the %Augment class.  
       \n \n dtkAugment callbacks are not supported (although hooks exists in IRIS
       to implement them if the need arises), and instead each %Augment
       object should define its own OSG callbacks, such as event handlers
       and node callbacks.
    */

    class Augment: public dtkAugment, public osg::Referenced
    {
    public:
    Augment(const char* name = NULL):dtkAugment(name), _active(true) { _augments.push_back(this) ; } ;
    Augment(bool addToList = true):dtkAugment(NULL), _active(addToList) { _augments.push_back(this) ; } ;
	// get/set active
	virtual bool getActive() { return _active ; } ;
	virtual void setActive(bool active) { _active = active ; } ;

	/** 
	    this is called by the SceneGraph's destructor, as the DSO's destructor doesn't seem to get called 

	    overload it to implement any cleanup procedures needed by the DSO before exiting
	*/
	virtual void cleanup() { dtkMsg.add(DTKMSG_DEBUG,"iris::Augment::Cleanup called\n") ; } ;

	// any DSO can receive a command to modify its behavior
	virtual bool control(const std::string& line) ;
	virtual bool control(const std::string& line, const std::vector<std::string>&) ;
	static Augment* getOrLoadDSO(const char* name) ;
	static std::vector<Augment*> getAugments() { return _augments ; } ;
    private:
	bool _active ;
	static std::vector<Augment*> _augments ;
    } ;
}
#endif
