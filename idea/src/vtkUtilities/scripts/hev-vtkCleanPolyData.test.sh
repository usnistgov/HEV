# ! /bin/sh

# Test cases for hev-vtkCleanPolyData

INDIR=/usr/local/HEV/idea/src/vtkUtilities/data
OUTDIR=testresults

# remove all files currently present in testresults/ folder
rm -f $OUTDIR/*
echo 'Removed all files in testresults/ folder.'

# 
./hev-vtkCleanPolyData $INDIR/cube.vtk $OUTDIR/cube.clean1.vtk
echo 'Created cube.clean1.vtk.'

./hev-vtkCleanPolyData --p 0.2 $INDIR/cube.vtk $OUTDIR/cube.clean2.vtk
echo 'Created cube.clean2.vtk.'

exit
