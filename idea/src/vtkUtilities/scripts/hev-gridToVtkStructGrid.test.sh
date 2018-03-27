# ! /bin/sh

# Test cases for hev-gridToVtkStructGrid

INDIR=/usr/local/HEV/idea/src/vtkUtilities/data
OUTDIR=testresults

# remove all files currently present in testresults/ folder
rm -f $OUTDIR/*
echo 'Removed all files in testresults/ folder.'

# ascii
./hev-gridToVtkStructGrid --dim 16 10 33 $INDIR/grid.1.data $INDIR/structGrid.1.coords $OUTDIR/newSG.1.vtk
echo 'Created newSG.1.vtk.'

# binary
./hev-gridToVtkStructGrid --dim 16 10 33 --i binary $INDIR/grid.1.data.bin $INDIR/structGrid.1.coords.bin $OUTDIR/newSG.1.bin.vtk
echo 'Created newSG.1.bin.vtk.'

exit
