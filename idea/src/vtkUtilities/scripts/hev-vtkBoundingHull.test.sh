# ! /bin/sh

# Test cases for hev-vtkBoundingHull

INDIR=/usr/local/HEV/idea/src/vtkUtilities/data
OUTDIR=testresults

# remove all files currently present in testresults/ folder
rm -f $OUTDIR/*
echo 'Removed all files in testresults/ folder.'

# 
./hev-vtkBoundingHull $INDIR/cube.vtk $OUTDIR/cube.hull.1.vtk
echo 'Created cube.hull.1.vtk.'

#
./hev-vtkBoundingHull --s 2 $INDIR/cube.vtk $OUTDIR/cube.hull.2.vtk
echo 'Created cube.hull.2.vtk.'

#
./hev-vtkBoundingHull $INDIR/sph.triCell.vtk $OUTDIR/sph.triCell.1.vtk
echo 'Created sph.triCell.1.vtk.'

#
./hev-vtkBoundingHull --s 2 $INDIR/sph.triCell.vtk $OUTDIR/sph.triCell.2.vtk
echo 'Created sph.triCell.2.vtk.'

# test 0
./hev-vtkBoundingHull --s 0 $INDIR/sph.triCell.vtk $OUTDIR/sph.triCell.0.vtk
echo 'Created sph.triCell.0.vtk.'

exit
