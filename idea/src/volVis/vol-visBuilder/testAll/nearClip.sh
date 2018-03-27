
vol-visBuilder  \
        --near \
        --outDir near \
        --volConv="--nDim 3 --dim 115 100 29 --transform log10 --inType float64 --outType float32" \
        Concentration_*.txt \
        good.tf \
        A

vol-visBuilder  \
        --noNear \
        --outDir noNear \
        --volConv="--nDim 3 --dim 115 100 29 --transform log10 --inType float64 --outType float32" \
        Concentration_*.txt \
        good.tf \
        A



vol-visBuilder  \
        --clip \
        --outDir clip \
        --volConv="--nDim 3 --dim 115 100 29 --transform log10 --inType float64 --outType float32" \
        Concentration_*.txt \
        good.tf \
        A

vol-visBuilder  \
        --noClip \
        --outDir noClip \
        --volConv="--nDim 3 --dim 115 100 29 --transform log10 --inType float64 --outType float32" \
        Concentration_*.txt \
        good.tf \
        A



