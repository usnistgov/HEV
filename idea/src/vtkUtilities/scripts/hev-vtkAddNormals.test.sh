# ! /bin/sh

# Test cases for hev-AddNormals

INDIR=/usr/local/HEV/idea/src/vtkUtilities/data
OUTDIR=testresults

#remove all files currently present in testresults/ folder
rm -f $OUTDIR/*
echo 'Removed all files in testresults/ folder.'

# Use irisfly to look at sph.facets.vtk. Should look very similar to input,
# but each triangle should be distinctly visible.
./hev-vtkAddNormals --fangle 0 $INDIR/sph.triCell.vtk $OUTDIR/sph.facets.vtk
echo 'Created sph.facets.vtk.'

# Result should be rather odd. You will be able to discern the divisions
# between the faces because of the change in color, but the shading
# should be more continuous across the corners of the cube.
./hev-vtkAddNormals --fangle 95 $INDIR/cube.vtk $OUTDIR/cube.smooth.vtk
echo 'Created cube.smooth.vtk.'

# shading test - angle specifications are in degrees rahter than radians.
# --fangle 89 should look correct, --fangle 91 should have odd shading.
./hev-vtkAddNormals --fangle 89 $INDIR/cube.vtk $OUTDIR/cube.89.vtk
echo 'Created cube.89.vtk.'
./hev-vtkAddNormals --fangle 91 $INDIR/cube.vtk $OUTDIR/cube.91.vtk
echo 'Created cube.91.vtk.'

# flip test. Should look like the light comes from below rather than above.
./hev-vtkAddNormals -f $INDIR/sph.triCell.vtk $OUTDIR/sph.flipped.vtk
echo 'Created sph.flipped.vtk.'

exit
