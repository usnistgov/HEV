#!/usr/bin/env python


import sys

import vtk



pR = vtk.vtkPolyDataReader ()
pR.SetFileName (sys.argv[1])


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
ren.ResetCamera()

iren.Initialize()

iren.Start()

