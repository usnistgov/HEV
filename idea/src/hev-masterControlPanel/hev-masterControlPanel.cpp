/*

  Have options to specify the off-label and the on-label

  Future options will allow more precise placement of the buttons, using
  both relative and absolute positioning, or automatic if not specified

  Also, need commands to replace the -fg and -bg command line options

  Other options will allow the font and up/down box styles to be specified

*/


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <osgDB/FileUtils>

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Round_Button.H>

#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Menu_Item.H>

#include <iris.h>

// default values
int size[2] = {75, 20} ;
int spacing[2] = {5, 5};
int rows = 4 ;
int columns = 3 ;
bool down = false ;
Fl_Color color = FL_BACKGROUND_COLOR ;
float timeout = 0 ;

// set to false to exit
bool running = true ;

// true if disabling ESCAPE and the window manager close button
bool noEscape = false ;

// fifo to read commnds while running
iris::FifoReader* fifo = NULL ;

// fifos to read and write button press events
char* readButtonEventFifoName = NULL ;
iris::FifoReader* readButtonEventFifo = NULL ;

char* writeButtonEventFifoName = NULL ;
iris::FifoReader* writeButtonEventFifo = NULL ;

// separate out the fltk arguments from the others
int fltk_argc;
char **fltk_argv;
int other_argc;
char **other_argv;

Fl_Window* window = NULL ;

// forward references
class radioGroup ;
class actionButton ;
class menuButton ;
class menuItem ;
class menuItemGroup ;
class resetButton ;
class rulerButton ;
class rulerBox ;

////////////////////////////////////////////////////////////////////////
// ignore escapes and close window buttons
void doNothingCB(Fl_Widget*, void*) { } ;

////////////////////////////////////////////////////////////////////////
// print the string if it'n length is greater than zero and flush it
void printCommand(std::string s)
{ 
    if (s.length())
    {
	printf("%s\n",s.c_str()) ;
	fflush(stdout) ;
    }
}
////////////////
// base class for any button type
class genericButton
{
public:
    genericButton() : color(color) { ; } ;
    Fl_Color color ; // button color
    std::string offLabel ; // button label when off
    std::string onLabel ; // optional button label when on
    unsigned int index ; // index into the button vector, to quickly get the index from the button object
    virtual actionButton* asActionButton() { return NULL ; } ; //faster than casting
    virtual menuButton* asMenuButton() { return NULL ; } ;
    virtual resetButton* asResetButton() { return NULL ; } ;
    virtual rulerButton* asRulerButton() { return NULL ; } ;
} ;

////////////////
// class for a action button
class actionButton : public genericButton
{
public:
    actionButton() : count(0), value(0), group(NULL), button(NULL) { ; } ;
    virtual actionButton* asActionButton() { return this ; } ;
    int count ; // how many times has the button been pressed?
    int value ; // button's current value- allows us to set it in INIT and then set initial button value
    std::vector<std::string> first ; // actions to do when button is pressed
    std::vector<std::string> on ;
    std::vector<std::string> off ;
    std::vector<std::string> cleanup ;
    radioGroup* group ;  // if not NULL, points to radioGroup object this button belongs to
    Fl_Button* button ; // cast this to either a Fl_Light_Button or Fl_Button
} ;

// data for a group of radio buttons, it's not a button
class radioGroup
{
public:
    radioGroup() : lastPressed(NULL), initEncountered(false) { ; } ;
    Fl_Color color ; // default color for buttons in group
    actionButton* lastPressed ;  // pointer to the last action button pressed
    // true if a button in the group has the INIT keyword
    bool initEncountered ;
} ;

////////////////
// class for ruler button
class rulerButton : public genericButton
{
public:
    rulerButton() : box(NULL), horizontal(false), vertical(false), llcorner(false) , ulcorner(false) , lrcorner(false) , urcorner(false) { ; } ;
    virtual rulerButton* asRulerButton() { return this ; } ;
    bool horizontal ;
    bool vertical ;
    bool ulcorner ;
    bool urcorner ;
    bool llcorner ;
    bool lrcorner ;
    rulerBox* box ; 
} ;

class rulerBox : public Fl_Box
{
public:
    // kludge- if I don't pass a space to the box constuctor, or do
    // something to draw, and nothing before this widget has done any
    // drawing, the lines aren't drawn.  there's probably something in the
    // base class I need to call, I tried activate and redraw, but nothing
    // worked.
    rulerBox(int x, int y, int w, int h, const char *l = 0) :  horizontal(false), vertical(false), llcorner(false) , ulcorner(false) , lrcorner(false) , urcorner(false), Fl_Box(x, y, w, h, " ") { ; } ;
    void draw()
    {
	Fl_Box::draw();
	int centerX = x()+w()/2 ;
	int centerY = y()+h()/2 ;
	int minX = x()-spacing[0]/2 ;
	int maxX = x()+w()+spacing[0]/2 ;
	int minY = y()-spacing[1]/2 ;
	int maxY = y()+h()+spacing[1]/2 ;
	if (horizontal) fl_line(minX, centerY, maxX, centerY); 
	if (vertical) fl_line(centerX, minY, centerX, maxY) ;
	if (llcorner) fl_line(centerX, minY, centerX, centerY, maxX, centerY) ;
	if (ulcorner) fl_line(centerX, maxY, centerX, centerY, maxX, centerY) ;
	if (lrcorner) fl_line(centerX, minY, centerX, centerY, minX, centerY) ;
	if (urcorner) fl_line(centerX, maxY, centerX, centerY, minX, centerY) ;
    }
    bool horizontal ;
    bool vertical ;
    bool ulcorner ;
    bool urcorner ;
    bool llcorner ;
    bool lrcorner ;
} ;

////////////////
// class for a menu or radiomenu button
class menuButton : public genericButton
{
public:
    menuButton() : radioMenu(false) { ; } ;
    virtual menuButton* asMenuButton() { return this ; } ;
    Fl_Menu_Button* button ;
    bool radioMenu ; // true if a radioMenu button
    std::vector<menuItem*> items ; // a vector of menuItem objects that define each menu item in the button
} ;

// data for a menu item in a menu buton
class menuItem 
{
public:
    menuItem() : count(0), menu(NULL), group(NULL), onOff(false) { ; } ;
    int index ; // index into menu button's array of items
    std::string offLabel ; // label in item when off
    std::string onLabel ; // optional label in item when on
    int count ; // how many times has the item been selected
    std::vector<std::string> first ;  // actions to take when item selected
    std::vector<std::string> on ;
    std::vector<std::string> off ;
    std::vector<std::string> cleanup ;
    menuButton* menu ; // the menu button this item is contained in
    bool onOff ; // easier to keep track of it manually
    menuItemGroup* group ; // if not NULL, points to the menuItemGroup object this item belongs to
} ;

// for a group of radio menu items
class menuItemGroup
{
public:
    menuItemGroup() : lastPressed(NULL), initEncountered(false) { ; } ;
    menuItem* lastPressed ; // pointer to the last menu item selected
    // true if a button in the group has the INIT keyword
    bool initEncountered ;
} ;

////////////////
// class for a reset button
resetButton* resetButtonPtr = NULL ; 
class resetButton : public genericButton
{
public:
    resetButton() 
    {
	resetButtonPtr = this ;
    }
    void clear()
    {
	dtkMsg.add(DTKMSG_DEBUG, "hev-masterControlPanel: clearing reset button\n") ;
	menuButtons.clear() ;
	actionButtons.clear() ;
	button->label(offLabel.c_str()) ;
	button->clear() ;
    }
    virtual resetButton* asResetButton() { return this ; } ;
    Fl_Light_Button* button ;
    static resetButton* instance ; 
    // list of buttons turned off
    // easy action buttons
    std::vector<actionButton*> actionButtons ;
    // pitfa menu buttons, with index to menu_item
    std::vector<std::pair<menuButton*, int> > menuButtons ;
} ;


////////////////
// file path
std::vector<std::string> path ;

////////////////
// all of the button data
std::vector<genericButton*> buttons ;

////////////////
// state variables

// last action button or menu item defined
actionButton* lastActionButton = NULL ;
menuItem* lastMenuItem = NULL ;
radioGroup* lastRadioGroup = NULL ;
menuItemGroup* lastMenuItemGroup = NULL ;

// set to current radio group or menu button or radio group of menu items
radioGroup* inRadioGroup = NULL ;
menuButton* inMenuButton = NULL ;
menuItemGroup* inMenuItemGroup = NULL ;

////////////////
// forward references
bool parseFile(std::string file, bool errorOnNoFile=true) ;
void makeButtons() ;

////////////////
// use this for groups of radio buttons ;
Fl_Group* group = NULL ;

