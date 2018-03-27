#! /bin/sh

#unzip flexomat.zip
#convert -resize 512x64! CINTA.GIF cinta.png
#osgconv FLEXOMET.3DS foo.osg #ignore errors
#sed -e 's/CINTA.GIF/cinta.png/' < foo.osg > flexomat.osg
cp flexomat.osg.savMe flexomat.osg
iris-DCS tape --examine flexomat.osg > flexomat-normalized.iris
#ivtext IRIS | ivscale .1 .2 .1 | ivtranslate -.63 0 -1.9 | sed -e 's/Times-Roman/Arial'/ > iris.iv
ivtext NIST | ivscale .1 .2 .1 | ivtranslate -.65 0 -1.9 | sed -e 's/Times-Roman/Arial'/ > iris.iv
iris-convert iris.iv flexomat-normalized.iris tape2.osg
iris-DCS tape --center --bound 1 tape2.osg > tape.iris
iris-convert tape.iris tape.osg
#cp tape.osg cinta.png $HEVROOT/idea/data

