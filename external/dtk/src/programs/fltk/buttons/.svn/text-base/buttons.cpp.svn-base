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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Button.H>
#include <dtk.h>

#include "buttons.h"

// from dtk-buttons.C
extern int running;
extern dtkSharedMem *shm;
extern unsigned char *buffer;
// buffer2 is NULL unless (number of buttons)*8 != shared memory size
extern unsigned char *buffer2; 
extern int num_bits; // number of bits in shared memory
extern int returnVal;

static int numberButtons = 0;
static int numberBytes = 0;
static Fl_Light_Button **button = NULL;

static size_t bytes;
static int bit_mask;

// put a value in DTK shared memory
// or print to stdout.

static void put_values(Fl_Widget *w=NULL, void *data=NULL)
{
  if(shm)
    {
      if(!buffer2)
	{
	  if(shm->write(buffer)) {returnVal=1; running = 0;}
	}
      else // The number of buttons is less than what will fit in the
	// given shared memory.
	// So we do not write to the extra bits so that others may
	// use them without this modifying them.
	{
	  if(shm->read(buffer2)) {returnVal=1; running = 0;}
	  if(bytes)
	    memcpy(buffer2,buffer,bytes);
	  if(bit_mask)
	    {
	      buffer2[bytes] |= (buffer[bytes]&bit_mask);
	      buffer2[bytes] &= ((buffer[bytes])|(~bit_mask));
	    }
	  if(shm->write(buffer2)) {returnVal=1; running = 0;}
	}    
    }
  else
    {
      for(int i=0; i<numberBytes; i++)
	printf("0%3.3o ", buffer[i]);
      printf("\n");
    }
}

static void button_callback(Fl_Widget *w, void *data)
{
  int num = (int) ((size_t) data);

  if(((Fl_Button *) w)->value())
    buffer[num/8] |= 01 << (num % 8);
  else
    buffer[num/8] &= ~(01 << (num % 8));
  put_values();
}


static void quit_callback(Fl_Widget *w, void *data)
{
  running = 0;
}

static void update_buttons(Fl_Widget *w, void *data)
{
  shm->read(buffer);
  for(int i=0; i<numberButtons; i++)
    button[i]->value(buffer[i/8] & (01 << (i % 8)));
}


int make_window(int num, int num_bytes, struct Button *s,
		const char *wlabel,
		int fltk_argc, char **fltk_argv)
{
  numberButtons = num;
  numberBytes = num_bytes;
  button = (Fl_Light_Button **)
    dtk_malloc(sizeof(Fl_Light_Button *)*numberButtons);

  Fl_Window* w = new Fl_Window(120, 90 + num*25, wlabel);

  Fl_Button *o;

  o = new Fl_Button(0,   0,  120, 25, "quit");
  o->callback(quit_callback, NULL);
  o = new Fl_Button(0,   25,  120, 25, "update memory");
  o->callback(put_values, NULL);
  o = new Fl_Button(0,   50,  120, 25, "update buttons");
  if(shm) o->callback(update_buttons, NULL);
  else o->deactivate();

  int i;
  for(i=0;i<num;i++)
    {
      Fl_Light_Button *o = button[i] =
	new Fl_Light_Button(0, 90 + i*25, 120, 25, s[i].label);
      o->callback(button_callback, (void *)((size_t)i));
      o->value(buffer[i/8] & (01 << i % 8));
    }

  if(buffer2)// The number of buttons is less than what will fit in
    // the given shared memory.  So we need some stuff to keep from
    // writing extra data to shared memory.
    {
      bit_mask = 0;
      for(i=0;i<numberButtons%8;i++)
	bit_mask |= (01 << i);
      bytes = numberButtons/8;

      if(bit_mask)
	buffer[bytes] &= bit_mask;

      // printf("bytes=%d bit_mask = 0%o\n",bytes,bit_mask);
    }

  w->end();
  w->show(fltk_argc, fltk_argv);

  return 0;
}
