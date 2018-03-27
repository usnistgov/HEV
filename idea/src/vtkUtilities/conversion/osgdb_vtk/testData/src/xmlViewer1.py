#!/usr/bin/env python


# import os

import vtk



pR = vtk.vtkXMLPolyDataReader ()
pR.SetFileName ("cube.cellNormColorsMod.vtp")








mapper = vtk.vtkPolyDataMapper()


mapper.SetInput(pR.GetOutput())






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

