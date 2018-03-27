/*

DTK service return values from dtkService.h:

// Tell the DTK server that all is happy.
#define DTKSERVICE_CONTINUE   0

// Tell the server to unload the current service.
#define DTKSERVICE_UNLOAD     3

// Tell the server that a fatal error has accorded, which will cause
// the DTK server to exit.
#define DTKSERVICE_ERROR     -1

the config file to use is determined as follows:

The filename "wanda.config" is used, unless an argument is passed when
loading the service.  If the argument is an absolute pathname, then it it is
used without any further processing.  Error out if the file does not exist.
If the supplied or argument filename is relative, then it is searched for in
the path, as follows:

1) If $DTK_SERVICE_CONFIG_PATH is set, look in these directories first,
stopping at the first match.

2) If $DTK_SERVICE_CONFIG_PATH is not set, or was set but not found above,
look in the directories "." and "DTK_SERVICE_DIR".  If not found in
either of these, error out.

The config file can contain the following commands.  It has the usual
syntax- # is a comment, keywords are case insensative, and blank lines and
duplicate spaces are ignored.

# name of the serial device to open to read the data
port filename

# if passed, then data is written to VRCO trackd compatible sys V shared memory
trackd

# what order to map the buttons.  b0 means to map wand button 0 to shared memory button 0, etc.
# duplicates are OK- one wand button can go to shared memory buttons.  For example:
# buttonorder 2 1 1 causes changes in wand button 2 to be reported as changes in shared memory button 0, and 
# changes in wand button 1 to be reported as changes in shared memory buttons 1 and 2.
# to cause a wand button to ignored, use the value -1
buttonorder b0 b1 b2

# what order to map the joysticks- same type of parameters are above
joystickorder j0 j1

# the joystick value can be plugged into a polynomial, A + Bj + Cj^2 + Dj^3 ...
# use the mapped joystick number- i.e. j0 or j1 for j, and and arbitrary number of polynomial
# factors following
joystickpolynomial j A B C D ...

# this is how long to wait after receiving no data before zeroing the joystick, in seconds
timeout N

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <dtk.h>
#include <dtk/dtkService.h>
# include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <errno.h>

// new-fangled constants
static const std::string default_configfile = "wanda.config" ;
static const std::string default_configpath = ".:"DTK_SERVICE_DIR ;
static const std::string default_port = "/dev/ttyUSB0" ;
static const bool default_trackd = false ;
static const float default_timeout = .2f ;

// these are shift values, not index values
static const int default_button0 = 1 ;
static const int default_button1 = 2 ;
static const int default_button2 = 4 ;
// these are index values
static const int default_joystick0 = 0 ;
static const int default_joystick1 = 1 ;

static const size_t read_buffer_size = 128 ;
// clamp joystick numbers to this absolute value
static const float max_xy_offset = 34.f ;

class Wanda : public dtkConfigService {
public:
    Wanda(const char *arg) ;
    virtual ~Wanda(void) ;
    int init();
    int serve() ;
  
private:
    int configureWanda();
    void clean_up(void) ;

    unsigned char map_buttons(unsigned char buttons) ;
    void map_joystick(float joystick[2], float mapped[2]) ;

    std::string joystick_name;
    std::string buttons_name;

    dtkSharedMem *button_shm ;
    dtkSharedMem *joystick_shm ;
  
    float joystick_state[2] ;
    float joystick_state_mapped[2] ;
    unsigned char button_state ;
    unsigned char button_state_mapped ;

    dtkVRCOSharedMem VRCO ;
    int termio_set_flag ;
    struct termios original_terminfo ;
  
    unsigned char read_buffer[read_buffer_size] ;
    int button_map[3] ;
    int joystick_map[2] ;
    std::vector<float> joystickpoly[2] ;


    // the wanda stops sending data when you release the joystick, but
    // doesn't always send a zero, so the joystick data can get stuck at
    // some non-zero value even though the joystick is released.

    // the basic idea to get around this is that you have 2 fds, one for the
    // wanda serial device, and one for a fifo.  You also have a timer.
    // When serve is called, it can have data on either the serial line or the
    // fifo.  Every time you read serial data, you process the data and
    // reset the timer.  When you get data on the fifo you just read and
    // toss it.  No matter which fd is seleced, you check the timer- if the
    // timer is less than the timeout value, you write to the fifo, ensuring
    // you'll get called again.  If the time is greater than or equal to the
    // timeout value you zero the joystick.  More serial data will start the
    // process over.  The downside is that you soak a CPU while you are
    // getting serial data, until you timeout.  But once you timeout, you
    // don't get selected again. You could put a sleep in the case where you
    // get data on the fifo, but that will slow down the entire dtk-server,
    // which is probably worse than soaking the CPU.


    // used to measure how much time has elapsed since we last got data
    dtkTime timeout_timer;

    // how long to wait after receiving no data before zeroing the joystick, in seconds
    float timeout ;

    // name of a fifo to get control after timeout
    char *fifo_name ;
    
    // data to write to fifo
    char fifo_data ;

    // fd to write to fifo
    int fifo_write ;

    // fd to read from fifo
    int fifo_read ;

    // fd to read from serial line
    int serial_read ;


} ;

////////////////////////////////////////////////////////////////////////
Wanda::Wanda(const char *arg) {
    button_state = 0;
    joystick_state[0] = 0;
    joystick_state[1] = 0;
}

////////////////////////////////////////////////////////////////////////
Wanda::~Wanda(void) {
    //~printf("destructor called\n") ;
    clean_up() ;
}

int Wanda::configureWanda()
{
    bool any_error = false;
    bool has_error = false;
    std::string error;
    std::vector<std::string> error_strings;
    std::vector<std::vector<std::string> > config = getConfig();
    dtkMsg.add( DTKMSG_NOTICE, "Examining configuration...\n" );
    for( unsigned int i=0;i<config.size();i++ )
    {
	if( config[i][0] == "dtkshm" )
	{
	    if( config[i].size() != 2 && config[i].size() != 3 )
		has_error = true;
	    if( config[i][1] != "joystick" && config[i][1] != "buttons" )
		has_error = true;
	    if( has_error )
	    {
		error = "The dtkShm option specifies the name of the dtkSharedMem\n"
		    "segment to be created and used for data output.\n"
		    "Syntax: serviceOption name dtkShm type shmName"
		    "Example: serviceOption ";
		error += getName();
		error += "dtkShm joystick jajoystick"
		    "where name was defined with the serviceType definition,\n"
		    "type is either \"joystick\" or \"buttons\" and\n"
		    "shmName is a valid name for a file.\n";
		error_strings.push_back( error );
	    }
	    else
	    {
		if( config[i][1] == "joystick" )
		    joystick_name = config[i][2];
		else
		    buttons_name = config[i][2];
	    }
	}
	else if( config[i][0] == "buttonorder" )
	{
	    int value[3];
	    if( config[i].size() != 4 )
		has_error = true;
	    else
	    {
		for( int j=1;j<4;j++ )
		{
		    if( convertStringToNumber( value[j], config[i][j].c_str() ) &&
			( value[j] < -1 || value[j] > 2 ) )
		    {
			has_error = true;
			error = "Unknown button ";
			error += j - 1;
			error += " value ";
			error += config[i][j];
			error_strings.push_back( error );
		    }
		}
	    }
	    if( has_error )
	    {
		error = "The buttonorder option specifies how to map button presses to dtk\n"
		    "shared memory.\n"
		    "Syntax: serviceOption name buttonorder button1 button2 button3\n"
		    "Example: serviceOption ";
		error += getName();
		error += " buttonorder 2 1 -1\n"
		    "where name was defined with the serviceType definition,\n"
		    "button1, button2, and button3 are the order in which each button is to\n"
		    "be represented in the shared memory.\n";
		error_strings.push_back( error );
	    }
	    else
	    {
		for( int j=0;j<3;j++ )
		{
		    switch( value[j] )
		    {
		    case -1:
			{
			    button_map[j] = -1;
			    break;
			}
		    case 0:
			{
			    button_map[j] = 1;
			    break;
			}
		    case 1:
			{
			    button_map[j] = 2;
			    break;
			}
		    case 2:
			{
			    button_map[j] = 4;
			    break;
			}
		    }
		}
	    }
	}
	else if( config[i][0] == "timeout" )
	{
	    float value[1];
	    if( config[i].size() != 2 )
		has_error = true;	
	    else
	    {
		if( convertStringToNumber( value[0], config[i][1].c_str() ) &&
		    ( value[0] < 0 ) )
		{
		    has_error = true;
		    error = "Invalid timeout value ";
		    error += config[i][1];
		    error_strings.push_back( error );
		}
	    }
	    if( has_error )
	    {
		error = "The timeout option specifies how long to wait until setting\n"
		    "the joystick to zero due to lack of data.\n"
		    "Syntax: serviceOption name timeout j\n"
		    "Example: serviceOption ";
		error += getName();
		error += " timeout .2\n"
		    "where name was defined with the serviceType definition,\n"
		    "j is a timeout value in seconds.\n" ;
		error_strings.push_back( error );
	    }
	    else
	    {
		timeout = value[0] ;
	    }
	}
	else if( config[i][0] == "joystickorder" )
	{
	    int value[2];
	    if( config[i].size() != 3 )
		has_error = true;
	    else
	    {
		for( int j=1;j<3;j++ )
		{
		    if( convertStringToNumber( value[j], config[i][j].c_str() ) &&
			( value[j] < -1 || value[j] > 1 ) )
		    {
			has_error = true;
			error = "Unknown joystick ";
			error += j - 1;
			error += " value ";
			error += config[i][j];
			error_strings.push_back( error );
		    }
		}
	    }
	    if( has_error )
	    {
		error = "The joystickorder option specifies how to map the joystick to dtk\n"
		    "shared memory.\n"
		    "Syntax: serviceOption name joystickorder js1 js2\n"
		    "Example: serviceOption ";
		error += getName();
		error += " joystickorder 2 1\n"
		    "where name was defined with the serviceType definition,\n"
		    "js1, and js2 are the order in which the joystick is to\n"
		    "be represented in the shared memory.\n";
		error_strings.push_back( error );
	    }
	    else
	    {
		for( int j=0;j<2;j++ )
		{
		    joystick_map[j] = value[j];
		}
	    }
	}
	else if( config[i][0] == "joystickpolynomial" || config[i][0] == "joystickpoly" )
	{
	    int js;
	    std::vector<float> values;
	    float value;
	    if( config[i].size() < 3 )
		has_error = true;
	    else
	    {
		if( convertStringToNumber( js, config[i][1].c_str() ) ||
		    ( js != 0 && js != 1 ) )
		{
		    has_error = true;
		    error = "The joystick must be specified by either \"0\" or \"1\"\n"
			"to designate the x or y joystick value.\n";
		    error_strings.push_back( error );
		}
		else
		{
		    for( unsigned int j=1;j<config[i].size();j++ )
		    {
			if( convertStringToNumber( value, config[i][j].c_str() ) )
			{
			    has_error = true;
			    error = "The value %s is not a valid numerical value.\n";
			    error_strings.push_back( error );
			}
			else
			    values.push_back( value );
		    }
		}
	    }
	    if( has_error )
	    {
		error = "The joystickpolynomial option specifies the coefficients of a\n"
		    "polynomial to alter the joystick values.\n"
		    "Syntax: serviceOption name joystickpolynomial js C0 ...\n"
		    "Example: serviceOption ";
		error += getName();
		error += " joystickpolynomial 0.8 1.2 0.2\n"
		    "where name was defined with the serviceType definition,\n"
		    "js is either \"0\" representing the x axis of the joystick\n"
		    "or \"1\" representing the y axis of the joystick,\n"
		    "C0 is the first coefficient, and ... can be all additional coefficients\n";
		error_strings.push_back( error );
	    }
	    else
	    {
		joystickpoly[js] = values;
	    }
	}
	else
	{
	    has_error = true;
	    std::string error = "The option ";
	    error += config[i][0];
	    error += " is not a valid configuration option\n";
	    error_strings.push_back( error );
	}
	if( has_error )
	{
	    any_error = true;
	    dtkMsg.add( DTKMSG_ERROR, "error in line: " );
	    for( unsigned int j=0;j<config[i].size();j++ )
	    {
		dtkMsg.append( "%s ", config[i][j].c_str() );
	    }
	    dtkMsg.append( "\n" );
	    for( unsigned int j=0;j<error_strings.size();j++ )
	    {
		dtkMsg.append( error_strings[j].c_str() );
	    }
	}
    }

    if( any_error )
	return DTKSERVICE_ERROR;

    return DTKSERVICE_CONTINUE;
}

int Wanda::init()
{
    // set defaults ;
    button_map[0] = default_button0 ;
    button_map[1] = default_button1 ;
    button_map[2] = default_button2 ;
    joystick_map[0] = default_joystick0 ;
    joystick_map[1] = default_joystick1 ;
    joystick_name = "joystick";
    buttons_name = "buttons";
    timeout_timer.reset(0.0,1);
    timeout = default_timeout ;

    int error;
    if( ( error = configureWanda() ) )
	return error;

    button_shm = NULL ;
    joystick_shm = NULL ;
    joystick_state[0] = 0.0f ;
    joystick_state[1] = 0.0f ;
    termio_set_flag = 0 ;
  
    if(getTrackd() && VRCO.isInvalid()) return DTKSERVICE_UNLOAD ; // error
  

    /************************************************************/
    /***************** open serial device file ******************/
    /************************************************************/
  
    if((serial_read = open(getPort().c_str(), O_RDWR|O_NONBLOCK)) < 0) {
	dtkMsg.add(DTKMSG_ERROR, 1,
		   "Wanda::Wanda() failed: open(\"%s\", "
		   "O_RDWR|O_NONBLOCK)"
		   " failed.\n", getPort().c_str()) ;
	clean_up() ;
	return DTKSERVICE_UNLOAD ;
    }
    fd_vector.push_back(serial_read) ;

    // get and save original termio info
    if(tcgetattr(serial_read, &original_terminfo) == -1) {
	dtkMsg.add(DTKMSG_ERROR, 1,
		   "Wanda::Wanda() failed: "
		   "tcgetattr(%d,,) failed.\n",
		   serial_read) ;
	clean_up() ;
	return DTKSERVICE_UNLOAD ;
    }
  
    struct termios terminfo ;
    terminfo.c_iflag = 0 ;
    terminfo.c_oflag = 0 ;
    terminfo.c_lflag = 0 ;
  
    terminfo.c_cflag = CS7 | HUPCL | CSTOPB | CREAD | CLOCAL ;
    if (cfsetospeed(&terminfo, getBaudFlag())  == -1) {
	dtkMsg.add(DTKMSG_ERROR, 1,
		   "Wanda::Wanda() failed: "
		   "cfsetospeed(%d,,) failed.\n",serial_read) ;
	clean_up() ;
	return DTKSERVICE_UNLOAD ;
    }
  
    if (cfsetispeed(&terminfo, getBaudFlag())  == -1) {
	dtkMsg.add(DTKMSG_ERROR, 1,
		   "Wanda::Wanda() failed: "
		   "cfsetispeed(%d,,) failed.\n",serial_read) ;
	clean_up() ;
	return DTKSERVICE_UNLOAD ;
    }
  
    for (int i=0 ; i<NCCS ; i++)
	terminfo.c_cc[i] = 0 ;
  
    if (tcsetattr(serial_read, TCSANOW, &terminfo) == -1) {
	dtkMsg.add(DTKMSG_ERROR, 1,
		   "Wanda::Wanda() failed: "
		   "tcsetattr(%d,,) failed.\n",serial_read) ;
	clean_up() ;
	return DTKSERVICE_UNLOAD ;
    }
    termio_set_flag = 1 ;
  
    if (tcflush(serial_read, TCIOFLUSH) == -1) {
	dtkMsg.add(DTKMSG_ERROR, 1,
		   "Wanda::Wanda() failed: "
		   "Could not flush serial port: "
		   "tcflush() failed.\n") ;
	clean_up() ;
	return DTKSERVICE_UNLOAD ;
    }
  
    /*************************************************************/
    /****************** create a fifo ****************************/
    /******** use to get control after a serial line timeout *****/
    /*************************************************************/

    fifo_name = tempnam("/tmp", "fake_wanda_file_") ;
    if (mkfifo(fifo_name, 0777) != 0)
    {
	dtkMsg.add(DTKMSG_ERROR, 1,
		   "Wanda::Wanda() failed: mkfifo(\"%s\", "
		   " 0777) failed.\n", fifo_name) ;
	clean_up() ;
	return DTKSERVICE_UNLOAD ;
    }
    fifo_read = open(fifo_name, O_RDONLY|O_NONBLOCK) ;
    fd_vector.push_back(fifo_read) ;

    fifo_write = open(fifo_name, O_WRONLY|O_NONBLOCK) ;

       
    // write a byte to the fifo so it'll be selected
    fifo_data = 1 ;
    if ((write(fifo_write, &fifo_data, sizeof(fifo_data))<0))
    {
	dtkMsg.add(DTKMSG_ERROR, 1,
		   "Wanda::Wanda() failed: can't write to fifo\n") ;
	clean_up() ;
	return DTKSERVICE_UNLOAD ;
    }
    

    /*************************************************************/
    /****************** get some DTK shared memory ***************/
    /*************************************************************/
  
    button_shm = new
	dtkSharedMem(sizeof(unsigned char),buttons_name.c_str(), &button_state) ;
    joystick_shm = new
	dtkSharedMem(sizeof(float)*2,joystick_name.c_str(), joystick_state) ;
  
    if(!button_shm || !joystick_shm) {
	// This will let you know why dtkSharedMem::dtkSharedMem()
	// spewed.
	dtkMsg.add(DTKMSG_ERROR,
		   "Wanda::Wanda() failed.\n") ;
	clean_up() ;
	return DTKSERVICE_ERROR;
    }

    return DTKSERVICE_CONTINUE;  
}

