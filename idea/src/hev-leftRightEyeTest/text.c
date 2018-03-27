/* text.c  */

/* Simple program demonstrates the use of text using GLUT and OpenGL
 * 
 * Also demonstrates the use of reShape, mouse, and keyboard
 * callback functions
 */


/* Jon McCormack, April 2003
 * Updated 20 March 2004: added extra callbacks
 * Updated 29 March 2004: removed platform dependencies
 */

#include <stdlib.h>
#include <stdio.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define MESSAGE		"Underworld live 17.03.04"

/*
 * local static variables: g_rotate used to keep track of
 * global rotation
 *
 */
static GLfloat g_rotate = 0;

/*
 * drawText
 *
 * Draws the specified message in both
 * raster and stroke text
 *
 */
void drawText(const char * message)
{
	/* raster pos sets the current raster position
	 * mapped via the modelview and projection matrices
	 */
	glRasterPos2f((GLfloat)0, (GLfloat)-400);

	/*
	 * write using bitmap and stroke chars
	 */
	while (*message) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *message);
		glutStrokeCharacter(GLUT_STROKE_ROMAN,*message++);
	}
}

/*
 * display
 *
 * This function is called by the GLUT to display the graphics
 * In this case it clears the screen then calls the function "drawText"
 * which will draw some text using GLUT
 *
 */
void display(void)
{
	/* set matrix mode to modelview */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* transformations to scale the stroke text -
     * notice how these only change stroke and raster text
	 * differently...
     */
    glScalef(0.001, 0.001, 0.001);
    glRotatef(g_rotate, 0, 0, 1.0);
    /* glTranslatef(-180, -180, 0.0); */

    glClear( GL_COLOR_BUFFER_BIT );
 	drawText(MESSAGE);

 	glFlush(); /* force OpenGL output */
}


/*
 * myReshape
 *
 * This function is called whenever the user (or OS) reshapes the
 * OpenGL window. The GLUT sends the new window dimensions (x,y)
 *
 */
void myReshape(int w, int h)
{
	/* set viewport to new width and height */
	/* note that this command does not change the CTM */
    glViewport(0, 0, w, h);

	/* 
	 * set viewing window in world coordinates 
	 */
    glMatrixMode(GL_PROJECTION); 
    glLoadIdentity(); /* init projection matrix */

    if (w <= h)
        glOrtho(-2.0, 2.0, -2.0 * (GLfloat) h / (GLfloat) w,
            2.0 * (GLfloat) h / (GLfloat) w, -1.0, 1.0);
    else
        glOrtho(-2.0 * (GLfloat) w / (GLfloat) h,
            2.0 * (GLfloat) w / (GLfloat) h, -2.0, 2.0, -1.0, 1.0);

	/* set matrix mode to modelview */
    glMatrixMode(GL_MODELVIEW);
}

/*
 * myKey
 *
 * responds to key presses from the user
 */
void myKey(unsigned char k, int x, int y)
{
	switch (k) {
		case 'q':
		case 'Q':	exit(0);
		break;
	default:
		printf("Unknown keyboard command \'%c\'.\n", k);
		break;
	}
}


/*
 * myMouse
 *
 * function called by the GLUT when the user presses a mouse button
 *
 * Here we increment the global rotation with each press - left to do a
 * positive increment, right for negative, middle to reset
 */
void myMouse(int btn, int state, int x, int y)
{   

    if(btn==GLUT_LEFT_BUTTON && state == GLUT_DOWN) g_rotate += 20.0;
	if(btn==GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) g_rotate = 0.0;
	if(btn==GLUT_RIGHT_BUTTON && state == GLUT_DOWN) g_rotate -= 20.0;

	/* force redisplay */
	glutPostRedisplay();
}   



/*
 * main
 *
 * Initialization and sets graphics callbacks
 *
 */
int main(int argc, char **argv)
{
	/* glutInit MUST be called before any other GLUT/OpenGL calls */
    glutInit(&argc, argv);

	/* need both double buffering and z buffer */

    glutInitDisplayMode(GLUT_RGB);
    glutInitWindowSize(500, 500);
    glutCreateWindow("Jon Mc Text Test");

	/* set callback functions */
    glutReshapeFunc(myReshape);
    glutDisplayFunc(display);
	glutIdleFunc(NULL);
	glutKeyboardFunc(myKey);
	glutMouseFunc(myMouse);

	/* set clear colour */
	glClearColor(1.0, 1.0, 1.0, 1.0);

	/* set current colour to black */
	glColor3f(0.0, 0.0, 0.0);

    glutMainLoop();

	return 0;
}
