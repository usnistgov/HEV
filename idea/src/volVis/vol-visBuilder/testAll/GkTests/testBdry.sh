
rm /var/tmp/*.nrrd
rm /var/tmp/*.nhdr
rm /var/tmp/*.raw



echo preparing bdry 1D
hev-volPrep  \
  --outDir . \
  --volConv "--nd 3 --dim 21 20 19 --inty float32 --infmt bin " \
   ~/HEV/idea/src/volume/data/bdry.*.raw \
  gk1D.def.tf \
  bdry.1D




echo preparing bdry 2D
hev-volPrep  \
  --outDir . \
  --volConv "--nd 3 --dim 21 20 19 --inty float32 --infmt bin " \
   ~/HEV/idea/src/volume/data/bdry.*.raw \
  gk2D.def.tf \
  bdry.2D




