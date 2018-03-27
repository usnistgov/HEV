// reads a savg file and transforms the data by the indicated translation, rotation and scale

#include <stdio.h>
#include <iostream>
#include <string>
#include <osg/Matrix>
#include <osg/Quat>
#include <osg/Vec3>
#include <iris.h>

// function templates
void usage() ;
void bomb(std::string line, unsigned int i) ;
unsigned int isDataLine(std::string line, unsigned int i, std::vector<std::string> *vec,
			double *x, double *y, double *z, 
			double *r, double *g, double *b, double *a, 
			double *xn, double *yn, double *zn) ;
bool parseArgs(int argc, char **argv, osg::Vec3d *euler, osg::Quat *attitude, osg::Vec3d *position, osg::Vec3d *scale) ;

osg::Matrix makeTransformation(osg::Quat attitude, osg::Vec3d position, osg::Vec3d scale) ;

int main (int argc, char **argv) {
    
    osg::Quat attitude ;
    osg::Vec3d euler ;
    osg::Vec3d position ;
    osg::Vec3d scale ;

    if (!parseArgs(argc, argv, &euler, &attitude, &position, &scale))
    {
	usage() ;
	return 1 ;
    }

    osg::Matrix transformation = makeTransformation(attitude, position, scale) ;

    // line number counter
    unsigned int linenum = 0 ;
    // a line from stdin
    std::string line ;
    // the line busted into words
    std::vector<std::string> vec ;
    // elements from a data line
    double x, y, z, r, g, b, a, xn, yn, zn ;

    // a transformed point
    osg::Matrix t ;

    while (getline(std::cin, line)){

	linenum++ ;

	unsigned int n = isDataLine(line, linenum, &vec, &x, &y, &z, &r, &g, &b, &a, &xn, &yn, &zn) ;
	if (vec.size()==0) continue ;

	//~fprintf(stderr, "n=%d, vec.size() = %d, vec[0] = \"%s\"\n",n,vec.size(),vec[0].c_str()) ;

	// check for sneaky hev-savgTransformation command
	if (iris::IsSubstring("hev-savgTransformation",vec[0]))
	{
	    int argc = vec.size() ;
	    char **argv = (char**) malloc(argc*sizeof(char*));
	    for (int i=0; i<argc; i++)
	    {
		argv[i] = strdup(vec[i].c_str()) ;
	    }
	    if (!parseArgs(argc, argv, &euler, &attitude, &position, &scale))
	    {
		usage() ;
		return 1 ;
	    }
	    transformation = makeTransformation(attitude, position, scale) ;
	    for (int i=0; i<argc; i++)
	    {
		free(argv[i]) ;
	    }
	    free(argv) ;
	    continue ;
	}
	
	// do special junk for text command
	if (n==0 && iris::IsSubstring("text",vec[0]))
	{
	    double data[3];
	    for (unsigned int i=0; i<vec.size();)
	    {
		if (iris::IsSubstring("string",vec[i]) && i+1<vec.size())
		{
		    printf("%s \"%s\"",vec[i].c_str(), vec[i+1].c_str()) ;
		    i++ ;
		}
		else if (iris::IsSubstring("xyz",vec[i]) && i+3<vec.size() &&
			 iris::StringToDouble(vec[i+1], data) &&
			 iris::StringToDouble(vec[i+2], data+1) &&
			 iris::StringToDouble(vec[i+3], data+2))
		{
		    printf("%s %.17g %.17g %.17g",vec[i].c_str(), position.x()+data[0], position.y()+data[1], position.z()+data[2]) ;
		    i+=3 ;
		}
		else if (iris::IsSubstring("hpr",vec[i]) && i+3<vec.size() &&
			 iris::StringToDouble(vec[i+1], data) &&
			 iris::StringToDouble(vec[i+2], data+1) &&
			 iris::StringToDouble(vec[i+3], data+2))
		{
		    osg::Matrix m = osg::Matrix::rotate(iris::EulerToQuat(data[0],data[1],data[2])) ;
		    m.postMultRotate(iris::EulerToQuat(euler[0], euler[1], euler[2])) ;

		    double h, p, r ;
		    iris::QuatToEuler(m.getRotate(), &h, &p, &r) ;
		    printf("%s %.17g %.17g %.17g",vec[i].c_str(), h, p, r) ;
		    i+=3 ;
		}
		else if (iris::IsSubstring("sxyz",vec[i]) && i+3<vec.size() &&
			 iris::StringToDouble(vec[i+1], data) &&
			 iris::StringToDouble(vec[i+2], data+1) &&
			 iris::StringToDouble(vec[i+3], data+2))
		{
		    printf("%s %.17g %.17g %.17g",vec[i].c_str(), scale.x()*data[0], scale.y()*data[1], scale.z()*data[2]) ;
		    i+=3 ;
		}
		else if (iris::IsSubstring("s",vec[i]) && i+1<vec.size() &&
			 iris::StringToDouble(vec[i+1], data))
		{
		    if (scale.x() != scale.y() || scale.x() != scale.z() || scale.x() != scale.z()) printf("SXYZ %.17g %.17g %.17g", scale.x()*data[0], scale.y()*data[0], scale.z()*data[0]) ;
		    else printf("%s %.17g",vec[i].c_str(), scale.x()*data[0]) ;
		    i++ ;
		}
		else  printf("%s",vec[i].c_str()) ;

		if (i<vec.size()-1) printf(" ") ;
		else printf("\n") ;
		i++ ;
	    }
	    continue ;
	}

	// just spew it back out if it isn't a data line
	if (n==0)
	{
	    printf("%s\n",line.c_str()) ;
	    continue ;
	}

	// process a data line

	// always do xyz
	t.makeTranslate(osg::Vec3d(x, y, z)) ;
	t.postMult(transformation) ;
	// you might have to use p.decompose if scaling!
	osg::Vec3d p = t.getTrans() ;

	unsigned int v ;
	if (n==3 || n==7) //xyz or xyzrgb, but just copy the rgb like the other words
	{
	    printf("%.17g %.17g %.17g",p.x(),p.y(),p.z()) ;
	    if (vec.size()>3)
	    {
		for (v=3; v<vec.size(); v++) printf(" %s",vec[v].c_str()) ;
	    }
	}
	else if (n==6 || n==10) //need to calc normal rotation too
	{
	    // I stole this from savg-scale
	    xn *= scale.y() * scale.z() ;
	    yn *= scale.x() * scale.z() ;
	    zn *= scale.x() * scale.y() ;
	    t.makeTranslate(osg::Vec3d(xn, yn, zn)) ;
	    t.postMultRotate(attitude) ;
	    osg::Vec3d np = t.getTrans() ;
	    np.normalize() ;
	    if (n==6)
	    {
		printf("%.17g %.17g %.17g %.17g %.17g %.17g",p.x(),p.y(),p.z(),np.x(),np.y(),np.z()) ;
		if (vec.size()>6)
		{
		    for (v=6; v<vec.size(); v++) printf(" %s",vec[v].c_str()) ;
		}
	    }
	    else
	    {
		printf("%.17g %.17g %.17g %s %s %s %s %.17g %.17g %.17g",p.x(),p.y(),p.z(),vec[3].c_str(),vec[4].c_str(),vec[5].c_str(),vec[6].c_str(),np.x(),np.y(),np.z()) ;
		if (vec.size()>10)
		{
		    for (v=10; v<vec.size()-1; v++) printf(" %s",vec[v].c_str()) ;
		}
	    }
	}
	printf("\n") ;
    }
    return 0 ;
}

