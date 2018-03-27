#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

#include <iris.h>

void usage()
{
    fprintf(stderr,"Usage: iris-getName file\n") ;
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

    // get the input files
    osg::Node* node = osgDB::readNodeFile(args[1]) ;
    if (!node) 
    {
	dtkMsg.add(DTKMSG_ERROR, "iris-getName: can't load model file \"%s\"\n",args[1]) ;
	return 1 ;
    }

    std::string name = node->getName() ;
    if (name == "")
    {
	dtkMsg.add(DTKMSG_WARNING, "iris-getName: model file \"%s\" does not return a node with a name\n",args[1]) ;
	return 0 ;
    }
    else printf("%s\n",name.c_str()) ;

    return 0 ;
}
