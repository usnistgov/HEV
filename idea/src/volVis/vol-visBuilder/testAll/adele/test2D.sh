
rm /var/tmp/*.nrrd
rm /var/tmp/*.nhdr
rm /var/tmp/*.raw

vol-visBuilder \
  --outDir s9559 \
  --volConv "--nd 3 --dim 512 512 119 --inty int16 --infmt bin --outty float" \
  set9559-1-119.raw \
  gk2D.tf \
  s9559.2D


