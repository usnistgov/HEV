#! /bin/bash

../hyv-phase2glyph -size .5   1  25 25 25    0 1 0 portcem02.1120011.298_4.phase
mv portcem02.1120011.298_4_g.savg portcem02.1120011.298_4_g_polygon.savg
env SAVG_VERBOSE=1 hev-osgconv portcem02.1120011.298_4_g_polygon.savg polygon.osg

../hyv-phase2glyph_new -size .5   1  25 25 25    0 1 0 portcem02.1120011.298_4.phase
mv portcem02.1120011.298_4_g.savg portcem02.1120011.298_4_g_tristrip.savg
env SAVG_VERBOSE=1 hev-osgconv  portcem02.1120011.298_4_g_tristrip.savg tristrip.osg

