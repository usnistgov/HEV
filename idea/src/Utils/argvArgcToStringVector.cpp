#include <string>
#include <vector>

#include "idea/Utils.h"

std::vector<std::string> idea::argvArgcToStringVector(const int argc, const char **argv)
{
  std::vector<std::string> vec ;
  for (int i=0; i<argc; i++)
    vec.push_back(argv[i]) ;
  return vec ;
}

