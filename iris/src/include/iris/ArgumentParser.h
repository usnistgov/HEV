#ifndef __IRIS_ARGUMENT_PARSER__
#define __IRIS_ARGUMENT_PARSER__

#include <osg/ArgumentParser>

namespace iris
{

    /**
       \brief The %ArgumentParser class is sub-classed from the osg::ArgumentParser
       class and adds a method to find options by substring, and restricts
       option searches to using two-dash-options.
    */

    ////////////////////////////////////////////////////////////////////////
    // add a findSubstring method to the osg::ArgumentParser class
    // and isOption only looks for two-dash-options 
    class ArgumentParser : public osg::ArgumentParser
    {
    public:
	ArgumentParser(int *argc, char **argv) : osg::ArgumentParser(argc, argv) {} ;
	/**
	   \brief find a substring of an argument using iris::IsSubstring
	*/
	int findSubstring(const std::string& str, const unsigned int len = 0) const ;
	/**
	   \brief returns \c true if argument at <i>pos</i> is a two-dash-option
	*/
	bool isOption (int pos) const ;
	/**
	   \brief return position of first two-dash-option in args, or 0 is none found
	*/
	int findOption(int start = 1) ;
	/**
	   \brief returns \c true if <i>str</i>is a two-dash-option
	*/
	static bool isOption(const char* str);
    } ;
}  
#endif
