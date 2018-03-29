#ifndef __IRIS_FIFO_READER__
#define __IRIS_FIFO_READER__

#include <linux/limits.h>
#include <fcntl.h>
#include <string>
#include <deque>

#include <dtk.h>

// reads lines from a fifo
namespace iris
{
    /**
       \brief The %FifoReader class simplifies the reading of lines of text from a fifo.
       \n \n Applications write to a fifo using shell redirection with lines of unequal
       length, and the data can span multiple system buffers.  \n \n An object of the
       %FifoReader class can simplifies reading the fifo data one line at a time.  The
       %FifoReader class takes care of data buffering and the actual fifo reads.
    */

    class FifoReader
    {
    public:
	FifoReader(const std::string name = "");
	~FifoReader() ;
	bool open(int flags = (O_RDONLY | O_NONBLOCK)) ; ;
	// if data is not NULL, set its reference to true if data returned
	// this lets you tell the difference between a blank line and no data, if you care
	bool readLine(std::string* line) ;
	// see SceneGraph class for an example of using this
	bool write(const std::string& line) ;
	void setName(const std::string name) { _name = name ; } ;
	const std::string getName() { return _name ; } ;
	void unlinkOnExit(bool u = true) { _unlinkOnExit = u ; } ;
	int getFD() { return _fd ; } ;
    private:
	int _fd ;
	std::string _name ;
	// make this arbitrarily large enough to hold any single pipe line and a few buffers at a time
	char _data[PIPE_BUF*16] ;
	// holds lines read from the fifo
	std::deque<std::string> _lines ;
	// this is the index into data of where we can add new data
	int _oldDataOffset ;
	bool _unlinkOnExit ;
	
    } ;
}
#endif
