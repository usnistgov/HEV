/*
 * leftRightEyeTest.c - based on plane.c
 *
 * Modifed by Steve Satterfield December 19, 2014
 *
 * Use as a simple stereo test for eye swap error.
 *
 * I added left eye and right eye text strings.
 *
 * While viewing with glasses, close left eye to see magenta "Right Eye",
 * close right eye to see cyan "Left Eye".
 * With one eye closed, stereo of rotating paper airplane goes flat.
 * With both eyes open, you should not see double lines.
 *
 * Added an undocumented (in man page) "-mono" flag to run in mono
 * to allow debugging on non-stereo desk top machine.
 *
 * 4/21/2017
 * Added an undocumented (in man page) "-window" flag to run in small
 * window instead of full screen.
 *
 */


/*
 *
 * plane.c (OpenGL version)
 * Draws rotating paper airplane in stereo. 
 *
 * Converted to use OpenGL Utility Toolkit
 * Rick Hammerstone, Digital, December 1996.
 *
 * Converted to OpenGL by Silicon Graphics Corp. 4Dgifts program tool.
 * Further modified by James S. Lipscomb and Keh-Shin Cheng, IBM Resrch,
 * March 1995. Uses Motif.
 *
 * Original program in GL by Robert Akka
 * StereoGraphics Corporation * April 2, 1991
 *
 * Compile with:
 * cc -o plane plane.c -lglut -lGLU -lGL -lXmu -lXi -lXext -lX11 -lm
 *
 * Hit escape to stop program. */ 

#include <stdio.h> 
#include <string.h> 
#include <math.h> 

#include <GL/glut.h>
#include <GL/freeglut_ext.h>

/* * Speed of rotation in degrees per update. */ 

#define VELOCITY -0.2
float yAngle = 0;

GLenum rgb, doubleBuffer;

int flip=0;
int flop=0;
const char *leftFront = "Left Front";
const char *rightFront = "Right Front";
const char *leftBack = "Left Back";
const char *rightBack = "Right Back";
static GLfloat g_rotate = 0;
int mono=0;
int window=0;


void Reshape(int width, int height) 
{
     glViewport(0, 0, width, height);
}

void Key(unsigned char key, int x, int y) 
{
     if (key == 27) exit(0);
}

void Init()
{
     glMatrixMode(GL_PROJECTION);
}





/*
 * drawText
 *
 * Draws the specified message in stroke text
 *
 */
void drawText(const char * message)
{
  glLineWidth(4);

  while (*message) {
    glutStrokeCharacter(GLUT_STROKE_ROMAN,*message++);
  }
}





void DrawAirplane() 
{
     static float airplane[9][3] = {
     { 0.0, 0.5, -4.5},
     { 3.0, 0.5, -4.5},
     { 3.0, 0.5, -3.5}, 
     { 0.0, 0.5, 0.5}, 
     { 0.0, 0.5, 3.25}, 
     { 0.0, -0.5, 5.5}, 
     {-3.0, 0.5, -3.5}, 
     {-3.0, 0.5, -4.5}, 
     { 0.0, -0.5, -4.5} 
};

     glPushMatrix();
     glScalef(0.008, 0.008, 0.008);
     glRotatef(g_rotate, 0, 0, 1.0);


     if (flop == 0) {
       if (flip == 0) {
	 flop=0;  flip=1;
	 glClearColor(.0, 0.0, 1.0,  0.0);
	 glClear(GL_COLOR_BUFFER_BIT);

	 glTranslatef(-700, 400, 0.0);
	 glColor3f(0.00, 1.00, 1.00);
	 drawText(leftFront);
	 //printf("leftFront    ");
       } else {
	 flop=1;   flip=0;
	 glClearColor(1.0, 0.0, 0.0,  0.0);
	 glClear(GL_COLOR_BUFFER_BIT);

	 glTranslatef(50, 400, 0.0);
	 glColor3f(1.00, 0.00, 1.00);
	 drawText(rightFront);
	 //printf("rightFront  ");
       }
     } else {
       if (flip == 0) {
	 flop=1;   flip=1;
	 glClearColor(0.0, 0.0, 1.0,  0.0);
	 glClear(GL_COLOR_BUFFER_BIT);

	 glTranslatef(-700, 000, 0.0);
	 glColor3f(1.00, 1.00, 0.00);
	 drawText(leftBack);
	 //printf("leftBack   \n");
       } else {
	 flop=1;   flip=0;
	 glClearColor(1.0, 0.0, 0.0,  0.0);
	 glClear(GL_COLOR_BUFFER_BIT);

	 glTranslatef(50, 000, 0.0);
	 glColor3f(1.00, 1.00, 1.00);
	 drawText(rightBack);
	 //printf("rightBack   \n");
       }
     }
     glPopMatrix();
}




void Plane(float yAngle) 
{
     //glRotatef(yAngle, 0, 1, 0);

     //glRotatef(-10, 1, 0, 0);
     DrawAirplane();
}

