rm outDir/*

cd outDir
ln -s ../Concentration_50.txt .
ln -s ../good.tf .


vol-visBuilder  --outDir . \
        --volConv="--nDim 3 --dim 115 100 29 --transform log10 --inType float64 --outType float32" \
        Concentration_50.txt \
        good.tf \
        A