////////////////////////////////////////////////////////////////////////
// given r g b in ascii, calculate a Fl_Color
bool getColor(std::string r, std::string g, std::string b, Fl_Color *color)
{
    float rf, gf, bf ;
    if (!iris::StringToFloat(r,&rf) || !iris::StringToFloat(g,&gf) || !iris::StringToFloat(b,&bf) ||
	rf<0.f || rf>1.f || gf<0.f || gf>1.f || bf<0.f || bf>1.f) return false ;
    *color = fl_rgb_color(uchar(rf*255), uchar(gf*255), uchar(bf*255)) ;
    return true ;
}

////////////////////////////////////////////////////////////////////////
// how many buttons will be visible??  
// have to calculate because of spacers and menu items
int getNumButtons()
{
#if 0
    int num = 0 ;
    for (unsigned int i=0; i<buttons.size(); i++)
    {
	if (!buttons[i] || buttons[i]->asActionButton() || buttons[i]->asResetButton() || buttons[i]->asMenuButton()) num++ ;
    }
    return num ;
#else
    return buttons.size() ;
#endif
}

////////////////////////////////////////////////////////////////////////
// set size of window based on number of buttons
void setWindowSize()
{
    int numButtons = getNumButtons() ;
    if (down) 
    {
	int r = rows ;
	if (numButtons<rows) r = numButtons ;
	window->size(((numButtons+(r-1))/r)*(size[0]+spacing[0])+spacing[0],
		     r*(size[1]+spacing[1])+spacing[1]) ; 
    }
    else 
    {
	int c = columns;
	if (numButtons<columns) c = numButtons ;
	window->size(c*(size[0]+spacing[0])+spacing[0],
		     ((numButtons+(c-1))/c)*(size[1]+spacing[1])+spacing[1]) ; 
    }
}
////////////////////////////////////////////////////////////////////////
// create the window to fit the defined buttons
Fl_Window* makeWindow()
{
    window = new Fl_Window(1,1,NULL) ;
    // disable killing the window using the close button or escape button
    if (noEscape) window->callback(doNothingCB);
    setWindowSize() ;
    makeButtons() ;
    window->end();
    window->show(fltk_argc, fltk_argv);


    return window ;
}

