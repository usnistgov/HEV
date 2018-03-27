#!/usr/bin/env python


# import os

import vtk



pR = vtk.vtkPolyDataReader ()
pR.SetFileName ("cube.ptUchar.vtk")
pR.Update()

pData = pR.GetOutput()

scalars = pData.GetPointData().GetScalars()

lut = vtk.vtkLookupTable ()

scalars.SetLookupTable (lut)

lut.SetVectorModeToMagnitude ()




xgW = vtk.vtkXMLDataSetWriter ()
xgW.SetInput (pData)
xgW.SetDataModeToAscii ()



polyW = vtk.vtkPolyDataWriter ()
polyW.SetFileTypeToASCII ()
polyW.SetInput (pData)



polyW.SetFileName ("cube.ptUcharVecLUT.vtk")
polyW.Write()

xgW.SetFileName ("cube.ptUcharVecLUT.vtp");
xgW.Write()



