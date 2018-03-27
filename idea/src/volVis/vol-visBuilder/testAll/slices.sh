
vol-visBuilder  \
        --slices 3 \
        --outDir slices \
        --volConv="--nDim 3 --dim 115 100 29 --transform log10 --inType float64 --outType float32" \
        Concentration_*.txt \
        good.tf \
        A

