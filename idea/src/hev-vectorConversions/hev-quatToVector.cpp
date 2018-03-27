#include <iostream>
#include <fstream>
#include <string>

#include <iris.h>

// reads stdin and converts quaternion x, y, z, w to x y z, leaving tokens
// before and after x y z w unchanged.  Use the option to specify how many
// tokens before x y z w should be skipped.
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
		  "hev-quatToVector: \"%s\" is not a valid positive integer\n",
		  argv[2]) ;
	  return 1 ;
	}
    }
  else
    {
      fprintf(stderr,
	      "Usage: hev-quatToVector [ -c n ]\n"
	      "  where n specifies the number of tokens to skip\n"
	      "  before converting h p r  to x y z\n") ;
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
		  "hev-quatToVector: line %d \"%s\" has too few tokens\n",
		  num, line.c_str()) ;
	  return 1 ;
	}

      out.erase();
      // toss leading words
      for (int i=0; i<c; i++)
	{
	  out = out + vec[i] + " " ;
	}

      // next three are xyzw, convert to xyz
      double xq, yq, zq, wq, x, y, z ;
      if (iris::StringToDouble(vec[c], &xq) &&
	  iris::StringToDouble(vec[c+1], &yq) &&
	  iris::StringToDouble(vec[c+2], &zq) &&
	  iris::StringToDouble(vec[c+3], &wq))
	{
	  osg::Vec3d v =  osg::Quat(xq, yq, zq, wq) * osg::Vec3d(0.0,1.0, 0.0) ;
	  out = out + 
	    iris::DoubleToString(v.x()) + " " + 
	    iris::DoubleToString(v.y()) + " " + 
	    iris::DoubleToString(v.z()) + " " ;
	}
      else
	{
	  fprintf(stderr,
		  "hev-quatToVector: line %d \"%s\" has an invalid x y z or w\n",
		  num, line.c_str()) ;
	}
	
      // and what's left
      for (int i=c+4; i<vec.size(); i++)
	{
	  out = out + vec[i] + " " ; 
	}

      printf("%s\n",out.c_str()) ;

      num++ ;
    }

    
  return 0 ;
}