////////////////////////////////////////////////////////////////////////
void usage()
{
    fprintf(stderr,"Usage: hev-savgTransformation [ --hpr H P R ] [ --quat Xq Yq Zq Wq ] [ --scale Xs Ys Zs] [ --translate | --xyz Xt Yt Zt ]\n") ;
}

////////////////////////////////////////////////////////////////////////
void bomb(std::string line, unsigned int linenum)
{
    fprintf(stderr,"hev-savgTransformation: line %d, \"%s\", unknown format\n",linenum,line.c_str()) ;
}

////////////////////////////////////////////////////////////////////////
// returns the data from a data line
// line is parsed and put into words in vec
// i is the line number for error messages
// 0 is returned if it isn't a data line
// 3 is returned is the data contains x y z
// 6 is returned is the data contains x y z xn yn zn
// 7 is returned is the data contains x y z r g b a
// 10 is returned is the data contains x y z r g b a xn yn zn
// to save extra parsing, x, y, z, r, g, b, a, xn, yn and zn are set as appropriate
// to save time I don't test for NULL for x, y, z, r, g, b, a, xn, yn
unsigned int isDataLine(std::string line, unsigned int linenum, std::vector<std::string> *vec,
			double *x, double *y, double *z, 
			double *r, double *g, double *b, double *a, 
			double *xn, double *yn, double *zn)
{
    *vec = iris::ParseString(line) ;

    unsigned int origVecSize = (*vec).size() ;

    if ((*vec).size() == 0) return 0 ;

    // does the line contain a comment?
    // if so tack it onto the end of the vector
    std::string::size_type commentIdx = line.find("#") ;
    if (commentIdx != std::string::npos)
    {
	std::string comment = line.substr(commentIdx) ;
	(*vec).push_back(comment) ;
	//~fprintf(stderr,"comment: \"%s\"\n",comment.c_str()) ;
    }
	
    //~fprintf(stderr,"%d args, first is \"%s\"\n",(*vec).size(),(*vec)[0].c_str()) ;
    
    // first see if it has at least 3 words and there's a number in the first word
    if ((*vec).size()>=3 && iris::StringToDouble((*vec)[0], x))
    {
	// get y and z- every data line has 'em
	if (!iris::StringToDouble((*vec)[1], y) ||
	    !iris::StringToDouble((*vec)[2], z)) 
	{
	    bomb(line, linenum) ;
	    return 0 ;
	}
	
	// does it contain the keyword "attribute" ?
	unsigned int attrIdx ;
	for (attrIdx=1; attrIdx<(*vec).size(); attrIdx++)
	{
	    //~fprintf(stderr,"\"%s\"\n",(*vec)[attrIdx].c_str()) ;
	    if (iris::IsSubstring("attribute",(*vec)[attrIdx])) break ;
	}
	//~fprintf(stderr, "origVecSize = %d, attrIdx = %d\n",origVecSize,attrIdx) ;

	// just x y z?
	if (origVecSize == 3 || attrIdx == 3) return 3 ;

	// how about x y z xn yn zn?
	if (origVecSize == 6 || attrIdx == 6)
	{
	    if (iris::StringToDouble((*vec)[3], xn) && 
		iris::StringToDouble((*vec)[4], yn) &&
		iris::StringToDouble((*vec)[5], zn)) return 6 ;
	    else
	    {
		bomb(line, linenum) ;
		return 0 ;
	    }
	}
	
	// how about x y z r g b a?
	if (origVecSize == 7 || attrIdx == 7)
	{
	    if (iris::StringToDouble((*vec)[3], r) && 
		iris::StringToDouble((*vec)[4], g) &&
		iris::StringToDouble((*vec)[5], b) &&
		iris::StringToDouble((*vec)[6], a)) return 7 ;
	    else
	    {
		bomb(line, linenum) ;
		return 0 ;
	    }
	}
	
	// how about x y z r g b a xn yn zn?
	if (origVecSize == 10 || attrIdx == 10)
	{
	    if (iris::StringToDouble((*vec)[3], r) && 
		iris::StringToDouble((*vec)[4], g) &&
		iris::StringToDouble((*vec)[5], b) &&
		iris::StringToDouble((*vec)[6], a) &&
		iris::StringToDouble((*vec)[7], xn) &&
		iris::StringToDouble((*vec)[8], yn) &&
		iris::StringToDouble((*vec)[9], zn)) return 10 ;
	    else
	    {
		bomb(line, linenum) ;
		return 0 ;
	    }
	}
	return 0 ;
    }
    
    else return 0 ;
}

