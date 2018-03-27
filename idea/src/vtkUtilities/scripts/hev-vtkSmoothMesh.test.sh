# ! /bin/sh

# Test cases for hev-vtkSmoothMesh

INDIR=/usr/local/HEV/idea/src/vtkUtilities/data
OUTDIR=testresults

#remove all files currently present in testresults/ folder
rm -f $OUTDIR/*
echo 'Removed all files in testresults/ folder.'

# rg2.revn.vtk should give you a yellow lumpy thing. One end should
# have some sharp corners. rg2.smA.vtk should look smoother. In
# particular, the corners should look smoother.
../vtkContour/hev-vtkContour 0.2 $INDIR/rectGrid.2.noise.vtk $OUTDIR/rg2.vtk
../vtkReverse/hev-vtkReverse -n $OUTDIR/rg2.vtk $OUTDIR/rg2.revn.vtk
echo 'Created rg2.revn.vtk. (and rg2.vtk)'
./hev-vtkSmoothMesh $OUTDIR/rg2.revn.vtk $OUTDIR/rg2.smA.vtk
echo 'Created rg2.smA.vtk.'

# Here is another try with different number of iterations. There should
# be much less smoothing.
./hev-vtkSmoothMesh --iter 2 $OUTDIR/rg2.revn.vtk $OUTDIR/rg2.smA.2.vtk
echo 'Created rg2.smA.2.vtk.'

# This is another try, this time with rectGrid.1.noise.vtk. There should
# be a difference between rg1.smA.vtk and rg1.smA.bsoff.vtk.
../vtkContour/hev-vtkContour 0.2 $INDIR/rectGrid.1.noise.vtk $OUTDIR/rg1.vtk
../vtkReverse/hev-vtkReverse -n $OUTDIR/rg1.vtk $OUTDIR/rg1.revn.vtk
echo 'Created rg1.revn.vtk. (and rg1.vtk)'
./hev-vtkSmoothMesh $OUTDIR/rg1.revn.vtk $OUTDIR/rg1.smA.vtk
echo 'Created rg1.smA.vtk.'
./hev-vtkSmoothMesh --bs off $OUTDIR/rg1.revn.vtk $OUTDIR/rg1.smA.bsoff.vtk
echo 'Created rg1.smA.bsoff.vtk.'
exit
