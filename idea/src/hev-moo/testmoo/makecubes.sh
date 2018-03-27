#!/bin/sh
savg-cube | savg-scale .25 | savg-wireframe > 000.savg
savg-cube | savg-scale .25 | savg-translate 0 0 1 | savg-rgb 0 0 1 > 001.savg
savg-cube | savg-scale .25 | savg-translate 0 1 0 | savg-rgb 0 1 0 > 010.savg
savg-cube | savg-scale .25 | savg-translate 0 1 1 | savg-rgb 0 1 1 > 011.savg
savg-cube | savg-scale .25 | savg-translate 1 0 0 | savg-rgb 1 0 0 > 100.savg
savg-cube | savg-scale .25 | savg-translate 1 0 1 | savg-rgb 1 0 1 > 101.savg
savg-cube | savg-scale .25 | savg-translate 1 1 0 | savg-rgb 1 1 0 > 110.savg
savg-cube | savg-scale .25 | savg-translate 1 1 1 | savg-rgb 1 1 1 > 111.savg

