// hev-glinfo

// steve@nist.gov
// July 27, 2009

#include <GL/glut.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char * argv[] )
{
    char *d;

    d=getenv("DISPLAY");
    if (d == NULL) {
      printf("DISPLAY: not defined\n");
      exit(0);
    }

    glutInit( &argc, argv ); // user can override options on the command line

    glutCreateWindow( "hev-glinfo" ); // opens a window: call -after- init
    
    printf("DISPLAY: %s\n",d);
    printf("GL_VERSION: %s\n",glGetString( GL_VERSION ));
    printf("GL_SHADING_LANGUAGE_VERSION: %s\n",glGetString( GL_SHADING_LANGUAGE_VERSION ));
    printf("GL_VENDOR: %s\n",glGetString( GL_VENDOR ));
    printf("GL_RENDERER: %s\n",glGetString( GL_RENDERER ));
    fflush(NULL);
    exit(0);

}
