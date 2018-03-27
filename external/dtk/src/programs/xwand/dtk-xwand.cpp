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
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <dtk.h>
#include <dtk/dtkDiffEq.h>
#include <dtk/dtkRK4.h>

#define PROGRAM_NAME "dtk-xwand"


#define DEFAULT_BUTTONS "buttons"
#define DEFAULT_JOYSTICK "joystick"


#define BUTTON_BITS_TERMINATOR      ((unsigned int) -1)
#define NO_XBUTTONS                 ((unsigned int) -2)

#define JOYSTICK_INDEX_UNSET        (-1)
#define NO_MOUSE_WARPING            (-2)


// Maximum rate in pixels/second given that the maxiumum joystick
// input is 1.
#define DEFAULT_RATE  (900.0f)
float rate = DEFAULT_RATE;

int running = 1;


// joystick is a buffer for the joystick values.
float joystick[2];


void difeq(float *xdot, const float *x, long double t)
{
  xdot[0] = rate*joystick[0]*joystick[0]*joystick[0];
  xdot[1] = -rate*joystick[1]*joystick[1]*joystick[1];
}

void signal_catcher(int sig)
{
  dtkMsg.add(DTKMSG_NOTICE, "caught signal %d ... exiting.\n",
	     sig);
  running = 0;
}

int Usage(void)
{
  dtkColor c(stdout);
  printf("\n"
	 "Usage: %s%s%s [%s-a%s|%s--activation-bit%s %sN%s] "
	 "[%s-b%s|%s--button-file%s %sFILE%s] "
	 "[%s-B%s|%s--button-bits%s %sN0%s [%sN1 ...%s]]\n    "
	 "[%s-j%s|%s--joystick-file%s %sFILE%s] "
	 "[%s-J%s|%s--joystick-indexes%s %sX Y%s] "
	 "[%s-r%s|%s--rate%s %sRATE%s]|"
	 "[%s-h%s|%s--help%s]\n"
	 "\n"
	 "  This can be used to move the X pointer and send X button events using a\n"
	 " wand, with buttons and joystick, in a CAVE or on an IDesk.\n"
	 "\n"
	 "  This is a single X server version of XWand.  It just moves the X pointer\n"
	 " using the first 2 floats in DTK shared memory file that contains the joystick\n"
	 " data.  The first float will determine the rate at which the pointer moves in\n"
	 " the X direction and the second will determine the rate at which the pointer\n"
	 " moves in the minus Y direction.  It uses bit state in the DTK shared memory for\n"
	 " sending real X buttons events.\n"
	 "\n",c.vil, PROGRAM_NAME, c.end, c.grn, c.end, c.grn, c.end, c.tur, c.end,
	 c.grn, c.end, c.grn, c.end, c.tur, c.end,
	 c.grn, c.end, c.grn, c.end, c.tur, c.end, c.tur, c.end,
	 c.grn, c.end, c.grn, c.end, c.tur, c.end,
	 c.grn, c.end, c.grn, c.end, c.tur, c.end,
	 c.grn, c.end, c.grn, c.end, c.tur, c.end,
	 c.grn, c.end, c.grn, c.end
	 );
  printf(
	 "          ARGUMENT OPTIONS\n"
	 "\n");
  printf(
	 "  %s-a%s|%s--activation-bit%s %sN%s  set the %sN%sth bit in the button shared memory to be\n"
	 "                      a value that toggles the action of this program to be\n"
	 "                      disabled and enabled. If the activation bit is not set\n"
	 "                      the action of this program will not is disabled and\n"
	 "                      enabled by any of the button bits.  Bit counting starts\n"
	 "                      at zero.\n"
	 "\n", c.grn, c.end, c.grn, c.end, c.tur, c.end, c.tur, c.end);
  printf(
	 "  %s-b%s|%s--button-file%s %sFILE%s  use the DTK shared memory that is in %sFILE%s for the\n"
	 "                      buttons.  The default button shared memory file is\n"
	 "                      \"%s\"\n"
	 "\n", c.grn, c.end, c.grn, c.end, c.tur, c.end, c.tur, c.end, DEFAULT_BUTTONS);
  printf(
	 "  %s-B%s|%s--button-bits%s %sN0 N1 ...%s  Tells which bits in DTK shared memory to use to\n"
	 "                      get the button state from.  Use the %sN0%s bit for the first\n"
	 "                      button, the %sN1%s bit for the second button, and so on. There\n"
	 "                      will be as many buttons as there are bits listed.  The\n"
	 "                      maximum number of buttons is 5 (X only has 5 buttons).  By\n"
	 "                      default there will be 3 buttons that use the first 3 bits.\n"
	 "                      Example `--button-bits 0 2 3' will use the first, third, and\n"
	 "                      forth bit that is in DTK shared memory for three mouse buttons.\n"
	 "                      If %sN0%s is a negitive integer the X button event sending will be\n"
	 "                      disabled.\n"
	 "\n", c.grn, c.end, c.grn, c.end, c.tur, c.end, c.tur, c.end, c.tur, c.end, c.tur, c.end);
  printf(
	 "  %s-h%s|%s--help%s           print this help than exit\n"
	 "\n", c.grn, c.end, c.grn, c.end);
  printf(
	 "  %s-j%s|%s--joystick-file%s %sFILE%s  use the DTK shared memory that is in %sFILE%s for\n"
	 "                      the joystick.  The default joystick shared memory file is\n"
	 "                      \"%s\".\n"
	 "\n", c.grn, c.end, c.grn, c.end, c.tur, c.end, c.tur, c.end, DEFAULT_JOYSTICK);
  printf(
	 "  %s-J%s|%s--joystick-indexes%s %sX Y%s  use the %sX%s float index in the float array that is\n"
	 "                      the data in shared memory.  By default it uses the first two\n"
	 "                      floats in the array. If %sX%s is a negitive integer the X pointer\n"
	 "                      moving will be disabled.\n"
	 "\n", c.grn, c.end, c.grn, c.end, c.tur, c.end, c.tur, c.end, c.tur, c.end);
  printf(
	 "  %s-r%s|%s--rate%s %sRATE%s      set the maximum pointer motion rate to %sRATE%s pixels/second,\n"
	 "                      that is assuming that the maximum joystick input value is one.\n"
	 "                      The rate at which the pointer moves is +/- %sRATE%s*J*J*J, where J\n"
	 "                      is the joystick value.  The default %sRATE%s is %.1f pixels/second.\n"
	 "\n", c.grn, c.end, c.grn, c.end, c.tur, c.end, c.tur, c.end, c.tur, c.end, c.tur, c.end, DEFAULT_RATE);
  return 1;
}

