#include <osgDB/ReadFile>

#include <iris/SceneGraph.h>
#include <iris/Utils.h>
#include <iris/ImmersivePane.h>

////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{

    // send messages to stderr
    dtkMsg.setFile(stderr) ;

    // always create a scenegraph object if reading files
    // but you don't need a viewer
    iris::SceneGraph isg(false) ;

    // use an ArgumentParser object to manage the program arguments.
    iris::ArgumentParser args(&argc,argv);

    // load DSOs
    iris::LoadFile(&args) ;
    
    struct 
    {
	osg::Vec2 extent ;
	osg::Vec3 center ;
	osg::Quat orientation ;
    } pane ;

    std::vector<osg::ref_ptr<iris::Pane> > pl = iris::Pane::getPaneList() ;
    if (pl.size()==0)
    {
	dtkMsg.add(DTKMSG_ERROR,"iris-immersivePaneBorders: no immersive pane found\n") ;
	return 1 ;
    }

    printf("GROUP iris-immersivePaneBorders\nADDCHILD iris-immersivePaneBorders scene\n") ;
    printf("LOAD iris-immersivePaneBordersBox border.osg\n") ;

    for (unsigned int i=0; i<pl.size(); i++)
    {
	iris::ImmersivePane* immersivePane = pl[i].get()->asImmersive() ;
	if (immersivePane)
	{
	    pane.extent = immersivePane->getExtent() ;
	    pane.center = immersivePane->getCenter() ;
	    pane.orientation = immersivePane->getOrientation() ;
#if 0
	    fprintf(stderr,"pane: in window %s\n",immersivePane->getWindow()->getName()) ;
	    fprintf(stderr,"     extent = %f, %f\n",pane.extent.x(),pane.extent.y()) ;
	    fprintf(stderr,"     center = %f, %f %f\n",pane.center.x(),pane.center.y(),pane.center.z()) ;
	    fprintf(stderr,"     orientation = %f, %f %f %f\n",pane.orientation.x(),pane.orientation.y(),pane.orientation.z(),pane.orientation.w()) ;

	    osg::Vec3 min, max ;
	    min.x() = pane.center.x() - pane.extent.x()/2.f ;
	    max.x() = pane.center.x() + pane.extent.x()/2.f ;

	    min.y() = max.y() = pane.extent.y() ;

	    min.z() = pane.center.z() - pane.extent.y()/2.f ;
	    max.z() = pane.center.z() + pane.extent.y()/2.f ;

	    osg::Vec3 rmin = pane.orientation * min ;
	    osg::Vec3 rmax = pane.orientation * max ;

	    fprintf(stderr,"     min = %f %f %f\n",min.x(), min.y(), min.z()) ;
	    fprintf(stderr,"     max = %f %f %f\n",max.x(), max.y(), max.z()) ;

	    fprintf(stderr,"     rmin = %f %f %f\n",rmin.x(), rmin.y(), rmin.z()) ;
	    fprintf(stderr,"     rmax = %f %f %f\n",rmax.x(), rmax.y(), rmax.z()) ;
#endif
	    std::string scs ;
	    scs = "iris-immersivePaneBordersSCS-" + iris::IntToString(i);
	    printf("QSCS %s %f %f %f  %f %f %f %f %f %f %f\nADDCHILD iris-immersivePaneBordersBox %s\nADDCHILD %s iris-immersivePaneBorders\n",
		   scs.c_str(), pane.center.x(),pane.center.y(),pane.center.z(),
		   pane.orientation.x(), pane.orientation.y(),pane.orientation.z(), pane.orientation.w(),
		   pane.extent.x(), 1.f, pane.extent.y(), scs.c_str(), scs.c_str()) ;
	}
    }
    fflush(stdout) ;

    return 0 ;
}


