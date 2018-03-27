# ! /bin/sh

# Test cases for hev-PolyDecimate

INDIR=/usr/local/HEV/idea/src/vtkUtilities/data
OUTDIR=testresults

#remove all files currently present in /testresults/ folder
rm -f testresults/*
echo 'Removed all files in testresults/.'

cp $INDIR/sph.triCell.vtk $OUTDIR/
echo 'Copied sph.triCell.vtk to testresults/ folder.'

#testing pro method
#target reduction 0.2 on sph.triCell.vtk. Output should
#be very similar to the input, but with fewer triangles.
./hev-vtkPolyDecimate 0.2 $OUTDIR/sph.triCell.vtk $OUTDIR/sph.0.2.vtk
echo 'Created sph.0.2.vtk.'

#run results through hev-vtkAddNormals to make differences more obvious
#(for visual comparison).
../vtkAddNormals/hev-vtkAddNormals --fangle 0 $OUTDIR/sph.triCell.vtk $OUTDIR/sph.facets.vtk
echo 'Created sph.facets.vtk.'
../vtkAddNormals/hev-vtkAddNormals --fangle 0 $OUTDIR/sph.0.2.vtk $OUTDIR/sph.0.2.facets.vtk
echo 'Created sph.0.2.facets.vtk.'

#doing the same thing as above with hev-vtkContour output

cp $INDIR/structPoints.2.noise_CF.vtk $OUTDIR/
echo 'Copied structPoints.2.noise_CF.vtk to testresults/ folder.'

./hev-vtkPolyDecimate 0.2 $OUTDIR/structPoints.2.noise_CF.vtk $OUTDIR/sP2.noise_CF.0.2.vtk
echo 'Created sP2.noise_CF.0.2.vtk.'

../vtkAddNormals/hev-vtkAddNormals --fangle 0 $OUTDIR/structPoints.2.noise_CF.vtk $OUTDIR/sP2.noise_CF.facets.vtk
echo 'Created sP2.noise_CF.facets.vtk.'
../vtkAddNormals/hev-vtkAddNormals --fangle 0 $OUTDIR/sP2.noise_CF.0.2.vtk $OUTDIR/sP2.noise_CF.0.2.facets.vtk
echo 'Created sP2.noise_CF.0.2.facets.vtk.'


#testing quad method
./hev-vtkPolyDecimate --m quad 0.2 $OUTDIR/sph.triCell.vtk $OUTDIR/Q.sph.0.2.vtk
echo 'Created Q.sph.0.2.vtk.'

#run results through hev-vtkAddNormals to make differences more obvious
#(for visual comparison).
../vtkAddNormals/hev-vtkAddNormals --fangle 0 $OUTDIR/sph.triCell.vtk $OUTDIR/Q.sph.facets.vtk
echo 'Created Q.sph.facets.vtk.'
../vtkAddNormals/hev-vtkAddNormals --fangle 0 $OUTDIR/Q.sph.0.2.vtk $OUTDIR/Q.sph.0.2.facets.vtk
echo 'Created Q.sph.0.2.facets.vtk.'


exit