////////////////////////////////////////////////////////////////////////
// parse a line of the MCP file
bool parseLine(const std::string& line) 
{
    std::vector<std::string> vec = iris::ParseString(line) ;
    if (vec.size() == 0) return true ;
    
    // this can be either an action button or a menu item
    if (iris::IsSubstring("button", vec[0],3) && (vec.size() == 2 || vec.size() == 3 || vec.size() == 6 || vec.size() == 7)) 
    {
	// onLabel given
	int onOffset = 0 ;
	std::string onLabel ;
	if (vec.size() == 3 || vec.size() == 7) 
	{
	    onLabel = vec[2] ;
	    onOffset = 1 ;
	}
	bool gotColor = false ;
	Fl_Color c ;
	if (vec.size() == 6+onOffset && iris::IsSubstring("color", vec[2+onOffset],3))
	{
	    if (!getColor(vec[3+onOffset], vec[4+onOffset], vec[5+onOffset], &(c)))
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: button %s, invalid r, g, b parameter\n", vec[1].c_str()) ;
		return false ;
	    }
	    gotColor = true ;
	}

	if (inMenuButton) // menu item
	{
	    if (gotColor)
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: button %s is a menu item, so can't take an individual color\n", vec[1].c_str()) ;
		return false ;
	    }
	    menuItem* item = new menuItem ;
	    item->offLabel = vec[1] ;
	    item->onLabel = onLabel ;
	    item->index =  inMenuButton->items.size() ;
	    inMenuButton->items.push_back(item) ;
	    item->menu = inMenuButton ;
	    if (inMenuItemGroup)
	    {
		dtkMsg.add(DTKMSG_DEBUG, "in menuItemGroup %p\n",inMenuItemGroup) ;
		item->group =  inMenuItemGroup ;
	    }
	    lastActionButton = NULL ;
	    lastMenuItem = item ;
	}
	else // actionButton
	{
	    actionButton* button = new actionButton ;
	    button->index = buttons.size() ;
	    buttons.push_back(button) ;
	    button->offLabel = vec[1] ;
	    button->onLabel = onLabel ;
	    if (gotColor) button->color = c ;
	    else button->color = color ;
	    if (inRadioGroup)
	    {
		dtkMsg.add(DTKMSG_DEBUG, "in radioGroup %p\n",inRadioGroup) ;
		button->group = inRadioGroup ;
		if (!gotColor) button->color = inRadioGroup->color ;
	    }
	    lastActionButton = button ;
	    lastMenuItem = NULL ;
	}

	return true ;
    }
    
    else if ((iris::IsSubstring("menu", vec[0],3) || iris::IsSubstring("radiomenu", vec[0],6)) && (vec.size() == 2 || vec.size() == 6)) 
    {
	if (inMenuButton)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: button %s, can't define a MENU button in a menu button\n",vec[1].c_str()) ;
	    return false ;
	}
	if (inRadioGroup)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: button %s, can't define a MENU button in a radio group\n",vec[1].c_str()) ;
	    return false ;
	}
	if (inMenuItemGroup)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: button %s, can't define a MENU button in a menu item group\n",vec[1].c_str()) ;
	    return false ;
	}
	menuButton* button = new menuButton ;
	if (iris::IsSubstring("radiomenu", vec[0],6))
	{
	    button->radioMenu = true ;
	    menuItemGroup* group = new menuItemGroup ;
	    dtkMsg.add(DTKMSG_DEBUG, "beginning new menuItemGroup in a radioMenu%p\n",group) ;
	    inRadioGroup = NULL ;
	    inMenuItemGroup = group ;
	}
	else button->radioMenu = false ;
	button->offLabel = vec[1] ;
	button->color = color ;
	if (vec.size() == 6 && iris::IsSubstring("color", vec[2],3))
	{
	    if (!getColor(vec[3], vec[4], vec[5], &(button->color)))
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: button %s, invalid r, g, b parameter\n",vec[1].c_str()) ;
		delete(button) ;
		return false ;
	    }
	}
	lastMenuItem = NULL ;
	lastActionButton = NULL ;
	inMenuButton = button ;
	button->index = buttons.size() ;
	buttons.push_back(button) ;
	return true ;
    }
    
    else if (iris::IsSubstring("radio", vec[0],3) && (vec.size() == 1 || vec.size() == 5)) 
    {
	if (inMenuButton && inMenuButton->radioMenu)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: button %s, can't define a new RADIO button in a radioMenu button\n",vec[1].c_str()) ;
	    return false ;
	}
	if (inRadioGroup)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: can't define a RADIO group in a radio group\n") ;
	    return false ;
	}
	if (inMenuItemGroup)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: can't define a RADIO group in a menu item group\n") ;
	    return false ;
	}

	Fl_Color c ;
	bool gotColor = false ;
	if (vec.size() == 5 && iris::IsSubstring("color", vec[1],3))
	{
	    if (!getColor(vec[2], vec[3], vec[4], &c))
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: invalid r, g, b parameter for radio group\n") ;
		return false ;
	    }
	    gotColor = true ;
	}

	dtkMsg.add(DTKMSG_DEBUG, "radio group default color c = 0x%x, gotColor = %d\n",c,gotColor) ;
	if (inMenuButton)
	{
	    if (gotColor)
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: radio group is in a menu item, so can't set an individual color\n", vec[1].c_str()) ;
		return false ;
	    }
	    menuItemGroup* group = new menuItemGroup ;
	    dtkMsg.add(DTKMSG_DEBUG, "beginning new menuItemGroup %p\n",group) ;
	    inRadioGroup = NULL ;
	    inMenuItemGroup = group ;
	}

	else
	{
	    radioGroup* group = new radioGroup ;
	    dtkMsg.add(DTKMSG_DEBUG, "beginning new radioGroup %p\n",group) ;
	    if (gotColor) group->color = c ;
	    else group->color = color ;
	    inRadioGroup = group ;
	    inMenuItemGroup = NULL ;
	}

	lastMenuItem = NULL ;
	lastActionButton = NULL ;
	return true ;
    }
    
    else if (iris::IsSubstring("end", vec[0],3) && vec.size() == 1) 
    {

	if (!inMenuButton && !inRadioGroup && !inMenuItemGroup)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: no radio group of menu button to END\n") ;
	    return false ;
	}

	else if (!inMenuButton && inRadioGroup)
	{
	    inRadioGroup = NULL ;
	}
	else if (inMenuButton && !inMenuItemGroup)
	{
	    inMenuButton = NULL ; 
	}
	else if (inMenuButton && inMenuItemGroup)
	{
	    if (inMenuButton->radioMenu)
	    {
		// if in a radioMenu you only need one END
		inMenuButton = NULL ;
		inMenuItemGroup = NULL ;
	    }
	    else
	    {
		// must be the end of a menu item group, since menus aren't allowed in groups, but groups are allowed in menus,
		// so the only way both can be set is if it's a menu in a group
		inMenuItemGroup = NULL ;
	    }
	}
	else
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: internal logic error in END statement!!  Please send the file to kelso@nist.gov\n") ;
	    return false ; 
	}
	lastMenuItem = NULL ;
	lastActionButton = NULL ;
	return true ;
    }
    
    else if (iris::IsSubstring("reset", vec[0],3) && (vec.size() == 2 || vec.size() == 3 || vec.size() == 6 || vec.size() == 7))
    {
	if (inMenuButton || inRadioGroup)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: can't have a RESET button in a menu button or radio button group\n") ;
	    return false ; 
	}
	if (resetButtonPtr) 
	{
	    dtkMsg.add(DTKMSG_WARNING, "hev-masterControlPanel: multiple RESET buttons defined\n") ;
	    return false ;
	}
	resetButton* button = new resetButton ;
	button->offLabel = vec[1] ;
	button->color = color ;
	// onLabel given
	int onOffset = 0 ;
	if (vec.size() == 3 || vec.size() == 7)
	{
	    button->onLabel = vec[2] ;
	    onOffset = 1 ;
	}
	if (vec.size() == 6+onOffset && iris::IsSubstring("color", vec[2+onOffset],3))
	{
	    if (!getColor(vec[3+onOffset], vec[4+onOffset], vec[5+onOffset], &(button->color)))
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: button %s, invalid r, g, b parameter\n",button->offLabel.c_str()) ;
		delete(button) ;
		return false ;
	    }
	}
	lastMenuItem = NULL ;
	lastActionButton = NULL ;
	button->index = buttons.size() ;
	buttons.push_back(button) ;
	return true ;
    }
    
    else if (iris::IsSubstring("label", vec[0],3) && (vec.size() == 1 || vec.size() == 2))
    {
	if (inMenuButton)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: can't have a LABEL in a menu button or radio button group\n") ;
	    return false ; 
	}
	genericButton* button = new genericButton ;
	if (vec.size() == 2) button->offLabel = vec[1] ;
	lastMenuItem = NULL ;
	lastActionButton = NULL ;
	button->index = buttons.size() ;
	buttons.push_back(button) ;
	return true ;
    }
    
    else if (iris::IsSubstring("cellruler", vec[0],3))
    {
	if (inMenuButton)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: can't have a CELLRULER in a menu button or radio button group\n") ;
	    return false ; 
	}
	bool horizontal = false ;
	bool vertical = false ;
	bool ulcorner = false ;
	bool urcorner = false ;
	bool llcorner = false ;
	bool lrcorner = false ;

	for (unsigned int i=1; i<vec.size(); i++)
	{
	    if (iris::IsSubstring("horizontal", vec[i],3)) horizontal = true ;
	    else if (iris::IsSubstring("vertical", vec[i],3)) vertical = true ;
	    else if (iris::IsSubstring("llcorner", vec[i],2)) llcorner = true ;
	    else if (iris::IsSubstring("lrcorner", vec[i],2)) lrcorner = true ;
	    else if (iris::IsSubstring("ulcorner", vec[i],2)) ulcorner = true ;
	    else if (iris::IsSubstring("urcorner", vec[i],2)) urcorner = true ;
	    else return false ;
	}

	rulerButton* button = new rulerButton ;
	button->horizontal = horizontal ;
	button->vertical = vertical ;
	button->llcorner = llcorner ;
	button->lrcorner = lrcorner ;
	button->ulcorner = ulcorner ;
	button->urcorner = urcorner ;
	lastMenuItem = NULL ;
	lastActionButton = NULL ;
	button->index = buttons.size() ;
	buttons.push_back(button) ;
	return true ;
    }
    
    else if (iris::IsSubstring("first", vec[0],3))
    {
	std::string cmd = line ;
	iris::ChopFirstWord(&cmd) ;
	if (!lastActionButton && !lastMenuItem)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: the last button defined was not an action button or menu item\n") ;
	    return false ;
	}
	if (lastActionButton) lastActionButton->first.push_back(cmd) ;
	else lastMenuItem->first.push_back(cmd) ;
	return true ;
    }

    else if (iris::IsSubstring("on", vec[0]))
    {
	std::string cmd = line ;
	iris::ChopFirstWord(&cmd) ;
	if (!lastActionButton && !lastMenuItem)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: the last button defined was not an action button or menu item\n") ;
	    return false ;
	}
	if (lastActionButton) lastActionButton->on.push_back(cmd) ;
	else lastMenuItem->on.push_back(cmd) ;
	return true ;
    }

    else if (iris::IsSubstring("cleanup", vec[0],3))
    {
	std::string cmd = line ;
	iris::ChopFirstWord(&cmd) ;
	if (!lastActionButton && !lastMenuItem)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: the last button defined was not an action button or menu item\n") ;
	    return false ;
	}
	if (lastActionButton) lastActionButton->cleanup.push_back(cmd) ;
	else lastMenuItem->cleanup.push_back(cmd) ;
	return true ;
    }

    else if (iris::IsSubstring("off", vec[0]))
    {
	std::string cmd = line ;
	iris::ChopFirstWord(&cmd) ;
	if (!lastActionButton && !lastMenuItem)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: the last button defined was not an action button or menu item\n") ;
	    return false ;
	}
	if (lastActionButton) lastActionButton->off.push_back(cmd) ;
	else lastMenuItem->off.push_back(cmd) ;
	return true ;
    }

    else if (vec.size()==1 && iris::IsSubstring("init", vec[0]))
    {
	if (!lastActionButton && !lastMenuItem)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: the last button defined was not an action button or menu item\n") ;
	    return false ;
	}
	if (lastActionButton) lastActionButton->value = 1 ;
	else lastMenuItem->onOff = true ;
	
	return true ;
    }

    else if ((vec.size()>1 && iris::IsSubstring("read", vec[0],3))) 
    {
	for (unsigned int i=1; i<vec.size(); i++)
	{
	    if (!parseFile(vec[i])) return false ;
	}
	return true ;
    }

    else if ((vec.size()>1 && iris::IsSubstring("sread", vec[0],3))) 
    {
	for (unsigned int i=1; i<vec.size(); i++)
	{
	    if (!parseFile(vec[i],false)) return false ;
	}
	return true ;
    }

    else if (iris::IsSubstring("size", vec[0],3) && vec.size() == 3)
    {
	int x, y ;
	if (!iris::StringToInt(vec[1],&x) || !iris::StringToInt(vec[2],&y))
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: invalid SIZE parameters %s %s\n",vec[1].c_str(),vec[2].c_str()) ;
	    return false ;
	}
	size[0] = x ; size[1] = y ;
	return true ;
    }

    else if (iris::IsSubstring("spacing", vec[0],3) && vec.size() == 3) 
    {
	int x, y ;
	if (!iris::StringToInt(vec[1],&x) || !iris::StringToInt(vec[2],&y))
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: invalid SPACING parameters %s %s\n",vec[1].c_str(),vec[2].c_str()) ;
	    return false ;
	}
	spacing[0] = x ; spacing[1] = y ;
	return true ;
    }

    else if (iris::IsSubstring("rows", vec[0],3) && vec.size() == 2) 
    {
	int r ;
	if (!iris::StringToInt(vec[1],&r))
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: invalid ROWS parameter %s\n",vec[1].c_str()) ;
	    return false ;
	}
	rows = r ;
	return true ;
    }

    else if (iris::IsSubstring("columns", vec[0],3) && vec.size() == 2) 
    {
	int c ;
	if (!iris::StringToInt(vec[1],&c))
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: invalid COLUMNS parameter %s\n",vec[1].c_str()) ;
	    return false ;
	}
	columns = c ;
	return true ;
    }

    else if (iris::IsSubstring("down", vec[0],3) && vec.size() == 1) 
    {
	down = true ;
	return true ;
    }

    else if (iris::IsSubstring("across", vec[0],3) && vec.size() == 1) 
    {
	down = false ;
	return true ;
    }
 
    else if (iris::IsSubstring("timeout", vec[0],3) && vec.size() == 2) 
    {
	float t ;
	if (!iris::StringToFloat(vec[1],&t))
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: invalid TIMEOUT parameter %s\n",vec[1].c_str()) ;
	    return false ;
	}
	timeout = t ;
	return true ;
    }

    else
    {
	dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: line \"%s\" is an unknown command\n",line.c_str()) ;
	return false ;
    }
}

