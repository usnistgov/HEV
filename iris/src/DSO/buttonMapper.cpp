#include <stdio.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

#include <iris/Augment.h>
#include <iris/SceneGraph.h>

#include <osgGA/GUIEventHandler>

// do this so this DSO's callbacks don't interfere with other DSO's
namespace buttonMapper
{
    // evil global variables to hopefully speed things up
    static std::vector<std::pair<unsigned char,dtkSharedMem*> > currentMap ;
    static dtkSharedMem* buttonShm = NULL ;

    ////////////////////////////////////////////////////////////////////////
    class buttonMapperEventHandler : public osgGA::GUIEventHandler 
    {
    public: 
	
	buttonMapperEventHandler(iris::Augment* dso) : _dso(dso), _oldButtons(0)
	{
	    
	} ;

	bool handle( const osgGA::GUIEventAdapter& ea,
		     osgGA::GUIActionAdapter& aa)
	{
	    if (!_dso || _dso->isInvalid())
	    {
		dtkMsg.add(DTKMSG_ERROR, "buttonMapper: can't get DSO object pointer!") ;
		return false ;
	    }

	    // every navigation should see if it's active if it wants to honor the active() method
	    if (!_dso || _dso->isInvalid() || !_dso->getActive()) return false ;

	    if (ea.getEventType() == osgGA::GUIEventAdapter::FRAME)
	    { 
		unsigned char buttons ;
		if (buttonShm)
		{
		    int ret = buttonShm->qread(&buttons) ;
		    if (ret > 0 && buttons != _oldButtons)
		    {
			_oldButtons = buttons ;
			//printf("buttons = %d\n",buttons) ;
			for (unsigned int i=0; i<currentMap.size(); i++)
			{
			    //printf("  currentMap[%d].first = %d\n",i, currentMap[i].first) ;
			    if (buttons & currentMap[i].first)
			    {
				unsigned int one = 1 ;
				currentMap[i].second->write(&one) ;
			    }
			    else
			    {
				unsigned int zero = 0 ;
				currentMap[i].second->write(&zero) ;
			    }
			}
		    }
		}
	    }
	    return false ;
	}
    private:
	iris::Augment* _dso ;
	unsigned char _oldButtons ;

    } ;

    ////////////////////////////////////////////////////////////////////////
    class buttonMapper : public iris::Augment
    {
    public:

	buttonMapper():iris::Augment("buttonMapper"), _badButtonShm(false)
	{
	    setDescription("routes buttons/masks to shared memory files") ;
	    _buttonMapperEventHandler = new buttonMapperEventHandler(this) ;
	    iris::SceneGraph::instance()->getViewer()->addEventHandler(_buttonMapperEventHandler.get());
	    validate(); 
	}
    
