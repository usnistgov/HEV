# bad .tf
vol-visBuilder  \
        --outDir badDir \
        --volConv="--nDim 3 --dim 115 100 29 --transform log10 --inType float64 --outType float32" \
        Concentration_*.txt \
        bad.tf \
        bad1

# bad .tf name
vol-visBuilder  \
        --cleanup \
        --outDir badDir \
        --volConv="--nDim 3 --dim 115 100 29 --transform log10 --inType float64 --outType float32" \
        Concentration_*.txt \
        nonexistent.tf \
        bad2

# bad .tf name
vol-visBuilder  \
        --noclean \
        --outDir badDir \
        --volConv="--nDim 3 --dim 115 100 29 --transform log10 --inType float64 --outType float32" \
        Concentration_*.txt \
        nonexistent.tf \
        bad3