////////////////////////////////////////////////////////////////////////
// find and process a file of MCP commands
bool parseFile(std::string file, bool errorOnNoFile)
{

    // do ${envvar} replacement
    iris::EnvvarReplace(file) ;

    // is whole filename a $envvar ?
    if (file.substr(0,1) == "$") file = getenv(file.substr(1).c_str()) ;

    // find file in path
    struct stat buf ;
    std::string pathName ;
    // if it looks like an absolute filename don't mess with the path
    if (file.substr(0,1) == "/") pathName = file ;
    else
    {
	for (unsigned int i=0; i<path.size(); i++)
	{
	    pathName = path[i] + "/" + file ;
	    if (!stat(pathName.c_str(),&buf)) break ;
	}
    }

    if (pathName == "") 
    {
	if (errorOnNoFile)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: can't find file \"%s\" in $MCP_FILE_PATH\n",file.c_str()) ;
	    return false ;
	} 
	else return true ;
    }

    dtkMsg.add(DTKMSG_INFO, "hev-masterControlPanel: reading from file %s\n",pathName.c_str()) ;
    std::ifstream mcpfile(pathName.c_str()) ;
    if (!mcpfile)
    {
	if (errorOnNoFile)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: can't open file \"%s\"\n",file.c_str()) ;
	    return false ;
	}
	else return true ;
    }
    std::string line ;
    while (getline(mcpfile,line))
    {
	dtkMsg.add(DTKMSG_DEBUG, "hev-masterControlPanel: line \"%s\"\n",line.c_str()) ;
	if (!iris::EnvvarReplace(line)) return false ;
	if (!parseLine(line)) return false ;
    }
    
    // is this needed?  does it close when it goes out of scope?
    mcpfile.close() ;
    dtkMsg.add(DTKMSG_DEBUG, "hev-masterControlPanel: successfully finished reading file %s\n",pathName.c_str()) ;
    return true ;
}

////////////////////////////////////////////////////////////////////////
// callback for a fifo's fd
void timerCallback(void*) 
{
    // reading in new menu entries while RESET is on can be a real mess
    if (!resetButtonPtr || (resetButtonPtr && ! resetButtonPtr->button->value()))
    {
	std::string line ;
	while (fifo->readLine(&line))
	{
	    dtkMsg.add(DTKMSG_INFO, "hev-masterControlPanel: timerCallback: read line \"%s\"\n",line.c_str()) ;	
	    if (parseFile(line))
	    {
		if (window) //rebuild window if it already exists
		{
		    static char label[1023] ;
		    int x = window->x() ;
		    int y = window->y() ;
		    strncpy(label, window->label(),1023) ;
		    //Fl::delete_widget(window) ;
		    delete window ;
		    makeWindow() ;
		    window->position(x,y) ;
		    window->label(label) ;
		    window->show() ;
		}
		else makeWindow() ;
	    }
	}
    }
    Fl::repeat_timeout(.05, timerCallback) ;
}

////////////////////////////////////////////////////////////////////////
// callback for an action button
// the user data is the actionButton object
void actionButtonCallback(Fl_Widget* w, void* d)
{

    actionButton* ab = static_cast<actionButton*>(d) ;

    // if NULL is passed for w, get the button from the data
    Fl_Button* flb ;
    if (w == NULL) flb = static_cast<Fl_Button*>(ab->button) ;
    else flb = static_cast<Fl_Button*>(w) ;
    
    dtkMsg.add(DTKMSG_DEBUG, "BUTTON %s, index = %d, count = %d, button value = %d\n",ab->offLabel.c_str(),ab->index,ab->count,flb->value()) ;

    // write to the fifo if specified
    if (writeButtonEventFifo) writeButtonEventFifo->write("ACTIONBUTTON " + 
							  iris::IntToString(ab->index) + " " + 
							  iris::IntToString(flb->value()) + "\n") ;

    // if you don't do this then the button will get turned off if a new button is added
    ab->value = flb->value() ;

    // turn off and reset the reset button if the button is turned on and the reset button is turned on
    if (w && flb->value() && resetButtonPtr && resetButtonPtr->button->value()) resetButtonPtr->clear() ;
    
    // swap labels if specified
    if (flb->value() && ab->onLabel != "") flb->label(ab->onLabel.c_str()) ;
    else flb->label(ab->offLabel.c_str()) ;
	    
    // if the button is being turned on and it's part of a radio group, turn off the last button pressed
    if (flb->value() && ab->group)
    {
	if (ab->group->lastPressed != NULL)
	{
	    dtkMsg.add(DTKMSG_DEBUG, "  radio group: last button pressed: %s\n",ab->group->lastPressed->offLabel.c_str()) ;
	    dtkMsg.add(DTKMSG_DEBUG, "    %d off commands\n",ab->group->lastPressed->off.size()) ;
	    for (unsigned int i=0; i<ab->group->lastPressed->off.size(); i++)
	    {
		dtkMsg.add(DTKMSG_DEBUG, "      %d: %s \n",i,ab->group->lastPressed->off[i].c_str()) ;
		printCommand(ab->group->lastPressed->off[i]) ;
	    }
	    // set to offLabel
	    ab->group->lastPressed->button->label(ab->group->lastPressed->offLabel.c_str()) ;
	    // turn off the last button pressed
	    ab->group->lastPressed->value = 0 ;
	}
	ab->group->lastPressed = ab ;
    }

    if (ab->count == 0)
    {
	// do first commands, or on commands if no first commands
	if (ab->first.size())
	{
	    dtkMsg.add(DTKMSG_DEBUG, "  %d first commands\n",ab->first.size()) ;
	    for (unsigned int i=0; i<ab->first.size(); i++)
	    {
		dtkMsg.add(DTKMSG_DEBUG, "    %d: %s \n",i,ab->first[i].c_str()) ;
		printCommand(ab->first[i]) ;
	    }
	}
	else if (ab->on.size())
	{
	    dtkMsg.add(DTKMSG_DEBUG, "  %d on commands\n",ab->on.size()) ;
	    for (unsigned int i=0; i<ab->on.size(); i++)
	    {
		dtkMsg.add(DTKMSG_DEBUG, "    %d: %s \n",i,ab->on[i].c_str()) ;
		printCommand(ab->on[i]) ;
	    }
	}
	else dtkMsg.add(DTKMSG_DEBUG, "  no first or on commands\n") ;
	ab->count++ ;
    }
    else
    {
	// 1 if a check button, always 0 if a regular button
	if (flb->value() || ab->off.size()==0)
	{
	    // event button, or button turned on
	    dtkMsg.add(DTKMSG_DEBUG, "  %d on commands\n",ab->on.size()) ;
	    for (unsigned int i=0; i<ab->on.size(); i++)
	    {
		dtkMsg.add(DTKMSG_DEBUG, "    %d: %s \n",i,ab->on[i].c_str()) ;
		printCommand(ab->on[i]) ;
	    }
	    ab->count++ ;
	}
	else
	{
	    // button turned off
	    dtkMsg.add(DTKMSG_DEBUG, "  %d off commands\n",ab->off.size()) ;
	    for (unsigned int i=0; i<ab->off.size(); i++)
	    {
		dtkMsg.add(DTKMSG_DEBUG, "    %d: %s \n",i,ab->off[i].c_str()) ;
		printCommand(ab->off[i]) ;
	    }
	}
    }
}

