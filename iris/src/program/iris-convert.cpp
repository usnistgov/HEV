#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgUtil/SmoothingVisitor>
#include <osgUtil/Statistics>

#include <iris.h>

void usage()
{
    fprintf(stderr,"Usage: iris-convert [ --verbose ] [ --help ] [ --setname nodename ] [ --clobberName ] [ --normals ] [ --static ] [ --copyprimitives ] [ --optimize optimization ] [ --nostatesets ] [ --nolighting ] infile [ ... ] outfile\n") ;
}

int main(int argc, char** argv)
{
    if (argc<3)
    {
	usage() ;
	return 1 ;
    }
    // send messages to stderr
    dtkMsg.setFile(stderr) ;

    // use an ArgumentParser object to manage the program arguments.
    iris::ArgumentParser args(&argc,argv);

    bool normals = false ;
    bool setName = false ;
    bool clobberName = false ;
    std::string nodeName ;
    std::string optimizations ;
    std::vector<std::string> load ;

    // for getting rid of statsets, dynamic variance
    iris::ConvertNodeVisitor nv ;

    // parse the options
    int i = 1;
    while (i=args.findOption(i))
    {
	if (iris::IsSubstring("--help",args[i],4))
	{
	    usage() ;
	    return 0 ;
	}
	else if (iris::IsSubstring("--normals",args[i],4))
	{
	    normals = true ;
	    args.remove(i) ;
	}
	else if (iris::IsSubstring("--verbose",args[i],4))
	{
	    if (!dtkMsg.isSeverity(DTKMSG_INFO)) dtkMsg.reset(DTKMSG_INFO) ;
	    args.remove(i) ;
	}
	else if (iris::IsSubstring("--clobbername",args[i],4))
	{
	    clobberName = true ;
	    args.remove(i) ;
	}
	else if (iris::IsSubstring("--setname",args[i],4))
	{
	    if (args.argc()>i+1)
	    {
		setName = true ;
		nodeName = args[i+1] ;
		args.remove(i,2) ;
	    }
	    else
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-convert: not enough parameters\n") ;
		return 1 ;
	    }
	}
	else if (iris::IsSubstring("--static",args[i],4))
	{
	    nv.setStatic(true) ;
	    args.remove(i) ;
	}
	else if (iris::IsSubstring("--copyprimitives",args[i],4))
	{
	    nv.setCopyPrimitives(true) ;
	    args.remove(i) ;
	}
	else if (iris::IsSubstring("--nostatesets",args[i],5))
	{
	    nv.setNoStateSets(true) ;
	    args.remove(i) ;
	}
	else if (iris::IsSubstring("--nolighting",args[i],5))
	{
	    nv.setNoLighting(true) ;
	    args.remove(i) ;
	}
	else if (iris::IsSubstring("--optimize",args[i],4))
	{
	    if (args.argc()>i+1)
	    {
		optimizations = args[i+1] ;
		args.remove(i,2) ;
	    }
	    else
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-convert: not enough parameters\n") ;
		return 1 ;
	    }
	}
	else
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris-convert: unknown option \"%s\"\n",args[i]) ;
	    return 1 ;
	}
    }

    if (args.argc()<2)
    {
	dtkMsg.add(DTKMSG_ERROR, "iris-convert: not enough file names\n") ;
	return 1 ;
    }

    osg::ref_ptr<osg::Group> group = new osg::Group ;

    // get the input files
    while(argc>2)
    {
	load.push_back(args[1]) ;	
	args.remove(1) ;
    }

    // load them
    osg::ref_ptr<osg::Node> node = osgDB::readNodeFiles(load);
    if (!node.valid())
    {
	dtkMsg.add(DTKMSG_ERROR, "iris-convert: no model files loaded\n") ;
	return 1 ;
    }
    
    if (normals)
    {
 	dtkMsg.add(DTKMSG_INFO, "iris-convert: adding smooth normals\n") ;
	osgUtil::SmoothingVisitor sv ;
	node->accept(sv) ;
    }

    if (nv.getStatic()) dtkMsg.add(DTKMSG_INFO, "iris-convert: setting DataVariance to static\n") ;
    if (nv.getCopyPrimitives()) dtkMsg.add(DTKMSG_INFO, "iris-convert: copying primitives\n") ;
    if (nv.getNoStateSets()) dtkMsg.add(DTKMSG_INFO, "iris-convert: removing StateSets\n") ;
    if (nv.getNoLighting()) dtkMsg.add(DTKMSG_INFO, "iris-convert: removing lighting\n") ;
    if (nv.getStatic() || nv.getNoStateSets() || nv.getNoLighting() || nv.getCopyPrimitives()) node->accept(nv) ;

    if (nodeName == "") nodeName = args[1] ;

    if (clobberName || (setName && node->getName() == ""))
    {
	dtkMsg.add(DTKMSG_INFO,"iris-convert: setting node name to %s\n",nodeName.c_str()) ;
	node->setName(nodeName) ;
    }

    // optimize last
    if (optimizations.length()>0)
    {
	iris::Optimize(node.get(), optimizations) ;
    }

    if (optimizations.size() == 0)
    {
	osgUtil::StatsVisitor stats;
	if (dtkMsg.isSeverity(DTKMSG_INFO))
	{
	    dtkMsg.add(DTKMSG_INFO, "iris-convert: scenegraph statistics:\n") ;
	    node->accept(stats);
	    stats.totalUpStats();
	    stats.print(std::cerr) ;
	}
    }

    // write to the output file
    dtkMsg.add(DTKMSG_INFO, "iris-convert: writing %s\n",args[1]) ;
    if (!osgDB::writeNodeFile(*node, args[1]))
    {
	dtkMsg.add(DTKMSG_ERROR, "iris-convert: Unable to write to file \"%s\"\n", args[1]) ;
    }
    return 0 ;
}
