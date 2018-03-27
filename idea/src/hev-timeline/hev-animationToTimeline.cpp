/*

  writes to stdout a timeline file containing control commands to create a
  flipbook animation.

  Input files and paramaters are similar to hev-animator.

 */

#include <stdio.h>
#include <osg/Node>
#include <osg/Group>
#include <osg/Sequence>
#include <osgDB/ReadFile>

#include <iris.h>

bool init(int argc, char** argv) ;
void usage() ;

bool descend = false ;
unsigned int init_first = 0 ;
unsigned int first = init_first ;
unsigned int init_last = 0 ;
unsigned int last ;
float init_frameTime = .1 ;
float frameTime = init_frameTime ;
unsigned int children = 0 ;
unsigned int current = 0 ;
std::vector< std::vector< std::string> > group_nodes ;

////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{

    // send messages to stderr
    dtkMsg.setFile(stderr) ;

    // parse the command line arguments and do other setup
    if (!init(argc, argv))
    {
	usage() ;
	return 1 ;
    }

    return 0 ;
}

////////////////////////////////////////////////////////////////////////
void usage()
{
    fprintf(stderr,"Usage: hev-animationToTimeline [ --first n ] [ --last n ] [ --time t ] [ --descend ] [ file ... ] \n") ;
}

////////////////////////////////////////////////////////////////////////
bool init(int argc, char** argv)
{
    if (argc < 2) return false ;
    
    // use an ArgumentParser object to manage the program arguments.
    iris::ArgumentParser args(&argc,argv);

    int pos ;

    if ((pos=args.findSubstring("--descend",3))>0)
    {
	descend = true ;
	args.remove(pos) ;
	dtkMsg.add(DTKMSG_INFO, "hev-animationToTimeline: descending into scenegraph\n") ;
    }

    if ((pos=args.findSubstring("--first",3))>0 && args.argc()>pos+1)
    {
	int first_frame ;
	if (!args.read(args.argv()[pos], osg::ArgumentParser::Parameter(first_frame)) || first_frame<0)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-animationToTimeline: illegal first frame number\n") ;
	    return false ;
	}
	first = init_first = first_frame ;
	current = first+1 ;
    }

    if ((pos=args.findSubstring("--last",3))>0 && args.argc()>pos+1)
    {
	int last_frame ;
	if (!args.read(args.argv()[pos], osg::ArgumentParser::Parameter(last_frame)) || last_frame<=0)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-animationToTimeline: illegal last frame number\n") ;
	    return false ;
	}
	last = init_last = last_frame ;
    }

    if ((pos=args.findSubstring("--time",3))>0 && args.argc()>pos+1)
    {
	float t ;
	if (!args.read(args.argv()[pos], osg::ArgumentParser::Parameter(t)) || t<=0)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-animationToTimeline: illegal frame time\n") ;
	    return false ;
	}
	frameTime = init_frameTime = t ;
    }

    if (args.findOption()!=0)
    {
	dtkMsg.add(DTKMSG_ERROR, "hev-animationToTimeline: illegal options\n") ;
	return false ;
    }


    {
	std::string path ;
	path = ".:"  ;
	char* osgfilepath = getenv("OSG_FILE_PATH") ;
	if (osgfilepath)
	{
	    path += ":" ;
	    path += osgfilepath ;
	}
	osgDB::Registry::instance()->setDataFilePathList(path) ;
    }


    if (args.argc() == 1) // no files given
    {
	dtkMsg.add(DTKMSG_ERROR, "hev-animationToTimeline: no files given\n") ; 
	return false ;
    }
    else // load files
    {
	for (int j=1; j<args.argc(); j++)
	{
	    dtkMsg.add(DTKMSG_INFO, "hev-animationToTimeline: reading file %s\n",argv[j]) ;
	    osg::Node* n = osgDB::readNodeFile(std::string(argv[j]));
	    if (!n)
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-animationToTimeline: unable to open file %s\n",argv[j]) ; 
		return false ;
	    }
	    osg::Group* g = dynamic_cast<osg::Group*>(n) ;
	    if (!g)
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-animationToTimeline: file %s is not based on osg::Group\n",argv[j]) ; 
		return false ;
	    }
	    
	    if (g->getNumChildren()==0)
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-animationToTimeline: node has zero children- nothing to do!\n") ;
		return false ;
	    }
	    
	    if (descend)
	    {
		std::string scoot = "" ;
		while (g->getNumChildren() == 1)
		{
		    dtkMsg.add(DTKMSG_INFO, "hev-animationToTimeline: %snode has only one child, descending\n", scoot.c_str()) ;
		    scoot += "  " ;
		    g = dynamic_cast<osg::Group*>(g->getChild(0)) ;
		    if (!g || g->getNumChildren()==0)
		    {
			dtkMsg.add(DTKMSG_ERROR, "hev-animationToTimeline: node has zero children- nothing to do!\n") ;
			return false ;
		    }
		}
	    }
	    
	    if (j==1)
	    {
		children = g->getNumChildren() ;
	    }
	    else
	    {
		if (children != g->getNumChildren())
		{
		    dtkMsg.add(DTKMSG_ERROR, "hev-animationToTimeline: %s does not have the same number of children as previous files\n",argv[j]) ; 
		    return false ;
		}
	    }
	    
	    std::vector< std::string > nms ;
	    std::string nm ; 

	    for (unsigned int k=0; k<children; k++)
	    {
		nm = g->getChild(k)->getName() ;
		if (nm == "") 
		{
		    dtkMsg.add(DTKMSG_INFO, "hev-animationToTimeline: node has a blank node name\n") ;
		    return false ;
		}
		else
		{
		    nms.push_back(nm) ;
		}
	    }
	    group_nodes.push_back(nms) ;
	}
	
	if (children == 1)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-animationToTimeline: node has only one child- nothing to do!\n") ;
	    return false ;
	}
	else if (init_last >= children) // last out of bounds
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-animationToTimeline: node has fewer children than --last value\n") ; 
	    return false ;
	}
    }

    if (init_last == 0) // last not given on the command line
    {
	init_last = last = children-1 ;
    }

    dtkMsg.add(DTKMSG_INFO, "hev-animationToTimeline: init_first = %d, first = %d, init_last = %d, last = %d, children = %d\n",init_first,first,init_last,last,children) ;


    // turn on first
    printf("# turn the first one on and all the others off\n") ;
    for (unsigned int i=0; i<group_nodes.size(); i++)
    {
	printf("%f NODEMASK %s ON\n",0.0, ((group_nodes[i])[first]).c_str()) ;
    }

    // and turn off the rest at the same time
    for (unsigned int f=first+1; f<last+1; f++)
    {
	for (unsigned int i=0; i<group_nodes.size(); i++)
	{
	    printf("%f NODEMASK %s OFF\n",0.0, ((group_nodes[i])[f]).c_str()) ;
	}
    }

    printf("\n# turn the prevous one off and the next one on\n") ;
    // and then loop through the rest of the child nodes, turning the new one on and the on one off
    for (unsigned int f=first+1; f<last+1; f++)
    {
	for (unsigned int i=0; i<group_nodes.size(); i++)
	{
	    printf("%f NODEMASK %s OFF\n",frameTime, ((group_nodes[i])[f-1]).c_str()) ;
	    printf("%f NODEMASK %s ON\n",0.0, ((group_nodes[i])[f]).c_str()) ;
	}
    }
    
    printf("\n# now sleep for frametime in case we're looping\n") ;
    printf("%f\n\n",frameTime) ;
    return true ;
}

