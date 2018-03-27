#include "idea/Utils.h"

#include <strings.h>

bool idea::isSubstring(const std::string keyword, const std::string supplied,
                       const unsigned int minLength)
{
  unsigned int m = minLength ;
  if (m==0)
    m = supplied.length() ;
  if (keyword.length() < m || supplied.length() < m)
    return false ;
  if (strncasecmp(keyword.c_str(), supplied.c_str(), m))
    return false ;
  else
    return true ;
}

