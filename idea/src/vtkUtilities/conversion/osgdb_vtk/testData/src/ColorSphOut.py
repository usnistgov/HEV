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



plyW = vtk.vtkPLYWriter ()
plyW.SetFileTypeToASCII ()
plyW.SetInput (colorIt.GetPolyDataOutput())
plyW.SetFileName ("sph.ply")
plyW.Write()



polyW = vtk.vtkPolyDataWriter ()
polyW.SetFileTypeToASCII ()
polyW.SetInput (colorIt.GetPolyDataOutput())
polyW.SetFileName ("sph.vtk")
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

