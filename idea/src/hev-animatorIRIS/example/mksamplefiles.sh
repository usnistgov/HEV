#! /bin/sh

#ivcone   | ivrgba 1 0 0 1 | ivscale .5 .5 .5 | ivtranslate -1 0 0 > a.iv
#ivsphere | ivrgba 0 1 0 1 | ivscale .5 .5 .5 | ivtranslate  0 0 0 > b.iv
#ivcube   | ivrgba 0 0 1 1 | ivscale .5 .5 .5 | ivtranslate  1 0 0 > c.iv
#ivcone   | ivrgba 1 0 1 1 | ivscale .5 .5 .5 | ivtranslate -1 0 0 > d.iv
#ivsphere | ivrgba 0 1 1 1 | ivscale .5 .5 .5 | ivtranslate  0 0 0 > e.iv
#ivcube   | ivrgba 1 1 0 1 | ivscale .5 .5 .5 | ivtranslate  1 0 0 > f.iv

ivcone   | ivrgba 1 0 0 1 | ivscale .5 .5 .5  > a.iv
ivsphere | ivrgba 0 1 0 1 | ivscale .5 .5 .5  > b.iv
ivcube   | ivrgba 0 0 1 1 | ivscale .5 .5 .5  > c.iv
ivcone   | ivrgba 1 0 1 1 | ivscale .5 .5 .5  > d.iv
ivsphere | ivrgba 0 1 1 1 | ivscale .5 .5 .5  > e.iv
ivcube   | ivrgba 1 1 0 1 | ivscale .5 .5 .5  > f.iv


iris-DCS tape2 --translate 2 0 0 tape.osg > tape2.iris
iris-convert tape2.iris tape2.osg