	bool control(const std::string& line, const std::vector<std::string>& vec)
	{
#if 0
	    dtkMsg.add(DTKMSG_INFO, "iris::buttonMapper: in the actual DSO, line = %s\n",line.c_str()) ;
	    for (int i=0; i<vec.size(); i++)
	    {
		printf("%d: %s\n",i,vec[i].c_str()) ;
	    }
#endif
	    bool onOff ;
	    if (iris::IsSubstring("buttons",vec[0],3) && vec.size() == 2)
	    {
		if (buttonShm)
		{
		    delete buttonShm ;
		    buttonShm = NULL ;
		}
		buttonShm = new dtkSharedMem(sizeof(unsigned char),vec[1].c_str()) ;
		if (buttonShm->isInvalid())
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris::buttonMapper: can't open button shared memory file %s\n",vec[1].c_str()) ;
		    buttonShm = NULL ;
		    _badButtonShm = true ;
		    return false ;
		}
		buttonShm->queue(256) ;

		dtkMsg.add(DTKMSG_INFO, "iris::buttonMapper: reading buttons from shared memory file %s\n",vec[1].c_str()) ;
		return true ;
	    }
	    else if (iris::IsSubstring("map",vec[0],3))
	    {
		if (vec.size()<4 || vec.size()%2 != 0) return false ;
		_pos = _buttonMap.find(vec[1]) ;
		if (_pos != _buttonMap.end()) 
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris::buttonMapper: map \"%s\" already in buttonMap\n",vec[1].c_str()) ;
		    return false ;
		}
		for (int j=2; j<vec.size(); j+=2)
		{
		    int n ;
		    unsigned char mask ;
		    if (!iris::StringToInt(vec[j],&n) || n<0 || n>255)
		    {
			dtkMsg.add(DTKMSG_ERROR, "iris::buttonMapper: invalid buton mask \"%s\"\n",vec[j].c_str()) ;
			return false ;
		    }
		    mask = (unsigned char)n ;
		    for (_pos = _buttonMap.lower_bound(vec[1]) ; _pos != _buttonMap.upper_bound(vec[1]); _pos++)
		    {
			std::pair<unsigned char,dtkSharedMem*> value = _pos->second ;
			if (value.first == mask)
			{
			    dtkMsg.add(DTKMSG_ERROR, "iris::buttonMapper: duplicate button mask %d for map\n",mask,vec[1].c_str()) ;
			    return false ;
			}
		    }
		    dtkSharedMem* mem = new dtkSharedMem(sizeof(unsigned char),vec[j+1].c_str()) ;
		    if (mem->isInvalid())
		    {
			dtkMsg.add(DTKMSG_ERROR, "iris::buttonMapper: can't open button shared memory file %s\n",vec[j+1].c_str()) ;
			return false ;
		    }
		    dtkMsg.add(DTKMSG_INFO, "iris::buttonMapper: map %s: mapping button mask %d to shared memory file %s\n",vec[1].c_str(),mask,vec[j+1].c_str()) ;
		    _buttonMap.insert(std::make_pair(vec[1], std::make_pair(mask,mem) ) ) ; 
		}
		return true ;
	    }
	    else if (iris::OnOff(vec[0],&onOff))
	    {
		if (onOff && vec.size() == 2)
		{
		    // look for name in map
		    _pos = _buttonMap.find(vec[1]) ;
		    if (_pos == _buttonMap.end()) 
		    {
			dtkMsg.add(DTKMSG_ERROR, "iris::buttonMapper: can't find a map with name %s\n",vec[1].c_str()) ;
			return false ;
		    }
		    // get the default button source if none has been set yet
		    if (!buttonShm && !_badButtonShm)
		    {
			dtkMsg.add(DTKMSG_INFO, "iris::buttonMapper: using default button shared memorys\n") ;
			buttonShm = new dtkSharedMem(sizeof(unsigned char),"buttons") ;
			if (buttonShm->isInvalid())
			{
			    dtkMsg.add(DTKMSG_ERROR, "iris::buttonMapper: can't open default button shared memory file %s\n") ;
			    buttonShm = NULL ;
			    _badButtonShm = true ;
			    return false ;
			}
			buttonShm->queue(256) ;
			buttonShm->flush() ;
		    }
		    // turn on the named one and turn off all the others
		    currentMap.clear() ;
		    for (_pos =_buttonMap.lower_bound(vec[1]) ; _pos != _buttonMap.upper_bound(vec[1]); _pos++)
		    {
			currentMap.push_back(_pos->second) ;
		    }
		}
		else if (!onOff && vec.size() == 1)
		{
		    // turn them all off
		    currentMap.clear() ;
		}
		else return false ;
		return true ;
	    }
	    // if the DSO defines a control method you have to call the base class's control method too
	    return iris::Augment::control(line,vec) ;
	}

    private:
	std::multimap<std::string,std::pair<unsigned char,dtkSharedMem*> > _buttonMap ;
	std::multimap<std::string,std::pair<unsigned char,dtkSharedMem*> >::iterator _pos ;
	bool _badButtonShm ;
	osg::ref_ptr<buttonMapperEventHandler> _buttonMapperEventHandler ;
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
    return new buttonMapper::buttonMapper ;
}

static int dtkDSO_unloader(dtkAugment* augment)
{
#if 0
    delete augment;
#endif
    return DTKDSO_UNLOAD_CONTINUE;
}

