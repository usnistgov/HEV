#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Light_Button.H>

#include <iris.h>

bool init(int argc, char **argv) ;
void usage() ;

Fl_Double_Window *main_window ;

// tells us if it's our turn to use the buttons
iris::ShmString* selector ;
std::string selectorShmName = "idea/selector" ;
// the name of our selector string
std::string selectorStr = "clip" ;

// displays "world" or "wand" based on idea/selector
Fl_Output *world ;

// read a button to toggle between wand and world
dtkSharedMem* buttonShm ;
std::string buttonShmName = "idea/buttons/left" ;

// shm containing world transform corresponding to sceneNode
dtkSharedMem* worldMatShm = NULL ;
std::string worldMatShmName = "idea/worldOffsetWand" ;

// parent under which to put clipParentNode when button is down and selector is active
std::string sceneNodeName = "offsetWand" ;

// value in shared memory- use wand if != 0
char useWandNode ;
// old value of the "switch to world node" button
char oldUseWandNode ;

// true of the selector says we're active
bool selected = false ;

void timer_callback(void*) ;

enum Clipper { WINDSHIELD, SLICE, WEDGE, CORNER, BOX, NONE } ;

Clipper currentClipper = NONE ;

// true if wireframe glyph is shown
bool showGlyphs = true ;

// a list of the glyphs, so they can be turned on/off
std::map<Clipper, std::string> glyphs ;

// separate out the fltk arguments from the others
int fltk_argc;
char **fltk_argv;
int other_argc;
char **other_argv;

// true if disabling ESCAPE and the window manager close button
bool noEscape = false ;

std::string parentNode = "irisflyClip" ;
std::string base ;

// node that contains the scale
// clip planes get hung under this node
std::string scaleNode ;

// vector of nodes to unload on exit
std::vector<std::string> nodes ;

std::string windshieldNode ;
std::string sliceNode ;
std::string wedgeNode ;
std::string cornerNode ;
std::string boxNode ;

std::vector<std::pair<std::string,std::string> > windshieldClipNodes ;
std::vector<std::pair<std::string,std::string> > sliceClipNodes ;
std::vector<std::pair<std::string,std::string> > wedgeClipNodes ;
std::vector<std::pair<std::string,std::string> > cornerClipNodes ;
std::vector<std::pair<std::string,std::string> > boxClipNodes ;

float scaleValue = 1 ;

