#include <iostream>
#include <fstream>
#include <string>

#include <osg/Quat>

#include <iris.h>

// reads stdin and converts a x y z vector to a x y z w quaternion, leaving
// tokens before and after x y z unchanged.  Use the option to specify how
// many tokens before x y z should be skipped.
int main(int argc, char **argv)
{

    int c ;

    if (argc == 1)
    {
	c = 0 ;
    }
    else if (argc == 3)
    {
	if (!iris::StringToInt(argv[2], &c) || c<0)
	{
	  fprintf(stderr,
		  "hev-vectorToQuat: \"%s\" is not a valid positive integer\n",
		  argv[2]) ;
	  return 1 ;
	}
    }
    else
    {
	fprintf(stderr,
		"Usage: hev-vectorToQuat [ -c n ]\n"
		"  where n specifies the number of tokens to skip\n"
		"  before converting vector x y z to quaternion x y z w\n") ;
	return 1 ;
    }

    std::string line ;
    int num = 0 ;
    std::string out ;

    while (getline(std::cin,line))
    {
      std::vector<std::string> vec = iris::ParseString(line) ;
	if (vec.size()<c+3)
	{
	    fprintf(stderr,
		    "hev-vectorToQuat: line %d \"%s\" has too few tokens\n",
		    num, line.c_str()) ;
	    return 1 ;
	}

	out.erase();
	// toss leading words
	for (int i=0; i<c; i++)
	{
	    out = out + vec[i] + " " ;
	}

	// next three are xyz, convert to xyzw
	double x, y, z, h, p, r ;
	if (iris::StringToDouble(vec[c], &x) &&
	    iris::StringToDouble(vec[c+1], &y) &&
	    iris::StringToDouble(vec[c+2], &z))
	{
	    osg::Quat q ;
	    q.makeRotate(osg::Vec3(0,1,0),osg::Vec3(x,y,z)) ;
	    out = out + 
		iris::DoubleToString(q.x()) + " " + 
		iris::DoubleToString(q.y()) + " " + 
		iris::DoubleToString(q.z()) + " " + 
		iris::DoubleToString(q.w()) + " " ;
	}
	else
	{
	    fprintf(stderr,
		    "hev-vectorToQuat: line %d \"%s\" has an invalid x y or z\n",
		    num, line.c_str()) ;
	}
	
	// and what's left
	for (int i=c+3; i<vec.size(); i++)
	{
	   out = out + vec[i] + " " ; 
	}

	printf("%s\n",out.c_str()) ;

	num++ ;
    }

    
    return 0 ;
}

