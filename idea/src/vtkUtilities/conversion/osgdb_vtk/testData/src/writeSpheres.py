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



polyW = vtk.vtkPolyDataWriter ()
polyW.SetFileTypeToASCII ()
polyW.SetInput (colorIt.GetPolyDataOutput())
polyW.SetFileName ("sph.pdw.vtk")
polyW.Write()


# GenericDataObjectWriter does same thing as PolyDataWriter in this case.
# File name extension does not affect operation.
gW = vtk.vtkGenericDataObjectWriter ()
gW.SetFileTypeToASCII ()
gW.SetInput (colorIt.GetPolyDataOutput())
gW.SetFileName ("sph.gdow.vtk")
gW.Write()



xgW = vtk.vtkXMLDataSetWriter ()
# xgW.SetFileTypeToASCII ()
xgW.SetDataModeToAscii ()
xgW.SetInput (colorIt.GetPolyDataOutput())
xgW.SetFileName ("sph.xdsw.vtk")
xgW.Write()
xgW.SetFileName ("sph.xdsw.vtkp")
xgW.Write()
xgW.SetFileName ("sph.xdsw.vtp")
xgW.Write()



stripper = vtk.vtkStripper ()
stripper.SetInput (colorIt.GetPolyDataOutput())

polyW.SetInput (stripper.GetOutput())
polyW.SetFileName ("sph.Tstrip.vtk")
polyW.Write()


edges = vtk.vtkExtractEdges()
edges.SetInput (colorIt.GetPolyDataOutput())

polyW.SetInput (edges.GetOutput())
polyW.SetFileName ("sph.lines.vtk")
polyW.Write()


stripper.SetInput (edges.GetOutput())
polyW.SetInput (stripper.GetOutput())
polyW.SetFileName ("sph.Lstrip.vtk")
polyW.Write()









mapper = vtk.vtkPolyDataMapper()
mapper.SetInput(colorIt.GetPolyDataOutput())

actor = vtk.vtkActor()
actor.SetMapper(mapper)

ren = vtk.vtkRenderer()
renWin = vtk.vtkRenderWindow()
renWin.AddRenderer(ren)
iren = vtk.vtkRenderWindowInteractor()
iren.SetRenderWindow(renWin)

ren.AddActor(actor)
ren.SetBackground(1,1,1)
renWin.SetSize(400,400)
ren.GetActiveCamera().Zoom(1.4)

iren.Initialize()

iren.Start()