////////////////////////////////////////////////////////////////////////
bool init(int argc, char **argv)
{
    int c = 1 ;
    //~fprintf(stderr,("argc = %d\n",argc) ;
    while (c<argc && argv[c][0] == '-' && argv[c][1] == '-')
    {
	//~fprintf(stderr,("c = %d, argv[%d] = %s\n",c,c,argv[c]) ;
	if (iris::IsSubstring("--noescape",argv[c],3))
	{
	    noEscape = true ;
	    //~fprintf(stderr,("setting noescape\n") ;
	    c++ ;
	}
	else if (iris::IsSubstring("--clipparentnode", argv[c],3) && c+1<argc)
	{
	    c++ ;
	    parentNode = argv[c] ;
	    //~fprintf(stderr,("setting parentNode to %s\n",parentNode.c_str()) ;
	    c++ ;
	}
	else if (iris::IsSubstring("--buttonshm", argv[c],3) && c+1<argc)
	{
	    c++ ;
	    buttonShmName = argv[c] ;
	    //~fprintf(stderr,("setting buttonShmName to %s\n",buttonShmName.c_str()) ;
	    c++ ;
	}
	else if (iris::IsSubstring("--worldMatshm", argv[c],3) && c+1<argc)
	{
	    c++ ;
	    worldMatShmName = argv[c] ;
	    //~fprintf(stderr,("setting worldMatShmName to %s\n",worldMatShmName.c_str()) ;
	    c++ ;
	}
	else if (iris::IsSubstring("--selectorStr", argv[c],13) && c+1<argc)
	{
	    c++ ;
	    selectorStr = argv[c] ;
	    //~fprintf(stderr,("setting selectorStr to %s\n",selectorStr.c_str()) ;
	    c++ ;
	}
	else if (iris::IsSubstring("--selectorShm", argv[c],13) && c+1<argc)
	{
	    c++ ;
	    selectorShmName = argv[c] ;
	    //~fprintf(stderr,("setting selectorShm to %s\n",selectorShmName.c_str()) ;
	    c++ ;
	}
	else if (iris::IsSubstring("--sceneNodeName", argv[c],3) && c+1<argc)
	{
	    c++ ;
	    sceneNodeName = argv[c] ;
	    //~fprintf(stderr,("setting sceneNodeName to %s\n",sceneNodeName.c_str()) ;
	    c++ ;
	}
	else return false ;
    }
 
    //~fprintf(stderr,("after loop c = %d, argc = %d\n",c, argc) ;
    if (c != argc) return false ;
    //~fprintf(stderr,("done with parsing options\n") ;

    buttonShm = new dtkSharedMem(sizeof(char), buttonShmName.c_str());
    if (buttonShm->isInvalid()) return false ;

    selector = new iris::ShmString(selectorShmName) ;

    base = parentNode + "-hev-clipperControl-" ;
    scaleNode =  base + "Scale" ;

    // create the parent node
    printf("MATRIX %s\n", parentNode.c_str()) ;

    // hang the scale node under the parent node
    printf("DCS %s\nADDCHILD %s %s\n", scaleNode.c_str(), scaleNode.c_str(), parentNode.c_str()) ;

    // create the clip planes
    printf("CLIPNODE clip0 OFF\n") ;
    printf("CLIPNODE clip1 OFF\n") ;
    printf("CLIPNODE clip2 OFF\n") ;
    printf("CLIPNODE clip3 OFF\n") ;
    printf("CLIPNODE clip4 OFF\n") ;
    printf("CLIPNODE clip5 OFF\n") ;
    nodes.push_back("clip0") ;
    nodes.push_back("clip1") ;
    nodes.push_back("clip2") ;
    nodes.push_back("clip3") ;
    nodes.push_back("clip4") ;
    nodes.push_back("clip5") ;

    // create the glyphs and nodes to put the clip planes under
    //windshield
    std::string type = "Windshield" ;
    windshieldNode = base + type ;
    printf("GROUP %s\nNODEMASK %s OFF\nADDCHILD %s %s\n", windshieldNode.c_str(), windshieldNode.c_str(), windshieldNode.c_str(), scaleNode.c_str()) ;
    nodes.push_back(windshieldNode) ;

    std::string glyph = IDEA_PRIV_DATA_DIR + type + "Glyph" ;
    glyphs[WINDSHIELD] = glyph ;
    printf("LOAD %s %s.osg\nNOCLIP %s\nADDCHILD %s %s\n", glyph.c_str(), glyph.c_str(), glyph.c_str(), glyph.c_str(), windshieldNode.c_str()) ;
    nodes.push_back(glyph) ;

    // one per clip plane
    std::string clip = windshieldNode + "Clip" ;
    std::string clip0SCS = clip + "0SCS" ;
    printf("SCS %s \nADDCHILD %s %s\n", clip0SCS.c_str(), clip0SCS.c_str(), windshieldNode.c_str()) ;
    nodes.push_back(clip0SCS) ;
    // nodes to turn on/off with addChild/removeChild (no duplicate parents)
    windshieldClipNodes.push_back(std::make_pair("clip0", clip0SCS)) ;
    
    // slice
    type = "Slice" ;
    sliceNode = base + type ;
    printf("GROUP %s\nNODEMASK %s OFF\nADDCHILD %s %s\n", sliceNode.c_str(), sliceNode.c_str(), sliceNode.c_str(), scaleNode.c_str()) ;
    nodes.push_back(sliceNode) ;

    glyph = IDEA_PRIV_DATA_DIR + type + "Glyph" ;
    glyphs[SLICE] = glyph ;
    printf("LOAD %s %s.osg\nNOCLIP %s\nADDCHILD %s %s\n", glyph.c_str(), glyph.c_str(), glyph.c_str(), glyph.c_str(), sliceNode.c_str()) ;
    nodes.push_back(glyph) ;

    // one per clip plane, should match offsets in Glyph
    clip = sliceNode + "Clip" ;
    clip0SCS = clip + "0SCS" ;
    printf("SCS %s \nADDCHILD %s %s\n", clip0SCS.c_str(), clip0SCS.c_str(), sliceNode.c_str()) ;
    nodes.push_back(clip0SCS) ;
    sliceClipNodes.push_back(std::make_pair("clip0", clip0SCS)) ;
    
    std::string clip1SCS = clip + "1SCS" ;
    printf("SCS %s 0 .1 0 0 180 0\nADDCHILD %s %s\n", clip1SCS.c_str(), clip1SCS.c_str(), sliceNode.c_str()) ;
    nodes.push_back(clip1SCS) ;
    sliceClipNodes.push_back(std::make_pair("clip1", clip1SCS)) ;

    // wedge
    type = "Wedge" ;
    wedgeNode = base + type ;
    printf("GROUP %s\nNODEMASK %s OFF\nADDCHILD %s %s\n", wedgeNode.c_str(), wedgeNode.c_str(), wedgeNode.c_str(), scaleNode.c_str()) ;
    nodes.push_back(wedgeNode) ;

    glyph = IDEA_PRIV_DATA_DIR + type + "Glyph" ;
    glyphs[WEDGE] = glyph ;
    printf("LOAD %s %s.osg\nNOCLIP %s\nADDCHILD %s %s\n", glyph.c_str(), glyph.c_str(), glyph.c_str(), glyph.c_str(), wedgeNode.c_str()) ;
    nodes.push_back(glyph) ;

    // one per clip plane, should match offsets in Glyph
    clip = wedgeNode + "Clip" ;
    printf("GROUP %s \nADDCHILD %s %s \n", clip.c_str(), clip.c_str(), wedgeNode.c_str()) ;
    nodes.push_back(clip) ;

    clip0SCS = clip + "0SCS" ;
    printf("SCS %s 0 0 0 45 0 0\nADDCHILD %s %s\n", clip0SCS.c_str(), clip0SCS.c_str(), clip.c_str()) ;
    nodes.push_back(clip0SCS) ;
    wedgeClipNodes.push_back(std::make_pair("clip0", clip0SCS)) ;
    
    clip1SCS = clip + "1SCS" ;
    printf("SCS %s 0 0 0 -45 0 0\nADDCHILD %s %s\n", clip1SCS.c_str(), clip1SCS.c_str(), clip.c_str()) ;
    nodes.push_back(clip1SCS) ;
    wedgeClipNodes.push_back(std::make_pair("clip1", clip1SCS)) ;

    // corner
    type = "Corner" ;
    cornerNode = base + type ;
    printf("GROUP %s\nNODEMASK %s OFF\nADDCHILD %s %s\n", cornerNode.c_str(), cornerNode.c_str(), cornerNode.c_str(), scaleNode.c_str()) ;
    nodes.push_back(cornerNode) ;

    glyph = IDEA_PRIV_DATA_DIR + type + "Glyph" ;
    glyphs[CORNER] = glyph ;
    printf("LOAD %s %s.osg\nNOCLIP %s\nADDCHILD %s %s\n", glyph.c_str(), glyph.c_str(), glyph.c_str(), glyph.c_str(), cornerNode.c_str()) ;
    nodes.push_back(glyph) ;

    // one per clip plane, should match offsets in Glyph
    clip = cornerNode + "Clip" ;
    printf("GROUP %s \nADDCHILD %s %s \n", clip.c_str(), clip.c_str(), cornerNode.c_str()) ;
    nodes.push_back(clip) ;

    clip0SCS = clip + "0SCS" ;
    printf("SCS %s 0 0 0 45 0 0\nADDCHILD %s %s\n", clip0SCS.c_str(), clip0SCS.c_str(), clip.c_str()) ;
    nodes.push_back(clip0SCS) ;
    cornerClipNodes.push_back(std::make_pair("clip0", clip0SCS)) ;
    
    clip1SCS = clip + "1SCS" ;
    printf("SCS %s 0 0 0 -45 0 0\nADDCHILD %s %s\n", clip1SCS.c_str(), clip1SCS.c_str(), clip.c_str()) ;
    nodes.push_back(clip1SCS) ;
    cornerClipNodes.push_back(std::make_pair("clip1", clip1SCS)) ;

    std::string clip2SCS = clip + "2SCS" ;
    printf("SCS %s 0 0 0 0 -90 0\nADDCHILD %s %s\n", clip2SCS.c_str(), clip2SCS.c_str(), clip.c_str()) ;
    nodes.push_back(clip2SCS) ;
    cornerClipNodes.push_back(std::make_pair("clip2", clip2SCS)) ;

    // box
    type = "Box" ;
    boxNode = base + type ;
    printf("GROUP %s\nNODEMASK %s OFF\nADDCHILD %s %s\n", boxNode.c_str(), boxNode.c_str(), boxNode.c_str(), scaleNode.c_str()) ;
    nodes.push_back(boxNode) ;

    glyph = IDEA_PRIV_DATA_DIR + type + "Glyph" ;
    glyphs[BOX] = glyph ;
    printf("LOAD %s %s.osg\nNOCLIP %s\nADDCHILD %s %s\n", glyph.c_str(), glyph.c_str(), glyph.c_str(), glyph.c_str(), boxNode.c_str()) ;
    nodes.push_back(glyph) ;

    // one per clip plane, should match offsets in Glyph
    clip = boxNode + "Clip" ;
    printf("GROUP %s \nADDCHILD %s %s \n", clip.c_str(), clip.c_str(), boxNode.c_str()) ;
    nodes.push_back(clip) ;

    clip0SCS = clip + "0SCS" ;
    // -Y
    printf("SCS %s 0 0 0 0 0 0\nADDCHILD %s %s\n", clip0SCS.c_str(), clip0SCS.c_str(), clip.c_str()) ;
    nodes.push_back(clip0SCS) ;
    boxClipNodes.push_back(std::make_pair("clip0", clip0SCS)) ;

    // +Y
    clip1SCS = clip + "1SCS" ;
    printf("SCS %s 0 .2 0 180 0 0\nADDCHILD %s %s\n", clip1SCS.c_str(), clip1SCS.c_str(), clip.c_str()) ;
    nodes.push_back(clip1SCS) ;
    boxClipNodes.push_back(std::make_pair("clip1", clip1SCS)) ;

    // -X
    clip2SCS = clip + "2SCS" ;
    printf("SCS %s -.1 0 0 -90 0 0\nADDCHILD %s %s\n", clip2SCS.c_str(), clip2SCS.c_str(), clip.c_str()) ;
    nodes.push_back(clip2SCS) ;
    boxClipNodes.push_back(std::make_pair("clip2", clip2SCS)) ;

    // +X
    std::string clip3SCS = clip + "3SCS" ;
    printf("SCS %s .1 0 0 90 0 0\nADDCHILD %s %s\n", clip3SCS.c_str(), clip3SCS.c_str(), clip.c_str()) ;
    nodes.push_back(clip3SCS) ;
    boxClipNodes.push_back(std::make_pair("clip3", clip3SCS)) ;

    // -Z
    std::string clip4SCS = clip + "4SCS" ;
    printf("SCS %s 0 0 -.1 0 90 0\nADDCHILD %s %s\n", clip4SCS.c_str(), clip4SCS.c_str(), clip.c_str()) ;
    nodes.push_back(clip4SCS) ;
    boxClipNodes.push_back(std::make_pair("clip4", clip4SCS)) ;

    // +Z
    std::string clip5SCS = clip + "5SCS" ;
    printf("SCS %s 0 0 .1 0 -90 0\nADDCHILD %s %s\n", clip5SCS.c_str(), clip5SCS.c_str(), clip.c_str()) ;
    nodes.push_back(clip5SCS) ;
    boxClipNodes.push_back(std::make_pair("clip5", clip5SCS)) ;

    fflush(stdout) ;

#if 0
    fprintf(stderr,"init: glyphs.size = %d\n",glyphs.size()) ;
    std::map<Clipper, std::string>::iterator pos ;
    for (pos = glyphs.begin(); pos != glyphs.end(); pos++)
    {
	fprintf(stderr,"%d: %s\n",pos->first,pos->second.c_str()) ;
    }
#endif

    // unload the scale node and ShmMatrixNode node when exiting
    nodes.push_back(scaleNode) ;
    nodes.push_back(parentNode) ;

    Fl::add_timeout(.05, timer_callback) ;

    return true ;
}

