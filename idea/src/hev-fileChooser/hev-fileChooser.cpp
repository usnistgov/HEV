#include <stdio.h>
#include <stdlib.h>

#include <FL/Fl.H>
#include <FL/Fl_File_Chooser.H>

#include <iris.h>


////////////////////////////////////////////////////////////////////////
static void usage()
{
    printf("Usage: hev-fileChooser [ --directory string ] [ --pattern string ] [ --title string ]\n") ;
}

////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) 
{

    std::string dir = getenv("HOME") ;
    std::string pattern ;
    std::string title = "hev-fileChooser" ;

    int i = 1 ;
    while (i<argc)
    {
	if (iris::IsSubstring("--directory", argv[i]))
	{
	    i++ ;
	    if (i<argc) 
	    {
		dir = argv[i] ;
	    }
	    else
	    {
		usage() ;
		return -1 ;
	    }
	    i++ ;
	}
	else if (iris::IsSubstring("--pattern", argv[i]))
	{
	    i++ ;
	    if (i<argc) 
	    {
		pattern = argv[i] ;
	    }
	    else
	    {
		usage() ;
		return -1 ;
	    }
	    i++ ;
	}
	else if (iris::IsSubstring("--title", argv[i]))
	{
	    i++ ;
	    if (i<argc) 
	    {
		title = argv[i] ;
	    }
	    else
	    {
		usage() ;
		return -1 ;
	    }
	    i++ ;
	}
	else
	{
	    usage() ;
	    return -1 ;
	}
    }


    Fl_File_Chooser *c = new Fl_File_Chooser(dir.c_str(), pattern.c_str(), Fl_File_Chooser::SINGLE, title.c_str()) ;
    c->preview(0) ;
    c->show() ;

    while (Fl::wait()) ;
    
    if (c->value())
    {
	printf(c->value()) ;
	return 0 ;
    }
    else
    {
	return 1 ;
    }
    
}