int parse_args(int argc, char **argv, char **joystickFile, char **buttonFile,
	       float *rate, unsigned int **button_bits, int *joystick_xy_index,
	       unsigned int *activation_bit)
{
  // Check for -h|--help first.
  int i=1;
  for(;i<argc;i++)
    if(!strcmp("-h",argv[i]) || !strcmp("--help",argv[i]))
      return Usage();

  // butt ugly parsing loop
  for(i=1;i<argc;)
    {
      char *str;
      if((str = dtk_getOpt("-j","--joystick-file",argc,(const char **)argv,&i)))
        *joystickFile=str;
      if((str = dtk_getOpt("-J","--joystick-indexes",argc,(const char **)argv,&i)))
	{
	  if(joystick_xy_index[0] != JOYSTICK_INDEX_UNSET)
	    {
	      dtkMsg.add(DTKMSG_ERROR, "bad \"-J|--joystick-indexes\" option\n"
			 "this option can only be set once\n");
	      return Usage();
	    }
	  if(i < argc && str[0] == '-')
	    {
	      joystick_xy_index[0] = NO_MOUSE_WARPING;
	      i++; // skip the Y option argument
	    }
	  else if(i < argc)
	    {
	      joystick_xy_index[0] = atoi(str);
	      joystick_xy_index[1] = atoi(argv[i++]);
	    }
	  else
	    {
	      dtkMsg.add(DTKMSG_ERROR, "bad \"-J|--joystick-indexes\" option:\n"
			 "missing Y.\n");
	      return Usage();
	    }
	}
      else if((str = dtk_getOpt("-b","--button-file",argc,(const char **)argv,&i)))
        *buttonFile=str;
      else if((str = dtk_getOpt("-B","--button-bits",argc,(const char **)argv,&i)))
	{
	  if((*button_bits)[0] != BUTTON_BITS_TERMINATOR) // been here before
	    {
	      dtkMsg.add(DTKMSG_ERROR, "Bad \"-B|--button-bits\" option:\n"
			 "this option cannot be used more than once.\n");
	      return Usage();
	    }
	  if(str[0] == '-') // means there will be no XButton events sent.
	    {
	      (*button_bits)[0] = NO_XBUTTONS;
	      // go to the next option argument skipping N1 N2 ...
	      while(i > argc && argv[i][0] != '-')
		i++;
	    }

	  (*button_bits) = (unsigned int *) dtk_realloc((*button_bits), 2*sizeof(unsigned int));
	  (*button_bits)[0] = atoi(str);
	  (*button_bits)[1] = BUTTON_BITS_TERMINATOR;
	  int j = 1;
	  for(;i < argc && argv[i][0] != '-' && j < 5;)
	    {
	      (*button_bits) = (unsigned int *) dtk_realloc((*button_bits), (j+2)*sizeof(unsigned int));
	      (*button_bits)[j] = atoi(argv[i++]);

	      // check for good values in this button bit
	      if((*button_bits)[j] > (unsigned int) -3)
		{
		  dtkMsg.add(DTKMSG_ERROR, "Bad \"-B|--button-bits\" option:\n"
			     "one or more bit values out of range.\n");
		  return Usage();
		}
	      // check that the activation_bit is not the same as a button_bit.
	      if(*activation_bit == (*button_bits)[j])
		{
		  dtkMsg.add(DTKMSG_ERROR,
			     "Bad combination of \"-B|--button-bits\" and \"-a|activation-bit\" options:\n"
			     "one or more of the button bits is the same as the activation bit.\n");
		  return Usage();
		}
	      int k=0;
	      for(;k < j; k++) // two or more values cannot be the same
		if((*button_bits)[k] == (*button_bits)[j])
		  {
		    dtkMsg.add(DTKMSG_ERROR, "Bad \"-B|--button-bits\" option:\n"
			       "two or more bit values are the same.\n");
		    return Usage();
		  }
	      (*button_bits)[++j] = BUTTON_BITS_TERMINATOR;
	    }
	  if(i < argc && j == 5 && argv[i][0] != '-')
	    {
	      dtkMsg.add(DTKMSG_ERROR, "Bad \"-B|--button-bits\" option:\n"
			 "there can only be up to 5 bits for the 5 X buttons.\n");
	      return Usage();
	    }
	}
      else if((str = dtk_getOpt("-r","--rate",argc,(const char **)argv,&i)))
        *rate=atof(str);
      else if((str = dtk_getOpt("-a","--activation-bit",argc,(const char **)argv,&i)))
	{
	  *activation_bit = atoi(str);
	  if(*activation_bit == BUTTON_BITS_TERMINATOR)
	    {
	      dtkMsg.add(DTKMSG_ERROR,
			 "Bad \"-a|activation-bit\" option:\n"
			 "the activation bit cannot be -1.\n");
	      return Usage();
	    }
	  int j;
	  // check that it is not the same as a button_bit.
	  for(j=0;(*button_bits)[j] != BUTTON_BITS_TERMINATOR;j++)
	    if(*activation_bit == (*button_bits)[j])
	      {
		dtkMsg.add(DTKMSG_ERROR,
			   "bad combination of \"-B|--button-bits\" and \"-a|activation-bit\" option:\n"
			   "one or more of the button bits is the same as the activation bit.\n");
		return Usage();
	      }
	}
      else
	{
	  dtkMsg.add(DTKMSG_ERROR, "bad option %s\n",
		     argv[i]);
	  return Usage();
	}
    }

  // setup the default button_bits if it's not set already
  if((*button_bits)[0] == BUTTON_BITS_TERMINATOR)
    {
      (*button_bits) = (unsigned int *) dtk_realloc((*button_bits), 4*sizeof(unsigned int));
      (*button_bits)[0] = 0;
      (*button_bits)[1] = 1;
      (*button_bits)[2] = 2;
      (*button_bits)[3] = BUTTON_BITS_TERMINATOR;

      // make sure that the activation_bit in not the same as a button
      // bit
      int j=0;
      for(;(*button_bits)[j] != BUTTON_BITS_TERMINATOR; j++)
	if(*activation_bit == (*button_bits)[j])
	  {
	    dtkMsg.add(DTKMSG_ERROR,
		       "Bad \"-a|activation-bit\" options:\n"
		       "one or more of the button bits is the same as the activation bit.\n");
	    return Usage();
	  }
    }
  // NO_XBUTTONS was just used to mark if the '-b|--button_bits'
  // option was present, since it differs from BUTTON_BITS_TERMINATOR,
  // but from here out it's not needed, we can use
  // BUTTON_BITS_TERMINATOR in it's place, in button_bits[0].
  else if((*button_bits)[0] == NO_XBUTTONS)
    (*button_bits)[0] = BUTTON_BITS_TERMINATOR;

  // setup the default joystick sahred memory input indexes
  if(joystick_xy_index[0] == JOYSTICK_INDEX_UNSET)
    {
      joystick_xy_index[0] = 0;
      joystick_xy_index[1] = 1;
    }
  else if(joystick_xy_index[0] == NO_MOUSE_WARPING)
    {
      joystick_xy_index[0] = JOYSTICK_INDEX_UNSET;
    }

  // If both the joystick and buttons are not used then there's no
  // point to running this program.
  if(joystick_xy_index[0] == JOYSTICK_INDEX_UNSET &&
     (*button_bits)[0] == BUTTON_BITS_TERMINATOR)
    {
      dtkMsg.add(DTKMSG_ERROR, "Your options request no X button "
		 "event generation or X mouse warping.\n");
      return Usage();
    }

  return 0;
}