////////////////////////////////////////////////////////////////////////
void usage()
{
    fprintf(stderr,"Usage: hev-clipperControl [ --noescape ] [--clipParentNode clipParentNodeName] [--buttonShm buttonShmName] [--sceneNode sceneNodeName] [--worldMatShm worldMatShmName] [--selectorShm selectorShmName] [--selectorStr string]\n") ;
}

////////////////////////////////////////////////////////////////////////
void toggleGlyphs(bool v)
{
    if (v != showGlyphs)
    {
	showGlyphs = v ;
	if (currentClipper != NONE)
	{
	    // toggle glyphs
	    //fprintf(stderr,"currentClipper = %d, NONE = %d\n",currentClipper, NONE) ;
	    {
		//fprintf(stderr,"glyphs.size = %d\n",glyphs.size()) ;
		std::map<Clipper, std::string>::iterator pos ;
		pos = glyphs.find(currentClipper) ;
		if (pos != glyphs.end()) 
		{
		    //fprintf(stderr,"toggling glyph %s\n",pos->second.c_str()) ;
		    if (v) printf("NODEMASK %s ON\n",pos->second.c_str()) ;
		    else printf("NODEMASK %s OFF\n",pos->second.c_str()) ;
		    fflush(stdout) ;
		}
	    }
	}
    }
}

