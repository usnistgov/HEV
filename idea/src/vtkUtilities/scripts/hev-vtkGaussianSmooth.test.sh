# ! /bin/sh

# Test cases for hev-vtkGaussianSmooth

INDIR=/usr/local/HEV/idea/src/vtkUtilities/data
OUTDIR=testresults

#remove all files currently present in /testresults/ folder
rm -f $OUTDIR/*
echo 'Removed all files in testresults/ folder.'

cp $INDIR/structPoints.2.noise.vtk $OUTDIR/sP2n.vtk
echo 'Copied structPoints.2.noise.vtk into testresults/sP2n.vtk.'

# test default settings
./hev-vtkGaussianSmooth $OUTDIR/sP2n.vtk $OUTDIR/sP2n.smooth.vtk
echo 'Created sP2n.smooth.vtk.'

# Run original and smoothed version through hev-vtkContour.
# Contour of smoothed version should look smoother than original.
../vtkContourFilter/hev-vtkContour 0.2 $OUTDIR/sP2n.vtk $OUTDIR/sP2n.CF.vtk
echo 'Created sP2n.CF.vtk.'
../vtkContourFilter/hev-vtkContour 0.2 $OUTDIR/sP2n.smooth.vtk $OUTDIR/sP2n.smooth.CF.vtk
echo 'Created sP2n.smooth.CF.vtk.'

# dev 0.8 and rad 0.3:
./hev-vtkGaussianSmooth --stddev 0.8 0.8 0.8 --rfactor 0.3 0.3 0.3 $OUTDIR/sP2n.vtk $OUTDIR/sP2n.dev0.8.rad0.3.vtk
echo 'Created sP2n.dev0.8.rad0.3.vtk.'
exit
