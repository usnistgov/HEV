# ! /bin/sh

# Test cases for hev-vtkVectorMagnitude

INDIR=/usr/local/HEV/idea/src/vtkUtilities/data
OUTDIR=testresults

#remove all files currently present in /testresults/ folder
rm -f testresults/*
echo 'Removed all files in testresults/.'

# create another vector vtk file from a raw file
./hev-gridToVtkImage --ncomp 3 --vector --dim 10 12 11 $INDIR/raw.vectors2.10_12_11.ascii $OUTDIR/vec2.vtk


# magnitude should be constant on each level, increasing between levels
./hev-vtkVectorMagnitude $OUTDIR/vec2.vtk $OUTDIR/vec2.mag.vtk

exit