////////////////////////////////////////////////////////////////////////
void switchToWorld()
{
    dtkMsg.add(DTKMSG_INFO, "hev-clipperControl: clip planes are under world\n") ;
    static bool first = true ;
    if (first) first = false ;
    else printf("REMOVECHILD %s %s\n",parentNode.c_str(), sceneNodeName.c_str()) ; 
    double worldMat[16] ;
    if (!worldMatShm) 
    {
	worldMatShm = new dtkSharedMem(sizeof(worldMat),worldMatShmName.c_str()) ;
    }
    if (worldMatShm->isValid()) worldMatShm->read(worldMat) ;
    // hang the parent node under the world node
    printf("MATRIX %s %f %f %f %f  %f %f %f %f  %f %f %f %f  %f %f %f %f  \nADDCHILD %s world\n", parentNode.c_str(),
	   worldMat[0], worldMat[1], worldMat[2], worldMat[3], 
	   worldMat[4], worldMat[5], worldMat[6], worldMat[7], 
	   worldMat[8], worldMat[9], worldMat[10], worldMat[11], 
	   worldMat[12], worldMat[13], worldMat[14], worldMat[15], parentNode.c_str()) ;
    fflush(stdout) ;
    world->value("          ") ;
    world->value("inactive") ;
    world->color(::fl_rgb_color(255, 150, 150)) ;
}

