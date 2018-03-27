FUNC RgbaLut1D CO2D
  VAL_TRANSFORM LOG10
  # COLOR_INTERP DIVERGING
  COLOR_INTERP RGB
  LIGHTING GRAD_OPAQUE
  NORMALIZE true
  PRE_INTEGRATE false
  LOOKUP_INTERP LINEAR
  TABLE_TYPE UCHAR
  TABLE_LEN 256
  COMBINE_RGBA true
  RGBLUT
      1.000000e-20                 0.0             1.0             0.0  
      1.208796e-20                 0.4             0.1             0.7  
      3.981072e-15                 0.2             0.2             0.6  
      6.843750e-07                 0.1             0.6             0.6  
      5.976826e-02                 0.1             0.6             0.2  
      1.000000e+01                 0.1             0.7             0.1  
  END_RGBLUT
  ALPHALUT
      1.000000e-20                 0.0
      1.208796e-20                 0.0
      3.981072e-15                 0.4
      6.843750e-07                 0.9
      5.976826e-02                 1.0
      1.000000e+01                 1.0
  END_ALPHALUT
END_FUNC CO2D


