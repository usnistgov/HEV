vol-visBuilder \
        --box 2 1 1 \
        --outDir box \
        --volConv="--nDim 3 --dim 115 100 29 --transform log10 --inType float64 --outType float32" \
        Concentration_*.txt \
        good.tf \
        A

