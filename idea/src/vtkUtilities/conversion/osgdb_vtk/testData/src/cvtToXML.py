#!/usr/bin/env python


# import os

import vtk




pR = vtk.vtkPolyDataReader ()
pR.SetFileName ("cube.cellNormColors.vtk");



xgW = vtk.vtkXMLDataSetWriter ()
xgW.SetInput (pR.GetOutput())
xgW.SetDataModeToAscii ()
xgW.SetFileName ("cube.cellNormColors.vtp");
xgW.Write()


