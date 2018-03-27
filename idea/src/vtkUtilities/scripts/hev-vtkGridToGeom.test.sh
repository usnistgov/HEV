# ! /bin/sh

# Test cases for hev-vtkGridToGeom

INDIR=/usr/local/HEV/idea/src/vtkUtilities/data
OUTDIR=testresults

#remove all files currently present in testresults/ folder
rm -f $OUTDIR/*
echo 'Removed all files in testresults/ folder.'

# test script with different inputs
# rectGrid
./hev-vtkGridToGeom $INDIR/rectGrid.2.vtk $OUTDIR/rectGrid.geom.vtk
echo 'Created rectGrid.geom.vtk.'

# structGrid
./hev-vtkGridToGeom $INDIR/structGrid.2.noise.vtk $OUTDIR/structGrid.2.noise.geom.vtk
echo 'Created structGrid.2.noise.geom.vtk.'

# structPoints
./hev-vtkGridToGeom $INDIR/structPoints.1.vtk $OUTDIR/structPoints.1.geom.vtk
echo 'Created structPoints.1.geom.vtk.'

# structPoints.noise
./hev-vtkGridToGeom $INDIR/structPoints.2.noise.vtk $OUTDIR/structPoints.2.noise.geom.vtk
echo 'Created structPoints.2.noise.geom.vtk.'

# unstructGrid
./hev-vtkGridToGeom $INDIR/unstructGrid.1.vtk $OUTDIR/unstructGrid.1.geom.vtk
echo 'Created unstructGrid.1.geom.vtk.'

exit
