#include <stdio.h>
#include <stdlib.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Roller.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Group.H>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <dtk.h>
#include <iris.h>

// widgets
Fl_Double_Window *main_window ;
Fl_Roller *xRoller, *yRoller, *zRoller ;
Fl_Roller *hRoller, *pRoller, *rRoller;
Fl_Roller *xsRoller, *ysRoller, *zsRoller ;
Fl_Input *xText, *yText, *zText ;
Fl_Input *hText, *pText, *rText  ;
Fl_Input *xsText, *ysText, *zsText ;
Fl_Light_Button *uniformScalingButton ;
Fl_Group *xLabel ;

// separate out the fltk arguments from the others
int fltk_argc;
char **fltk_argv;
int other_argc;
char **other_argv;

// true if disabling ESCAPE and the window manager close button
bool noEscape = false ;

double step = .002f ;

// true if doing uniform scaling
bool uniformScaling = false ;

// set to name of shared memory if --shm option passed
std::string transformationShmName ;
dtkSharedMem *transformationShm = NULL ;

// set to name of DCS node if --dcs option passed
std::string dcs ;

// these are in xyz hpr sx sy sy
osg::Vec3d xyz(0., 0., 0.) ;
osg::Vec3d hpr(0., 0., 0.) ;
osg::Vec3d scale(1., 1., 1.) ;

osg::Vec3d initXYZ(xyz) ;
osg::Vec3d initHPR(hpr) ;
osg::Vec3d initScale(scale) ;

// function templates
void reset() ;
void setUniformScaling(bool v) ;
void updateGUI() ;
void writeTransformation() ;
bool setup(int argc, char **argv) ;
void usage() ;
void updateTransformation() ;
void updateX(double d) ;
void updateY(double d) ;
void updateZ(double d) ;
void updateH(double d) ;
void updateP(double d) ;
void updateR(double d) ;
void updateXs(double d) ;
void updateYs(double d) ;
void updateZs(double d) ;

////////////////////////////////////////////////////////////////////////
// ignore escapes and close window buttons
void doNothingCB(Fl_Widget*, void*) { } ;

////////////////////////////////////////////////////////////////////////
void setUniformScaling(bool v)
{
    if (v!= uniformScaling)
    {
	uniformScaling = v ;
	if (uniformScaling)
	{
	    xLabel->deactivate() ;
	    ysRoller->deactivate() ;
	    zsRoller->deactivate() ;
	    ysText->deactivate() ;
	    zsText->deactivate() ;
	}
	else
	{
	    xLabel->activate() ;
	    ysRoller->activate() ;
	    zsRoller->activate() ;
	    ysText->activate() ;
	    zsText->activate() ;
	}
	updateGUI() ;
    }
    
}

////////////////////////////////////////////////////////////////////////
// reset to initial value
void reset()
{
    xyz = initXYZ ;
    hpr = initHPR ;
    scale = initScale ;
    updateGUI() ;
    writeTransformation() ;
}

////////////////////////////////////////////////////////////////////////
void writeTransformation()
{
    osg::Matrix m ;
    m.preMultTranslate(xyz) ;
    m.preMultRotate(iris::EulerToQuat(hpr[0],hpr[1],hpr[2])) ;
    m.preMultScale(scale) ;

    if (transformationShm) transformationShm->write(m.ptr()) ;

    if (dcs != "") 
    {
	printf("MATRIX %s %f %f %f %f  %f %f %f %f  %f %f %f %f  %f %f %f %f\n", dcs.c_str(),
	       m(0,0), m(0,1), m(0,2), m(0,3),
	       m(1,0), m(1,1), m(1,2), m(1,3),
	       m(2,0), m(2,1), m(2,2), m(2,3),
	       m(3,0), m(3,1), m(3,2), m(3,3)) ;
	fflush(stdout) ;
    }
	
}

////////////////////////////////////////////////////////////////////////
void usage()
{
    fprintf(stderr,"Usage: hev-cubeTransformationRollers [ --shm shmName ] [ --dcs nodename ] [ --noescape ] [ --xyz x y z ] [ --hpr h p r ] [ --scale xs ys zs ] [ --step s ] \n") ;
}

////////////////////////////////////////////////////////////////////////
void updateTransformation()
{
    writeTransformation() ;
    updateGUI() ;
    
}

