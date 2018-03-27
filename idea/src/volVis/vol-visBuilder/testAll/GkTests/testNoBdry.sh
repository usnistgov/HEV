
rm /var/tmp/*.nrrd
rm /var/tmp/*.nhdr
rm /var/tmp/*.raw



echo preparing noBdry 1D
hev-volPrep  \
  --outDir . \
  --volConv "--nd 3 --dim 21 20 19 --inty float32 --infmt bin " \
   ~/HEV/idea/src/volume/data/noBdry.*.raw \
  gk1D.def.tf \
  noBdry.1D




echo preparing noBdry 2D
hev-volPrep  \
  --outDir . \
  --volConv "--nd 3 --dim 21 20 19 --inty float32 --infmt bin " \
   ~/HEV/idea/src/volume/data/noBdry.*.raw \
  gk2D.def.tf \
  noBdry.2D




