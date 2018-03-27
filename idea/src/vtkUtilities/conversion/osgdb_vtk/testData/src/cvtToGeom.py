#!/usr/bin/env python


import sys

import vtk



reader = vtk.vtkGenericDataObjectReader ()
reader.SetFileName (sys.argv[1]);


geom = vtk.vtkGeometryFilter()
geom.SetInput (reader.GetOutput());


polyW = vtk.vtkPolyDataWriter ()
polyW.SetFileTypeToASCII ()
polyW.SetInput (geom.GetOutput())
polyW.SetFileName (sys.argv[2])
polyW.Write()







