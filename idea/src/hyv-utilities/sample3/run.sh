#! /bin/bash

echo .125 > hyv_subvol_002.iso

../hyv-phase2vtk  5 5 6  hyv_subvol_002.phase


hev-tclVtkIsosurface `cat hyv_subvol_002.iso`  hyv_subvol_002.vtk hyv_subvol_002.iv


hev-tclVtkIsosurfaceClean < hyv_subvol_002.iv | ivmaterial -di 0.8 0.0 0.0 -am .6 .6 .6 -sp .8 .8 .8 -sh .4 > hyv_subvol_002.ivu

iris-convert --nostatesets hyv_subvol_002.ivu hyv_subvol_002.osgb

hev-materialTransform --rmMaterials --colormode OFF --rootOnly --emission 0 0 0 1 --specular .8 .8 .8 1 --ambient  .6 .6 .6 1 --shininess 51 --diffuse 0.8 0.0 0.0 1 hyv_subvol_002.osgb hyv_subvol_002.osgt


../hyv-phase2glyphVarying -d --scale .2  2 5 5 6  0.8 0.0 0.0  hyv_subvol_002.phase

hev-moo hyv_subvol_002.osgt hyv_subvol_002_g.savg > moo.iris

rplot -hist  hyv_subvol_002.phase &

irisfly --ex --back 1 1 1 moo.iris
