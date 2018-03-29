#ifndef __IRIS_UTILS__
#define __IRIS_UTILS__

#include <signal.h>

#include <iostream>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <iris/Augment.h>
#include <iris/ArgumentParser.h>

#include <osgGA/GUIEventHandler>
#include <osg/Quat>
#include <osg/Matrix>
#include <osg/Transform>

namespace iris
{
    /**
       \brief erase the first word (non-whitespace) and the spaces around the word
       \return the chopped word
    */
    std::string ChopFirstWord(std::string* line) ;

    /**
       \brief erase the first word (non-whitespace) and the spaces around the word
       \return the rest of the line
    */
    std::string ChopFirstWord(const std::string& line) ;

    /**
       \brief converts a dtkCoord to an osg::Matrix
    */
    osg::Matrix CoordToMatrix(const dtkCoord& c) ;

    /**
       \brief converts the elements of a dtkCoord to an osg::Matrix
    */
    osg::Matrix CoordToMatrix(const double x, const double y, const double z, const double h, const double p, const double r) ;

    /**
       \brief a convenience function for calling osg::Matrix::decompose. Each
       pointer parameter is checked for NULL and set if the pointer is
       NULL. Use osg::Matrix::getTrans() if just getting translation. If
       you know the scale is osg::Vec3d(1.0, 1.0, 1.0) then it's OK to call
       osg::Matrix::getRotate() to just get rotation. If in doubt, call
       this function. osg::Matrix::getRotate() has a bug and won't return
       the right value if scale isn't osg::Vec3d(1.0, 1.0, 1.0).
    */
    void Decompose (const osg::Matrix &matrix, osg::Vec3d *translation, osg::Quat *rotation, osg::Vec3d *scale=NULL, osg::Quat *scaleOrientation=NULL) ;

    double DegreesToRadians(double x) ;

    /**
       \brief converts a double to a string
       \param n is set to the binary representation of the number
       \returns the formatted representation of the number
       \returns \c true if the conversion was successful
    */
    std::string DoubleToString(double n) ;    

    /**
       \brief performs environment variable substitution. The environment
       variable must be of the form ${FOO} where FOO is the environment
       variable's name. If the specified environment variable does not exist
       it is removed from the string. If ${ is found in the string without a
       terminating } the string is truncated at ${.

       \param str is the string to process

       \returns true if all environment variables exist, false otherwise
    */

    bool EnvvarReplace(std::string& str) ;

    /**
       \brief converts an Euler angle to a osg::Quat
       \param h heading in degrees
       \param p pitch in degrees
       \param r roll in degrees
    */
    osg::Quat EulerToQuat(double h, double p, double r) ;

    /**
       \brief returns a std::set of osg::Node pointers to nodes in the IRIS scenegraph with the given name.
       \param name the name of the node to find
       \param useNodeName if true, matches on <i>name</i>, otherwise \b ALL nodes in the scenegraph are returned
       \param travMode specifies how to traverse the scenegraph
    */
    std::set<osg::Node*> FindNode(const std::string& name, const bool useNodeName = true, const osg::NodeVisitor::TraversalMode travMode=osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN) ;


    /**
       \brief converts a float to a string
       \param n is set to the binary representation of the number
       \returns the formatted representation of the number
       \returns \c true if the conversion was successful
    */
    std::string FloatToString(float n) ;    

    /**
       \brief returns the bounding box of the node
    */

    osg::BoundingBox GetBoundingBox(osg::Group* group) ;

    /**
       \brief reads a line from an istream, timing out if no data is read

       \param line is the adress of a std::string to receive the data

       \param t is a timeout value in seconds and must be non-negative

       \param to is the adddress of a boolean, and if not NULL, is set to true if a timeout occurred

       \param stream is a std::istream object for reading the data

       \returns true if any data was read.

    */
    bool GetLine(std::string* line, double t=0, bool* to=NULL, std::istream& stream=std::cin) ;

#if 0
    /**
       \brief returns the osg::Matrix contained in the osg::Transform node
    */
    osg::Matrix GetMatrix(const osg::Transform* t) ;
#endif

    /**
       \brief returns the osg::Matrix that goes from the child node to the parent node
    */
    osg::Matrix GetMatrix(const osg::Group* parent, const osg::Node* child) ;

    
    /**
       \brief returns the sleep time in seconds.
    */
    double GetSleep() ;

    /**
       \brief returns the sleep time in microseconds- good for input to usleep
    */
    unsigned long GetUsleep() ;

    /**
      \brief returns the name of a non-existent file.  
      
      the file will have <i>suffix</i> appended to it, if provided
      
      If the <i>dirname</i> parameter is not supplied, the file will be in:
      - \c /dev/shm, if it exists, or if not, then 
      - \c /var/tmp, if it exists, or if not, then
      - \c /tmp, if it exists, or if not, then
      - \c $HOME, if it exists, or if not, then
      - .
      and will have the name
      \n <i>basename</i> + "-" + \c getuid() + "-" + \c getpid() + "-" + \c time(NULL)

      if <i>basename</i> is not supplied, the string \c "irisTempFile" will be used
      
      \c null is returned on error

      \warning the file is only non-existent at the time of the function call.
      It doesn't mean that you can open it or write to it.  It's also
      unlikely, but not impossible, for the file to exist by the time you go
      to use it.
      
    */
    char *GetTempFileName(const char* suffix = NULL, const char* dirname = NULL, const char* basename = "irisTempFile") ;
    
