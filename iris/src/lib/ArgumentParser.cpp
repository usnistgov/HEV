#include <dtk.h>

#include <iris/ArgumentParser.h>
#include <iris/Utils.h>

namespace iris
{

    ////////////////////////////////////////////////////////////////////////
    int ArgumentParser::findSubstring(const std::string& str, const unsigned int len) const
    {
	for (unsigned int i=0; i<*(_argc); i++)
	{
	    if (IsSubstring(str.c_str(), _argv[i], len)) return i ; 
	}
	return -1 ;
    } ;

    ////////////////////////////////////////////////////////////////////////
    bool ArgumentParser::isOption (int pos) const
    {
	return (_argv[pos][0] == '-' && _argv[pos][1] == '-' ) ;
    } ;

    ////////////////////////////////////////////////////////////////////////
    bool ArgumentParser::isOption (const char* str)
    {
	return (str && str[0] == '-' && str[1] == '-' ) ;
    } ;

    ////////////////////////////////////////////////////////////////////////
    int ArgumentParser::findOption(int start)
    {
	if (start<1)
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::ArgumentParser::findOption: start must be greater than 1\n") ;
	    return 0 ;
	}
	for (int pos=start;pos<*_argc;++pos)
	{
	    if (isOption(pos)) return pos ;
	}
	return 0 ;
    }

}