////////////////////////////////////////////////////////////////////////
// create an action button based on previous MCP commands
void makeActionButton(actionButton* ab, int scoot, int x, int y)
{
    dtkMsg.add(DTKMSG_DEBUG, "%*sACTION %s\n",scoot,"",ab->offLabel.c_str()) ;

    if (ab->group)
    {
	if (ab->group != lastRadioGroup || !group) 
	{
	    group = new Fl_Group(0,0,0,0) ;
	    group->size(group->parent()->w(),group->parent()->h()) ;
	    group->begin() ;
	    dtkMsg.add(DTKMSG_DEBUG, "beginning Fl_Group %p\n",group) ;
	}
	
	lastRadioGroup = ab->group ;
	ab->button =  new Fl_Round_Button(x, y, size[0], size[1], ab->offLabel.c_str()) ;
	ab->button->type(FL_RADIO_BUTTON) ;
    }
    else
    {
	if (group)
	{
	    dtkMsg.add(DTKMSG_DEBUG, "ending Fl_Group %p\n",group) ;
	    group->end() ;
	    group = NULL ;
	}

	if (ab->off.size()==0) ab->button =  new Fl_Button(x, y, size[0], size[1], ab->offLabel.c_str()) ;
	else  ab->button =  new Fl_Check_Button(x, y, size[0], size[1], ab->offLabel.c_str()) ;
    }

    ab->button->box(FL_UP_BOX) ;
    ab->button->value(ab->value) ;
    ab->button->color(ab->color) ;
    ab->button->callback(actionButtonCallback, static_cast<void*>(ab)) ;
	
    dtkMsg.add(DTKMSG_DEBUG, "%*s  color 0x%x\n",scoot,"",ab->color) ;
    unsigned int c = ab->first.size() ;
    dtkMsg.add(DTKMSG_DEBUG, "%*s  %d first commands\n",scoot,"",c) ;
    for (unsigned int i=0; i<ab->first.size(); i++)
    {
	dtkMsg.add(DTKMSG_DEBUG, "%*s    %d: %s\n",scoot,"",i+1,ab->first[i].c_str()) ;
    }
    c = ab->on.size() ;
    dtkMsg.add(DTKMSG_DEBUG, "%*s  %d on commands\n",scoot,"",c) ;
    for (unsigned int i=0; i<ab->on.size(); i++)
    {
	dtkMsg.add(DTKMSG_DEBUG, "%*s    %d: %s\n",scoot,"",i+1,ab->on[i].c_str()) ;
    }
    c = ab->off.size() ;
    dtkMsg.add(DTKMSG_DEBUG, "%*s  %d off commands\n",scoot,"",c) ;
    for (unsigned int i=0; i<ab->off.size(); i++)
    {
	dtkMsg.add(DTKMSG_DEBUG, "%*s    %d: %s\n",scoot,"",i+1,ab->off[i].c_str()) ;
    }
    c = ab->cleanup.size() ;
    dtkMsg.add(DTKMSG_DEBUG, "%*s  %d cleanup commands\n",scoot,"",c) ;
    for (unsigned int i=0; i<ab->cleanup.size(); i++)
    {
	dtkMsg.add(DTKMSG_DEBUG, "%*s    %d: %s\n",scoot,"",i+1,ab->cleanup[i].c_str()) ;
    }
    if (ab->group)
    {
	radioGroup* rg = ab->group ;
	dtkMsg.add(DTKMSG_DEBUG, "%*s  part of radioGroup %p\n",scoot,"",rg) ;
    }

    // do we init the button?
    if (ab->value && ab->count == 0)
    {
	if (ab->group == NULL && ab->off.size() == 0) // not a radio button, a regular button
	{
	    dtkMsg.add(DTKMSG_WARNING, "hev-masterControlPanel: %s has no OFF commands, INIT is invalid\n",ab->offLabel.c_str()) ;
	    ab->value = 0 ; ab->button->value(0) ;
	}
	else
	{
	    if (ab->group && ab->group->initEncountered)
	    {
		dtkMsg.add(DTKMSG_WARNING, "hev-masterControlPanel: %s is in a group that already has an INIT command, ignoring\n",ab->offLabel.c_str()) ;
		ab->value = 0 ; ab->button->value(0) ;
	    }
	    else
	    {
		dtkMsg.add(DTKMSG_DEBUG, "%*s  will be initialized\n",scoot,"") ;
		actionButtonCallback(NULL, static_cast<void*>(ab)) ;
		if (ab->group) ab->group->initEncountered = true ;
	    }
	}
    }

#if 0
    if (!ab->group && ab->first.size() == 0 && ab->on.size() == 0 && ab->off.size() == 0)
    {
	dtkMsg.add(DTKMSG_WARNING, "hev-masterControlPanel: %s contains no commands, deactivating\n",ab->offLabel.c_str()) ;
	ab->button->deactivate() ;
    }
#endif

}

////////////////////////////////////////////////////////////////////////
// create a generic button (LABEL) based on previous MCP commands
void makeGenericButton(genericButton* gb, int scoot, int x, int y)
{
    dtkMsg.add(DTKMSG_DEBUG, "%*sGENERIC %s\n",scoot,"",gb->offLabel.c_str()) ;

    Fl_Box* generic = new Fl_Box(x, y, size[0], size[1], gb->offLabel.c_str()) ;

}

////////////////////////////////////////////////////////////////////////
// create a ruler button (CELLRULER) based on previous MCP commands
void makeRulerButton(rulerButton* rb, int scoot, int x, int y)
{
    dtkMsg.add(DTKMSG_DEBUG, "%*sRULER \n",scoot,"") ;

    rulerBox* ruler = new rulerBox(x, y, size[0], size[1]) ;
    ruler->horizontal = rb->horizontal ;
    ruler->vertical = rb->vertical ;
    ruler->llcorner = rb->llcorner ;
    ruler->lrcorner = rb->lrcorner ;
    ruler->ulcorner = rb->ulcorner ;
    ruler->urcorner = rb->urcorner ;
    
}

////////////////////////////////////////////////////////////////////////
// callback for an event menu button
// the user data is the menuItem object
void menuEventButtonCallback(Fl_Widget* w, void* d)
{
    menuItem* mi = static_cast<menuItem*>(d) ;
    Fl_Menu_Button* flmb ;
    if (w == NULL) flmb = mi->menu->button ;
    else flmb = static_cast<Fl_Menu_Button*>(w) ;

    dtkMsg.add(DTKMSG_DEBUG, "MENU BUTTON %s, index = %d\n",mi->menu->offLabel.c_str(),mi->menu->index) ;
    dtkMsg.add(DTKMSG_DEBUG, "MENU EVENT ITEM %s, count = %d, value = %d\n",mi->offLabel.c_str(),mi->count,mi->index) ;
    // the value of the menu button is the index into the array of menu items
    //dtkMsg.add(DTKMSG_DEBUG, " menu value %d\n",int(flmb->value())) ;

    // the value of the button should stay 0, so the reset button won't try to turn it off

    if (mi->count == 0 && mi->first.size()>0)
    {
	// do first commands
	dtkMsg.add(DTKMSG_DEBUG, "  first press\n") ;
	dtkMsg.add(DTKMSG_DEBUG, "  %d first commands\n",mi->first.size()) ;
	for (unsigned int i=0; i<mi->first.size(); i++)
	{
	    dtkMsg.add(DTKMSG_DEBUG, "    %d: %s \n",i,mi->first[i].c_str()) ;
	    printCommand(mi->first[i]) ;
	}
    }
    else if (mi->on.size())
    {
	dtkMsg.add(DTKMSG_DEBUG, "  %d on commands\n",mi->on.size()) ;
	for (unsigned int i=0; i<mi->on.size(); i++)
	{
	    dtkMsg.add(DTKMSG_DEBUG, "    %d: %s \n",i,mi->on[i].c_str()) ;
	    printCommand(mi->on[i]) ;
	}
	mi->count++ ;
    }
    else dtkMsg.add(DTKMSG_DEBUG, "  no first or on commands\n",mi->on.size()) ;
    mi->count++ ;


}

////////////////////////////////////////////////////////////////////////
// callback for a toggle menu button
// the user data is the menuItem object
void menuToggleButtonCallback(Fl_Widget* w, void* d)
{
    menuItem* mi = static_cast<menuItem*>(d) ;
    Fl_Menu_Button* flmb ;
    if (w == NULL) flmb = mi->menu->button ;
    else flmb = static_cast<Fl_Menu_Button*>(w) ;

    dtkMsg.add(DTKMSG_DEBUG, "MENU BUTTON %s, index = %d\n",mi->menu->offLabel.c_str(),mi->menu->index) ;
    dtkMsg.add(DTKMSG_DEBUG, "MENU TOGGLE ITEM %s, count = %d, value = %d\n",mi->offLabel.c_str(),mi->count,mi->index) ;

    mi->onOff = !mi->onOff ;

    // the value of the menu button is the index into the array of menu items
    dtkMsg.add(DTKMSG_DEBUG, " onOff = %d, menu value %d, index = %d\n",mi->onOff, int(flmb->value()),mi->index) ;

    // turn off and reset the reset button if the button is turned on and the reset button is turned on
    if (w && mi->onOff && resetButtonPtr && resetButtonPtr->button->value()) resetButtonPtr->clear() ;

    // swap labels if specified
    Fl_Menu_Item* flmi = const_cast<Fl_Menu_Item*>(flmb->menu()) ; 
    if (mi->onOff && mi->onLabel != "") (flmi+(flmb->value()))->label(mi->onLabel.c_str()) ; 
    else (flmi+(flmb->value()))->label(mi->offLabel.c_str()) ;
	    
    if (mi->count == 0)
    {
	// do first commands, or on commands if no first commands
	dtkMsg.add(DTKMSG_DEBUG, "  first press\n") ;
	if (mi->first.size())
	{
	    dtkMsg.add(DTKMSG_DEBUG, "  %d first commands\n",mi->first.size()) ;
	    for (unsigned int i=0; i<mi->first.size(); i++)
	    {
		dtkMsg.add(DTKMSG_DEBUG, "    %d: %s \n",i,mi->first[i].c_str()) ;
		printCommand(mi->first[i]) ;
	    }
	}
	else if (mi->on.size())
	{
	    dtkMsg.add(DTKMSG_DEBUG, "  %d on commands\n",mi->on.size()) ;
	    for (unsigned int i=0; i<mi->on.size(); i++)
	    {
		dtkMsg.add(DTKMSG_DEBUG, "    %d: %s \n",i,mi->on[i].c_str()) ;
		printCommand(mi->on[i]) ;
	    }
	}
	else dtkMsg.add(DTKMSG_DEBUG, "  no first or on commands\n",mi->on.size()) ;
	mi->count++ ;
    }
    else
    {
	if (mi->onOff)
	{
	    // button turned on
	    dtkMsg.add(DTKMSG_DEBUG, "  %d on commands\n",mi->on.size()) ;
	    for (unsigned int i=0; i<mi->on.size(); i++)
	    {
		dtkMsg.add(DTKMSG_DEBUG, "    %d: %s \n",i,mi->on[i].c_str()) ;
		printCommand(mi->on[i]) ;
	    }
	    mi->count++ ;
	}
	else
	{
	    // button turned off
	    dtkMsg.add(DTKMSG_DEBUG, "  %d off commands\n",mi->off.size()) ;
	    for (unsigned int i=0; i<mi->off.size(); i++)
	    {
		dtkMsg.add(DTKMSG_DEBUG, "    %d: %s \n",i,mi->off[i].c_str()) ;
		printCommand(mi->off[i]) ;
	    }
	}
    }
}

