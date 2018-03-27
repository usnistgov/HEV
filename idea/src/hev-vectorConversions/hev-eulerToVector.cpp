#include <iostream>
#include <fstream>
#include <string>

#include <iris.h>

// reads stdin and converts h, p, r to x y z, leaving tokens before and after
// h p r unchanged.  Use the option to specify how many tokens before h p r
// should be skipped.
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
		  "hev-eulerToVector: \"%s\" is not a valid positive integer\n",
		  argv[2]) ;
	  return 1 ;
	}
    }
  else
    {
      fprintf(stderr,
	      "Usage: hev-eulerToVector [ -c n ]\n"
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
		  "hev-eulerToVector: line %d \"%s\" has too few tokens\n",
		  num, line.c_str()) ;
	  return 1 ;
	}

      out.erase();
      // toss leading words
      for (int i=0; i<c; i++)
	{
	  out = out + vec[i] + " " ;
	}

      // next three are hpr, convert to xyz
      double h, p, r ;
      if (iris::StringToDouble(vec[c], &h) &&
	  iris::StringToDouble(vec[c+1], &p) &&
	  iris::StringToDouble(vec[c+2], &r))
	{
	  osg::Vec3d v = iris::EulerToQuat(h, p, r) * osg::Vec3d(0.0, 1.0, 0.0) ;
	  out = out + 
	    iris::DoubleToString(v.x()) + " " + 
	    iris::DoubleToString(v.y()) + " " + 
	    iris::DoubleToString(v.z()) + " " ;
	}
      else
	{
	  fprintf(stderr,
		  "hev-eulerToVector: line %d \"%s\" has an invalid h p or r\n",
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

