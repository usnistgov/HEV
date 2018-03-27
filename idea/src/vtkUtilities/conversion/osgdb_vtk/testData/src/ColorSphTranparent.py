#!/usr/bin/env python


# import os

import vtk
# from vtk.util.misc import vtkGetDataRoot
# VTK_DATA_ROOT = vtkGetDataRoot()




# try:
  # VTK_DATA = os.environ['VTK_DATA']
# except KeyError:
  # VTK_DATA = '../../../vtkdata/'

# from libVTKCommonPython import *
# from libVTKGraphicsPython import *

# Example demonstrates use of abstract vtkDataSetToDataSetFilter
# (i.e., vtkElevationFilter - an abstract filter)

sphere = vtk.vtkSphereSource()
sphere.SetPhiResolution(12)
sphere.SetThetaResolution(12)

colorIt = vtk.vtkElevationFilter()
colorIt.SetInput(sphere.GetOutput())
colorIt.SetLowPoint(0,0,-1)
colorIt.SetHighPoint(0,0,1)

poly = colorIt.GetPolyDataOutput ()
poly.Update()
poly.GetPointData().Update()

# the following is a vtkDataArray
scalArr = poly.GetPointData().GetScalars()

lut = vtk.vtkLookupTable()
scalArr.SetLookupTable (lut)

lut.SetAlphaRange(0.5, 1.0)
lut.Build()




polyW = vtk.vtkPolyDataWriter ()
polyW.SetFileTypeToASCII ()
polyW.SetInput (colorIt.GetPolyDataOutput())
polyW.SetFileName ("sph.transVert.vtk")
polyW.Write()





