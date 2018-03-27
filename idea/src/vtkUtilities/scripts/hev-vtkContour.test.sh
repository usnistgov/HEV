# ! /bin/sh

# Test cases for hev-vtkContour.

INDIR=/usr/local/HEV/idea/src/vtkUtilities/data
OUTDIR=/testresults/

#remove all files currently present in /testresults/ folder
rm -f /testresults/*
echo 'Removed all files in testresults/ folder.'

# The following test produces surf2.vtk which should look like two
# surfaces (green and yellow). Each should be one octant of an ellipsoid.
# The yellow surface is small and more or less parallel to the bigger
# green surface.
hev-vtkContour 2 0.2 0.6 $INDIR/structPoints.1.vtk $OUTDIR/surf2.vtk
echo 'Created surf2.vtk.'

# result: E20_c.vtk looks like a green cookie in irisfly.
hev-vtkContour 80 1 3 $INDIR/E20.vtk $OUTDIR/E20_c.vtk
echo 'Created E20_c.vtk.'

# result: The output (surf1.vtk) should be a smooth green curved surface
# which is somewhat pointy at one end and blunt at the other.
hev-vtkContour 0.4 $INDIR/rectGrid.2.vtk $OUTDIR/surf1.vtk
echo 'Created surf1.vtk.'


# result: surf3.vtk should be a lumpy yellow ellipsoid surrounded by eight
# green fragments of a larger lumpy ellipsoid.
hev-vtkContour 2 0.2 0.6 $INDIR/structPoints.2.noise.vtk $OUTDIR/surf3.vtk
echo 'Created surf3.vtk.'

# result: surf4.vtk should look somewhat like surf3.vtk but sort of twisted.
hev-vtkContour 2 0.2 0.6 $INDIR/structGrid.2.noise.vtk $OUTDIR/surf4.vtk
echo 'Created surf4.vtk.'

# result: lines1.vtk should look like two yellow lines and two green lines
# that follow helical paths.
hev-vtkContour 2 0.2 0.6 $INDIR/unstructGrid.1.vtk $OUTDIR/lines1.vtk
echo 'Created lines1.vtk.'

exit