////////////////////////////////////////////////////////////////////////
void switchToWand()
{
    dtkMsg.add(DTKMSG_INFO, "hev-clipperControl: clip planes are under %s\n", sceneNodeName.c_str()) ;
    printf("REMOVECHILD %s world\n",parentNode.c_str()) ; 
    // reset the parent node's matrix and hang the parent node under the scene node
    printf("DCS %s 0 0 0\nADDCHILD %s %s\n", parentNode.c_str(), parentNode.c_str() ,sceneNodeName.c_str()) ; 
    fflush(stdout) ;
    world->value("          ") ;
    world->value("active") ;
    world->color(::fl_rgb_color(150, 255, 150)) ;
    
}

////////////////////////////////////////////////////////////////////////
// do this every "frame"
void timer_callback(void*)
{
    // the first time the timer is getting called
    static bool first = true ;

    // are we active?
    //~fprintf(stderr,"selector string = %s, selectorStr = %s, selectorShm = %s\n",selector->getString().c_str(),selectorStr.c_str(),  selector->getShmName().c_str()) ;
    if (selector->getString() == selectorStr)
    {
	selected = true ;

	// read the "switch to world node" button 
	buttonShm->read(&useWandNode) ;

	// if the first time through, use world, set initial and old button value
	if (first)
	{
	    first = false ;
	    switchToWorld() ;
	    oldUseWandNode = useWandNode = 0 ;
	}

	// did the button change?
	if (useWandNode != oldUseWandNode)
	{
	    oldUseWandNode = useWandNode ;
	    if (useWandNode) 
	    {
		switchToWand() ;
	    }
	    else 
	    {
		switchToWorld() ;
	    }
	}
    }

    else
    {
	// first time since being unselected
	if (selected && useWandNode)
	{
	    selected = false ;
	    switchToWorld() ;
	    oldUseWandNode = useWandNode = 0 ;
	}
    }
    Fl::repeat_timeout(.05, timer_callback) ;
}

