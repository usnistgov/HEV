# ! /bin/sh

# Test cases for hev-vtkGridResample

INDIR=/usr/local/HEV/idea/src/vtkUtilities/data
OUTDIR=testresults

# remove all files currently present in testresults/ folder
rm -f $OUTDIR/*
echo 'Removed all files in testresults/ folder.'


#resampling rectGrid with structPoints and vice-versa for X, Y and Z files
#it seems to get "confused" when using structPoints as the source (--r) and
#rectGrid as the input. X and Z output seems fine, output is wrong for Y
#output. Suspecting a bug in vtk.

# X
./hev-vtkGridResample --r $INDIR/structPoints.coordData.X.vtk $INDIR/rectGrid.coordData.X.vtk $OUTDIR/spFromRg.X.vtk
echo 'Created spFromRg.X.vtk.'

./hev-vtkGridResample --r $INDIR/rectGrid.coordData.X.vtk $INDIR/structPoints.coordData.X.vtk $OUTDIR/rgFromSp.X.vtk
echo 'Created rgFromSp.X.vtk.'

#  Y
./hev-vtkGridResample --r $INDIR/structPoints.coordData.Y.vtk $INDIR/rectGrid.coordData.Y.vtk $OUTDIR/spFromRg.Y.vtk
echo 'Created spFromRg.Y.vtk.'

./hev-vtkGridResample --r $INDIR/rectGrid.coordData.Y.vtk $INDIR/structPoints.coordData.Y.vtk $OUTDIR/rgFromSp.Y.vtk
echo 'Created rgFromSp.Y.vtk.'

# Z
./hev-vtkGridResample --r $INDIR/structPoints.coordData.Z.vtk $INDIR/rectGrid.coordData.Z.vtk $OUTDIR/spFromRg.Z.vtk
echo 'Created spFromRg.Z.vtk.'

./hev-vtkGridResample --r $INDIR/rectGrid.coordData.Z.vtk $INDIR/structPoints.coordData.Z.vtk $OUTDIR/rgFromSp.Z.vtk
echo 'Created rgFromSp.Z.vtk.'

exit