////////////////////////////////////////////////////////////////////////
// callback for a radio menu button
// the user data is the menuItem object
void menuRadioButtonCallback(Fl_Widget* w, void* d)
{
    menuItem* mi = static_cast<menuItem*>(d) ;
    Fl_Menu_Button* flmb ;
    if (w == NULL) flmb = mi->menu->button ;
    else flmb = static_cast<Fl_Menu_Button*>(w) ;

    dtkMsg.add(DTKMSG_DEBUG, "MENU BUTTON %s, index = %d\n",mi->menu->offLabel.c_str(),mi->menu->index) ;
    dtkMsg.add(DTKMSG_DEBUG, "MENU RADIO ITEM %s, count = %d, value = %d\n",mi->offLabel.c_str(),mi->count,mi->index) ;
    // the value of the menu button is the index into the array of menu items
    //dtkMsg.add(DTKMSG_DEBUG, " menu value %d\n",int(flmb->value())) ;

    mi->onOff = true ;

    // turn off and reset the reset button if the reset button is turned on
    if (w && resetButtonPtr && resetButtonPtr->button->value()) resetButtonPtr->clear() ;

    // swap labels if specified
    Fl_Menu_Item* flmi = const_cast<Fl_Menu_Item*>(flmb->menu()) ;
    if (mi->onLabel != "") (flmi+(flmb->value()))->label(mi->onLabel.c_str()) ;
    else (flmi+(flmb->value()))->label(mi->offLabel.c_str()) ;
	    
    // if a radioMenu button, set the menu button's labe to be the item's label
    if (mi->menu->radioMenu)
    {
	flmb->label((flmi+(flmb->value()))->label()) ;
    }
    // if the button's part of a radio group, turn off the last button pressed
    if (mi->group)
    {
	if (mi->group->lastPressed != NULL)
	{
	    menuItem* lastMi = mi->group->lastPressed ;
	    dtkMsg.add(DTKMSG_DEBUG, "  last button pressed: %s\n",lastMi->offLabel.c_str()) ;
	    dtkMsg.add(DTKMSG_DEBUG, "    %d off commands\n",lastMi->off.size()) ;
	    lastMi->onOff = false ;
	    for (unsigned int i=0; i<lastMi->off.size(); i++)
	    {
		dtkMsg.add(DTKMSG_DEBUG, "      %d: %s \n",i,lastMi->off[i].c_str()) ;
		printCommand(lastMi->off[i]) ;
	    }
	    Fl_Menu_Item* fllmi = const_cast<Fl_Menu_Item*>(lastMi->menu->button->menu()) ;
	    (fllmi+(lastMi->index))->label(mi->group->lastPressed->offLabel.c_str()) ;
	}
	mi->group->lastPressed = mi ;
    }

    if (mi->count == 0 && mi->first.size()>0)
    {
	// do first commands, or on commands if no first commands
	dtkMsg.add(DTKMSG_DEBUG, "  first press\n") ;
	dtkMsg.add(DTKMSG_DEBUG, "  %d first commands\n",mi->first.size()) ;
	for (unsigned int i=0; i<mi->first.size(); i++)
	{
	    dtkMsg.add(DTKMSG_DEBUG, "    %d: %s \n",i,mi->first[i].c_str()) ;
	    printCommand(mi->first[i]) ;
	}
    }
    else if (mi->on.size())
    {
	dtkMsg.add(DTKMSG_DEBUG, "  %d on commands\n",mi->on.size()) ;
	for (unsigned int i=0; i<mi->on.size(); i++)
	{
	    dtkMsg.add(DTKMSG_DEBUG, "    %d: %s \n",i,mi->on[i].c_str()) ;
	    printCommand(mi->on[i]) ;
	}
    }
    else dtkMsg.add(DTKMSG_DEBUG, "  no first or on commands\n",mi->on.size()) ;
    mi->count++ ;
}

