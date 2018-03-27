#include <stdio.h>

int main(int argc, char **argv)
{

    if (argc>1)
    {
	fprintf(stderr,"Usage: hev-savgCube # no options allowed\n") ;
	return 1 ;

    }
    printf("nooptimize\n") ;

    // -YZ plane
    printf("tristrip\n") ;
    printf("-1  1  1  -1  0 0\n") ;
    printf("-1  1 -1  -1  0 0\n") ;
    printf("-1 -1  1  -1  0 0\n") ;
    printf("-1 -1 -1  -1  0 0\n") ;

    // -XY plane
    printf("-1 -1 -1   0  0 -1\n") ;
    printf("-1  1 -1   0  0 -1\n") ;
    printf(" 1 -1 -1   0  0 -1\n") ;
    printf(" 1  1 -1   0  0 -1\n") ;

    // +YZ plane
    printf(" 1  1 -1   1  0  0\n") ;
    printf(" 1  1  1   1  0  0\n") ;
    printf(" 1 -1 -1   1  0  0\n") ;
    printf(" 1 -1  1   1  0  0\n") ;

    // transition with a degenerate triangle
    printf(" 1 -1  1   0  0  0\n") ;
    printf("-1 -1 -1   0  0  0\n") ;

    // -XZ plane
    printf("-1 -1 -1   0 -1  0\n") ;
    printf(" 1 -1 -1   0 -1  0\n") ;
    printf("-1 -1  1   0 -1  0\n") ;
    printf(" 1 -1  1   0 -1  0\n") ;

    // +XY plane
    printf(" 1 -1  1   0  0  1\n") ;
    printf(" 1  1  1   0  0  1\n") ;
    printf("-1 -1  1   0  0  1\n") ;
    printf("-1  1  1   0  0  1\n") ;

    // +XZ plane
    printf("-1  1  1   0  1  0\n") ;
    printf(" 1  1  1   0  1  0\n") ;
    printf("-1  1 -1   0  1  0\n") ;
    printf(" 1  1 -1   0  1  0\n") ;

    return 0 ;
}
