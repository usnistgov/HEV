
#include "RgbaLut1D.h"

main ()
        {

        VolVis::RgbaLut1D *lutA = new VolVis::RgbaLut1D;
        lutA->setToDefaultLut ();
        lutA->setLighting (VolVis::LM_GRAD_OPAQUE);
        lutA->write ("lutA.tf");
        lutA->writeGLSL ("lutA.tf.glsl");
        lutA->writeTexture (0, "lutA.txtr0.mha");
        lutA->writeTexture (1, "lutA.txtr1.mha");
        lutA->writeTexture (0, "tmp/lutA.txtr2.mha");

        VolVis::RgbaLut1D *lutB = new VolVis::RgbaLut1D;
        lutB->setColorInterp (VolVis::C_INTERP_HSV);
        float rgba[4];
        rgba[0] = 5;
        rgba[1] = 6;
        rgba[2] = 7;
        rgba[3] = 8;
        lutB->setInterpPtRGBA (20.0, rgba);
        rgba[0] = 1;
        rgba[1] = 2;
        rgba[2] = 3;
        rgba[3] = 4;
        lutB->setInterpPtRGBA (10.0, rgba);
        rgba[0] = 15;
        rgba[1] = 16;
        rgba[2] = 17;
        rgba[3] = 18;
        lutB->setInterpPtRGBA (200.0, rgba);
        lutB->write ("lutB.tf");
        lutB->writeGLSL ("lutB.tf.glsl");

        VolVis::RgbaLut1D *lutC = new VolVis::RgbaLut1D;
        lutC->read ("lutC.in.tf");

        lutC->write ("lutC.out.tf");
        lutC->writeGLSL ("lutC.out.tf.glsl");




        return 0;
        } // end of main
