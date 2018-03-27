FUNC RgbaLut1D simple
  COLOR_INTERP CIE
  PRE_INTEGRATE false
  LOOKUP_INTERP LINEAR
  TABLE_TYPE FLOAT
  TABLE_LEN 256
  COMBINE_RGBA true
  NORMALIZE true
  LIGHTING GRAD_OPAQUE
  RGBLUT
        0 1 0 0
        255 0 0 1
  END_RGBLUT
  ALPHALUT
        0 0
        50 0
        51 0
        80 1
        105 0
        106 0
        150 0
        154 1
        199 1
        200 0
        250 0
        251 0
        255 0
  END_ALPHALUT
END_FUNC simple