void StereoProjection(float left, float right, float bottom, float top, float near, float far, float   
                       zero_plane, float dist, float eye)

/* Perform the perspective projection for one eye's subfield. The projection is in the direction
of the negative z axis. -6.0, 6.0, -4.8, 4.8, left, right, bottom, top = the coordinate range, in
the plane of zero parallax setting, which will be displayed on the screen. The ratio between
(right-left) and (top-bottom) should equal the aspect ratio of the display.
6.0, -6.0, near, far = the z-coordinate values of the clipping planes. 0.0, zero_plane = the
z-coordinate of the plane of zero parallax setting. 14.5, dist = the distance from the center of
projection to the plane of zero parallax. -0.31 eye = half the eye separation;
 positive for the right eye subfield, negative for the left eye subfield. */ 

{
     float xmid, ymid, clip_near, clip_far, topw, bottomw, leftw, rightw, dx, dy, n_over_d;
     dx = right - left;
     dy = top - bottom;
     xmid = (right + left) / 2.0;
     ymid = (top + bottom) / 2.0;
     clip_near = dist + zero_plane - near;
     clip_far = dist + zero_plane - far;
     n_over_d = clip_near / dist;
     topw = n_over_d * dy / 2.0;
     bottomw = -topw;
     rightw = n_over_d * (dx / 2.0 - eye);
     leftw = n_over_d *(-dx / 2.0 - eye);
          /* Need to be in projection mode for this. */ 
     glLoadIdentity();
     glFrustum(leftw, rightw, bottomw, topw, clip_near, clip_far);
     glTranslatef(-xmid - eye, -ymid, -zero_plane - dist);
}

void DrawScene(void) 
{
     glDrawBuffer(doubleBuffer ? GL_BACK : GL_FRONT);
     //     glClearColor(0.0, 0.0, 0.0,  0.0);
     //     glClear(GL_COLOR_BUFFER_BIT);

     glDrawBuffer(doubleBuffer ? GL_BACK_LEFT : GL_FRONT_LEFT);
     glPushMatrix();
     StereoProjection(-6.0, 6.0, -4.8, 4.8, 6.0, -6.0, 0.0, 14.5, -0.31);
     Plane(yAngle);
     glPopMatrix();

     glDrawBuffer(doubleBuffer ? GL_BACK_RIGHT : GL_FRONT_RIGHT);
     glPushMatrix();
     StereoProjection(-6.0, 6.0, -4.8, 4.8, 6.0, -6.0, 0.0, 14.5, 0.31);
     Plane(yAngle);
     glPopMatrix();

     if (doubleBuffer) glutSwapBuffers();
     else glFlush();
     yAngle -= VELOCITY;
     if (yAngle < 0) yAngle = 360.0 + yAngle;
           /* degrees */ 
}

GLenum Args(int argc, char **argv) 
{
     GLint i;
     rgb = GL_TRUE;

     doubleBuffer = GL_TRUE;
     for (i = 1;i < argc; i++) 
     {
          if (strcmp(argv[i], "-ci") == 0) 
          {
          rgb = GL_FALSE;
          }

          else if (strcmp(argv[i], "-rgb") == 0) 
          {
          rgb = GL_TRUE;
          }

          else if (strcmp(argv[i], "-sb") == 0) 
          {
          doubleBuffer = GL_FALSE;
          }

          else if (strcmp(argv[i], "-db") == 0) 
          {
          doubleBuffer = GL_TRUE;
          }

          else if (strcmp(argv[i], "-mono") == 0)  // Add for debuging at desk
          {
	    mono=1;
          }

          else if (strcmp(argv[i], "-window") == 0)  // Add for debuging at desk
          {
	    window=1;
          }

          else 
	  {
          printf("%s (Bad option).\n", argv[i]);
          return GL_FALSE;
          }
 
     }
     return GL_TRUE;
}

main(int argc, char **argv) 
{
  float wx=1000, wy=1000;
     GLenum type;
     glutInit(&argc, argv);

     Args(argc, argv);

     if (mono) {
       type = 0;
     } else {
       type = GLUT_STEREO;
     }
     type |= (rgb) ? GLUT_RGB : GLUT_INDEX;
     type |= (doubleBuffer) ? GLUT_DOUBLE : GLUT_SINGLE;
     type |= GLUT_BORDERLESS;
     type |= GLUT_CAPTIONLESS;
     glutInitDisplayMode(type);

     wx=300;
     wy=200;
     glutInitWindowSize(wx, wy);

     glutCreateWindow("LeftRightEyeTest");
     if (window==0) {
       glutFullScreen();
     }
  

     Init();
     glutReshapeFunc(Reshape);
     glutKeyboardFunc(Key);
     glutIdleFunc(DrawScene);
     glutDisplayFunc(DrawScene);
     glutMainLoop();
}
