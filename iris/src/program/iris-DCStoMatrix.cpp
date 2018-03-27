#include <iris.h>

void usage()
{
    fprintf(stderr,"Usage: iris-DCStoMatrix --translate x y z --euler h p r --quat qx qy qz qw -scale s [ sy sz ]\n") ;
}

int main(int argc, char** argv)
{

    // send messages to stderr
    dtkMsg.setFile(stderr) ;

    osg::Vec3 pos(0.f, 0.f, 0.f) ;
    osg::Quat att(0.f, 0.f, 0.f, 1.0f) ;
    osg::Vec3 scale(1.f, 1.f, 1.f) ;

    // use an ArgumentParser object to manage the program arguments.
    iris::ArgumentParser args(&argc,argv);

    int i = 1;
    while (i=args.findOption(i))
    {
	if (iris::IsSubstring("--translate",args[i],4))
	{
	    if (i+3>=args.argc())
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCStoMatrix: not enough parameters\n") ;
		return 1 ;
	    }
	    if (!args.read(args[i],pos.x(),pos.y(),pos.z()))
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCStoMatrix: invalid translation \"%s,%s,%s\"\n",args[i+1],args[i+2],args[i+3]) ;
		return 1 ;
	    }
	}

	else if (iris::IsSubstring("--euler",args[i],4))
	{
	    if (i+3>=args.argc())
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCStoMatrix: not enough parameters\n") ;
		return 1 ;
	    }
	    float h, p, r ;
	    if (args.read(args[i],h,p,r))
	    {
		att = iris::EulerToQuat(h, p, r) ;
	    }
	    else
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCStoMatrix: invalid euler \"%s,%s,%s\"\n",args[i+1],args[i+2],args[i+3]) ;
		return 1 ;
	    }
	}

	else if (iris::IsSubstring("--quat",args[i],4))
	{
	    if (i+4>=args.argc())
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCStoMatrix: not enough parameters\n") ;
		return 1 ;
	    }
	    if (!args.read(args[i],att.x(),att.y(),att.z(),att.w()))
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-DCStoMatrix: invalid quat \"%s,%s,%s,%s\"\n",args[i+1],args[i+2],args[i+3],args[i+4]) ;
		return 1 ;
	    }
	}

	else if (iris::IsSubstring("--scale",args[i],4))
	{
	    // is it a uniform scale?
	    // out of params? option after one number?
	    if ((i+2)==args.argc() || ((i+2)<args.argc() && args.isOption(i+2))) 
	    {
		if (args.read(args[i],scale.x()))
		{
		    scale.y() = scale.z() = scale.x() ;
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
		    dtkMsg.add(DTKMSG_ERROR, "iris-DCStoMatrix: not enough parameters\n") ;
		    return 1 ;
		}
		if (!args.read(args[i],scale.x(),scale.y(),scale.z()))
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris-DCStoMatrix: invalid translation \"%s,%s,%s\"\n",args[i+1],args[i+2],args[i+3]) ;
		    return 1 ;
		}
	    }
	}
	else
	{
	    dtkMsg.add(DTKMSG_ERROR, "iris-DCStoMatrix: unknown option \"%s\"\n",args[i]) ;
	    return 1 ;
	}
    }

    osg::Matrix mat ;
    mat.preMultTranslate(pos);
    mat.preMultRotate(att);
    mat.preMultScale(scale);
    
    printf("%f %f %f %f  %f %f %f %f  %f %f %f %f  %f %f %f %f  \n",
	   mat(0,0),mat(0,1),mat(0,2),mat(0,3),
	   mat(1,0),mat(1,1),mat(1,2),mat(1,3),
	   mat(2,0),mat(2,1),mat(2,2),mat(2,3),
	   mat(3,0),mat(3,1),mat(3,2),mat(3,3)) ;

    return 0 ;
}