////////////////////////////////////////////////////////////////////////
// pass in argc/argv parameters just like from exec
// parse and set attitude, position, scale
// return true if no errors encountered
bool parseArgs(int argc, char **argv, osg::Vec3d *euler, osg::Quat *attitude, osg::Vec3d *position, osg::Vec3d *scale)
{

    *attitude = osg::Quat(0,0,0,1) ;
    *euler = osg::Vec3d(0,0,0) ;
    *position = osg::Vec3d(0,0,0) ;
    *scale = osg::Vec3d(1,1,1) ;

    std::string command = "# the data below were processed by the command \"hev-savgTransformation" ;

    double data[4] ;
    for (int i=1; i<argc ;) 
    {
	//~fprintf(stderr,"argv[%d]=%s\n",i,argv[i]) ;
	command += " " + std::string(argv[i]) ;
	if (iris::IsSubstring("--hpr",argv[i]))
	{
	    i++ ;
	    if (i+3>argc)
	    {
		return false ;
	    }
	    if (!iris::StringToDouble(argv[i], data) ||
		!iris::StringToDouble(argv[i+1], data+1) ||
		!iris::StringToDouble(argv[i+2], data+2))
	    {
		return false ;
	    }
	    (*euler)[0] = data[0] ;
	    (*euler)[1] = data[1] ;
	    (*euler)[2] = data[2] ;
	    *attitude = iris::EulerToQuat(data[0],data[1],data[2]) ;
	    command += " " + std::string(argv[i]) + " " + std::string(argv[i+1]) + " " + std::string(argv[i+2]) ;
	    i+=3 ;
	}
	else if (iris::IsSubstring("--scale",argv[i]))
	{
	    i++ ;
	    if (i+3>argc)
	    {
		return false ;
	    }
	    if (!iris::StringToDouble(argv[i], data) ||
		!iris::StringToDouble(argv[i+1], data+1) ||
		!iris::StringToDouble(argv[i+2], data+2))
	    {
		return false ;
	    }
	    *scale = osg::Vec3d(data[0],data[1],data[2]) ;
	    command += " " + std::string(argv[i]) + " " + std::string(argv[i+1]) + " " + std::string(argv[i+2]) ;
	    i+=3 ;
	}
	else if (iris::IsSubstring("--quat",argv[i]))
	{
	    i++ ;
	    if (i+4>argc)
	    {
		return false ;
	    }
	    if (!iris::StringToDouble(argv[i], data) ||
		!iris::StringToDouble(argv[i+1], data+1) ||
		!iris::StringToDouble(argv[i+2], data+2) ||
		!iris::StringToDouble(argv[i+3], data+3))
	    {
		return false ;
	    }
	    *attitude = osg::Quat(data[0],data[1],data[2],data[3]) ;
	    command += " " + std::string(argv[i]) + " " + std::string(argv[i+1]) + " " + std::string(argv[i+2]) + " " + std::string(argv[i+3]) ;
	    i+=4 ;
	}
	else if (iris::IsSubstring("--translate",argv[i]) || iris::IsSubstring("--xyz",argv[i]))
	{
	    i++ ;
	    if (i+3>argc)
	    {
		return false ;
	    }
	    if (!iris::StringToDouble(argv[i], data) ||
		!iris::StringToDouble(argv[i+1], data+1) ||
		!iris::StringToDouble(argv[i+2], data+2))
	    {
		return false ;
	    }
	    *position = osg::Vec3d(data[0],data[1],data[2]) ;
	    command += " " + std::string(argv[i]) + " " + std::string(argv[i+1]) + " " + std::string(argv[i+2]) ;
	    i+=3 ;
	}
	else
	{
	    return false ;
	}
    }
    // print a header at the top of the file
    printf("%s\"\n",command.c_str()) ;
    
    return true ;
}

////////////////////////////////////////////////////////////////////////
osg::Matrix makeTransformation(osg::Quat attitude, osg::Vec3d position, osg::Vec3d scale)
{
    osg::Matrix transformation ;
    //~fprintf(stderr, "attitude = %.17g %.17g %.17g %.17g\n",attitude.x(), attitude.y(), attitude.z(), attitude.w()) ;
    //~fprintf(stderr, "position = %.17g %.17g %.17g\n",position.x(), position.y(), position.z()) ;
    //~fprintf(stderr, "scale = %.17g %.17g %.17g\n",scale.x(), scale.y(), scale.z()) ;
#if 1
    transformation.makeRotate(attitude);
    transformation.postMultTranslate(position);
    transformation.preMultScale(scale);
#else
    transformation.preMultTranslate(position);
    transformation.preMultRotate(attitude);
    transformation.preMultScale(scale);
#endif


#if 0  
    // this dies if scale is non-uniform
    osg::Matrix m = DOSGUtil::getMatrix(transformation) ;
    m.print(stderr) ;
#endif

    return transformation ;
}
