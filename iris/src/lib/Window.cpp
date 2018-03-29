#include <dtk.h>
#include <iris/SceneGraph.h>
#include <iris/Window.h>
#include <iris/Pane.h>
#include <iris/Utils.h>

namespace iris
{

    ////////////////////////////////////////////////////////////////////////
    bool Window::control(const std::string& line)
    {
	return control(line, ParseString(line)) ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool Window::control(const std::string& line, const std::vector<std::string>& vec)
    {
	dtkMsg.add(DTKMSG_INFO, "iris::Window::control passed: \"%s\"\n",line.c_str()) ;

	// stereo
	bool onOff ;
	Window* win ;
	if (vec.size()==3 && (win = findWindow(vec[0].c_str())) && IsSubstring("stereo", vec[1], 3) && OnOff(vec[2],&onOff))
	{
	    if (onOff) dtkMsg.add(DTKMSG_INFO, "iris::Window::control: turning on stereo for window named %s\n",vec[0].c_str()) ;
	    else dtkMsg.add(DTKMSG_INFO, "iris::Window::control: turning off stereo for window named %s\n",vec[0].c_str()) ;
	    win->setStereo(onOff) ;
	    return true ;
	}
	else if (vec.size()==2 && IsSubstring("stereo", vec[0], 3) && OnOff(vec[1],&onOff))
	{
	    if (onOff) dtkMsg.add(DTKMSG_INFO, "iris::Window::control: turning on stereo for all windows\n") ;
	    else dtkMsg.add(DTKMSG_INFO, "iris::Window::control: turning off stereo for all windows\n") ;
	    for (unsigned int i=0 ; i<_windowList.size() ; i++)
	    {
		(_windowList[i])->setStereo(onOff) ;	
	    }
	    return true ;
	}

	// misc traits
	else if (vec.size()>=4 && (vec.size()%2==0) && (win = findWindow(vec[0].c_str())) && IsSubstring("traits", vec[1], 3))
	{
	    std::vector<std::string> vec2 = vec ;
	    vec2.erase(vec2.begin()) ;
	    vec2.erase(vec2.begin()) ;
	    _setTraits(win,vec2) ;
	    return true ;
	}
	else if (vec.size()>=3 && (vec.size()%2==1) && IsSubstring("traits", vec[0], 3))
	{
	    std::vector<std::string> vec2 = vec ;
	    vec2.erase(vec2.begin()) ;
	    for (unsigned int i=0 ; i<_windowList.size() ; i++)
	    {
		_setTraits(_windowList[i].get(),vec2) ;	
	    }
	    return true ;
	}
	return false ;
    }

    ////////////////////////////////////////////////////////////////////////
    void Window::_setTraits(Window* win, std::vector<std::string> vec)
    {
	if (SceneGraph::instance()->isRealized())
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::Window::control: traits can not be set after windows are realized\n") ;
	    return ; 
	}

	if (vec.size()%2 != 0)
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::Window::control: odd number of traits pairspassed to _setTraits\n") ;
	    return ;
	}

	if (!win)
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::Window::control: NULL window pointer passed to _setTraits\n") ;
	    return ;
	}

	osg::GraphicsContext::Traits* traits = win->getTraits() ;

	for (unsigned int i=0; i<vec.size(); i+=2)
	{
	    //dtkMsg.add(DTKMSG_INFO, "iris::Window::control: window \"%s\", %s = %s\n",win->getName(),vec[i].c_str(), vec[i+1].c_str()) ;
	    bool onOff ;
	    int j ;
	    if (IsSubstring("decoration", vec[i], 3) && OnOff(vec[i+1],&onOff)) traits->windowDecoration = onOff ;
	    else if (IsSubstring("x",vec[i], 1) && StringToInt(vec[i+1],&j)) traits->x = j ;
	    else if (IsSubstring("y",vec[i], 1) && StringToInt(vec[i+1],&j)) traits->y = j ;
	    else if (IsSubstring("width", vec[i], 3) && StringToInt(vec[i+1],&j)) traits->width = j ;
	    else if (IsSubstring("height", vec[i], 3) && StringToInt(vec[i+1],&j)) traits->height = j ;
	    else dtkMsg.add(DTKMSG_ERROR, "iris::Window::control: invalid traits pair: %s %s\n",vec[i].c_str(), vec[i+1].c_str()) ;
	}
    }

    ////////////////////////////////////////////////////////////////////////
    void Window::realize()
    {
	if (_realized) return ;

	osg::ref_ptr<osg::GraphicsContext::WindowingSystemInterface> wsi = osg::GraphicsContext::getWindowingSystemInterface();
	if (!wsi.get()) 
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::Window::realize: no WindowSystemInterface available, cannot create windows.\n",getName()) ;
	    return ;
	}
    
	_gc = osg::GraphicsContext::createGraphicsContext(getTraits());
	if (!_gc.valid())
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::Window::realize: %s: Graphics Window has not been created successfully.\n",getName());
	    return ;
	}

	// default color outside the viewport is black
	_gc->setClearColor( osg::Vec4f(0.f, 0.f, 0.f, 0.0f) );
	_gc->setClearMask( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	//fprintf(stderr,"gc = %p, mask = %d\n",_gc.get(),GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) ;

	//fprintf(stderr, "iris::Window::realize() %s, stereo = %d\n",getName(),getStereo()) ; 

	// these are defaults, so why set?
	//getTraits()->screenNum = 0 ;
	//getTraits()->sharedContext = 0;

	for (unsigned int p=0; p<_paneList.size(); p++)
	{
	    // this'll create the cameras
	    Pane* np = _paneList[p].get() ;
	    np->realize() ;
	}
    
	_realized = true ;
    }

    ////////////////////////////////////////////////////////////////////////
    bool Window::addPane(Pane* pane) 
    { 
	// is the pane already in the list?
	for (unsigned int i = 0; i<_paneList.size(); i++)
	{
	    if (_paneList[i].get() == pane) return false;
	}
	_paneList.push_back(pane) ; 
	return true ;
    } 


    ////////////////////////////////////////////////////////////////////////
    Window* const Window::findWindow(const char* name)
    {
	for (unsigned int i=0 ; i<_windowList.size() ; i++)
	{
	    if (!strcmp(_windowList[i].get()->getName(),name)) return _windowList[i].get() ;
	}
	dtkMsg.add(DTKMSG_ERROR, "iris::Window::getWindow: no window named \"%s\" found.\n",name) ;
	return NULL ;
    }

    std::vector<osg::ref_ptr<Window> > Window::_windowList ;

}
