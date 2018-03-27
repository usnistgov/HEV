// create a single tristrip that draws an approxiation to a sphere- uses
// degenerate triangles to avoid breaking the strip- see
// http://in4k.untergrund.net/html_articles/hugi_27_-_coding_corner_polaris_sph
// for gory details

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// n=3, 3 bands, 24 points, 22 triangles, 4 degenerate
// n=4, 7 bands, 112 points, 110 triangles, 12 degenerate
// n=5, 15 bands, 480 points, 478 triangles, 28 degenerate
// n=6, 31 bands, 1984 points, 1982 triangles, 60 degenerate
// n=7, 63 bands, 8064 points, 8062 triangles, 124 degenerate
// n=8, 127 bands, 32512 points, 32510 triangles, 252 degenerate
// n=9, 255 bands, 130560 points, 130558 triangles, 508 degenerate
// n=10, 511 bands, 523264 points, 523262 triangles, 1020 degenerate


void usage()
{
    fprintf(stderr,"hev-savgSphere [val]\n") ;
}

int main(int argc, char* argv[])
{
    

    int Band_Power ;
    if (argc == 1) Band_Power = 5 ;
    else Band_Power = (int)atof(argv[1]) ; // how many bands - need at least 3

    if (argc>2 || Band_Power<3)
    {
	usage() ;
	return 1 ;
    }

    const double R = 1 ; // -radius
    int Band_Points = (int)pow(2.f, (double)Band_Power) ; // 2^Band_Power = total points in a band.
    int Band_Mask = (Band_Points-2) ;
    int Sections_In_Band = ((Band_Points/2)-1) ;
    int Total_Points = (Sections_In_Band*Band_Points) ;
    // remember - for each section in a band, we have a band
    double Section_Arc = (2.f*M_PI/(double)Sections_In_Band) ;

    printf("# n=%d, %d bands, %d points, %d triangles, %d degenerate\n",
	   Band_Power,
	   (int)pow(2.f, (double)(Band_Power-1))-1,
	   Total_Points,
	   Total_Points-2,
	   Band_Points-4) ;
    //printf("# %d bands\n",Band_Power) ;
    //printf("# %d points in a band\n",Band_Points) ;
    //printf("# %d sections in band\n",Sections_In_Band) ;
    //printf("lines\n") ;
    printf("nooptimize\ntristrips\n") ;

    for (int i=0;i<Total_Points;i++)
    {  
	// rotation about the X axis, or pitch, in "band" units, 0 -> Sections_In_Band
	// this zigzags up and down every step across the band
	// using last bit to alternate, then add band number
	double pitch=(i&1)+(i>>Band_Power) ;
	
	// spin about the Z axis, or heading
	// (i&Band_Mask)>>1 == Local Z value in the band
	// (i>>Band_Power)*((Band_Points/2)-1) == how many bands have we processed?
	// Remember - we go "right" one value for every 2 points.
	// i>>bandpower - tells us our band number
	double heading=((i&Band_Mask)>>1)+((i>>Band_Power)*(Sections_In_Band));

	//printf("#i = %3.3d, pitch = %d, heading = %d\n",i, (int)pitch,(int)heading) ;

	// convert to radians
	pitch*=(Section_Arc/2.f); // remember - 180° * rot, not 360
	heading*=(Section_Arc); 
	
	//printf("#i = %3.3d, pitch = %.17g, heading = %.17g\n",i, pitch*M_PI, heading*M_PI) ;

	double x = -R*sin(pitch)*sin(heading) ;
	double y = -R*sin(pitch)*cos(heading) ;
	double z = -R*cos(pitch) ;
	
	//printf("%.17g %.17g %.17g  %.17g %.17g %.17g 1 %.17g %.17g %.17g\n", x, y, z, 1, y*2.5, z*2.5, x, y, z) ;
	printf("%.17g %.17g %.17g %.17g %.17g %.17g\n", x, y, z, x, y, z) ;
	
    }
    
    return 0;
}