////////////////////////////////////////////////////////////////////////
void windshield(bool v)
{
    static bool onOff = false ;
    if (v == onOff) return ;
    onOff = v ;

    if (v)
    {
	dtkMsg.add(DTKMSG_INFO, "hev-clipperControl: turning on windshield clipping\n") ;
	currentClipper = WINDSHIELD ;
	// turn on the top level node so the clipping planes will track
	printf("NODEMASK %s ON\n",windshieldNode.c_str()) ;
	if (showGlyphs) printf("NODEMASK %s ON\n",glyphs[currentClipper].c_str()) ;
	for (unsigned int i=0; i<windshieldClipNodes.size(); i++)
	{
	    printf("ADDCHILD %s %s\n", windshieldClipNodes[i].first.c_str(), windshieldClipNodes[i].second.c_str()) ;
	}
    }
    else
    {
	dtkMsg.add(DTKMSG_INFO, "hev-clipperControl: turning off windshield clipping\n") ;
	printf("NODEMASK %s OFF\n",glyphs[currentClipper].c_str()) ;
	for (unsigned int i=0; i< windshieldClipNodes.size(); i++)
	{
	    printf("REMOVECHILD %s %s\n", windshieldClipNodes[i].first.c_str(), windshieldClipNodes[i].second.c_str()) ;
	}
	currentClipper = NONE ;
    }
    fflush(stdout) ;
}

////////////////////////////////////////////////////////////////////////
void slice(bool v)
{
    static bool onOff = false ;
    if (v == onOff) return ;
    onOff = v ;

    if (v)
    {
	dtkMsg.add(DTKMSG_INFO, "hev-clipperControl: turning on slice clipping\n") ;
	currentClipper = SLICE ;
	// turn on the top level node so the clipping planes will track
	printf("NODEMASK %s ON\n",sliceNode.c_str()) ;
	if (showGlyphs) printf("NODEMASK %s ON\n",glyphs[currentClipper].c_str()) ;
	for (unsigned int i=0; i<sliceClipNodes.size(); i++)
	{
	    printf("ADDCHILD %s %s\n", sliceClipNodes[i].first.c_str(), sliceClipNodes[i].second.c_str()) ;
	}
    }
    else
    {
	dtkMsg.add(DTKMSG_INFO, "hev-clipperControl: turning off slice clipping\n") ;
	printf("NODEMASK %s OFF\n",glyphs[currentClipper].c_str()) ;
	for (unsigned int i=0; i< sliceClipNodes.size(); i++)
	{
	    printf("REMOVECHILD %s %s\n", sliceClipNodes[i].first.c_str(), sliceClipNodes[i].second.c_str()) ;
	}
	currentClipper = NONE ;
    }
    fflush(stdout) ;
}

////////////////////////////////////////////////////////////////////////
void wedge(bool v)
{
    static bool onOff = false ;
    if (v == onOff) return ;
    onOff = v ;

    if (v)
    {
	dtkMsg.add(DTKMSG_INFO, "hev-clipperControl: turning on wedge clipping\n") ;
	currentClipper = WEDGE ;
	// turn on the top level node so the clipping planes will track
	printf("NODEMASK %s ON\n",wedgeNode.c_str()) ;
	if (showGlyphs) printf("NODEMASK %s ON\n",glyphs[currentClipper].c_str()) ;
	for (unsigned int i=0; i<wedgeClipNodes.size(); i++)
	{
	    printf("ADDCHILD %s %s\n", wedgeClipNodes[i].first.c_str(), wedgeClipNodes[i].second.c_str()) ;
	}
    }
    else
    {
	dtkMsg.add(DTKMSG_INFO, "hev-clipperControl: turning off wedge clipping\n") ;
	printf("NODEMASK %s OFF\n",glyphs[currentClipper].c_str()) ;
	for (unsigned int i=0; i< wedgeClipNodes.size(); i++)
	{
	    printf("REMOVECHILD %s %s\n", wedgeClipNodes[i].first.c_str(), wedgeClipNodes[i].second.c_str()) ;
	}
	currentClipper = NONE ;
    }
    fflush(stdout) ;
}