////////////////////////////////////////////////////////////////////////
// update rollers and text output with world or dgl value
void updateGUI()
{
    
    xRoller->value(xyz.x()) ;
    yRoller->value(xyz.y()) ;
    zRoller->value(xyz.z()) ;
    hRoller->value(hpr[0]) ;
    pRoller->value(hpr[1]) ;
    rRoller->value(hpr[2]) ;
    xsRoller->value(scale.x()) ;
    ysRoller->value(scale.y()) ;
    zsRoller->value(scale.z()) ;
    
    char s[20] ;

    sprintf(s,"%f",xyz.x()) ;
    xText->value(s) ;
    sprintf(s,"%f",xyz.y()) ;
    yText->value(s) ;
    sprintf(s,"%f",xyz.z()) ;
    zText->value(s) ;

    sprintf(s,"%f",hpr[0]) ;
    hText->value(s) ;
    sprintf(s,"%f",hpr[1]) ;
    pText->value(s) ;
    sprintf(s,"%f",hpr[2]) ;
    rText->value(s) ;
    sprintf(s,"%f",hpr[2]) ;

    sprintf(s,"%f",scale.x()) ;
    xsText->value(s) ;
    sprintf(s,"%f",scale.y()) ;
    ysText->value(s) ;
    sprintf(s,"%f",scale.z()) ;
    zsText->value(s) ;
}


////////////////////////////////////////////////////////////////////////
void updateX(double d)
{
    xyz.x() = d ;
    updateTransformation() ;
}
////////////////////////////////////////////////////////////////////////
void updateY(double d)
{
    xyz.y() = d ;
    updateTransformation() ;
}
////////////////////////////////////////////////////////////////////////
void updateZ(double d)
{
    xyz.z() = d ;
    updateTransformation() ;
}

////////////////////////////////////////////////////////////////////////
void updateH(double d)
{
    hpr[0] = d ;
    updateTransformation() ;
}

////////////////////////////////////////////////////////////////////////
void updateP(double d)
{
    hpr[1] = d ;
    updateTransformation() ;
}
////////////////////////////////////////////////////////////////////////
void updateR(double d)
{
    hpr[2] = d ;
    updateTransformation() ;
}

////////////////////////////////////////////////////////////////////////
void updateXs(double d)
{
    if (uniformScaling)
    {
	double ratio = d/scale.x() ;
	updateYs(scale.y()*ratio) ;
	updateZs(scale.z()*ratio) ;
    }
    scale.x() = d ;
    updateTransformation() ;

}
////////////////////////////////////////////////////////////////////////
void updateYs(double d)
{
    scale.y() = d ;
    updateTransformation() ;
}
////////////////////////////////////////////////////////////////////////
void updateZs(double d)
{
    scale.z() = d ;
    updateTransformation() ;
}

