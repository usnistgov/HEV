# ! /bin/sh

# Test cases for hev-gridToVtkImage

INDIR=/usr/local/HEV/idea/src/vtkUtilities/data
OUTDIR=testresults

# remove all files currently present in testresults/ folder
rm -f $OUTDIR/*
echo 'Removed all files in testresults/ folder.'

# tests use each ascii and binary versions of a float, short and uchar file
# float
./hev-gridToVtkImage --d 2 3 4 $INDIR/raw_float_234.ascii $OUTDIR/f234.ascii.vtk
echo 'Created f234.ascii.vtk'

./hev-gridToVtkImage --i binary --d 2 3 4 $INDIR/raw_float_234.bin $OUTDIR/f234.bin.vtk
echo 'Created f234.bin.vtk'

# short
./hev-gridToVtkImage --d 3 4 5 --t short $INDIR/raw_short_345.ascii $OUTDIR/s345.ascii.vtk
echo 'Created s345.ascii.vtk'

./hev-gridToVtkImage --i binary --d 3 4 5 --t short $INDIR/raw_short_345.bin $OUTDIR/s345.bin.vtk
echo 'Created s345.bin.vtk'

# uchar
./hev-gridToVtkImage --d 2 3 4 --t uchar $INDIR/raw_uchar_234.ascii $OUTDIR/u234.ascii.vtk
echo 'Created u234.ascii.vtk'

./hev-gridToVtkImage --i binary --d 2 3 4 --t uchar $INDIR/raw_uchar_234.bin $OUTDIR/u234.bin.vtk
echo 'Created u234.bin.vtk'


# create vector vtk file from a raw file
./hev-gridToVtkImage --ncomp 3 --vector --dim 10 12 11 $INDIR/raw.vectors2.10_12_11.ascii $OUTDIR/vec2.vtk
echo 'Created vec2.vtk'


exit