////////////////////////////////////////////////////////////////////////
void corner(bool v)
{
    static bool onOff = false ;
    if (v == onOff) return ;
    onOff = v ;

    if (v)
    {
	dtkMsg.add(DTKMSG_INFO, "hev-clipperControl: turning on corner clipping\n") ;
	currentClipper = CORNER ;
	// turn on the top level node so the clipping planes will track
	printf("NODEMASK %s ON\n",cornerNode.c_str()) ;
	if (showGlyphs) printf("NODEMASK %s ON\n",glyphs[currentClipper].c_str()) ;
	for (unsigned int i=0; i<cornerClipNodes.size(); i++)
	{
	    printf("ADDCHILD %s %s\n", cornerClipNodes[i].first.c_str(), cornerClipNodes[i].second.c_str()) ;
	}
    }
    else
    {
	dtkMsg.add(DTKMSG_INFO, "hev-clipperControl: turning off corner clipping\n") ;
	printf("NODEMASK %s OFF\n",glyphs[currentClipper].c_str()) ;
	for (unsigned int i=0; i< cornerClipNodes.size(); i++)
	{
	    printf("REMOVECHILD %s %s\n", cornerClipNodes[i].first.c_str(), cornerClipNodes[i].second.c_str()) ;
	}
	currentClipper = NONE ;
    }
    fflush(stdout) ;
}

////////////////////////////////////////////////////////////////////////
void box(bool v)
{
    static bool onOff = false ;
    if (v == onOff) return ;
    onOff = v ;

    if (v)
    {
	dtkMsg.add(DTKMSG_INFO, "hev-clipperControl: turning on box clipping\n") ;
	currentClipper = BOX ;
	// turn on the top level node so the clipping planes will track
	printf("NODEMASK %s ON\n",boxNode.c_str()) ;
	if (showGlyphs) printf("NODEMASK %s ON\n",glyphs[currentClipper].c_str()) ;
	for (unsigned int i=0; i<boxClipNodes.size(); i++)
	{
	    printf("ADDCHILD %s %s\n", boxClipNodes[i].first.c_str(), boxClipNodes[i].second.c_str()) ;
	}
    }
    else
    {
	dtkMsg.add(DTKMSG_INFO, "hev-clipperControl: turning off box clipping\n") ;
	printf("NODEMASK %s OFF\n",glyphs[currentClipper].c_str()) ;
	for (unsigned int i=0; i< boxClipNodes.size(); i++)
	{
	    printf("REMOVECHILD %s %s\n", boxClipNodes[i].first.c_str(), boxClipNodes[i].second.c_str()) ;
	}
	currentClipper = NONE ;
    }
    fflush(stdout) ;
}

////////////////////////////////////////////////////////////////////////
void clipCB(Clipper c, bool v=false)
{

    // turn off before turning on to keep clip planes from having multiple parents
    if (c == WINDSHIELD)
    {
	slice(!v) ;
	wedge(!v) ;
	corner(!v) ;
	box(!v) ;
	windshield(v) ;
    }
    else if (c == SLICE)
    {
	windshield(!v) ;
	wedge(!v) ;
	corner(!v) ;
	box(!v) ;
	slice(v) ;
    }
    else if (c == WEDGE)
    {
	windshield(!v) ;
	slice(!v) ;
	corner(!v) ;
	box(!v) ;
	wedge(v) ;
    }
    else if (c == CORNER)
    {
	windshield(!v) ;
	slice(!v) ;
	wedge(!v) ;
	box(!v) ;
	corner(v) ;
    }
    else if (c == BOX)
    {
	windshield(!v) ;
	slice(!v) ;
	wedge(!v) ;
	corner(!v) ;
	box(v) ;
    }
    else if (c == NONE)
    {
	windshield(false) ;
	slice(false) ;
	wedge(false) ;
	corner(false) ;
	box(false) ;
    }
}

////////////////////////////////////////////////////////////////////////
void scale(float v)
{
    scaleValue = powf(10.f,v) ;
    dtkMsg.add(DTKMSG_DEBUG, "hev-clipperControl: scale %f -> %f\n",v,scaleValue) ;
    printf("DCS %s 0 0 0 0 0 0 %f %f %f\n",scaleNode.c_str(),scaleValue,scaleValue,scaleValue) ;
    fflush(stdout) ;
}

////////////////////////////////////////////////////////////////////////
void cleanup()
{
    dtkMsg.add(DTKMSG_INFO, "hev-clipperControl: cleaning up\n") ;
    clipCB(NONE,false) ;

    for (int i=nodes.size()-1; i>=0; i--) printf("UNLOAD %s\n",nodes[i].c_str()) ;
    fflush(stdout) ;
}

