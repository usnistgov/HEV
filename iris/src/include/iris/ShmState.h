#ifndef __IRIS_STATE__
#define __IRIS_STATE__

#include <dtk.h>
#include <osg/Matrix>
#include <osg/Transform>

namespace iris
{
    /**
       \brief The %ShmState class is used by the writeState DSO and the
       iris-readState program.  The class specifies what data should be
       stored in which DTK shared memory files.  The data contains the state
       of the running IRIS program, such as frame number, node
       transformations, navigation values, and head and wand position.
    */

    class ShmState : public osg::Referenced
    {
    public:
	
	enum DataType { MATRIX, FLOAT, INT, STRING } ;

	/**
	   \brief The %DataType structure stores the data associated with an
	   item handled by the ShmState class.  It is used as the value in a
	   std::map, with a std::string label being used as the key.
	*/

	struct DataElement
	{
	    dtkSharedMem* shm ; 
	    DataType type ;
	    osg::Matrix matrixValue ;
	    float floatValue ;  
	    int intValue ;
	    std::string stringValue ;
	} ;

	ShmState() { _init() ; } ;

	void update() ;
	std::map<std::string, DataElement>* getMap() { return &_map ; } ;

	/**
	   \brief returns a pointer to a dtk shared memory file with the
	   specified type and label.

	   \returns NULL if the specified state item can't be found

	 */
	dtkSharedMem* getShm(DataType type, std::string label) ;

	static const std::string shmPrefix ;
	static const int maxStringValueSize ;

    private:
	std::map<std::string, DataElement> _map ;

	void _init() ;

	void _initTransformNode(DataElement *d, std::string shmName) ;
	void _updateTransformNode(DataElement *d, MatrixTransform* node) ;

	void _initFloat(DataElement *d, std::string shmName) ;
	void _updateFloat(DataElement *d, float shmData) ;

	void _initInt(DataElement *d, std::string shmName) ;
	void _updateInt(DataElement *d, int shmData) ;

	void _initString(DataElement *d, std::string shmName) ;
	void _updateString(DataElement *d, const std::string& shmData) ;

	DataElement _head ;
	
	DataElement _wand ;
	
	DataElement _world ;
	
	DataElement _nav ;

	DataElement _response ;

	DataElement _refTime ;

	DataElement _frameNumber ;

	DataElement _frameRate ;
	double _lastReferenceTime ;

	DataElement _navName ;

	DataElement _numNavs ;


    } ;
}

#endif
