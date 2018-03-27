# ! /bin/sh

# Test cases for hev-vtkGridSubset

INDIR=/usr/local/HEV/idea/src/vtkUtilities/data
OUTDIR=testresults

# remove all files currently present in testresults/ folder
rm -f $OUTDIR/*
echo 'Removed all files in testresults/ folder.'

# test on 3 different kinds of grids.
./hev-vtkGridSubset 5 10 3 7 5 20 $INDIR/rectGrid.1.vtk $OUTDIR/rG.5_10.3_7.5_20.vtk
echo 'Created rG.5_10.3_7.5_20.vtk.'

./hev-vtkGridSubset 5 10 3 7 5 20 $INDIR/structGrid.1.vtk $OUTDIR/sG.5_10.1.5_10.1.5_10.vtk
echo 'Created sG.5_10.1.5_10.1.5_10.vtk.'

./hev-vtkGridSubset 5 10 3 7 5 20 $INDIR/structPoints.1.vtk $OUTDIR/sP.5_10.3_7.5_20.vtk
echo 'Created sP.5_10.3_7.5_20.vtk.'

exit
