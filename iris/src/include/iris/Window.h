#ifndef __IRIS_WINDOW__
#define __IRIS_WINDOW__

#include <osg/Referenced>
#include <osg/GraphicsContext>
#include <iris/Pane.h>

namespace iris
{

    /**
       \brief The %Window class creates an osg::GraphicsContext::Traits
       structure and a osg::GraphicsContext, effectively defining a graphics
       window.  
       \n \n Windows can be either stereo or mono.  A %Window has one or more Pane
       objects atached to it.
    */

    class Window : public osg::Referenced
    {
    public:

    Window(const char* name, bool addToList = true) : _realized(false), _traits(NULL), _gc(NULL)  
	{ 
	    if (name) _name = strdup(name) ;
	    else _name = NULL ;
	    if (addToList) _windowList.push_back(this) ;
	    _traits = new osg::GraphicsContext::Traits ;
	    _traits->windowName = _name ;
	    _traits->swapGroupEnabled = true ;
	} ;

    Window(bool addToList = true) : _traits(NULL), _gc(NULL), _name(NULL) 
	{ 
	    if (addToList) _windowList.push_back(this) ;
	    _traits = new osg::GraphicsContext::Traits ;
	    _traits->swapGroupEnabled = true ;
	} ;

	// anybody can mess with the list
	static std::vector<osg::ref_ptr<Window> > const getWindowList() { return _windowList ; } ;
		
	virtual bool control(const std::string& line) ;
	virtual bool control(const std::string& line, const std::vector<std::string>&) ;

	bool getStereo() { return getTraits()->quadBufferStereo ; } ;
	void setStereo(bool stereo) { getTraits()->quadBufferStereo = stereo ; } ;
	    
	osg::GraphicsContext::Traits* const getTraits() { return _traits.get() ; } ;

	osg::GraphicsContext* const getGraphicsContext() { return _gc.get() ; } ;

	// anybody can mess with the list
	std::vector<osg::ref_ptr<Pane> > const getPaneList() { return _paneList ; } ;

	// add returns true if the pane isn't already in the list
	bool addPane(Pane* pane) ;
#if 0
	// remove returns true if the pane was in the list
	bool removePane(Pane* pane) ;
#endif
	// why doesn't this use std::string instead of char*?
	static Window* const findWindow(const char* name) ;
	const char* getName() { return _name ; } ;
	void setName(const char* name) { _name = name ; } ;

	// create the window
	virtual void realize() ;

    private:
	static std::vector<osg::ref_ptr<Window> > _windowList ;
	void _setTraits(Window* win, std::vector<std::string> vec) ;
	bool _realized ;
	std::vector<osg::ref_ptr<Pane> > _paneList ;
	osg::ref_ptr<osg::GraphicsContext::Traits> _traits ;
	osg::ref_ptr<osg::GraphicsContext> _gc ;
	const char* _name ;
    } ;

}
#endif