////////////////////////////////////////////////////////////////////////
// create an menu button, and the menu items it contains, based on previous MCP commands
void makeMenuButton(menuButton* mb, int scoot, int x, int y)
{
    dtkMsg.add(DTKMSG_DEBUG, "%*sMENU %s\n",scoot,"",mb->offLabel.c_str()) ;	
    dtkMsg.add(DTKMSG_DEBUG, "%*s  color 0x%x\n",scoot,"",mb->color) ;
    mb->button = new Fl_Menu_Button(x, y, size[0], size[1], mb->offLabel.c_str()) ;
    mb->button->color(mb->color) ;
    
    dtkMsg.add(DTKMSG_DEBUG, "%*s  with %d items\n",scoot,"",mb->items.size()) ;

    // one more than the number of menu items- put NULL at the end
    Fl_Menu_Item* items = new Fl_Menu_Item[mb->items.size()+1] ;
    for (unsigned int i=0; i<mb->items.size(); i++)
    {
	memset(&(items[i]), 0, sizeof(Fl_Menu_Item)) ;
	items[i].label(mb->items[i]->offLabel.c_str()) ;
#if 0
	printf("text = %s\n",items[i].text) ;
	printf("shortcut_ = %d\n",items[i].shortcut_) ;
	printf("callback_ = %p\n",items[i].callback_) ;
	printf("user_data_ = %p\n",items[i].user_data_) ;
	printf("flags = %d\n",items[i].flags) ;
	printf("labeltype_ = %d\n",items[i].labeltype_) ;
	printf("labelfont_ = %d\n",items[i].labelfont_) ;
	printf("labelsize_ = %d\n",items[i].labelsize_) ;
	printf("labelcolor_ = %d\n",items[i].labelcolor_) ;
#endif

	if (mb->items[i]->off.size()>0)
	{
	    items[i].flags = FL_MENU_TOGGLE ;
	    items[i].callback(menuToggleButtonCallback, static_cast<void*>(mb->items[i])) ;
	}
	else
	{
	    items[i].flags = 0 ;	
	    items[i].callback(menuEventButtonCallback, static_cast<void*>(mb->items[i])) ;
	}
	dtkMsg.add(DTKMSG_DEBUG, "%*s    %d: %s\n",scoot,"",i,items[i].text) ;
	dtkMsg.add(DTKMSG_DEBUG, "%*s      value = %d, onOff = %d\n",scoot,"",items[i].value(), mb->items[i]->onOff) ;
	dtkMsg.add(DTKMSG_DEBUG, "%*s      %d first commands\n",scoot,"",mb->items[i]->first.size()) ;
	for (unsigned int j=0; j<mb->items[i]->first.size(); j++)
	{
	    dtkMsg.add(DTKMSG_DEBUG, "%*s        %d: %s\n",scoot,"",j,mb->items[i]->first[j].c_str()) ;
	}
	dtkMsg.add(DTKMSG_DEBUG, "%*s      %d on commands\n",scoot,"",mb->items[i]->on.size()) ;
	for (unsigned int j=0; j<mb->items[i]->on.size(); j++)
	{
	    dtkMsg.add(DTKMSG_DEBUG, "%*s        %d: %s\n",scoot,"",j,mb->items[i]->on[j].c_str()) ;
	}
	dtkMsg.add(DTKMSG_DEBUG, "%*s      %d off commands\n",scoot,"",mb->items[i]->off.size()) ;
	for (unsigned int j=0; j<mb->items[i]->off.size(); j++)
	{
	    dtkMsg.add(DTKMSG_DEBUG, "%*s        %d: %s\n",scoot,"",j,mb->items[i]->off[j].c_str()) ;
	}
	dtkMsg.add(DTKMSG_DEBUG, "%*s      %d cleanup commands\n",scoot,"",mb->items[i]->cleanup.size()) ;
	for (unsigned int j=0; j<mb->items[i]->cleanup.size(); j++)
	{
	    dtkMsg.add(DTKMSG_DEBUG, "%*s        %d: %s\n",scoot,"",j,mb->items[i]->cleanup[j].c_str()) ;
	}
	
	if (mb->items[i]->group)
	{
	    dtkMsg.add(DTKMSG_DEBUG, "%*s      part of group %p\n",scoot,"",mb->items[i]->group) ;
	    items[i].flags = FL_MENU_RADIO ;
	    if (mb->items[i]->group != lastMenuItemGroup)
	    {
		dtkMsg.add(DTKMSG_DEBUG, "%*s      starting new group at button %d\n",scoot,"",mb->items[i]->group,i) ;
		dtkMsg.add(DTKMSG_DEBUG, "%*s      ending group at button %d\n",scoot,"",i-1) ;
		if (i>0) items[i-1].flags |= FL_MENU_DIVIDER ;
		lastMenuItemGroup = mb->items[i]->group ;
	    }
	    items[i].callback(menuRadioButtonCallback, static_cast<void*>(mb->items[i])) ;
	}

    }
    // do a zero'd one at the end so FLTK will know it's the end of the array
    memset(&(items[mb->items.size()]), 0, sizeof(Fl_Menu_Item)) ;
    mb->button->menu(items) ;
    
    // init stuff
    // need to do this last, after button gets menu items assigned
    for (unsigned int i=0; i<mb->items.size(); i++)
    {
	// do we init the button?
	if (mb->items[i]->onOff) 
	{
	    // manually set the menu item, as usually this is done by pressing it in the GUI
	    items[i].set() ;
	    // and set the button to index the item
	    mb->button->value(i) ;

	    if (mb->items[i]->count == 0) 
	    {
		dtkMsg.add(DTKMSG_DEBUG, "%*s      will be initialized\n",scoot,"") ;
		// set it back to false, as the callback sets it to true
		mb->items[i]->onOff = false ;

		if (items[i].flags & FL_MENU_TOGGLE)
		{
		    if (mb->items[i]->off.size() == 0) dtkMsg.add(DTKMSG_WARNING, "hev-masterControlPanel: %s has no OFF commands, INIT is invalid\n",mb->items[i]->offLabel.c_str()) ;
		    else 
		    {
			menuToggleButtonCallback(NULL,static_cast<void*>(mb->items[i])) ;
		    }
		}
#if 0
		// huh, what's this spozed to do?
		else if (items[i].flags & 0) dtkMsg.add(DTKMSG_WARNING, "hev-masterControlPanel: %s has no OFF commands, INIT is invalid\n",mb->items[i]->offLabel.c_str()) ;
#endif
		else if (items[i].flags & FL_MENU_RADIO)
		{
#if 0
		    if (mb->items[i]->off.size() == 0) dtkMsg.add(DTKMSG_WARNING, "hev-masterControlPanel: %s has no OFF commands, INIT is invalid\n",mb->items[i]->offLabel.c_str()) ;
		    else 
#endif
		    {
			if (mb->items[i]->group->initEncountered)
			{
			    dtkMsg.add(DTKMSG_WARNING, "hev-masterControlPanel: %s is in a group that already has an INIT command, ignoring\n",mb->items[i]->offLabel.c_str()) ;
			    // turn off the menu item
			    items[i].clear() ;
			}
			else 
			{
			    mb->items[i]->group->initEncountered = true ;
			    menuRadioButtonCallback(NULL, static_cast<void*>(mb->items[i])) ; 
			}
		    }
		}
		else dtkMsg.add(DTKMSG_WARNING, "hev-masterControlPanel: %s is a menu button of unknown type!\n",mb->items[i]->offLabel.c_str()) ;
	    }
	}
    }
}

////////////////////////////////////////////////////////////////////////
// callback for the reset button
void resetButtonCallback(Fl_Widget* w)
{
    Fl_Light_Button* fllb = static_cast<Fl_Light_Button*>(w) ;
    dtkMsg.add(DTKMSG_DEBUG, "RESET button pressed, value = %d\n",fllb->value()) ;

    actionButton* ab ;
    menuButton* mb ;

    if (fllb->value())
    {
	// getting turned on, turn off the other buttons
	for (unsigned int i=0; i<buttons.size(); i++)
	{
	    ab = buttons[i]->asActionButton() ;
	    mb = buttons[i]->asMenuButton() ;
	    if (ab && int(ab->button->value()) != 0)
	    {
		// action button is turned on
		ab->button->value(0) ;
		ab->button->label(ab->offLabel.c_str()) ;
		// save if turned off
		resetButtonPtr->actionButtons.push_back(ab) ;
		for (unsigned int j=0; j<ab->off.size(); j++) 
		{
		    dtkMsg.add(DTKMSG_DEBUG,"  %s OFF: %s\n", ab->offLabel.c_str(), ab->off[j].c_str()) ;
		    printCommand(ab->off[j]) ;
		}
	    }
	    else if (mb)
	    {
		mb->button->value(0) ;
		for (unsigned int j=0; j<mb->items.size(); j++) 
		{
		    Fl_Menu_Item* flmi = const_cast<Fl_Menu_Item*>(mb->button->menu()) ;
		    (flmi+j)->clear() ;
		    if (mb->items[j]->onOff)
		    {
			// menu item is turned on
			(flmi+j)->label(mb->items[j]->offLabel.c_str()) ;
			// save if turned off
			resetButtonPtr->menuButtons.push_back(std::make_pair(mb,j)) ;
			for (unsigned int k=0; k<mb->items[j]->off.size(); k++) 
			{
			    dtkMsg.add(DTKMSG_DEBUG,"  %s OFF: %s\n", mb->items[j]->offLabel.c_str(), mb->items[j]->off[k].c_str()) ;
			    printCommand(mb->items[j]->off[k]) ;
			}
		    }
		    mb->items[j]->onOff = false ;	
		}
	    }
	}
	if (resetButtonPtr->onLabel != "") fllb->label(resetButtonPtr->onLabel.c_str()) ;
    }
    else
    {
	// getting turned off, turn on the buttons we turned off
	// first do the actonButtons
	for (unsigned int i=0; i<resetButtonPtr->actionButtons.size(); i++)
	{
	    ab = resetButtonPtr->actionButtons[i] ;
	    ab->button->value(1) ;
	    if (ab->onLabel != "") ab->button->label(ab->onLabel.c_str()) ;
	    for (unsigned int j=0; j<ab->on.size(); j++) 
	    {
		dtkMsg.add(DTKMSG_DEBUG,"  %s ON: %s\n", ab->offLabel.c_str(), ab->on[j].c_str()) ;
		printCommand(ab->on[j]) ;
	    }
	}
	// then the pitfa menu buttons
	for (unsigned int i=0; i<resetButtonPtr->menuButtons.size(); i++)
	{
	    mb = resetButtonPtr->menuButtons[i].first ;
	    int itemIndex = resetButtonPtr->menuButtons[i].second ;
	    Fl_Menu_Item* flmi = const_cast<Fl_Menu_Item*>(mb->button->menu()) ;
	    (flmi+itemIndex)->set() ;
	    if (mb->items[itemIndex]->onLabel != "") (flmi+itemIndex)->label(mb->items[itemIndex]->onLabel.c_str()) ;
	    for (unsigned int k=0; k<mb->items[itemIndex]->on.size(); k++) 
	    {
		dtkMsg.add(DTKMSG_DEBUG,"  %s ON: %s\n", mb->items[itemIndex]->offLabel.c_str(), mb->items[itemIndex]->on[k].c_str()) ;
		printCommand(mb->items[itemIndex]->on[k]) ;
	    }
	    mb->items[itemIndex]->onOff = true ;	
	}
	resetButtonPtr->menuButtons.clear() ;
	resetButtonPtr->actionButtons.clear() ;
	fllb->label(resetButtonPtr->offLabel.c_str()) ;
    }
}

////////////////////////////////////////////////////////////////////////
// create a reset button based on previous MCP commands
void makeResetButton(resetButton* rb, int scoot, int x, int y)
{
    dtkMsg.add(DTKMSG_DEBUG, "%*sRESET %s\n",scoot,"",rb->offLabel.c_str()) ;	
    dtkMsg.add(DTKMSG_DEBUG, "%*s  color 0x%x\n",scoot,"",rb->color) ;
    rb->button = new Fl_Light_Button(x, y, size[0], size[1], rb->offLabel.c_str()) ;
    rb->button->color(rb->color) ;
    rb->button->value(0) ;
    rb->button->callback(resetButtonCallback) ;
}

