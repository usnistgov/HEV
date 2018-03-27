#!/usr/bin/env python


import sys

import vtk



reader = vtk.vtkPolyDataReader ()
reader.SetFileName (sys.argv[1]);


pdNormals = vtk.vtkPolyDataNormals()
pdNormals.SetFeatureAngle (0.0)
pdNormals.SplittingOn ()
pdNormals.ConsistencyOn ()
pdNormals.ComputeCellNormalsOn ()
pdNormals.SetInput (reader.GetOutput())







polyW = vtk.vtkPolyDataWriter ()
polyW.SetFileTypeToASCII ()
polyW.SetInput (pdNormals.GetOutput())
polyW.SetFileName (sys.argv[2])
polyW.Write()




