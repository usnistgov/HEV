# ! /bin/sh

# Test cases for hev-vtkVectorGlyph

INDIR=/usr/local/HEV/idea/src/vtkUtilities/data
OUTDIR=testresults

# remove all files currently present in /testresults/ folder
rm -f testresults/*
echo 'Removed all files in testresults/.'

################################

# Here are three tests.  Each test has these partss:
# 
#         a. take grad of a grid, i
#         b. gen vec glyphs at each point, 
#         c. gen iso surf of same grid
#         d. gen vec glyphs from iso surf normals
#         e. visually compare (b) with  with (c) (should be perp)
#         f. visually compare (d) with  with (c) (should be perp)



./hev-vtkGradient $INDIR/structPoints.2.vtk $OUTDIR/sp2.grad.vtk
./hev-vtkVectorGlyph --sc 20 $OUTDIR/sp2.grad.vtk $OUTDIR/sp2.gradGlyph.vtk
./hev-vtkContour 0.3 $INDIR/structPoints.2.vtk $OUTDIR/sp2.iso0.3.vtk

echo Vectors should be reasonably close to perpendicular at contour surface
irisfly --ex $OUTDIR/sp2.gradGlyph.vtk $OUTDIR/sp2.iso0.3.vtk

# again vectors should be perpendicular to contour surface
# they point inward; need to look inside
./hev-vtkVectorGlyph -n $OUTDIR/sp2.iso0.3.vtk $OUTDIR/sp2.normGlyph.vtk
irisfly --ex $OUTDIR/sp2.normGlyph.vtk $OUTDIR/sp2.iso0.3.vtk



./hev-vtkGradient $INDIR/rectGrid.2.vtk $OUTDIR/rg2.grad.vtk
./hev-vtkVectorGlyph --sc 10 $OUTDIR/rg2.grad.vtk $OUTDIR/rg2.gradGlyph.vtk
./hev-vtkContour 0.3 $INDIR/rectGrid.2.vtk $OUTDIR/rg2.iso0.3.vtk

# vectors should be reasonably close to perpendicular at contour surface
irisfly --ex $OUTDIR/rg2.gradGlyph.vtk $OUTDIR/rg2.iso0.3.vtk

# again vectors should be perpendicular to contour surface
# they point inward; need to look inside
./hev-vtkVectorGlyph -n $OUTDIR/rg2.iso0.3.vtk $OUTDIR/rg2.normGlyph.vtk
irisfly --ex $OUTDIR/rg2.normGlyph.vtk $OUTDIR/rg2.iso0.3.vtk




./hev-vtkGradient $INDIR/structGrid.2.vtk $OUTDIR/sg2.grad.vtk
./hev-vtkVectorGlyph --sc 10 $OUTDIR/sg2.grad.vtk $OUTDIR/sg2.gradGlyph.vtk
./hev-vtkContour 0.3 $INDIR/structGrid.2.vtk $OUTDIR/sg2.iso0.3.vtk

# vectors should be reasonably close to perpendicular at contour surface
irisfly --ex $OUTDIR/sg2.gradGlyph.vtk $OUTDIR/sg2.iso0.3.vtk

# again vectors should be perpendicular to contour surface
# they point inward; need to look inside
./hev-vtkVectorGlyph -n $OUTDIR/sg2.iso0.3.vtk $OUTDIR/sg2.normGlyph.vtk
irisfly --ex $OUTDIR/sg2.normGlyph.vtk $OUTDIR/sg2.iso0.3.vtk




exit


