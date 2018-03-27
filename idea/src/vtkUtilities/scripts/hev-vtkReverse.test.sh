# ! /bin/sh

# Test cases for hev-vtkReverse

INDIR=/usr/local/HEV/idea/src/vtkUtilities/data
OUTDIR=testresults

#remove all files currently present in testresults/ folder
rm -f $OUTDIR/*
echo 'Removed all files in testresults/ folder.'

# test 4 different input files with all 3 options each (cell
# and normals reversal, cell reversal only, normals reversal
# only).
# cube both
./hev-vtkReverse $INDIR/cube.vtk $OUTDIR/cube.both.vtk
echo 'Created cube.both.vtk.'

# cube cell only
./hev-vtkReverse -c $INDIR/cube.vtk $OUTDIR/cube.c.vtk
echo 'Created cube.c.vtk.'

# cube normals only
./hev-vtkReverse -n $INDIR/cube.vtk $OUTDIR/cube.n.vtk
echo 'Created cube.n.vtk.'

# lineCell
./hev-vtkReverse $INDIR/sph.lineCell.vtk $OUTDIR/sph.lineCell.both.vtk
echo 'Created sph.lineCell.both.vtk.'

# lineCell cell only
./hev-vtkReverse -c $INDIR/sph.lineCell.vtk $OUTDIR/sph.lineCell.c.vtk
echo 'Created sph.lineCell.c.vtk.'

# lineCell normals only
./hev-vtkReverse -n $INDIR/sph.lineCell.vtk $OUTDIR/sph.lineCell.n.vtk
echo 'Created sph.lineCell.n.vtk.'

# polyVertCell
./hev-vtkReverse $INDIR/sph.polyVertCell.vtk $OUTDIR/sph.polyVertCell.both.vtk
echo 'Created sph.polyVertCell.both.vtk.'

# lineCell cell only
./hev-vtkReverse -c $INDIR/sph.polyVertCell.vtk $OUTDIR/sph.polyVertCell.c.vtk
echo 'Created sph.polyVertCell.c.vtk.'

# lineCell normals only
./hev-vtkReverse -n $INDIR/sph.polyVertCell.vtk $OUTDIR/sph.polyVertCell.n.vtk
echo 'Created sph.polyVertCell.n.vtk.'

# triCell
./hev-vtkReverse $INDIR/sph.triCell.vtk $OUTDIR/sph.triCell.both.vtk
echo 'Created sph.triCell.both.vtk.'

# triCell cell only
./hev-vtkReverse -c $INDIR/sph.triCell.vtk $OUTDIR/sph.triCell.c.vtk
echo 'Created sph.triCell.c.vtk.'

# triCell normals only
./hev-vtkReverse -n $INDIR/sph.triCell.vtk $OUTDIR/sph.triCell.n.vtk
echo 'Created sph.triCell.n.vtk.'

exit
