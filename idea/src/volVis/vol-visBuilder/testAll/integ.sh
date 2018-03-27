vol-visBuilder \
        --rayStep 3 \
        --outDir integration \
        --volConv="--nDim 3 --dim 115 100 29 --transform log10 --inType float64 --outType float32" \
        Concentration_*.txt \
        good.tf \
        step3

vol-visBuilder \
        --noRandStep  \
        --outDir integration \
        --volConv="--nDim 3 --dim 115 100 29 --transform log10 --inType float64 --outType float32" \
        Concentration_*.txt \
        good.tf \
        noRand

vol-visBuilder \
        --scaleOpacity 30 \
        --outDir integration \
        --volConv="--nDim 3 --dim 115 100 29 --transform log10 --inType float64 --outType float32" \
        Concentration_*.txt \
        good.tf \
        op30

vol-visBuilder \
        --rayStep 3 \
        --integration linear \
        --outDir integration \
        --volConv="--nDim 3 --dim 115 100 29 --transform log10 --inType float64 --outType float32" \
        Concentration_*.txt \
        good.tf \
        lin3

