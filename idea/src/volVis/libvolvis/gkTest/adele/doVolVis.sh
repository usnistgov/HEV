
BINDIR=$HEVROOT/external/teem/teem-1.6.0-src/linux.64/bin

$BINDIR/unu save -f nrrd -e ascii -i set9559.nhdr -o set9559.asc.nrrd

# Extract the ascii data without the header.
./extractNrrdData < set9559.asc.nrrd > set9559.asc.dat

./extractNrrdData < opac.1D.asc.nrrd | ./mkRgbaFromAlpha > opac.1D.def.dat
./extractNrrdData < opac.1D.s1.5.asc.nrrd | ./mkRgbaFromAlpha > opac.1D.s1.5.dat
./extractNrrdData < opac.1D.s2.4.asc.nrrd | ./mkRgbaFromAlpha > opac.1D.s2.4.dat
./extractNrrdData < opac.1D.s4.asc.nrrd | ./mkRgbaFromAlpha > opac.1D.s4.dat


hev-convertDataArray --nDim 3 --dim 512 512 119 --inType float32 --inFmt asc --outFmt bin set9559.asc.dat set9559.mha

hev-convertDataArray --nDim 1 --dim 256 --nComp 4 --inType float32 --inFmt asc --outFmt bin opac.1D.def.dat  opac.1D.def.mha
hev-convertDataArray --nDim 1 --dim 256 --nComp 4 --inType float32 --inFmt asc --outFmt bin opac.1D.s1.5.dat  opac.1D.s1.5.mha
hev-convertDataArray --nDim 1 --dim 256 --nComp 4 --inType float32 --inFmt asc --outFmt bin opac.1D.s2.4.dat  opac.1D.s2.4.mha
hev-convertDataArray --nDim 1 --dim 256 --nComp 4 --inType float32 --inFmt asc --outFmt bin opac.1D.s4.dat  opac.1D.s4.mha

exit

hev-volPrep --outDir volVis --box .715 1 .527 set9559.mha alpha.0.01.tf set9559

cp -f opac.1D.*.mha volVis
cp -f tf.opac.1D.*.sh volVis