    /**
       \brief returns an X11 Window ID given a iris::Window name
    */
    ::Window GetWindowByName(::Display* dpy, const char* name) ;

    /**
       \brief converts an int to a string
       \param n is set to the binary representation of the number
       \returns the formatted representation of the number
       \returns \c true if the conversion was successful
    */
    std::string IntToString(int n) ;    

    /**
       \brief looks for the supplied string in the keyword string

       Returns \c true if <i>supplied</i> matches the first <i>minLength</i>
       characters of <i>keyword</i>, or all of the characters of
       <i>supplied</i> match <i>keyword</i> if <i>minLength</i> is zero or
       omitted.
       
    */
    bool IsSubstring(const std::string& keyword, const std::string& supplied, const unsigned int minLength = 0, bool matchCase = false) ;

    /**
       \brief loads one or more model files or DSOs

       If the file has a suffix, use OSG to load it as a model file and add
       it as a child to "node" if both the pointer to the loaded file and
       node are non-NULL.  

       If name doesn't have a suffix, load it as a DSO.

       If setName is true, the node's name will be set to the file name

       \c true is returned if all files were loaded.  

    */
    bool LoadFile(const std::string& name, osg::Group* group = NULL, bool setName = false) ;

    /**
       \brief loads one model file or DSO

       If the file has a suffix, use OSG to load it as a model file and add
       it as a child to <i>group</i> if the pointers to the loaded file and
       <i>group</i> are non-\c NULL.  

       If <i>name</i> doesn't have a suffix, load it as a DSO.

       If <i>setName</i> is \c true, the <i>group</i>'s name will be set to the <i>name</i>

       \c true is returned if the file was loaded.  
    */
    bool LoadFile(const std::vector<std::string>& names, osg::Group* group = NULL, bool setName = false) ;

    /**
       \brief loads one or more model files or DSOs

       If the file has a suffix, use OSG to load it as a model file and add
       it as a child to <i>group</i> if the pointers to the loaded file and
       <i>group</i> are non-\c NULL.  

       If <i>name</i> doesn't have a suffix, load it as a DSO.

       If <i>setName</i> is \c true, the <i>group</i>'s name will be set to the <i>name</i>

       \c true is returned if all files were loaded.  

       \note Loaded files are removed from <i>args</i>.  <i>args</i>\c [0] is skipped, as it is the
       name of the program.
    */
    bool LoadFile(ArgumentParser* args, osg::Group* group = NULL, bool setName = false) ;

    /**
       \brief loads one or more model files or DSOs

       If the file has a suffix, use OSG to load it as a model file and add
       it as a child to <i>group</i> if the pointers to the loaded file and
       <i>group</i> are non-\c NULL.  

       If <i>name</i> doesn't have a suffix, load it as a DSO.

       If <i>setName</i> is \c true, the <i>group</i>'s name will be set to the <i>name</i>

       \c true is returned if all files were loaded.  

       \note argv[0] is skipped, as it is the name of the program.
    */
    bool LoadFile(int argc, char** argv, osg::Group* group = NULL, bool setName = false) ;

    /**
       \brief converts a long to a string
       \param n is set to the binary representation of the number
       \returns the formatted representation of the number
       \returns \c true if the conversion was successful
    */
    std::string LongToString(long n) ;    

    /**
       \brief converts an osg::Matrix to a dtkCoord
    */
    dtkCoord MatrixToCoord(const osg::Matrix& m) ;

    /**
       \brief converts an osg::matrix to the elements of a dtkCoord
    */
    void MatrixToCoord(const osg::Matrix& mat,  double* x, double* y, double* z, double* h, double* p, double* r) ;

    /**
       \brief converts the rotation in an osg::Matrix to an Euler angle
       \param mat the matrix containing the rotation
       \param h sets heading in degrees
       \param p sets pitch in degrees
       \param r sets roll in degrees
    */
    void MatrixToEuler(const osg::Matrix& mat, double* h, double* p, double* r) ;

    /**
       \brief return \c true and set <i>onOff</i> to \c true if <i>string</i> is on/ye[s]/tru[e]/ena[ble]/1/act[ive]
       \n return \c true and set <i>onOff</i> to \c false if <i>string</i> is of[f]/no/fal[se]/dis[able]/0/ina[ctive]
       \n return \c false otherwise

       matching is case insensative
    */
    bool OnOff(const std::string& str, bool* onOff) ;

