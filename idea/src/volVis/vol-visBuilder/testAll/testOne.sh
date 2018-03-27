
vol-visBuilder  \
        --outDir one \
        --volConv="--nDim 3 --dim 115 100 29 --transform log10 --inType float64 --outType float32" \
        Concentration_50.txt \
        good.tf \
        one

vol-visBuilder  \
        --outDir one \
        --volConv="--nDim 3 --dim 100 115 29 --transform log10 --inType float64 --outType float32" \
        Concentration_50.txt \
        good.tf \
        B

