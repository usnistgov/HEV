// reads in point data from stdin and creates a binary point file
// each line of stdin should contain three numbers representing an XYZ position
#include <stdio.h>
#include <osg/Vec3>
#include <iris.h>
int main(int argc, char** argv)
{
    if (argc != 2)
    {
	fprintf(stderr, "Usage: exampleBinaryPoints filename\n") ;
	return 1 ;
    }

    FILE* pointsFile = fopen(argv[1],"w");
    if (!pointsFile)
    {
	dtkMsg.add(DTKMSG_ERROR, "exampleBinaryPoints: can't open file \"%s\"\n",argv[1]) ;
	return 1 ;
    }

    // read the points into a vector
    std::string line ;
    std::vector<std::string> words ;
    double* points = NULL ;
    unsigned int numPoints = 0 ;
    unsigned int pointsSize ;
    while (iris::GetLine(&line))
    {
	words = iris::ParseString(line) ;
	if (words.size() != 3)
	{
	    dtkMsg.add(DTKMSG_ERROR, "exampleBinaryPoints: invalid line \"%s\"\n",line.c_str()) ;
	    unlink(argv[1]) ;
	    return 1 ;
	}
	double point[3] ;
	if (!iris::StringToDouble(words[0], point) || !iris::StringToDouble(words[1], point+1) || !iris::StringToDouble(words[2], point+2))
	{
	    dtkMsg.add(DTKMSG_ERROR, "exampleBinaryPoints: invalid number in line \"%s\"\n",line.c_str()) ;
	    unlink(argv[1]) ;
	    return 1 ;
	}
	pointsSize = 3*sizeof(double)*(numPoints+1) ;
	points = (double*) realloc(points, pointsSize) ;
	*(points+(3*numPoints)) = point[0] ;
	*(points+(3*numPoints)+1) = point[1] ;
	*(points+(3*numPoints)+2) = point[2] ;
	numPoints++ ;
    }
    
    if (fwrite(&numPoints,sizeof(unsigned int),1,pointsFile) != 1)
    {
	dtkMsg.add(DTKMSG_ERROR, "exampleBinaryPoints: error writing count to file \"%s\"\n",argv[1]) ;
	unlink(argv[1]) ;
	return 1 ;
    }
    if (fwrite(points,pointsSize,1,pointsFile) != 1)
    {
	dtkMsg.add(DTKMSG_ERROR, "exampleBinaryPoints: error writing points to file \"%s\"\n",argv[1]) ;
	unlink(argv[1]) ;
	return 1 ;
    }
    fclose(pointsFile);
    return 0 ;
}
