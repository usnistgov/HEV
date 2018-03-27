# ! /bin/sh

# Test cases for hev-gridToVtkRectGrid

INDIR=/usr/local/HEV/idea/src/vtkUtilities/data
OUTDIR=testresults

# remove all files currently present in testresults/ folder
rm -f $OUTDIR/*
echo 'Removed all files in testresults/ folder.'

# 
./hev-gridToVtkRectGrid --dim 16 10 33 $INDIR/grid.1.data $INDIR/rectGrid.1.X $INDIR/rectGrid.1.Y $INDIR/rectGrid.1.Z $OUTDIR/newRG.1.vtk
echo 'Created newRG.1.vtk.'

# 
./hev-gridToVtkRectGrid --dim 16 10 33 --i binary $INDIR/grid.1.data.bin $INDIR/rectGrid.1.X.bin $INDIR/rectGrid.1.Y.bin $INDIR/rectGrid.1.Z.bin $OUTDIR/newRG.1.bin.vtk
echo 'Created newRG.1.bin.vtk.'

exit
