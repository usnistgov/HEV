# ! /bin/sh

# Test cases for hev-vtkGradient

INDIR=/usr/local/HEV/idea/src/vtkUtilities/data
OUTDIR=testresults

# remove all files currently present in testresults/ folder
rm -f $OUTDIR/*
echo 'Removed all files in testresults/ folder.'

# 
./hev-vtkGradient $INDIR/f432.vtk $OUTDIR/f432.grad.vtk
echo 'Created f432.grad.vtk.'

#
./hev-vtkGradient $INDIR/structPoints.1.vtk $OUTDIR/sP1.grad.vtk
echo 'Created sP1.grad.vtk.'

# Should get vectors that point away from the (0,0,0) position in the
# volume of data (although the positions on the boundary of the array
# will not exactly do this).
./hev-vtkGradient $INDIR/sphData.sp.1.vtk $OUTDIR/sphData.sp.1.grad.vtk
echo 'Created sphData.sp.1.grad.vtk.'

exit
