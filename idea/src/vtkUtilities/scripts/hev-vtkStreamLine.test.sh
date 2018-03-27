# ! /bin/sh

# Test cases for hev-vtkStreamLine

INDIR=/usr/local/HEV/idea/src/vtkUtilities/data
OUTDIR=testresults

# remove all files currently present in /testresults/ folder
rm -f testresults/*
echo 'Removed all files in testresults/.'


# create another vector vtk file from a raw file
./hev-gridToVtkImage --ncomp 3 --vector --dim 10 12 11 $INDIR/raw.vectors2.10_12_11.ascii $OUTDIR/vec2.vtk


# Streamline should be a distorted helix.  Circular but
# with increasing movement in Z as it ascends
#
# streamline with default params:
./hev-vtkStreamLine --start 4.5 2.5 0.0 $OUTDIR/vec2.vtk $OUTDIR/sl2.A.vtk
#
# streamline same as above, but will probably be longer
# and smoother
./hev-vtkStreamLine --maxTime 1000 --timeStep 0.1 --start 4.5 2.5 0.0 $OUTDIR/vec2.vtk $OUTDIR/sl2.B.vtk

echo Streamlines should be distorted helix.  Circular but
echo with increasing movement in Z as it ascends
echo
echo Should see short streamline
irisfly --ex $OUTDIR/sl2.A.vtk   # short streamline
echo Should see long streamline
irisfly --ex $OUTDIR/sl2.B.vtk   # long streamline
echo Both streamlines. Should see minimal deviation between two streamlines
irisfly --ex $OUTDIR/sl2.A.vtk $OUTDIR/sl2.B.vtk  # see both; any visible deviation?

exit
