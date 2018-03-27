vol-visBuilder \
        --txtrU 3 \
        --poly \
        --outDir poly \
        --volConv="--nDim 3 --dim 115 100 29 --transform log10 --inType float64 --outType float32" \
        Concentration_*.txt \
        good.tf \
        A

savg-cylinder 6 open | savg-translate 0 0 -0.5 | savg-scale 0.3 0.3 2 > poly/hex.savg

