
rm /var/tmp/*.nrrd
rm /var/tmp/*.nhdr
rm /var/tmp/*.raw



echo preparing set39 1D
vol-visBuilder \
  --outDir s39 \
  --volConv "--nd 3 --dim 512 512 550 --inty int16 --infmt bin --outty float" \
  set39-100-650.raw \
  gk1D.def.tf \
  s39.1D



echo preparing set39 2D
vol-visBuilder \
  --outDir s39 \
  --volConv "--nd 3 --dim 512 512 550 --inty int16 --infmt bin --outty float" \
  set39-100-650.raw \
  gk2D.def.tf \
  s39.2D


