FUNC RgbaLut1D jello
  # red to green color ramp with nothing fully opaque
  COLOR_INTERP CIELAB
  # LIGHTING EMIT_ONLY
  LIGHTING GRAD_OPAQUE
  NORMALIZE true
  TABLE_TYPE FLOAT
  TABLE_LEN 256
  COMBINE_RGBA true      # this is a comment
  RGBLUT
      0.0    0.8 0.1 0
      0.2    0.1 0.8 0
  END_RGBLUT
  ALPHALUT
      0.0  0.4
      0.1  0.5
      0.2  0.9
  END_ALPHALUT
END_FUNC jello


