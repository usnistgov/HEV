#! /bin/sh

savgFN=toy-block

# Toy wood block
savg-cube | savg-color -r 0.406 -g 0.203 -b 0.0 > ${savgFN}.savg


savg-text "A" | savg-translate    0   -1.1   -0.6                              | sed -e '$s/$/ JUSTIFICATION CENTER/' >> ${savgFN}.savg
savg-text "B" | savg-translate    0    1.1   -0.6 | savg-rotate  180    0   0  | sed -e '$s/$/ JUSTIFICATION CENTER/' >> ${savgFN}.savg
savg-text "C" | savg-translate    0    0.6   -1.1 | savg-rotate    0   90   0  | sed -e '$s/$/ JUSTIFICATION CENTER/' >> ${savgFN}.savg
savg-text "D" | savg-translate    0   -0.6    1.1 | savg-rotate    0  -90   0  | sed -e '$s/$/ JUSTIFICATION CENTER/' >> ${savgFN}.savg
savg-text "E" | savg-translate   1.1  -0.6    0   | savg-rotate   90    0  90  | sed -e '$s/$/ JUSTIFICATION CENTER/' >> ${savgFN}.savg
savg-text "F" | savg-translate  -1.1  -0.6    0   | savg-rotate  -90    0 -90  | sed -e '$s/$/ JUSTIFICATION CENTER/'  >> ${savgFN}.savg

savgfly --examine ${savgFN}.savg toy-block.ive
