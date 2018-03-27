/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2003  Virginia Tech
 * 
 * This is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License (GPL) as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software, in the top level source directory in a file
 * named "COPYING.GPL"; if not, see it at:
 * http://www.gnu.org/copyleft/gpl.html or write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 */
/* This file was originally written by Eric Tester.  Many
 * modifications have been made by Lance Arsenault.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <dtk/_config.h>

#include <FL/gl.h>
#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#ifdef DTK_ARCH_DARWIN
# include <OpenGL/glu.h>
#else
# include <GL/glu.h>
#endif
#include "gnomonWindow.h"

#define PI  (3.141592654f)


static GLfloat mat_specular[] = { 2.0, 2.0, 2.0, 1.0 };
static GLfloat mat_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat mat_ambient[] = { .5, .5, .5, 1.0 };
static GLfloat mat_shininess[] = { 70.0 };
//static GLfloat mat_emission [] = { 0.3, 0.2, 0.2, 0.0 };

static GLfloat light_position[] = { 100.0, 100.0, 100.0, 0.0 };
static GLfloat white [] = { 1.0, 1.0, 1.0, 0.5 };
static GLfloat blue [] = { 0.0, 0.0, 1.0, 1.0 };
static GLfloat yellow [] = { 1.0, 1.0, 0.0, 0.0 };
static GLfloat red [] = { 1, 0.1, 0.05, 1.0 }; 
static GLfloat green [] = { 0.0, 1.0, 0.0, 0.0 };

GnomonWindow::GnomonWindow(int x, int y, int w, int h): Fl_Gl_Window(x,y,w,h)
{
  xyzhpr[0] = 0.0f; xyzhpr[1] = 0.0f; xyzhpr[2] = 0.0f;
  xyzhpr[3] = 0.0f; xyzhpr[4] = 0.0f; xyzhpr[5] = 0.0f;

  quad = 0;

  Vert = Horz = PI/2.0f;
  Scale = 2.0f;

  // coordinates for the box
  boxv0[0] = -.5f; boxv0[1] = -.5f; boxv0[2] = -.5f;
  boxv1[0] =  .5f; boxv1[1] = -.5f; boxv1[2] = -.5f;
  boxv2[0] =  .5f; boxv2[1] =  .5f; boxv2[2] = -.5f;
  boxv3[0] = -.5f; boxv3[1] =  .5f; boxv3[2] = -.5f;
  boxv4[0] = -.5f; boxv4[1] = -.5f; boxv4[2] =  .5f;
  boxv5[0] =  .5f; boxv5[1] = -.5f; boxv5[2] =  .5f;
  boxv6[0] =  .5f; boxv6[1] =  .5f; boxv6[2] =  .5f;
  boxv7[0] = -.5f; boxv7[1] =  .5f; boxv7[2] =  .5f;

  // coordinates for the skinny axis in the center of the box
  axisXR[0]= 0.5f;    axisXR[1]= 0.0f;  axisXR[2]=  0.0f;
  axisXL[0]= 0.0f;    axisXL[1]= 0.0f;  axisXL[2]=  0.0f;
  axisZT[0]= 0.0f;    axisZT[1]= 0.5f;  axisZT[2]=  0.0f;
  axisZB[0]= 0.0f;    axisZB[1]= 0.0f;  axisZB[2]=  0.0f;
  axisYF[0]= 0.0f;    axisYF[1]= 0.0f;  axisYF[2]=  0.0f;
  axisYB[0]= 0.0f;    axisYB[1]= 0.0f;  axisYB[2]= -0.5f;
}

void GnomonWindow::initQuad()
{
  quad = gluNewQuadric();
}

// draws the XYZ axis
void GnomonWindow::drawAxis()
{
#ifdef DTK_ARCH_DARWIN
  gluDeleteQuadric( quad );
  quad = gluNewQuadric();
#endif

  glEnable(GL_DEPTH_TEST);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glEnable(GL_LIGHTING);
  glMaterialfv (GL_FRONT, GL_DIFFUSE, mat_diffuse);
  
  // X 
  glPushMatrix();
  glMaterialfv (GL_FRONT, GL_AMBIENT, red);
  glPushMatrix();
  glRotatef(90,0,1,0);
  gluCylinder (quad,.03,.04,.8, 15,15); 
  glPopMatrix();    
  glTranslatef (.8,0,0);
  glRotatef (90, 0,1,0);
  gluCylinder (quad,.1,0,.2,15,15);
  glPopMatrix();
  
  // Y
  glPushMatrix();
  glMaterialfv (GL_FRONT, GL_AMBIENT, green);
  glPushMatrix();
  glRotatef(180,1,0,0);
  gluCylinder (quad,.03,.04,.8, 15,15); 
  glPopMatrix();
  glTranslatef (0,0,-.8);
  glRotatef (180, 1,0,0);
  gluCylinder (quad,.1,0,.2, 15,15);
  glPopMatrix();
  
  // Z
  glPushMatrix();
  glMaterialfv (GL_FRONT, GL_AMBIENT, blue);
  glPushMatrix();
  glRotatef(-90,1,0,0);
  gluCylinder (quad,.03,.04,.8, 15,15); 
  glPopMatrix();   
  glTranslatef (0,.8,0);
  glRotatef(-90,1,0,0);
  gluCylinder (quad,.1,0,.2, 15,15);
  glPopMatrix();
  
  // sphere
  glPushMatrix();
  glMaterialfv (GL_FRONT, GL_AMBIENT, yellow); 
  gluSphere(quad, .08,15,15);
  glPopMatrix();
  
  glDisable(GL_LIGHTING);
  glFlush();
}


// draws the cube
void GnomonWindow::drawCube() {

  glShadeModel(GL_SMOOTH);
  glPushMatrix ();
    glLineWidth(2);
    glBegin(GL_LINES);
   
      glColor4f(1.0, 0.0, 0.0, .5);
      glVertex3fv(axisXR);
      glVertex3fv(axisXL);
      glColor4f(0.0, 0.0, 1.0, .5);
      glVertex3fv(axisZT);
      glVertex3fv(axisZB);
      glColor4f(0.0, 1.0, 0.0, .5);
      glVertex3fv(axisYF);
      glVertex3fv(axisYB);

      glColor4fv(white);   // right
      glVertex3fv(boxv0);
      glVertex3fv(boxv1);
      glColor4fv(white);
      glVertex3fv(boxv1);
      glVertex3fv(boxv2);
		glColor4fv(white);
      glVertex3fv(boxv2);
      glVertex3fv(boxv3);
		glColor4fv(white);   // vertical
      glVertex3fv(boxv3);
      glVertex3fv(boxv0);
		glColor4fv(white);
      glVertex3fv(boxv4);
      glVertex3fv(boxv5);
		glColor4fv(white);
      glVertex3fv(boxv5);
      glVertex3fv(boxv6);
		glColor4fv(white);
      glVertex3fv(boxv6);
      glVertex3fv(boxv7);
		glColor4fv(white);
      glVertex3fv(boxv7);
      glVertex3fv(boxv4);
		glColor4fv(white);   // left
      glVertex3fv(boxv0);
      glVertex3fv(boxv4);
		glColor4fv(white);
      glVertex3fv(boxv1);
      glVertex3fv(boxv5);
		glColor4fv(white);
      glVertex3fv(boxv2);
      glVertex3fv(boxv6);
		glColor4fv(white);
      glVertex3fv(boxv3);
      glVertex3fv(boxv7);
    glEnd();
  glPopMatrix();
}

// draws the GL display
void GnomonWindow::draw()
{
  if (!valid())
    {
      //printf("file=%s line=%d\n",__FILE__, __LINE__);
      
      // lighting and material stuff
      glClearColor ((GLclampf) 0.0f, (GLclampf) 0.0f,
		    (GLclampf) 0.0f, (GLclampf) 0.0f);
      //printf("file=%s line=%d\n",__FILE__, __LINE__);
      glShadeModel(GL_SMOOTH);
      glEnable(GL_DEPTH_TEST);
      
      //glMaterialfv (GL_FRONT, GL_SPECULAR, mat_specular);
      glMaterialfv (GL_FRONT, GL_SHININESS, mat_shininess);
      //glLightfv (GL_LIGHT0, GL_POSITION, light_position);
      
      //glEnable(GL_LIGHTING);
      //glEnable(GL_COLOR_MATERIAL);
      //glEnable(GL_LIGHT0);
      // resetView();
      //printf("file=%s line=%d\n",__FILE__, __LINE__);
    }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // does all calculations for the view changes
  glMatrixMode(GL_PROJECTION);
     glLoadIdentity();
     float x = (Scale*sin(Vert)*cos(Horz));
     float y = (Scale*sin(Vert)*sin(Horz));
     float z = (Scale*cos(Vert));
     float ww = (float)w(); float hh = (float)h();
     glViewport((GLint) 0, (GLint) 0,(GLint) ww, (GLint) hh);
     gluPerspective(90, (ww/hh), .1, 100);

	gluLookAt (x,-z,y,0,0,0,0, 1,0);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   
   // light stuff
   glPushMatrix();
     glPushMatrix();
       glLightfv (GL_LIGHT0, GL_POSITION, light_position);
       glLightfv (GL_LIGHT0, GL_AMBIENT, mat_ambient);
       glLightfv (GL_LIGHT0, GL_DIFFUSE, mat_diffuse);
       glLightfv (GL_LIGHT0, GL_SPECULAR, mat_specular);
       glEnable (GL_LIGHT0);	
     glPopMatrix();
     
	  // drawing display		 
     glPushMatrix();
       glTranslatef(xyzhpr[0],0,0);
		 glTranslatef(0,0,-xyzhpr[1]);
		 glTranslatef(0,xyzhpr[2],0);

   	 glRotatef(xyzhpr[3],0,1,0);
   	 glRotatef(xyzhpr[4],1,0,0);
   	 glRotatef(xyzhpr[5],0,0,-1);
   	 drawAxis();
     glPopMatrix();
     glScalef(2,2,2);
     drawCube();
   glPopMatrix();
   glFlush();
}

// resets view to look down the -Y axis
void GnomonWindow::resetView()
{
   Vert = Horz = PI/2.0f;
   Scale = 2.0f;
   glMatrixMode(GL_PROJECTION);
 	  glLoadIdentity();
	  float ww = (float)w();
	  float hh = (float)h();
	  glViewport(0,0,(int) ww,(int) hh);
	  gluPerspective(90, (ww/hh), .1, 100);
	  gluLookAt (0,0,0,0,0,0, 0, 1,0);
     glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glFlush();
   
   glPushMatrix();
     drawAxis();
     drawCube();
   glPopMatrix();
   redraw();
}
