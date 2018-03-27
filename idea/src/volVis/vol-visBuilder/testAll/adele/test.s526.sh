
rm /var/tmp/*.nrrd
rm /var/tmp/*.nhdr
rm /var/tmp/*.raw



echo preparing set526 1D
hev-volPrep  \
  --outDir . \
  --volConv "--nd 3 --dim 512 512 119 --inty int16 --infmt bin --outty float" \
  set526-1-119.raw \
  gk1D.def.tf \
  s526.1D



echo preparing set526 1D
hev-volPrep  \
  --outDir . \
  --volConv "--nd 3 --dim 512 512 119 --inty int16 --infmt bin --outty float" \
  set526-1-119.raw \
  gk2D.def.tf \
  s526.2D


