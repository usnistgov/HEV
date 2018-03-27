

vol-visBuilder --outDir volVis \
    --volConv="--nDim 3 --dim 115 100 29 --transform log10 --inType float64 --outType float32" \
    concen_*.dat atmos.tf CO2
