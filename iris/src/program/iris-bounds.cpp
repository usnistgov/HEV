#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

#include <iris.h>

void usage()
{
    fprintf(stderr,"Usage: iris-bounds --help --sphere --box file [ ... ]\n") ;
}

int main(int argc, char** argv)
{
    if (argc<2)
    {
	usage() ;
	return 1 ;
    }

    // send messages to stderr
    dtkMsg.setFile(stderr) ;

    // use an ArgumentParser object to manage the program arguments.
    iris::ArgumentParser args(&argc,argv);

    // always create a scenegraph object if reading files
    // but you don't need a viewer
    iris::SceneGraph isg(false) ;

    bool sphere = false ;
    bool box = false ;

    // parse the options
    int i = 1;
    while (i=args.findOption(i))
    {
	if (iris::IsSubstring("--sphere",args[i],4))
	{
	    sphere = true ;
	    args.remove(i) ;
	}
	else if (iris::IsSubstring("--box",args[i],4))
	{
	    box = true ;
	    args.remove(i) ;
	}
	else if (iris::IsSubstring("--help",args[i],4))
	{
	    usage() ;
	    return 0 ;
	}
	else
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris-bounds: unknown option \"%s\"\n",args[i]) ;
	    usage() ;
	    return 1 ;
	}
    }

    if (args.argc()<2)
    {
	dtkMsg.add(DTKMSG_ERROR, "iris-bounds: not enough file names\"%s\"\n",args[i]) ;
	return 1 ;
    }

    osg::ref_ptr<osg::Group> group = new osg::Group ;

    // get the input files
    while(argc>1)
    {
	osg::Node* child = osgDB::readNodeFile(args[1]) ;
	if (!child) 
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris-bounds: can't load model file \"%s\"\n",args[1]) ;
	    return 1 ;
	}
	group->addChild(child) ;
	args.remove(1) ;
    }

    osg::BoundingSphere s = group->getBound() ;
    osg::BoundingBox bb = iris::GetBoundingBox(group) ;

    // defaults if nothing specified
    if (!sphere && !box) sphere = box = true ;

    if (sphere) printf("sphere_radius: %+8.8f\nsphere_center: %+8.8f %+8.8f %+8.8f\n",s.radius(), s.center().x(), s.center().y(), s.center().z()) ;
    if (box) printf("box_min:    %+8.8f %+8.8f %+8.8f\nbox_max:    %+8.8f %+8.8f %+8.8f\nbox_extent: %+8.8f %+8.8f %+8.8f\n",bb.xMin(), bb.yMin(), bb.zMin(), bb.xMax(), bb.yMax(), bb.zMax(), bb.xMax()-bb.xMin(), bb.yMax()-bb.yMin(), bb.zMax()-bb.zMin()) ;
    if (box) printf("box_center: %+8.8f %+8.8f %+8.8f\n",(bb.xMax()+bb.xMin())/2.f, (bb.yMax()+bb.yMin())/2.f, (bb.zMax()+bb.zMin())/2.f) ;
    
    return 0 ;
}
