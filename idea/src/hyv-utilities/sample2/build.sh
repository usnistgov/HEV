#! /bin/sh

../hyv-img2phase 1 '$c[1]' sample.img

../hyv-phase2vtk 25 25 25 sample_1.phase

../hyv-phase2iso sample_1.phase

# Test both iso surface to ivu and to savg
../hyv-vtk2ivu 1 1.0 .5 .94  sample_1.vtk
../hyv-vtk2savg 1 --debug sample_1.vtk

# and a directory of files

rm -fr foo
mkdir foo
for i in 1 2 3 4
do

  cp sample_1.vtk foo/sample_${i}_1.vtk
  cp sample_1.iso foo/sample_${i}_1.iso

done
../hyv-vtk2savg 1 --debug foo


../hyv-phase2glyph --debug --size .25 1 25 25 25  .8 .8 0 sample_1.phase




# Create a wire frame bounding cube
systemSize="25 25 25"
qvd-wireBox --noclip $systemSize .5 .5 .5 temp.ive

trans=`echo $systemSize | awk '{print $1/2,$2/2,$3/2}'`
dosg-transform --translate $trans temp.ive Box.ive
rm -f temp.ive
