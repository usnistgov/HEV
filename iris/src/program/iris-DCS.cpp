#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osg/Matrix>
#include <osg/Group>

#include <iris.h>

// save up the model files and transformations
enum TransformationType { TRANSLATE, SCALE, ROTATE, CENTER, BOUND, EXAMINE} ;
struct Transformation
{
    osg::Vec3f translate ;
    osg::Quat rotate ;
    osg::Vec3f scale ;
    TransformationType type ;
} ;

void usage()
{
    fprintf(stderr,"Usage: iris-DCS nodename [ -- help ] [ --translate x y z ] [ --euler h p r ] [ --quat x y z w ] [ -- scale x [ y z ] ] [ --center [ 0 0 0 ] ] [ --bound [ 1 ] ] [ --examine ] [ --load infile ... ] [infile ... ]\n") ;
}

int main(int argc, char** argv)
{
    if (argc<2)
    {
	usage() ;
	return 1 ;
    }

    // send messages to stderr
    dtkMsg.setFile(stderr) ;

    // use an ArgumentParser object to manage the program arguments.
    iris::ArgumentParser args(&argc,argv);

    // always create a scenegraph object if reading files
    // but you don't need a viewer
    iris::SceneGraph isg(false) ;

    std::vector<Transformation> transformations ;
    Transformation transformation ;
    std::vector<std::string> load ;

    bool translate = false ;
    bool scale = false ;
    bool center = false ;
    osg::Vec3 centerPosition = osg::Vec3(0.f, 0.f, 0.f);
    bool bound = false ;
    double boundRadius = 1.f ;
    bool examine = false ;
    std::string nodeName ;
    bool matrixOutput = false ;

    if (args.argc()<2)
    {
	usage() ;
	return 1 ;
    }

    nodeName = args[1] ;
    args.remove(1) ;

    printf("# iris-DCS applied the following options to create the DCS:\n") ;
    printf("#  nodeName = \"%s\"\n",nodeName.c_str()) ;

    
    int i = 1;
    while (i=args.findOption(i))
    {
	if (iris::IsSubstring("--help",args[i],4))
	{
	    usage() ;
	    return 0 ;
	}
	else if (iris::IsSubstring("--translate",args[i],4))
	{
	    transformation.type = TRANSLATE ;
	    translate = true ;
	    if (i+3>=args.argc())
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCS: not enough parameters\n") ;
		return 1 ;
	    }
	    double x, y, z ;
	    if (args.read(args[i],x,y,z))
	    {
		printf("#  translate %f %f %f\n",x,y,z) ;
		transformation.translate = osg::Vec3(x,y,z) ;
		transformations.push_back(transformation) ;
	    }
	    else
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCS: invalid translation \"%s,%s,%s\"\n",args[i+1],args[i+2],args[i+3]) ;
		return 1 ;
	    }
	}
	else if (iris::IsSubstring("--euler",args[i],4))
	{
	    transformation.type = ROTATE ;
	    if (i+3>=args.argc())
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCS: not enough parameters\n") ;
		return 1 ;
	    }
	    double h, p, r ;
	    if (args.read(args[i],h,p,r))
	    {
		printf("#  euler %f %f %f\n",h,p,r) ;
		transformation.rotate = iris::EulerToQuat(h, p, r) ;
		transformations.push_back(transformation) ;
	    }
	    else
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCS: invalid euler \"%s,%s,%s\"\n",args[i+1],args[i+2],args[i+3]) ;
		return 1 ;
	    }
	}
	else if (iris::IsSubstring("--quat",args[i],4))
	{
	    transformation.type = ROTATE ;
	    if (i+4>=args.argc())
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCS: not enough parameters\n") ;
		return 1 ;
	    }
	    double x, y, z, w ;
	    if (args.read(args[i],x,y,z,w))
	    {
		printf("#  quat %f %f %f %f\n",x,y,z,w) ;
		transformation.rotate = osg::Quat(x,y,z,w) ;
		transformations.push_back(transformation) ;
	    }
	    else
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCS: invalid quat \"%s,%s,%s,%s\"\n",args[i+1],args[i+2],args[i+3],args[i+4]) ;
		return 1 ;
	    }
	}
	else if (iris::IsSubstring("--scale",args[i],4))
	{
	    scale = true ;
	    double x, y, z ;
	    transformation.type = SCALE ;
	    // is it a uniform scale?
	    // out of params? option after one number?
	    if ((i+2)==args.argc() || ((i+2)<args.argc() && args.isOption(i+2))) 
	    {
		if (args.read(args[i],x))
		{
		    y = z = x ;
		}
		else
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris-DCS: invalid scale \"%s\"\n",args[i+1]) ;
		    return 1 ;
		}
	    }
	    else // try for non-uniform scale
	    {
		if (i+3>=args.argc())
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris-DCS: not enough parameters\n") ;
		    return 1 ;
		}
		if (!args.read(args[i],x,y,z))
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris-DCS: invalid translation \"%s,%s,%s\"\n",args[i+1],args[i+2],args[i+3]) ;
		    return 1 ;
		}
	    }
	    printf("#  scale %f %f %f\n",x,y,z) ;
	    transformation.scale = osg::Vec3(x,y,z) ;
	    transformations.push_back(transformation) ;

	}
	else if (iris::IsSubstring("--center",args[i],4))
	{
	    if (examine)
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCS: can't specify both --examine and --center\n") ;
		return 1 ;
	    }
	    if (center)
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCS: can't specify more than one --center\n") ;
		return 1 ;
	    }
	    if (translate)
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCS: can't specify --translate before --center\n") ;
		return 1 ;
	    }
	    center = true ;
	    double x, y, z ;
	    transformation.type = CENTER ;
	    // if center isn't given it's 0,0,0
	    if ((i+3)==args.argc() || ((i+1)<args.argc() && args.isOption(i+1))) 
	    {
		x = y = z = 0.f ;
		args.remove(i) ;
	    }
	    else
	    {
		if (i+3>=args.argc())
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris-DCS: not enough parameters\n") ;
		    return 1 ;
		}
		if (!args.read(args[i],x,y,z))
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris-DCS: invalid center \"%s,%s,%s\"\n",args[i+1],args[i+2],args[i+3]) ;
		    return 1 ;
		}
	    }
	    printf("#  center %f %f %f\n",x,y,z) ;
	    transformation.translate = osg::Vec3(x,y,z) ;
	    transformations.push_back(transformation) ;
	}
	else if (iris::IsSubstring("--bound",args[i],4))
	{
	    if (examine)
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCS: can't specify both --examine and --bound\n") ;
		return 1 ;
	    }

	    if (bound)
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCS: can't specify more than one --bound\n") ;
		return 1 ;
	    }
	    if (scale)
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCS: can't specify --scale before --bound\n") ;
		return 1 ;
	    }
	    bound = true ;
	    transformation.type = BOUND ;
	    double b ;
	    // if bound isn't given it's 1
	    if ((i+1)==args.argc() || ((i+1)<args.argc() && args.isOption(i+1))) 
	    {
		b = 1 ;
		args.remove(i) ;
	    }
	    else
	    {
		if (!args.read(args[i],b))
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris-DCS: invalid bound \"%s\"\n",args[i+1]) ;
		    return 1 ;
		}
	    }
	    transformation.scale = osg::Vec3(b,b,b) ;
	    transformations.push_back(transformation) ;
	    
	}
	else if (iris::IsSubstring("--examine",args[i],4))
	{
	    if (examine)
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCS: can't specify more than one --examine\n") ;
		return 1 ;
	    }
	    if (translate || scale)
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCS: can't specify --translate or --scale before --examine\n") ;
		return 1 ;
	    }
	    if (bound || center)
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCS: can't specify --bound or --center with --examine\n") ;
		return 1 ;
	    }
	    examine = true ;
	    printf("#  examine\n") ;
	    transformation.type = EXAMINE ;
	    transformations.push_back(transformation) ;
	    args.remove(i) ;
	}
	else if (iris::IsSubstring("--matrix",args[i],4))
	{
	    matrixOutput = true ;
	    args.remove(i) ;
	}
	else if (iris::IsSubstring("--load",args[i],4))
	{
	    if ((i+1)==args.argc())
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCS: not enough parameters\n") ;
		return 1 ;
	    }
	    // where's the next option?
	    int j = args.findOption(i+1) ;
	    if (j==0) j=argc ;
	    if ((j-i)==1)
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCS: not enough parameters\n") ;
		return 1 ;
	    }
	    for (int k=i+1; k<j; k++)
	    {
		load.push_back(args[k]) ;
	    }
	    args.remove(i,j-i) ;
	}
	else
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris-DCS: unknown option \"%s\"\n",args[i]) ;
	    return 1 ;
	}
    }

    // add the model files at the end of the line
    while(argc>1)
    {
	load.push_back(args[1]) ;
	args.remove(i) ;
    }

    osg::ref_ptr<osg::Group> group = new osg::Group ;

    // store an array of node names, using the file name for the node name if the node doesn't have a name
    std::vector<std::string> childNames ;
    // read in the files
    if (load.size()>0) printf("# and loaded the following files:\n") ;
    for (int i=0; i<load.size(); i++)
    {
	osg::Node* child = osgDB::readNodeFile(load[i]) ;
	if (!child) 
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris-DCS: can't load model file \"%s\"\n",load[i].c_str()) ;
	    return 1 ;
	}
	group->addChild(child) ;
	if (child->getName() != "") childNames.push_back(child->getName()) ;
	else childNames.push_back(load[i]) ;
	printf("#  \"%s\" with node name \"%s\"\n",load[i].c_str(),childNames[i].c_str()) ;
    }

    // now compute bound && center && examine
    if ((bound || center || examine) && load.size() == 0)
    {
	dtkMsg.add(DTKMSG_ERROR, "iris-DCS: can't specify --center, --bound or --examine without also loading model files\n",args[i]) ;
	return 1 ;
    }

    // generate a matrix with the compounded transformation
    osg::Matrix mat ;
    osg::BoundingSphere s ;
    for (int i=0; i<transformations.size(); i++)
    {
	//TRANSLATE, SCALE, ROTATE, CENTER, BOUND, EXAMINE, IRIS
	if (transformations[i].type == TRANSLATE)
	{
	    mat.postMult(osg::Matrix::translate(transformations[i].translate)) ;
	}
	else if (transformations[i].type == ROTATE)
	{
	    mat.postMult(osg::Matrix::rotate(transformations[i].rotate)) ;
	}
	else if (transformations[i].type == SCALE)
	{
	    mat.postMult(osg::Matrix::scale(transformations[i].scale)) ;
	}
	else if (transformations[i].type == CENTER)
	{
	    s = group->getBound() ;
	    printf("#center: radius = %f, center = %f %f %f\n",s.radius(), s.center().x(), s.center().y(), s.center().z()) ;
	    mat.postMult(osg::Matrix::translate(transformations[i].translate - s.center())) ;
	}
	else if (transformations[i].type == BOUND)
	{
	    s = group->getBound() ;
	    printf("#bound: radius = %f, center = %f %f %f\n",s.radius(), s.center().x(), s.center().y(), s.center().z()) ;
	    mat.postMult(osg::Matrix::scale(transformations[i].scale/s.radius())) ;
	}
	else if (transformations[i].type == EXAMINE)
	{
	    s = group->getBound() ;
	    printf("#examine: radius = %f, center = %f %f %f\n",s.radius(), s.center().x(), s.center().y(), s.center().z()) ;
	    // lifted from SceneGraph::examine

	    // where its center will initially be located, in normalized units
	    const osg::Vec3 examineCenter(0.f,2.f,0.f) ;
	    
	    // the radius of a cube -1 to 1 in normalized units
	    // will be co-planar with toggleObjects cube
	    //const double examineBound = sqrt(3.f) ;
	    // just give it a bounding sphere of 1
	    const double examineBound = 1.f ;
	    
	    // the units in the matrix are in the cordinate system of the parent node, so the
	    // arguments are in normalized coordinates
	    
	    double examineScale = examineBound/s.radius() ;
	    osg::Vec3 examineOffset = examineCenter - s.center()*examineScale ;

	    // make big objects smaller and larger objects smaller so they will fit in the window
	    mat.postMult(osg::Matrix::scale(osg::Vec3(examineScale,examineScale,examineScale))) ;

	    // need to take into account the size/center of the object and where you want it to go
	    // examineOffset is in normalized coordinates, so scale s.center() to make it smaller
	    // if object is big, bigger if object is small
	    mat.postMult(osg::Matrix::translate(examineOffset)) ;

	}
    }

    if (matrixOutput)
    {
	iris::PrintMatrixControlCommand(mat, nodeName) ;
    }
    else 
    {
	// get the DCS out of the matrix
	osg::Vec3d translation ;
	osg::Quat quat ;
	osg::Vec3d scale ;
	iris::Decompose(mat, &translation, &quat, &scale) ;
	double h, p, r ;
	iris::QuatToEuler(quat, &h, &p, &r) ;
	
	printf("DCS \"%s\" ",nodeName.c_str()) ;
	// non-uniform scale
	if (scale.x() != scale.y() || scale.x() != scale.z() || scale.y() != scale.z()) printf("%.17g %.17g %.17g  %.17g %.17g %.17g  %.17g %.17g %.17g\n",translation.x(), translation.y(), translation.z(), h, p, r, scale.x(), scale.y(), scale.z()) ;
	else if (scale.x() != 1.f) printf("%.17g %.17g %.17g  %.17g %.17g %.17g  %.17g",translation.x(), translation.y(), translation.z(), h, p, r, scale.x()) ;
	else if (quat != osg::Quat(0.f, 0.f, 0.f, 1.f)) printf("%.17g %.17g %.17g  %.17g %.17g %.17g",translation.x(), translation.y(), translation.z(), h, p, r) ;
	else if (translation != osg::Vec3(0.f, 0.f, 0.f))  printf("%.17g %.17g %.17g",translation.x(), translation.y(), translation.z()) ;
	printf("\n") ;
    }



    // and send out the LOADCHILD/ADDCHILD commands
    for (int i=0; i<childNames.size(); i++)
    {
	printf("LOAD \"%s\" \"%s\"\nADDCHILD \"%s\" \"%s\"\n",childNames[i].c_str(),load[i].c_str(),childNames[i].c_str(),nodeName.c_str()) ;
    }

    printf("RETURN \"%s\"\n",nodeName.c_str()) ;

    return 0 ;
}
