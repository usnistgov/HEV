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
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Repeat_Button.H>

/****************************************************************************
 ***** This is how we built many programs with one set of source files ******
 ***************************************************************************/

#ifdef floatSliders
#  define VALUATOR_TYPE     Fl_Slider
#  define PROGRAM_NAME      "dtk-floatSliders"
#  define FLOAT_TYPE        float
#  define FLOAT_NAME        "float"
#  define sliders
#  include <FL/Fl_Slider.H>
#endif

#ifdef floatAdjusters
#  define VALUATOR_TYPE     Fl_Adjuster
#  define PROGRAM_NAME      "dtk-floatAdjusters"
#  define FLOAT_TYPE        float
#  define FLOAT_NAME        "float"
#  define adjusters
#  include <FL/Fl_Adjuster.H>
#endif

#ifdef floatCounters
#  define VALUATOR_TYPE     Fl_Counter
#  define PROGRAM_NAME      "dtk-floatCounters"
#  define FLOAT_TYPE        float
#  define FLOAT_NAME        "float"
#  define counters
#  include <FL/Fl_Counter.H>
#endif

#ifdef floatRollers
#  define VALUATOR_TYPE     Fl_Roller
#  define PROGRAM_NAME      "dtk-floatRollers"
#  define FLOAT_TYPE        float
#  define FLOAT_NAME        "float"
#  define rollers
#  include <FL/Fl_Roller.H>
#endif

#ifdef floatValueInputs
#  define VALUATOR_TYPE     Fl_Value_Input
#  define PROGRAM_NAME      "dtk-floatValueInputs"
#  define FLOAT_TYPE        float
#  define FLOAT_NAME        "float"
#  define valueInputs
#  include <FL/Fl_Value_Input.H>
#endif



#ifdef doubleSliders
#  define VALUATOR_TYPE     Fl_Slider
#  define PROGRAM_NAME      "dtk-doubleSliders"
#  define FLOAT_TYPE        double
#  define FLOAT_NAME        "double"
#  define sliders
#  include <FL/Fl_Slider.H>
#endif

#ifdef doubleAdjusters
#  define VALUATOR_TYPE     Fl_Adjuster
#  define PROGRAM_NAME      "dtk-doubleAdjusters"
#  define FLOAT_TYPE        double
#  define FLOAT_NAME        "double"
#  define adjusters
#  include <FL/Fl_Adjuster.H>
#endif

#ifdef doubleCounters
#  define VALUATOR_TYPE     Fl_Counter
#  define PROGRAM_NAME      "dtk-doubleCounters"
#  define FLOAT_TYPE        double
#  define FLOAT_NAME        "double"
#  define counters
#  include <FL/Fl_Counter.H>
#endif

#ifdef doubleRollers
#  define VALUATOR_TYPE     Fl_Roller
#  define PROGRAM_NAME      "dtk-doubleRollers"
#  define FLOAT_TYPE        double
#  define FLOAT_NAME        "double"
#  define rollers
#  include <FL/Fl_Roller.H>
#endif

#ifdef doubleValueInputs
#  define VALUATOR_TYPE     Fl_Value_Input
#  define PROGRAM_NAME      "dtk-doubleValueInputs"
#  define FLOAT_TYPE        double
#  define FLOAT_NAME        "double"
#  define valueInputs
#  include <FL/Fl_Value_Input.H>
#endif



#ifdef intSliders
#  define VALUATOR_TYPE     Fl_Slider
#  define PROGRAM_NAME      "dtk-intSliders"
#  define FLOAT_TYPE        int
#  define FLOAT_NAME        "int"
#  define sliders
#  include <FL/Fl_Slider.H>
#endif

#ifdef intAdjusters
#  define VALUATOR_TYPE     Fl_Adjuster
#  define PROGRAM_NAME      "dtk-intAdjusters"
#  define FLOAT_TYPE        int
#  define FLOAT_NAME        "int"
#  define adjusters
#  include <FL/Fl_Adjuster.H>
#endif

#ifdef intCounters
#  define VALUATOR_TYPE     Fl_Counter
#  define PROGRAM_NAME      "dtk-intCounters"
#  define FLOAT_TYPE        int
#  define FLOAT_NAME        "int"
#  define counters
#  include <FL/Fl_Counter.H>
#endif

#ifdef intRollers
#  define VALUATOR_TYPE     Fl_Roller
#  define PROGRAM_NAME      "dtk-intRollers"
#  define FLOAT_TYPE        int
#  define FLOAT_NAME        "int"
#  define rollers
#  include <FL/Fl_Roller.H>
#endif

#ifdef intValueInputs
#  define VALUATOR_TYPE     Fl_Value_Input
#  define PROGRAM_NAME      "dtk-intValueInputs"
#  define FLOAT_TYPE        int
#  define FLOAT_NAME        "int"
#  define valueInputs
#  include <FL/Fl_Value_Input.H>
#endif



extern void update_valuators(Fl_Widget *w=NULL, void *data=NULL);
extern Fl_Light_Button *update_contButton;
extern Fl_Window *mainWindow;

extern int is_running;
extern int return_val;

struct ValuatorLimits
{
  char *label;
  double start, reset, max, min;
};

extern int make_window(int num, struct ValuatorLimits *s, const char *wlabel,
		       int update_continous,
		       int fltk_argc, char **fltk_argv);

#ifdef DTK_ARCH_WIN32_VCPP
# define snprintf  _snprintf

  extern void bzero(void *p, size_t s);
#endif /* #ifdef DTK_ARCH_WIN32_VCPP */
