#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

#include <iris/SceneGraph.h>
#include <iris/Utils.h>
#include <iris/Nav.h>
#include <iris/Pane.h>
#include <iris/Window.h>
#include <iris/ConvertNodeVisitor.h>
#include <iris/ArgumentParser.h>

int main(int argc, char **argv)
{
    // send messages to stderr
    dtkMsg.setFile(stderr) ;


    std::string logFileName ;
    bool logOn = false ;
    bool examine = false ;
    bool setName = false ;
    float redBackground = 0.f, greenBackground = 0.f, blueBackground = 0.f ;

    // use an ArgumentParser object to manage the program arguments.
    iris::ArgumentParser args(&argc,argv);

    // what's this used for?  what/when are all the other class signatures of osg::ApplicationUsage for??
    osg::ref_ptr<osg::ApplicationUsage> usage = new osg::ApplicationUsage("iris-viewer [ --setname ] [--examine] [--background r g b] [ --scene file ... ] [ --nav file ... ] [ --ether file ... ] [ --world file ... ] [ --log  ] [ --logfile filename ] [file ...]") ;
    args.setApplicationUsage(usage) ;

    int pos ;
    std::vector<std::string> sceneFiles ;
    if ((pos=args.findSubstring("--scene",4))>0 && args.argc()>pos+1)
    {
	// everything is a model file until the next parameter or the end of parameters
	int pos2 = args.findOption(pos+1) ;
	if (pos2==0) pos2=args.argc() ;
	for (int i=pos+1; i<pos2; i++)
	{
	    sceneFiles.push_back(args[i]) ;
	}
	args.remove(pos,pos2-pos) ;
    }
    std::vector<std::string> navFiles ;
    if ((pos=args.findSubstring("--nav",4))>0 && args.argc()>pos+1)
    {
	// everything is a model file until the next parameter or the end of parameters
	int pos2 = args.findOption(pos+1) ;
	if (pos2==0) pos2=args.argc() ;
	for (int i=pos+1; i<pos2; i++)
	{
	    navFiles.push_back(args[i]) ;
	}
	args.remove(pos,pos2-pos) ;
    }
    std::vector<std::string> etherFiles ;
    if ((pos=args.findSubstring("--ether",4))>0 && args.argc()>pos+1)
    {
	// everything is a model file until the next parameter or the end of parameters
	int pos2 = args.findOption(pos+1) ;
	if (pos2==0) pos2=args.argc() ;
	for (int i=pos+1; i<pos2; i++)
	{
	    etherFiles.push_back(args[i]) ;
	}
	args.remove(pos,pos2-pos) ;
    }
    std::vector<std::string> worldFiles ;
    if ((pos=args.findSubstring("--world",4))>0 && args.argc()>pos+1)
    {
	// everything is a model file until the next parameter or the end of parameters
	int pos2 = args.findOption(pos+1) ;
	if (pos2==0) pos2=args.argc() ;
	for (int i=pos+1; i<pos2; i++)
	{
	    worldFiles.push_back(args[i]) ;
	}
	args.remove(pos,pos2-pos) ;
    }
    if ((pos=args.findSubstring("--log",4))>0) logOn = args.read(args.argv()[pos]) ;
    if ((pos=args.findSubstring("--logfile",6))>0) args.read(args.argv()[pos],osg::ArgumentParser::Parameter(logFileName)) ;
    if ((pos=args.findSubstring("--examine",4))>0) examine = args.read(args.argv()[pos]) ;
    if ((pos=args.findSubstring("--setname",4))>0) setName = args.read(args.argv()[pos]) ;
    if ((pos=args.findSubstring("--background",4))>0) args.read(args.argv()[pos], 
							      osg::ArgumentParser::Parameter(redBackground),
							      osg::ArgumentParser::Parameter(greenBackground),
							      osg::ArgumentParser::Parameter(blueBackground)) ;

    // any options we didn't ask about?
    if (args.containsOptions())
    {
	dtkMsg.add(DTKMSG_ERROR, "iris-viewer: unknown args:\n") ;
	for (unsigned int i=0; i<args.argc(); i++)
	{
	  // this will check for both single and double dash options
	  if (args.argv()[i][0] == '-') dtkMsg.add(DTKMSG_ERROR, "iris-viewer:   %s\n",args.argv()[i]) ;
	}
	return 1 ;
    }

    if (args.errors())
    {
	dtkMsg.add(DTKMSG_ERROR, "iris-viewer: error parsing args\n") ;
	return 1 ;
    }

    // creates the scenegraph and default viewer
    iris::SceneGraph isg ;
    
    if (isg.isInvalid())
    {
	dtkMsg.add(DTKMSG_ERROR, "iris-viewer: couldn't create iris::SceneGraph object\n") ;
	return 1;
    }

    // set logging options
    if (logFileName != "") isg.setLogFile(logFileName) ;
    if (logOn) isg.setLog(logOn) ;

    // set default clear color for new cameras
    isg.setClearColor(osg::Vec4(redBackground,greenBackground,blueBackground,1.)) ;

    // load DSOs and model files and put model files under the specified nodes
    iris::LoadFile(sceneFiles, iris::SceneGraph::instance()->getSceneNode(),setName);
    iris::LoadFile(navFiles, iris::SceneGraph::instance()->getNavNode(),setName);
    iris::LoadFile(etherFiles, iris::SceneGraph::instance()->getEtherNode(),setName);
    iris::LoadFile(worldFiles, iris::SceneGraph::instance()->getWorldNode(),setName);
    iris::LoadFile(&args, iris::SceneGraph::instance()->getWorldNode(),setName);
    
    // map all loaded nodes, hopefully this'll make node lookups quicker
    iris::SceneGraph::instance()->mapNode("scene") ;

    if (examine) isg.examine() ;
    
#if 0
    // print out scenegraph node transformations if not identity
    iris::MatrixTransform* n = iris::SceneGraph::instance()->getSceneNode() ;
    osg::Vec3 p = n->getPosition() ;
    osg::Quat a = n->getAttitude() ;
    osg::Vec3 s = n->getScale() ;
    osg::Vec3 v = n->getPivotPoint() ;
    
    if (p!=osg::Vec3(0,0,0))   printf("scene node: pos = %f %f %f\n", p.x(),p.y(),p.z()) ;
    if (a!=osg::Quat(0,0,0,1)) printf("scene node: att = %f %f %f %f\n", a.x(),a.y(),a.z(),a.w()) ;
    if (s!=osg::Vec3(1,1,1))   printf("scene node: sca = %f %f %f\n", s.x(),s.y(),s.z()) ;
    if (v!=osg::Vec3(0,0,0))   printf("scene node: piv = %f %f %f\n", v.x(),v.y(),v.z()) ;
    
    p = iris::Nav::getPosition() ;
    a = iris::Nav::getAttitude() ;
    s = iris::Nav::getScale() ;
    v = iris::Nav::getPivotPoint() ;
    
    if (p!=osg::Vec3(0,0,0))   printf("nav:   pos = %f %f %f\n", p.x(),p.y(),p.z()) ;
    if (a!=osg::Quat(0,0,0,1)) printf("nav:   att = %f %f %f %f\n", a.x(),a.y(),a.z(),a.w()) ;
    if (s!=osg::Vec3(1,1,1))   printf("nav:   sca = %f %f %f\n", s.x(),s.y(),s.z()) ;
    if (v!=osg::Vec3(0,0,0))   printf("nav:   piv = %f %f %f\n", v.x(),v.y(),v.z()) ;
    if (iris::Nav::getPivotNode()) printf("nav: piv node = \"%s\"\n",iris::Nav::getPivotNode()->getName().c_str()) ; 
    
    n = iris::SceneGraph::instance()->getWorldNode() ;
    p = n->getPosition() ;
    a = n->getAttitude() ;
    s = n->getScale() ;
    v = n->getPivotPoint() ;
    
    if (p!=osg::Vec3(0,0,0))   printf("world node: pos = %f %f %f\n", p.x(),p.y(),p.z()) ;
    if (a!=osg::Quat(0,0,0,1)) printf("world node: att = %f %f %f %f\n", a.x(),a.y(),a.z(),a.w()) ;
    if (s!=osg::Vec3(1,1,1))   printf("world node: sca = %f %f %f\n", s.x(),s.y(),s.z()) ;
    if (v!=osg::Vec3(0,0,0))   printf("world node: piv = %f %f %f\n", v.x(),v.y(),v.z()) ;

    printf("%d navigation and %d window DSOs loaded\n",iris::Nav::getNavList().size(),iris::Window::getWindowList().size()) ;
    
    if (iris::Nav::getNavList().size()>0) printf("current navigation: %s\nnavigations loaded:\n",iris::Nav::getCurrentNav()->getName()) ;
    for (unsigned int i=0; i<iris::Nav::getNavList().size(); i++)
    {
	iris::Nav* nav = (iris::Nav::getNavList())[i].get() ;
	printf("  %d: %s\n",i,nav->getName()) ;
    }
    
    
    printf("windows loaded:\n") ;
    for (unsigned int i=0; i<iris::Window::getWindowList().size(); i++)
    {
	iris::Window* window = (iris::Window::getWindowList())[i].get() ;
	printf("  %d: %s\n",i,window->getName()) ;
    }
#endif    

    // first frame will automatically realize
    //isg.realize() ;
    isg.frame() ;

    while (isg.run()) ;

    return 0 ;
}   

