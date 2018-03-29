#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iris/FifoReader.h>

namespace iris
{

    FifoReader::FifoReader(const std::string name) : _fd(-1), _oldDataOffset(0), _unlinkOnExit(false) { _name = name ; } ;
    
    FifoReader::~FifoReader() 
    { 
	if (_unlinkOnExit)
	{
	    // don't do anything if someone alreeady got rid of the fifo
	    struct stat buf ;
	    int ret ;
	    ret = stat(_name.c_str(), &buf) ;
	    if (ret != 0) return ;
	    dtkMsg.add(DTKMSG_INFO, "iris::FifoReader::destructor unlinking fifo %s\n",_name.c_str()) ;  
	    ret = unlink(_name.c_str()) ;
	    if (ret) dtkMsg.add(DTKMSG_ERROR, "iris::FifoReader::destructor error unlinking fifo %s\n",_name.c_str()) ;
	}
    } ;

    bool FifoReader::open(int flags)
    {
	if (_name == "")
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::FifoReader::open: no filename has been given\n", _name.c_str()) ;
	    return false ;
	}

	struct stat buf ;
	int ret = stat(_name.c_str(), &buf) ;
	// does file exist but isn't a fifo?
	if (ret == 0)
	{
	    if (! (S_ISFIFO(buf.st_mode)))
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris::FifoReader::open: file %s exists, but isn't a fifo\n", _name.c_str()) ;
		return false ;
	    }
	}
	else
	{
	    ret = mkfifo(_name.c_str(), 0777);
	    if (ret != 0)
	    {
		perror("FifoReader::open: mkfifo: ");
		return false ;
	    }
	}
	
	//fprintf(stderr,"iris::FifoReader::open: flags = %d\n",flags) ;
	_fd = ::open(_name.c_str(), flags) ;
	if (_fd<0)
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::FifoReader::open: unable to open fifo %s\n", _name.c_str()) ; 
	    return false ;
	}
	return true ; 
    } ;
    
    bool FifoReader::readLine(std::string* line) 
    {
	if (!line)
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::FifoReader::readLine: NULL line passed\n", _name.c_str()) ; 
	    return false ;
	}

	if (_fd<0)
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris::FifoReader::readLine: fifo %s is not open\n", _name.c_str()) ; 
	    return false ;
	}

	bool gotData = false ;

	if (_lines.size()>0)
	{
	    *line = _lines.front() ;
	    _lines.pop_front() ;
	    return true ;
	    //~fprint(stderr,"iris::FifoReader::readLine: lines already has data, returning true\n") ;
	}

	// need more data!
	else
	{
	    int bytes ;
	    //~fprintf(stderr,"before read\n") ;
	    bytes=read(_fd, _data+_oldDataOffset, PIPE_BUF) ;
	    //~fprintf(stderr,"after read, bytes = %d, data = %s\n",bytes,_data+_oldDataOffset) ;
	    if (bytes>0)
	    {
		if (_oldDataOffset+bytes>sizeof(_data))
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris::FifoReader:: data overrun.  Crashing soon.\n");
		    return false ;
		}


		// this is the index into data of the start of a line
		int lineOffset = 0 ;

		// find each \n in the buffer, convert it to a NULL and process the line
		int i ;
		for (i=_oldDataOffset; i<bytes+_oldDataOffset; i++)
		{
		    if (_data[i] == '\0')
		    {
			dtkMsg.add(DTKMSG_WARNING,"iris::FifoReader:: skipping NULL\n") ;
			continue ;
		    }
		    if (_data[i] == '\n') 
		    {
			_data[i] = '\0' ;
			if (!gotData)
			{
			    // just return the first one, don't save it for later
			    *line = _data ;
			    gotData = true ;
			    //~fprint(stderr,"iris::FifoReader::readLine: first one \"%s\"\n",(*line).c_str()) ;
			}
			else
			{
			    // and if more than one line read save it for later
			    _lines.push_back(_data+lineOffset) ;
			    //~fprint(stderr,"iris::FifoReader::readLine: adding to deque \"%s\"\n",_data+lineOffset) ;
			}
			lineOffset = i+1 ;  
		    }
		}
		
		// any unprocessed data that didn't end in a NL?
		
		// no \n found in whole buffer
		if (lineOffset == 0) _oldDataOffset += bytes;
		// scoot data to get rid of processed lines
		else if (bytes+_oldDataOffset>lineOffset)
		{
		    memmove(_data, _data+lineOffset, bytes+_oldDataOffset-lineOffset) ;
		    _oldDataOffset = bytes+_oldDataOffset-lineOffset ;
		}
		else _oldDataOffset = 0 ;
	    }
	    else
	    {
		//~fprint(stderr,"iris::FifoReader::readLine: bytes<=0, returning false\n") ;
		gotData=false ;
	    }
	}
	//~fprint(stderr,"iris::FifoReader::readLine: line = \"%s\"\n",line.c_str()) ;
	return gotData ;
    } ;

    bool FifoReader::write(const std::string& line) 
    {
	//fprintf(stderr,"line = \"%s\" size %d\n",line.c_str(),line.size()) ;
	int ret ;
	ret = ::write(_fd, line.c_str(), line.size()) ;
	if (ret != line.size())
	{
	    dtkMsg.add(DTKMSG_WARNING,"iris::FifoReader::writeLine: fifo %s, write returns %d, tried to write %d bytes\n",_name.c_str(),ret,line.size()) ;
	    return false ;
	}
	else return true ;
    } ;
}