////////////////////////////////////////////////////////////////////////
bool setup(int argc, char **argv)
{
    
    int i = 1 ;
    
    while (i<argc)
    {
	if (iris::IsSubstring("--noescape",argv[i],5))
	{
	    noEscape = true ;
	    //~fprintf(stderr,("setting noescape\n") ;
	    i++ ;
	}

	else if (iris::IsSubstring("--xyz",argv[i]))
	{
	    i++ ;
	    if (i+2<argc)
	    {
		if ( (!iris::StringToDouble(argv[i], &(initXYZ.x()))) || 
		     (!iris::StringToDouble(argv[i+1], &(initXYZ.y()))) || 
		     (!iris::StringToDouble(argv[i+2], &(initXYZ.z()))) )
		{
		    fprintf(stderr, "hev-cubeTransformationRollers: invalid x, y, or z parameter\n") ;
		    return false ;
		}
	    }
	    else
	    {
		fprintf(stderr, "hev-cubeTransformationRollers: not enough arguments\n") ;
		return false ;
	    }
	    i+=3 ;
	}
	
	else if (iris::IsSubstring("--hpr",argv[i]))
	{
	    i++ ;
	    if (i+2<argc)
	    {
		if ( (!iris::StringToDouble(argv[i], &(initHPR[0]))) || 
		     (!iris::StringToDouble(argv[i+1], &(initHPR[1]))) || 
		     (!iris::StringToDouble(argv[i+2], &(initHPR[2]))) )
		{
		    fprintf(stderr, "hev-cubeTransformationRollers: invalid h, p, or r parameter\n") ;
		    return false ;
		}
	    }
	    else
	    {
		fprintf(stderr, "hev-cubeTransformationRollers: not enough arguments\n") ;
		return false ;
	    }
	    i+=3 ;
	}
	
	else if (iris::IsSubstring("--scale",argv[i]))
	{
	    i++ ;
	    if (i+2<argc)
	    {
		if ( (!iris::StringToDouble(argv[i], &(initScale.x()))) || 
		     (!iris::StringToDouble(argv[i+1], &(initScale.y()))) || 
		     (!iris::StringToDouble(argv[i+2], &(initScale.z()))) )
		{
		    fprintf(stderr, "hev-cubeTransformationRollers: invalid xs, ys, or zs parameter\n") ;
		    return false ;
		}
	    }
	    else
	    {
		fprintf(stderr, "hev-cubeTransformationRollers: not enough arguments\n") ;
		return false ;
	    }
	    i+=3 ;
	}
	
	else if (iris::IsSubstring("--step",argv[i]))
	{
	    i++ ;
	    if (i<argc)
	    {
		if (!iris::StringToDouble(argv[i], &step))
		{
		    fprintf(stderr, "hev-cubeTransformationRollers: invalid step parameter\n") ;
		    return false ;
		}
	    }
	    else
	    {
		fprintf(stderr, "hev-cubeTransformationRollers: not enough arguments\n") ;
		return false ;
	    }
	    i++ ;
	}
	
	else if (iris::IsSubstring("--shm",argv[i]))
	{
	    i++ ;
	    if (i<argc)
	    {
		transformationShmName = argv[i] ;
	    }
	    else
	    {
		fprintf(stderr, "hev-cubeTransformationRollers: not enough arguments\n") ;
		return false ;
	    }
	    i++ ;
	}

	else if (iris::IsSubstring("--dcs",argv[i]))
	{
	    i++ ;
	    if (i<argc)
	    {
		dcs = argv[i] ;
	    }
	    else
	    {
		fprintf(stderr, "hev-cubeTransformationRollers: not enough arguments\n") ;
		return false ;
	    }
	    i++ ;
	}

	else if (iris::IsSubstring("--extent",argv[i]))
	{
	    i++ ;
	    if (i<argc)
	    {
		osg::Group* group = new osg::Group ;
		osg::Node* node = osgDB::readNodeFile(argv[i]) ;
		if (!node)
		{
		    fprintf(stderr, "hev-cubeTransformationRollers: can't load extent file\n") ;
		    return false ;
		}
		group->addChild(node) ;
		osg::BoundingBox bb = iris::GetBoundingBox(group);
		osg::Vec3 scaleXYZ((bb.xMax()-bb.xMin())/2.0,
				   (bb.yMax()-bb.yMin())/2.0,
				   (bb.zMax()-bb.zMin())/2.0) ;

		//printf("center = %f %f %f\n",bb.center().x(),bb.center().y(),bb.center().z()) ;
		//printf("scaleXYZ = %f %f %f\n",scaleXYZ.x(),scaleXYZ.y(),scaleXYZ.z()) ;
		initXYZ = bb.center() ;
		initScale = scaleXYZ ;
	    }
	    else
	    {
		fprintf(stderr, "hev-cubeTransformationRollers: not enough arguments\n") ;
		return false ;
	    }
	    i++ ;
	}

	else
	{
	    fprintf(stderr, "hev-cubeTransformationRollers: unknown argument\n") ;
	    return false ;
	}
	
    }

    xyz = initXYZ ;
    hpr = initHPR ;
    scale = initScale ;

    if (dcs == "" && transformationShmName == "")
    {
	fprintf(stderr, "hev-cubeTransformationRollers: neither --shm or --dcs given\n") ;
	return false ;
    }

    transformationShm = new dtkSharedMem(sizeof(osg::Matrix), transformationShmName.c_str()) ;
    if (transformationShm->isInvalid())
    {
	fprintf(stderr, "hev-cubeTransformationRollers: can't open shared memory\n") ;
	return false ;
    }
	
    writeTransformation() ;
    return true ;
}
 