int main(int argc, char **argv)
{
  dtkMsg.setPreMessage("%s%s%s", dtkMsg.color.vil,
                       PROGRAM_NAME, dtkMsg.color.end);

  char *joystickFile = DEFAULT_JOYSTICK;
  char *buttonFile = DEFAULT_BUTTONS;
  unsigned int *button_bits = (unsigned int *) dtk_malloc(sizeof(unsigned int));
  button_bits[0] = BUTTON_BITS_TERMINATOR; // -1 terminater of list of bit offsets
  int joystick_xy_index[2] = { JOYSTICK_INDEX_UNSET, 1 };
  unsigned int activation_bit = BUTTON_BITS_TERMINATOR;

  if(parse_args(argc, argv, &joystickFile, &buttonFile, &rate,
		&button_bits, joystick_xy_index, &activation_bit)) return 1;

  int is_active = (activation_bit != BUTTON_BITS_TERMINATOR)?0:1;

  Display *dpy;
  if ((dpy = XOpenDisplay(NULL)) == NULL)
    {
      dtkMsg.add(DTKMSG_ERROR, "XOpenDisplay(\"%s\") failed.\n",
		 NULL);
      return 1;
    }

  /*************************************************************************
   ****************** initialize the x mouse warping dynamics **************
   *************************************************************************/

  dtkSharedMem joystickShm(joystickFile);
  if(joystickShm.isInvalid()) return 1;

  if(joystick_xy_index[0] >= ((int) (joystickShm.getSize()/sizeof(float))) ||
     joystick_xy_index[0] < 0 ||
     joystick_xy_index[1] >= ((int) (joystickShm.getSize()/sizeof(float))) ||
     joystick_xy_index[1] < 0)
    return dtkMsg.add(DTKMSG_ERROR, 0, 1, "The DTK shared memory file "
		      "\"%s\" is no good (%d floats) for\n"
		      "float array with an index of %d and/or %d.\n",
		      joystickShm.getName(), joystickShm.getSize()/sizeof(float),
		      joystick_xy_index[0], joystick_xy_index[1]);
  joystickShm.setAutomaticByteSwapping(sizeof(float));

  float *joystick_in = (float *) dtk_malloc(joystickShm.getSize());

  dtkRK4<float,float> rk4(difeq, 2, 0.02);
  float xy[2] = { -2.0f, -2.0f };
  int x, y;
  dtkTime Time(0.0, 1);


  /*************************************************************************
   ****************** initialize X button event sending code ***************
   *************************************************************************/

  dtkSharedMem buttonShm(buttonFile);
  if(buttonShm.isInvalid() || buttonShm.queue()) return 1;

  unsigned char *button = (unsigned char *) dtk_malloc(buttonShm.getSize());
  unsigned char button_state = 0; // first 5 bits are used to store the
			      // button press values
  if(buttonShm.flush()<0 || buttonShm.read(button)) return 1; // error

  { // check the button_bits again given the size of the shared memory and
    // initialize the state of the buttons the is perceived here.

    int j=0;
    for(;button_bits[j] != BUTTON_BITS_TERMINATOR; j++)
      {
	if(button_bits[j] >= ((unsigned int) 8*buttonShm.getSize()))
	  return dtkMsg.add(DTKMSG_ERROR, 0, 1, "The DTK shared memory file \"%s\" "
			    "is too small for reading the %d-th bit, starting at index=0. \n"
			    "There are only %d bits total in the shared memory file.\n",
			    buttonShm.getName(), button_bits[j], 8*buttonShm.getSize());

	// set the jth bit, if button_bits[j] == 24 then set the jth bit to the 24th bit of button
	button_state |=
	  ((button[(button_bits[j]/8)] & (01 << (button_bits[j] % 8)))? (01 << j): 00);
      }
  }

  if(activation_bit != BUTTON_BITS_TERMINATOR &&
     activation_bit >=  ((unsigned int) 8*buttonShm.getSize()))
    return dtkMsg.add(DTKMSG_ERROR, 0, 1, "The DTK shared memory file \"%s\" "
		      "is too small for reading the %d-th bit as the "
		      "activation bit, starting at index=0.\n"
		      "There are only %d bits total in the shared memory file.\n",
		      buttonShm.getName(), activation_bit, 8*buttonShm.getSize());

  /**********************************************************************************
   **********************************************************************************
   **********************************************************************************/

  //printf("initial button state= 0%o shm=%d\n", button_state, button[0]);

  signal(SIGTERM, signal_catcher);
  signal(SIGINT, signal_catcher);
  signal(SIGQUIT, signal_catcher);

  // run at 50 Hz
  dtkRealtime_init(0.02); // interval timer and Alarm wrapper

  Window root_window = DefaultRootWindow(dpy);

  int was_active = (is_active)?0:1;
  int was_set = 0;

  /**********************************************************************************
   ********************************* main loop **************************************
   **********************************************************************************/

  while(running)
    {
      if(!is_active)
	{
	  if(was_active != is_active) // just became inactive
	    {
	      dtkMsg.add(DTKMSG_NOTICE,"%s is now inactive.\n",
			 PROGRAM_NAME);
	      was_active = is_active;
	    }

	  // look for a activation bit being unset
	  if(was_set)
	    {
	      int i = buttonShm.qread(button);
	      while(i > 0)
		{
		  if(!(button[(activation_bit/8)] & (01 << (activation_bit % 8))))
		    {
		      was_set = 0;
		      break;
		    }
		  i = buttonShm.qread(button);
		}
	      if(i<0) return 1; // error: buttonShm.qread(button) failed
	    }
	  else // look for activation bit being set
	    {
	      int i = buttonShm.qread(button);
	      while(i > 0)
		{
		  if((button[(activation_bit/8)] & (01 << (activation_bit % 8))))
		    {
		      is_active = 1;
		      break;
		    }
		  i = buttonShm.qread(button);
		}
	      if(i<0) return 1; // error: buttonShm.qread(button) failed
	    }
	}
      else // is_active is TRUE
	{
	  if(was_active != is_active) // just became active
	    {
	      dtkMsg.add(DTKMSG_NOTICE,"%s is now active.\n",
			 PROGRAM_NAME);
	      was_active = is_active;

	      // reset the X button stuff
	      if(button_bits[0] != BUTTON_BITS_TERMINATOR)
		{
		  if(buttonShm.flush() < 0) return 1; // error
		  int j=0;
		  for(;button_bits[j] != BUTTON_BITS_TERMINATOR; j++)
		    button_state |=
		      ((button[(button_bits[j]/8)] & (01 << (button_bits[j] % 8)))? (01 << j): 00);
		}

	      // reset the mouse warping dynamics
	      if(joystick_xy_index[0] != JOYSTICK_INDEX_UNSET)
		{
		  x = ((int) xy[0]) + 3;
		  rk4.setCurrentTime(Time.get());
		}
	    }

	  /*************************************************************/
	  /********** First do button presses *************************/
	  /*************************************************************/
	  if(button_bits[0] != BUTTON_BITS_TERMINATOR)
	    {
	      int i = buttonShm.qread(button);
	      while(i > 0)
		{
		  unsigned int j;
		  for(j=0;button_bits[j] != BUTTON_BITS_TERMINATOR;j++)
		    {
		      // compare the state of the jth button that is local to
		      // that which is in shared memory at the button_bits[j]
		      // bit.
		      unsigned int bit_mask = (01 << j);
		      unsigned int shm_bit_state =
			((button[(button_bits[j]/8)] & (01 << (button_bits[j] % 8)))? bit_mask: 00);
		      
		      if((unsigned int) (button_state & bit_mask) != shm_bit_state)
			{
			  int XButton[] = {Button1, Button2, Button3, Button4, Button5};
			  if(shm_bit_state) // It's pressed set the bit
			    button_state |= bit_mask;
			  else // It's not pressed unset the bit
			    button_state &= ~bit_mask;
			  if(!XTestFakeButtonEvent(dpy, XButton[j], shm_bit_state?01:00, CurrentTime))
			    return dtkMsg.add(DTKMSG_ERROR, 0, 1, "XTestFakeButtonEvent() failed.\n");
			  XFlush(dpy);
			}
		    }
		  // look for activation bit being set
		  if(activation_bit != BUTTON_BITS_TERMINATOR &&
		     (button[(activation_bit/8)] & (01 << (activation_bit % 8))))
		    {
		      is_active = 0;
		      was_set = 1;
		      break;
		    }
		  i = buttonShm.qread(button);
		}
	      if(i<0) return 1; // error: buttonShm.qread(button) failed
	    }

	  /*************************************************************/
	  /***** Now do pointer motion based on joystick input *********/
	  /*************************************************************/
	  if(joystick_xy_index[0] != JOYSTICK_INDEX_UNSET)
	    {
	      if(joystickShm.read(joystick_in)) return 1; // error
	      joystick[0] = joystick_in[joystick_xy_index[0]];
	      joystick[1] = joystick_in[joystick_xy_index[1]];
	      
	      
	      Window root, w;
	      int wx, wy;
	      unsigned int m;
	      if(!XQueryPointer(dpy, root_window, &root, &w,
				&x, &y, &wx, &wy, &m))
		return dtkMsg.add(DTKMSG_ERROR, 0, 1, "XQueryPointer() failed.\n");

	      // If this seems to not be the only thing moving the
	      // mouse reset the (float to int) converted values.  We
	      // use floats for the dynamical state to try to keep
	      // continuity in the motion, using integers can make it
	      // jurky.
	      if(x != (int) xy[0] && y != (int) xy[1])
		{
		  xy[0] = x;
		  xy[1] = y;
		}
	      
	      rk4.go(xy, Time.get());
	      int dx = ((int) xy[0]) - x;
	      int dy = (((int) xy[1]) - y);
	      
	      if((joystick[0] != 0.0f || joystick[1] != 0.0f) &&
		 !XWarpPointer(dpy, None, None, 0,0, 0,0, dx, dy))
		return dtkMsg.add(DTKMSG_ERROR, 0, 1, "XWarpPointer() failed.\n");
	      XFlush(dpy);
	    }
	}
      
      // wait (like sleep() but better) for next interval in the 50 Hz cycle
      dtkRealtime_wait();
    }

  dtkRealtime_close();
}
