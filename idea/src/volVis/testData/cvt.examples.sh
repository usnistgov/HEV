

hev-convertDataArray --transform log  concen_50.dat  concen_50.log.raw
hev-convertDataArray --trans log10 --norm -20 10 --outType float64 concen_50.dat  c_50.logNorm.mha
hev-convertDataArray --nDim 3 --dim 100 115 29 --trans log10 --norm -20 10 --outType float64 concen_50.dat  c_50.vol.logNorm.mha
hev-convertDataArray --nDim 3 --dim 100 115 29 --trans log10 --norm -20 10 --outType float64 concen_*.dat  c.%04d.mha

