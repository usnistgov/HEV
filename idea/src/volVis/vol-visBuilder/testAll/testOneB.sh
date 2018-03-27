
vol-visBuilder  \
        --outDir oneB \
        --volConv="--nDim 3 --dim 115 100 29 --transform log10 --inType float64 --outType float32 --infSub -1000 1000" \
        Concentration_50.txt \
        good.tf \
        oneB


