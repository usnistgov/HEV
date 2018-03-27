
BINDIR=$HEVROOT/external/teem/teem-1.6.0-src/linux.64/bin

$BINDIR/unu save -f nrrd -e ascii -i engine-crop.nrrd -o engine-crop.asc.nrrd

# Extract the ascii data without the header.
./extractNrrdData < engine-crop.asc.nrrd > engine-crop.asc.dat

./extractNrrdData < opac.1D.asc.nrrd | ./mkRgbaFromAlpha > opac.1D.def.dat
./extractNrrdData < opac.1D.s1.5.asc.nrrd | ./mkRgbaFromAlpha > opac.1D.s1.5.dat
./extractNrrdData < opac.1D.s2.4.asc.nrrd | ./mkRgbaFromAlpha > opac.1D.s2.4.dat
./extractNrrdData < opac.1D.s4.asc.nrrd | ./mkRgbaFromAlpha > opac.1D.s4.dat


hev-convertDataArray --nDim 3 --dim 149 208 110 --inType float32 --inFmt asc --outFmt bin engine-crop.asc.dat engine-crop.mha

hev-convertDataArray --nDim 1 --dim 256 --nComp 4 --inType float32 --inFmt asc --outFmt bin opac.1D.def.dat  opac.1D.def.mha
hev-convertDataArray --nDim 1 --dim 256 --nComp 4 --inType float32 --inFmt asc --outFmt bin opac.1D.s1.5.dat  opac.1D.s1.5.mha
hev-convertDataArray --nDim 1 --dim 256 --nComp 4 --inType float32 --inFmt asc --outFmt bin opac.1D.s2.4.dat  opac.1D.s2.4.mha
hev-convertDataArray --nDim 1 --dim 256 --nComp 4 --inType float32 --inFmt asc --outFmt bin opac.1D.s4.dat  opac.1D.s4.mha

hev-volPrep --outDir volVis --box .715 1 .527 engine-crop.mha alpha.0.01.tf engCrop

cp -f opac.1D.*.mha volVis
cp -f tf.opac.1D.*.sh volVis