////////////////////////////////////////////////////////////////////////
void Wanda::clean_up(void) {
    if(button_shm) {
	delete button_shm ;
	button_shm = NULL ;
    }
  
    if(joystick_shm) {
	delete joystick_shm ;
	joystick_shm = NULL ;
    }
  
    if(serial_read > -1) {
	if(termio_set_flag) {
	    termio_set_flag = 0 ;
	    if (tcsetattr(serial_read, TCSANOW, &original_terminfo) == -1)
		dtkMsg.add(DTKMSG_WARN, 1,
			   "Wanda::~Wanda() failed: failed "
			   "to reset serial device"
			   " termio settings: "
			   "tcsetattr(%d,,) failed.\n",serial_read) ;
	}
	close(serial_read) ;
	serial_read = -1 ;
    }

    if(fifo_write > -1) {
	close(fifo_write) ;
    }

    if(fifo_read > -1) {
	close(fifo_write) ;
	unlink(fifo_name) ;
    }

}

////////////////////////////////////////////////////////////////////////
int Wanda::serve(void) {

#if 0
    timeval now ;
    gettimeofday(&now, NULL) ;
    double nowf = now.tv_sec +  ((double)now.tv_usec)/1000000.0 ;
    std::cout << "serve called at time " << std::fixed << std::setprecision(20) << nowf << std::endl ;
#endif

    for (unsigned int i=0; i<fd_set_vector.size(); i++)
    {
	if (fd_set_vector[i] == serial_read)
	{
	    // reset timer
	    timeout_timer.reset(0.0,1);
	    // process serial data
	    ssize_t bytes_read =
		read(serial_read, read_buffer, read_buffer_size) ;
	    //~printf("after serial read, %d bytes read\n",bytes_read) ;
	    
	    /*************************************************************/
	    /*** checking error cases first. Cause it's easier that way **/
	    /*************************************************************/
	    
	    if(bytes_read <= 0) {
		if(bytes_read == 0) { // This should never happen.
		    dtkMsg.add(DTKMSG_ERROR, 1,
			       "Wanda::serve() failed: "
			       "read() read no data.\n") ;
		    return DTKSERVICE_CONTINUE ;
		} else { // if(bytes_read < 0) // error
		    dtkMsg.add(DTKMSG_ERROR, 1,
			       "Wanda::serve() failed: "
			       "read() failed.\n") ;
		    return DTKSERVICE_CONTINUE ;
		}
	    }
	    
	    /*************************************************************/
	    /***************** process the data read *********************/
	    /*************************************************************/
	    
	    static bool gotStatusByte1 = false ;
	    static bool gotStatusByte2 = false ;
	    static int byteNum = 1 ;
	    static unsigned char button[3] = {0,0,0} ;
	    char bit ;
	    static signed char dx, dy ;
	    
	    for(unsigned char *ptr=read_buffer ;ptr < read_buffer+bytes_read ;ptr++) {
		if (!gotStatusByte1 && !gotStatusByte2 && *ptr==0x4D) {
		    gotStatusByte1 = true ;
		    //~printf("gotStatusByte1\n") ;
		    
		} else if (gotStatusByte1 && !gotStatusByte2 && *ptr==0x33) {
		    gotStatusByte2 = true ;
		    //~printf("gotStatusByte2\n") ;
		    
		} else if (gotStatusByte1 && gotStatusByte2) {
		    
		    if ((*ptr & 0x40) && ((byteNum == 1) || (byteNum == 4))) {
			//~printf("------------------\n") ;
			//~printf("Byte 1 = 0x%2.2x\n",*ptr) ;
			if (button[0] != (bit = (*ptr & 0x10))) {
			    button[0] = bit ;
			    button_state &= ~(0x01) ;
			    button_state |= (*ptr & 0x10)>>4 ;
			    button_state_mapped = map_buttons(button_state) ;
			    button_shm->write(&button_state_mapped);
			    //~printf("button[0] = %d\n\n\n",button[0]) ;
			}
			if (button[2] != (bit = (*ptr & 0x20))) {
			    button[2] = bit ;
			    button_state &= ~(0x04) ;
			    button_state |= (*ptr & 0x20)>>3 ;
			    button_state_mapped = map_buttons(button_state) ;
			    button_shm->write(&button_state_mapped);
			    //~printf("button[2] = %d\n\n",button[2]) ;
			}
			dx = (*ptr & 0x3)<<6 ;
			dy = (*ptr & 0xC)<<4 ;
			
			//~printf("    dx = %d\n", dx) ;
			//~printf("    dy = %d\n", dy) ;
			//~printf("    dx = %x\n", dx) ;
			//~printf("    dy = %x\n", dy) ;
			//~printf("joystick = (%+3.3f %+3.3f) buttons = 0x%2x\n", joystick_state[0], joystick_state[1], button_state) ;
			byteNum = 2 ;
			
		    } else if (!(*ptr & 0x40) && (byteNum == 2)) {
			//~printf("Byte 2 = 0x%2.2x\n",*ptr) ;
			//dx |= (*ptr & 0x3F) ;
			dx |= *ptr ;
			joystick_state[0] = -(float)dx/max_xy_offset ;
			if (joystick_state[0]>1.f)
			    joystick_state[0] = 1.f ;
			else if (joystick_state[0]<-1.f)
			    joystick_state[0] = -1.f ;
			//~printf("    dx = %d\n", dx) ;
			//~printf("    dy = %d\n", dy) ;
			//~printf("    dx = %x\n", dx) ;
			//~printf("    dy = %x\n", dy) ;
			//~printf("joystick = (%+3.3f %+3.3f) buttons = 0x%2x\n", joystick_state[0], joystick_state[1], button_state) ;
			byteNum = 3 ;
			
		    } else if (!(*ptr & 0x40) && (byteNum == 3)) {
			//~printf("Byte 3 = 0x%2.2x\n",*ptr) ;
			//dy |= (*ptr & 0x3F) ;
			dy |= *ptr ;
			joystick_state[1] = (float)dy/max_xy_offset ;
			if (joystick_state[1]>1.f)
			    joystick_state[1] = 1.f ;
			else if (joystick_state[1]<-1.f)
			    joystick_state[1] = -1.f ;
			// apply polynomial to joystick values before writing
			for (int j=0; j<2; j++)
			{
			    // how many factors
			    unsigned int s = (joystickpoly[j]).size() ;
			    if (s>0)
			    {
				// original joystick value
				float jv = joystick_state[j] ;
				// value of last polynomial factor
				joystick_state[j] = (joystickpoly[j])[s-1] ;
				// loop through polynomial factors from largest to smallest
				for (int i=s-2; i>=0; i--)
				{
				    joystick_state[j] = joystick_state[j]*jv + (joystickpoly[j])[i] ;
				    
				}
			    }
			}
			map_joystick(joystick_state, joystick_state_mapped) ;  
			joystick_shm->write(joystick_state_mapped);
			
			//~printf("    dx = %d\n", dx) ;
			//~printf("    dy = %d\n", dy) ;
			//~printf("    dx = %x\n", dx) ;
			//~printf("    dy = %x\n", dy) ;
			//~printf("joystick = (%+3.3f %+3.3f) buttons = 0x%2x\n", joystick_state[0], joystick_state[1], button_state) ;
			byteNum = 4 ;
			
		    } else if ((byteNum == 4) &&  ((*ptr == 0x20) || (*ptr == 0x00))) {
			//~printf("Byte 4 = 0x%2.2x\n",*ptr) ;
			if (button[1] != (bit = (*ptr & 0x20))) {
			    button[1] = bit ;
			    button_state &= ~(0x02) ;
			    button_state |= (*ptr & 0x20)>>4;
			    button_state_mapped = map_buttons(button_state) ;
			    button_shm->write(&button_state_mapped);
			    //~printf("button[1] = %d\n\n\n",button[1]) ;
			}
			byteNum = 1 ;
			
		    } else {
			//~printf("\n\nHuh? = 0x%2.2x\n\n\n",*ptr) ;
		    }
		}
	    } //for(unsigned char *ptr=read_buffer ;ptr < end_ptr ;ptr++)
	    
	}
	else if (fd_set_vector[i] == fifo_read)
	{
	    // read & ignore data
	    read(fifo_read, read_buffer, read_buffer_size) ;
	}
	else
	{
	dtkMsg.add(DTKMSG_WARN, 1,
		   "Wanda::serve() unknown fd = %d, ignoring\n",fd_set_vector[i]) ;
	}

	// write data to fifo so select doesn't hang
	if (timeout_timer.get() < timeout)
	{
	    write(fifo_write, &fifo_data, sizeof(fifo_data)) ;
	}
	else
	{
	    // zero joystick data
	    dtkMsg.add(DTKMSG_DEBUG, 1,
		       "Wanda::serve() zeroing joystick on serial line timeout\n") ;
	    
	    // apply polynomial to joystick values before writing
	    // since it's zero, just use 0 power of polynomial
	    for (int j=0; j<2; j++)
	    {
		// how many factors
		unsigned int s = (joystickpoly[j]).size() ;
		if (s>0)
		{
		    joystick_state[j] = (joystickpoly[j])[0] ;
		}
		else
		{
		    joystick_state[j] = 0.f ;
		}
	    }
	    map_joystick(joystick_state, joystick_state_mapped) ;  
	    joystick_shm->write(joystick_state_mapped);
	}
	
    }

    /* for VRCO (tm) trackd (tm) compatiablity */
    if (getTrackd())
    {
	int i ;
	for(i=0 ;i<TD_BUTTON_ARRAY_NUM ;i++)
	    VRCO.button[i] = (button_state & (01 << i)) >> i ;
	VRCO.val[0] = joystick_state[0] ;
	VRCO.val[1] = joystick_state[0] ;
    }
    return DTKSERVICE_CONTINUE ;
}

unsigned char  Wanda::map_buttons(unsigned char buttons)
{
    unsigned char mapped = 0 ;
    if (button_map[0]!=-1 && buttons&button_map[0])
	mapped |= 1 ;
    if (button_map[1]!=-1 && buttons&button_map[1])
	mapped |= 2 ;
    if (button_map[2]!=-1 && buttons&button_map[2])
	mapped |= 4 ;
    //printf("buttons = %x, mapped = %x\n",buttons,mapped) ;
    return mapped ;
}

void Wanda::map_joystick(float joystick[2], float mapped[2])
{
    mapped[0] = mapped[1] = 0.f ;
    if (joystick_map[0] != -1)
	mapped[0] = joystick[joystick_map[0]] ;
    if (joystick_map[1] != -1)
	mapped[1] = joystick[joystick_map[1]] ;
}

////////////////////////////////////////////////////////////////////////
// The DTK C++ loader/unloader functions

static dtkService *dtkDSO_loader(const char *arg) {
    return new Wanda( arg );
}

static int dtkDSO_unloader(dtkService *wanda) {
    delete wanda ;
    //~printf("---------------------------------------------\n") ;
    return DTKDSO_UNLOAD_CONTINUE ; // success
}
