#include <string.h>
#include <stdio.h>

int main(argc, argv)
{
  char outString[1024];
  strcpy(outString, "exec savg-sphere |savg-color -r 1 -g 1 | savg-scale .125 .125 .125; savg-sphere |savg-color -r 1 -g 1| savg-scale .08 .08 .08 | savg-translate 0 -.1 -.1; savg-cone 40 | savg-scale .085 .085 .06 | savg-rotate 0 1 0 | savg-translate 0 .000050 .09; "
"savg-cone 40 | savg-scale .075 -.075 .10 | savg-rotate 180 180 0 |" 
"savg-translate 0 -.1 -.1 | savg-color -r .9 -g .4 -b .4 -a 1; savg-sphere 50 |savg-scale .01 .01 .01 | savg-translate -.04 -.13 -.16 "
" |savg-color -r 0 -g 0 -b 0 -a 1; savg-sphere 50 | savg-scale .01 .01 .01"
"| savg-translate .04 -.13 -.16 | savg-color -r 0 -g 0 -b 0 -a 1 ");

  system(outString);

}

