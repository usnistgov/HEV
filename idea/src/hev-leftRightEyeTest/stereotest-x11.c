/*
 * adopted from SGI's /usr/share/src/OpenGL/teach/xlib/too-simple.c
 *
 * by Greg Couch, UCSF Computer Graphics Lab, 2016
 *
 * compile with:  cc file.c -lGLU -lGL -lX11
 */

#include <stdio.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <unistd.h>

unsigned int width = 200, height = 200;

static int attributeList[] = { 
	GLX_LEVEL, 0,
	GLX_DOUBLEBUFFER,
	GLX_RGBA,
	GLX_RED_SIZE, 1,
	GLX_GREEN_SIZE, 1,
	GLX_BLUE_SIZE, 1,
	GLX_STEREO,
	None
};

static Bool
WaitForNotify(Display *dpy, XEvent *e, XPointer arg)
{
	return (e->type == MapNotify) && (e->xmap.window == (Window) arg);
}

static void
draw(Display *dpy, Window win, GLXContext cx)
{
	/* connect the context to the window */
	glXMakeCurrent(dpy, win, cx);

	glViewport(0, 0, width, height);

	/* draw left eye */
	glDrawBuffer(GL_BACK_LEFT);
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/* draw right eye */
	glDrawBuffer(GL_BACK_RIGHT);
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/* make visible */
	glXSwapBuffers(dpy, win);
}

int
main(int argc, char **argv)
{
	Display		*dpy;
	XVisualInfo	*vi;
	Colormap	cmap;
	XSetWindowAttributes	swa;
	Window		win;
	GLXContext	cx;
	XEvent		event;
	GLboolean	isStereo;
	GLenum		error;

	/* get a connection */
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "unable to get connection to X server\n");
		return 1;
	}

	/* get an appropriate visual */
	vi = glXChooseVisual(dpy, DefaultScreen(dpy), attributeList);
	if (vi == NULL) {
		fprintf(stderr, "unable to get stereo visual\n");
		return 1;
	}
	printf("visualid = 0x%lx\n", (unsigned long) vi->visualid);

	/* create a GLX context */
	cx = glXCreateContext(dpy, vi, NULL, True);

	/* create a color map */
	cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual,
		AllocNone);

	/* create a window */
	swa.colormap = cmap;
	swa.border_pixel = 0;
	swa.event_mask = StructureNotifyMask|KeyPressMask|ButtonPressMask;
	win = XCreateWindow(dpy, RootWindow(dpy, vi->screen), 0, 0, width, height,
		0, vi->depth, InputOutput, vi->visual,
		CWBorderPixel|CWColormap|CWEventMask, &swa);
	XMapWindow(dpy, win);
	XIfEvent(dpy, &event, WaitForNotify, (XPointer) win);

	printf("Double check that the context says it is stereo:\n");
	printf("  the following line should print '.... = 1'\n");
	glGetBooleanv(GL_STEREO, &isStereo);
	printf("  glGetBoolean(GLX_STEREO) = %d\n", isStereo);
	printf("Left eye is red.\n");
	printf("Right eye is blue.\n");
	printf("Together they are magneta.\n");
	/*
	printf("Press any key or mouse button to exit.\n");
	*/

	draw(dpy, win, cx);
	while ((error = glGetError()) != GL_NO_ERROR)
		printf("%s\n", gluErrorString(error));

	for (;;) {
		XNextEvent(dpy, &event);
		switch (event.type) {
		  /*
			case ButtonPress:
			case KeyPress:
			exit(0);
		  */

			case ConfigureNotify:
				width = event.xconfigure.width;
				height = event.xconfigure.height;
				break;

			case Expose:
				draw(dpy, win, cx);
				break;
		}
	}
	return 0;
}
