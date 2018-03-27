#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#include "idea/Utils.h"

bool idea::fltkOptions(const int argc, const char **argv,
                       int *fltk_argc, char ***fltk_argv,
                       int *other_argc, char ***other_argv) 
{

  /*
    FLTK options are:
    -d[isplay] host:n.n
    -g[eometry] WxH+X+Y
    -t[itle] windowtitle
    -n[ame] classname
    -i[conic]
    -fg color
    -bg color
    -bg2 color
  */

  std::vector<std::string> fltk_args2 ;
  fltk_args2.push_back("-bg2") ;
  fltk_args2.push_back("-bg") ;
  fltk_args2.push_back("-di") ;
  fltk_args2.push_back("-fg") ;
  fltk_args2.push_back("-g") ;
  fltk_args2.push_back("-na") ;
  fltk_args2.push_back("-s") ;
  fltk_args2.push_back("-ti") ;

  std::vector<std::string> fltk_args1 ;
  fltk_args1.push_back("-dn") ;
  fltk_args1.push_back("-i") ;
  fltk_args1.push_back("-k") ;
  fltk_args2.push_back("-nod") ;
  fltk_args2.push_back("-nok") ;
  fltk_args2.push_back("-not") ;
  fltk_args2.push_back("-to") ;

  // unlike dtk, we aren't nice about null pointers
  if (!argv || !fltk_argc || !fltk_argv || !other_argc || !other_argv ||argc<1) 
    return false ;
  
  *other_argv = (char **) malloc(sizeof(char *)*(argc+1)) ;
  (*other_argv)[0] = (char *) argv[0] ;
  (*other_argv)[1] = NULL ;
  *other_argc = 1 ;

  *fltk_argv = (char **) malloc(sizeof(char *)*(argc+1)) ;
  (*fltk_argv)[0] = (char *) argv[0] ;
  (*fltk_argv)[1] = NULL ;
  *fltk_argc = 1 ;

  std::vector<std::string> arg, other, fltk ;

  arg =  idea::argvArgcToStringVector(argc, argv) ;

  // start at one since we skip argv[0]
  bool fltkOptionFound ;
  for (int i=1; i<arg.size(); i++)
    {
      fltkOptionFound = false ;
      for (int j=0; j<fltk_args1.size(); j++)
	{
	  if (idea::isSubstring(arg[i],fltk_args1[j]))
	    {
	      fltk.push_back(arg[i]) ;
	      fltkOptionFound = true ;
	      break ;
	    }
	}
      for (int j=0; j<fltk_args2.size(); j++)
	{
	  if (idea::isSubstring(arg[i],fltk_args2[j]))
	    {
	      fltk.push_back(arg[i]) ;
	      i++ ;
	      fltk.push_back(arg.at(i)) ;
	      fltkOptionFound = true ;
	      break ;
	    }
	}
      if (!fltkOptionFound)
	other.push_back(arg[i]) ;
    }

  *other_argc += other.size() ;
  *other_argv = (char **) realloc(*other_argv, sizeof(char *)*(2+other.size())) ;
  for (int i=0; i<other.size(); i++)
    (*other_argv)[i+1] = strdup(other[i].c_str()) ;
  (*other_argv)[other.size()+1] = NULL ;

  *fltk_argc += fltk.size() ;
  *fltk_argv = (char **) realloc(*fltk_argv, sizeof(char *)*(2+fltk.size())) ;
  for (int i=0; i<fltk.size(); i++)
    (*fltk_argv)[i+1] = strdup(fltk[i].c_str()) ;
  (*fltk_argv)[fltk.size()+1] = NULL ;

  return true ;
}