////////////////////////////////////////////////////////////////////////
// make all buttons of all types based on previous MCP commands
void makeButtons()
{
    if (buttons.size() == 0) return ;

    group = NULL ;
    dtkMsg.add(DTKMSG_DEBUG, "size = %d %d \n",size[0], size[1]) ;
    dtkMsg.add(DTKMSG_DEBUG, "spacing = %d %d \n",spacing[0], spacing[1]) ;
    dtkMsg.add(DTKMSG_DEBUG, "rows = %d \n",rows) ;
    dtkMsg.add(DTKMSG_DEBUG, "columns = %d \n",columns) ;
    if (down) dtkMsg.add(DTKMSG_DEBUG, "buttons go down\n") ;
    else dtkMsg.add(DTKMSG_DEBUG, "buttons go across\n") ;
    dtkMsg.add(DTKMSG_DEBUG, "space for %d buttons allocated to window\n",getNumButtons()) ;
    int scoot = 0 ;
    int x = spacing[0]; 
    int y = spacing[1] ;

    for (unsigned int i=0; i<buttons.size(); i++)
    {
	dtkMsg.add(DTKMSG_DEBUG, "button %d at %d %d\n",i,x,y) ;
	actionButton* ab = buttons[i]->asActionButton() ; 
	resetButton* rb = buttons[i]->asResetButton() ; 
	menuButton* mb = buttons[i]->asMenuButton() ; 
	rulerButton* cb = buttons[i]->asRulerButton() ; 

	if (ab) makeActionButton(ab, scoot, x, y) ;
	else if (rb) makeResetButton(rb, scoot, x, y) ;
	else if (mb) makeMenuButton(mb, scoot, x, y) ;
	else if (cb) makeRulerButton(cb, scoot, x, y) ;
	else makeGenericButton(buttons[i], scoot, x, y) ;

	if (down)
	{
	    if ((i+1)%rows == 0)
	    {
		x += size[0] + spacing[0] ; 
		y = spacing[1];
	    }
	    else y += size[1] + spacing[1] ; 
	}
	else
	{
	    if ((i+1)%columns == 0)
	    {
		x = spacing[0];
		y += size[1] + spacing[1] ; 
	    }
	    else x += size[0] + spacing[0] ;
	}
    }
}

////////////////////////////////////////////////////////////////////////
// pretty simple usage, eh?  minimal command line parameters so they can all be specified in the MCP files
void usage() 
{
    fprintf(stderr,"Usage: hev-masterControlPanel [--noescape] [--fifo filename] [ file ... ]\n") ;
}

////////////////////////////////////////////////////////////////////////
// for catching the KILL, QUIT, ABRT, TERM and INT signals
static void signal_catcher(int sig)
{
    dtkMsg.add(DTKMSG_DEBUG,"hev-masterControlPanel: PID %d caught signal %d, starting exit sequence ...\n", getpid(), sig);
    running = false ;
    iris::Signal(SIG_DFL);
}

////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{

    // catch signals that kill us off
    iris::Signal(signal_catcher);

    // send dtk messages to stdout
    dtkMsg.setFile(stderr) ;

    if(dtkFLTKOptions_get(argc, (const char **) argv,
			  &fltk_argc,  &fltk_argv,
			  &other_argc, &other_argv)) 
    {
	usage() ;
	return 1 ;
    }
    

    int c = 1 ;
    while (c<other_argc && (other_argv[c][0] == '-' && other_argv[c][1] == '-'))
    {
	if (iris::IsSubstring("--noescape",other_argv[c],3))
	{
	    noEscape = true ;
	    c++ ;
	}
	else if (iris::IsSubstring("--fifo",other_argv[c],3))
	{
	    c++ ;
	    if (c == other_argc)
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-masterControlPanel: no fifo name given!\n") ;
		usage() ;
		return 1 ;
	    }
	    dtkMsg.add(DTKMSG_INFO, "hev-masterControlPanel: reading from fifo %s\n",other_argv[c]) ;
	    fifo = new iris::FifoReader(other_argv[c]) ;
	    fifo->unlinkOnExit() ;
	    c++ ;
	}
	else
	{
	    usage() ;
	    return 1 ;
	}
    }

    readButtonEventFifoName = getenv("MCP_READ_BUTTON_EVENTS") ;
    if (readButtonEventFifoName)
    {
	readButtonEventFifo = new iris::FifoReader(readButtonEventFifoName) ;
	readButtonEventFifo->unlinkOnExit() ;
	if (!readButtonEventFifo->open()) return 1 ;
    }

    writeButtonEventFifoName = getenv("MCP_WRITE_BUTTON_EVENTS") ;
    if (writeButtonEventFifoName)
    {
	writeButtonEventFifo = new iris::FifoReader(writeButtonEventFifoName) ;
	writeButtonEventFifo->unlinkOnExit() ;
	if (!writeButtonEventFifo->open(O_WRONLY)) return 1 ;
    }

    // get the file search path into a vector
    path.push_back(".") ;
    char* mcpPath = getenv("MCP_FILE_PATH") ;
    if (mcpPath)
    {
	osgDB::FilePathList fpl ;
	osgDB::convertStringPathIntoFilePathList(mcpPath, fpl) ;
	while(fpl.size()>0)
	{
	    if (fpl.front() != "") path.push_back(fpl.front()) ;
	    fpl.pop_front() ;
	}
    }

    // first load any files specified in the envvar
    std::string str ;
    char* mcpFiles = getenv("MCP_FILES") ;
    if (mcpFiles)
    {
	osgDB::FilePathList fpl ;
	osgDB::convertStringPathIntoFilePathList(mcpFiles, fpl) ;
	while(fpl.size()>0)
	{
	    if (fpl.front() != "")
	    {
		if (!parseFile(fpl.front())) return false ;
	    }
	    fpl.pop_front() ;
	}
    }

    // then load the files on the command line
    // skip 0- the name of the mcp executable
    for (unsigned int i=c; i<other_argc; i++)
    {
	if (!parseFile(other_argv[i])) return false ;
    }

    dtkMsg.add(DTKMSG_DEBUG, "hev-masterControlPanel: reading from stdin\n") ;
    std::string line ;
    bool timeout ;
    while (iris::GetLine(&line, 0.5f, &timeout))
    {
	dtkMsg.add(DTKMSG_DEBUG, "hev-masterControlPanel: line \"%s\"\n",line.c_str()) ;
	bool ret = parseLine(line) ;
	if (!ret) return false ;
	dtkMsg.add(DTKMSG_DEBUG, "hev-masterControlPanel: successfully finished reading stdin\n") ;
    }
    
    if (timeout)
    {
	dtkMsg.add(DTKMSG_DEBUG, "hev-masterControlPanel: no data read from stdin\n") ;	
    }
    
    if (buttons.size() > 0) window = makeWindow() ;

    if (fifo)
    {
	if (!fifo->open()) return 1 ;
	Fl::add_timeout(.05, timerCallback) ;
    }
    
    while (running)
    {
	if (buttons.size() > 0) 
	{
	    if (Fl::wait() == 0) break ;
	}
	else Fl::wait(.05f) ;
    }

    actionButton* ab ;
    menuButton* mb ;

    dtkMsg.add(DTKMSG_INFO,"hev-masterControlPanel: calling cleanup commands\n") ;
    for (unsigned int i=0; i<buttons.size(); i++)
    {
	ab = buttons[i]->asActionButton() ;
	if (ab && ab->count > 0)
	{
	    for (unsigned int j=0; j<ab->cleanup.size(); j++) 
	    {
		dtkMsg.add(DTKMSG_DEBUG,"hev-masterControlPanel:     cleanup %s: %s\n", ab->offLabel.c_str(), ab->cleanup[j].c_str()) ;
		printCommand(ab->cleanup[j]) ;
	    }
	}
	mb = buttons[i]->asMenuButton() ;
	if (mb)
	{
	    for (unsigned int j=0; j<mb->items.size(); j++) 
	    {
		if (mb->items[j]->count > 0)
		{
		    for (unsigned int k=0; k<mb->items[j]->cleanup.size(); k++) 
		    {
			dtkMsg.add(DTKMSG_DEBUG,"hev-masterControlPanel:     cleanup %s: %s\n", mb->items[j]->offLabel.c_str(), mb->items[j]->cleanup[k].c_str()) ;
			printCommand(mb->items[j]->cleanup[k]) ;
		    }
		}
	    }
	}
    }

    if (fifo) delete fifo ;
    if (readButtonEventFifo) delete readButtonEventFifo ;
    if (writeButtonEventFifo) delete writeButtonEventFifo ;
    return 0 ;
}
