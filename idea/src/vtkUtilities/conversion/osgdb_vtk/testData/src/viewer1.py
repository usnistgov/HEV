#!/usr/bin/env python


# import os

import vtk



pR = vtk.vtkPolyDataReader ()
pR.SetFileName ("sph.lines.vtk")
pR.SetFileName ("polyEx2.vtk")
pR.SetFileName ("example1.vtk")
pR.SetFileName ("cube5.vtk")
pR.SetFileName ("cube.cellNormColors.vtp")
pR.SetFileName ("cube.ptUchar.vtk")
pR.SetFileName ("cube.ptUcharVecLUT.vtk")
pR.SetFileName ("sph.vtk")



# gdoR = vtk.vtkGenericDataObjectReader ()
# gdoR.SetFileName ("sph.pdw.vtk")


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

