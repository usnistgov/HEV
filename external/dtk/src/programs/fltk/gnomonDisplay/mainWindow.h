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

class GLDisplay
{
  private: 

  /*-- variables --*/ 
    
  bool connectionFlag;
  dtkClient *client;
  int dtkIndex;
  Fl_Window* aboutWin;    

  void createAbout();
    
  Fl_Window* mainWin;		    // the main window
  Fl_Value_Output **valOutput;    // all the value input widgets
  Fl_Roller **roller;		    // all the roller inputs
  Fl_Button *Connect;	    	    // the connect button
  Fl_Button *Update;		    // the update widget button
  Fl_Light_Button *Continuous;    // the continuous update widget button
  Fl_Input *Input;		    // the input bar
  Fl_Value_Input *Index;
  Axis_GL* axis;		    // the open_GL axis display
    
    /*-- callbacks --*/ 
  void output_cb(Fl_Widget*,int);  // updates the display based on input values
  static void continuous_cb(Fl_Widget*, dtk_gnomonDisplay*);
  // continuously updates the display
  static bool Connect_cb (Fl_Widget*, dtk_gnomonDisplay*);
  // connects to the DTK shared memory that the user specifies
  static void update_cb (Fl_Widget*,dtk_gnomonDisplay*);
  // updates widget with the appropriate 
  // portion of dtk-shared mem values
  static void viewZoom_cb (Fl_Widget*,dtk_gnomonDisplay*);
  static void viewVert_cb (Fl_Widget*,dtk_gnomonDisplay*);
  static void viewHorz_cb (Fl_Widget*,dtk_gnomonDisplay*);
  static void resetView_cb(Fl_Widget*, dtk_gnomonDisplay*);
  static void about_cb(Fl_Widget*, dtk_gnomonDisplay*);
  static void AboutOk_cb(Fl_Widget*, dtk_gnomonDisplay*);
  static void quit_cb(Fl_Widget*, void*);  // quits the program
    
public:

  dtkSharedMem* dtksharedMem;
  const char* dtksegName;     // the file name of the DTK shared memory
  			      // the user would like to connect to
  bool continuousFlag;
  float *xyzhpr_dtkMem;	// local memory holding values to store/read
			// to/from dtk-shared mem for XYZHPR
  float *dtkbuffer;
 
  GLDisplay(); 	   // default constructor
  void make_window();	   // creates the main window
  void show();  	   // displays the window
  void redraw();	   // testing something
  ~GLDisplay();    // destructor
};
