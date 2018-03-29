#include <iris/Augment.h>
#include <iris/SceneGraph.h>
#include <iris/Utils.h>

namespace iris
{

    bool Augment::control(const std::string& line)
    {
	return control(line, ParseString(line)) ;
    }

    bool Augment::control(const std::string& line, const std::vector<std::string>& vec)
    {
	if (vec.size()==0)
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::Augment: DSO %s loaded, but no control command passed\n",getName()) ;
	    return true ;
	}

	// this implements a DSO specific control command in the base class
	// right now there aren't any class (i.e. static method) control commands
	// how to distinguish between class and instance control commands?
	if (IsSubstring("active",vec[0],3) && vec.size() == 2)
	{
	    bool onOff ;
	    if (OnOff(vec[1],&onOff))
	    {
		if (onOff)
		{
		    dtkMsg.add(DTKMSG_INFO, "iris::Augment: DSO %s is now active\n",getName()) ;
		    setActive(true) ;
		}
		else
		{
		    dtkMsg.add(DTKMSG_INFO, "iris::Augment: DSO %s is inactive\n",getName()) ;
		    setActive(false) ;
		}
		return true ;
	    }
	    else
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::Augment: DSO %s::control: invalid active paramater: \"%s\"\n",getName(),vec[1].c_str()) ;  
		return false ;
	    }
	}
	else if (IsSubstring("active",vec[0],3) && vec.size() != 2)
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::Augment: DSO %s::control: wrong number of paramaters for active command\n",getName()) ;  
	    return false ;
	}
	else 
	{
	    dtkMsg.add(DTKMSG_INFO, "iris::Augment: control: %s passed: \"%s\"\n",getName(),line.c_str()) ;
	    return false ;
	}
	return false ;
    } 


    ////////////////////////////////////////////////////////////////////////
    Augment* Augment::getOrLoadDSO(const char* name)
    {
#if 1
	//fprintf(stderr,"Augment::getOrLoadDSO(%s)\n",name) ;
	const char* b = rindex(name,'/') ;
	char* basename ;
	if (b==NULL) basename = strdup(name) ;
	else basename = strdup(b+1) ;
	//fprintf(stderr,"name = %s, basename = %s\n",name,basename) ;

	dtkAugment* augment = SceneGraph::instance()->check(basename) ;
	if (!augment)
	{
	    //fprintf(stderr,"%s %d: before calling load\n",__FILE__,__LINE__) ;
	    SceneGraph::instance()->load(name) ;
	    //fprintf(stderr,"%s %d: after calling load, before calling instance\n",__FILE__,__LINE__) ;
	    augment = static_cast<dtkManager*>(SceneGraph::instance())->get(basename) ;
	    //fprintf(stderr,"%s %d: dtkAugment = %p\n",__FILE__,__LINE__,dynamic_cast<Augment*>(augment)) ;
	}
	//else fprintf(stderr,"%s already loaded\n",name) ;

	// uncomment this for a bad free()!!
	//free(basename) ;

	//fprintf(stderr,"Augment::getOrLoadDSO %s returning %p\n",name,augment) ;
	Augment* irisAugment = dynamic_cast<Augment*>(augment) ;
	return irisAugment ; 
	_augments.push_back(irisAugment) ;
#else
	//fprintf(stderr,"Augment::getOrLoadDSO(%s)\n",name) ;
	char* b = rindex(name,'/') ;
	char* basename ;
	if (b==NULL) basename = strdup(name) ;
	else basename = b+1 ;
	//fprintf(stderr,"name = %s, basename = %s\n",name,basename) ;

	dtkAugment* augment = SceneGraph::instance()->check(basename) ;
	if (!augment)
	{
	    //fprintf(stderr,"%s %d: before calling load\n",__FILE__,__LINE__) ;
	    SceneGraph::instance()->load(name) ;
	    //fprintf(stderr,"%s %d: after calling load, before calling instance\n",__FILE__,__LINE__) ;
	    augment = static_cast<dtkManager*>(SceneGraph::instance())->get(basename) ;
	    //fprintf(stderr,"%s %d: dtkAugment = %p\n",__FILE__,__LINE__,dynamic_cast<Augment*>(augment)) ;
	}
	//else fprintf(stderr,"%s already loaded\n",name) ;

	// uncomment this for a bad free()!!
	//free(basename) ;

	//fprintf(stderr,"Augment::getOrLoadDSO %s returning %p\n",name,augment) ;
	return dynamic_cast<Augment*>(augment) ;
#endif
    }

    ////////////////////////////////////////////////////////////////////////
    std::vector<Augment*> Augment::_augments ;
    
}