    /**
       \brief run the osgUtil::Optimizer on the given scenegraph node. The
       optimizations string is a space or colon separated list of
       osgUtil::Optimizer::OptimizationOptions. If optimizations is omitted
       the defaul set of optimizations will be used.

       \returns true if all optimizations are valid and node is a valid node
       pointer. returns false otherwise

     */
    bool Optimize(osg::Node* node, const std::string& optimizations = "default") ;

    /**
       \brief convert a string into separate words, sort of like a shell
       does, returning a vector of strings with one string per word.

       Text within "" or '' is treated as one parameter. The \\ character
       will escape the next character.  An unescaped # throws away the rest of
       the line.  Leading and trailing white space is ignored.
       
       Does not do error checking for terminating quotes- an eol or comment
       terminates the quote.
    */
    std::vector<std::string> ParseString(const std::string& line) ;

    /**
       \brief prints the contents of an osg::Matrix in a form suitable for
       debugging; 4 lines, each with four components,

       \param mat the matrix to print
       \param file the FILE to receive the output
    */
    void PrintMatrix(const osg::Matrix& mat, FILE* file = stderr) ;

    /**
       \brief prints a MATRIX control command using the values from the
       osg::Matrix

       \param mat the matrix to print
       \param nodeName the name of the scenegraph node
       \param file the FILE to receive the output
    */
    void PrintMatrixControlCommand(const osg::Matrix& mat, const std::string& nodeName, FILE* file = stdout) ;

    /**
       \brief converts a osg::Quat to an Euler angle
       \param q quaternion to be converted
       \param h sets heading in degrees
       \param p sets pitch in degrees
       \param r sets roll in degrees
    */
    void QuatToEuler(const osg::Quat& q, double* h, double* p, double* r) ;

    double RadiansToDegrees(double x) ;

    /**
       \brief removes an osgGA::GUIEventHandler from the osgVierer::Viewer's list

       \param eventHandler is a pointer to the event handler to be removed
    */
    void RemoveEventHandler(osgGA::GUIEventHandler* eventHandler) ;
    
    /** 
	\brief sets the default sleep time for getUsleep() and getSleep().  

	The envirornment variable $IRIS_SLEEP sets the default value, or if this isn't set, .001 is used.  
    */
    bool SetSleep(double t) ;

    /**
       \brief sets the signal catcher for IRIS's favorite set of signals
    */
    bool Signal(sighandler_t handler) ;

    /**
       \brief dumps a backtrace to STDERR
    */
    void DumpBacktrace() ;

    /**
       \brief converts a string to a double
       \param line contains the formatted representation of the number
       \param d is set to the binary representation of the number
       \returns \c true if the conversion was successful
    */
    bool StringToDouble(const std::string& line, double* d) ;

    /**
       \brief converts a string to a float
       \param line contains the formatted representation of the number
       \param f is set to the binary representation of the number
       \returns \c true if the conversion was successful
    */
    bool StringToFloat(const std::string& line, float* f) ;

    /**
       \brief converts a string to an int
       \param line contains the formatted representation of the number
       \param i is set to the binary representation of the number
       \returns \c true if the conversion was successful
    */
    bool StringToInt(const std::string& line, int* i) ;

    /**
       \brief converts a string to a long
       \param line contains the formatted representation of the number
       \param l is set to the binary representation of the number
       \returns \c true if the conversion was successful
    */
    bool StringToLong(const std::string& line, long* l) ;

    /**
       \brief converts a string to an unsigned int
       \param line contains the formatted representation of the number
       \param i is set to the binary representation of the number
       \returns \c true if the conversion was successful
    */
    bool StringToUInt(const std::string& line, unsigned int* i) ;

    /**
       \brief converts a vector of text strings into an array of 9 doubles
       representing X Y Z translation, H P R Euler angle and Sx Sy Sz
       scales.

       \param vec is a std::vector<std::string> with each string containing a number.
       \param coord is an array of 9 doubles
       \returns \c true if the conversion was successful

       The size of vec determines how the numbers in vec will be interpreted. If vec has:

       - 0 elements \c coord is set to  0,0,0 0,0,0 1,1,1

       - 3 elements \c coord is set to  x,y,z 0,0,0 1,1,1

       - 6 elements \c coord is set to  x,y,z h,p,r 1,1,1

       - 7 elements \c coord is set to  x,y,z h,p,r s,s,s

       - 9 elements \c coord is set to  x,y,z h,p,r sx,sy,sz

       - 16 elements \c coord is set to the matrix represented by the 16
         elements. The elements are in osg::Matrix order, so the translation
         values are in elements 13, 14 and 15.
       
    */

    bool VectorOfStringsToCoord(const std::vector<std::string>& vec, double coord[9]) ;

    /**
       \brief returns a vector of double-quoted strings (i.e. "foo bar")
       containing the results of running the wordexp(3) function on the
       input string. Wordexp performs a shell-like expansion of the string,
       including globbing, tilde expansion and variable expansion. Each
       character string returned by wordexp is entered as an std::string
       item of the returned std::vector.
     */
    std::vector<std::string> Wordexp(const std::string& s) ;

}

#endif
