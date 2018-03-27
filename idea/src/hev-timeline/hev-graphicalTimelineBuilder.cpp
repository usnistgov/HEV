/* hev-graphicalTimelineBuilder
 * --------------------------------
 * Author: Vinay Sriram (SURF 2016)
 *
 * This program is intended to provide a GUI version of the timeline builder
 * utility, allowing nav matrix captures to be done with the click of a button,
 * and allowing a delta time to be placed in between consecutive captures.
 */

// Standard Includes
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>

// FLTK Includes
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/fl_ask.H>

// Widgets
static Fl_Double_Window *main_window;
static Fl_Button *capture_button;
static Fl_Button *done_button;
static Fl_Button *play_button;
static Fl_Button *save_button;
static Fl_Text_Editor *text_edit;
static Fl_Text_Buffer *text_buf;
static Fl_Check_Button *orientation_button;
static Fl_Input *delta_time_input;
static Fl_Input *filename_field;

// Other Fields
static std::string timelineFileName;
static bool fn_updated;
static bool running;

/* Done Button Callback */
static void doneCallback(Fl_Button*, void*) 
{
  if(!fn_updated) {
    int response = fl_ask("Quit without saving?");
    if(response == 0) return;
  }
  running = false;
}

/* Play Button Callback */
static void playCallback(Fl_Button*, void*) 
{ 
  /* Create the temporary timeline file */
  std::string tempFileName = std::string("/tmp/current_capture.temp.timeline");
  std::string tempIrisName = tempFileName + ".iris";
  std::ofstream newStream;
  newStream.open(tempFileName.c_str());
  char *text = text_buf->text();
  newStream << text;
  newStream.flush();
  newStream.close();
  free(text);
  /* Convert file to .iris and load it into the control FIFO */
  std::string command1 = std::string("hev-timelineToIris < ") + tempFileName + " > " + tempIrisName;
  system(command1.c_str());
  std::string command2 = std::string("echo LOAD ") + tempIrisName + " > $IRIS_CONTROL_FIFO"; 
  system(command2.c_str());
}

/* Save Button Callback */
static void saveCallback(Fl_Button*, void*) {
  timelineFileName = std::string(filename_field->value());
  if(timelineFileName == "") return;
  std::ofstream newStream(timelineFileName.c_str());
  char *text = text_buf->text();
  newStream << text;
  free(text);
  newStream.close();
  fn_updated = true;
}

/* Capture Nav Matrix Callback */
static void captureNavMatrixCallback(Fl_Button*, void*) 
{  
  /* Basic Error Checking */
  char *returnBuffer; 
  const char *value = delta_time_input->value();
  double timeValue = strtod(value, &returnBuffer);
  if(strlen(value) == 0 || strlen(returnBuffer) != 0) {
    std::cerr << "Invalid time delta provided." << std::endl; 
    return; // Should accept next request, this one is bad
  }
  /* Get Nav Matrix */
  std::stringstream ss;
  ss << timeValue << " " ;
  FILE *matrixFile;
  if(orientation_button->value()) {
    matrixFile = popen("hev-matrixToCoord --xyz iris/state/nav", "r");
  } else {
    matrixFile = popen("hev-matrixToCoord --xyzhprs iris/state/nav", "r");
  }
  /* Write Nav Matrix to Text Buffer */
  char c = fgetc(matrixFile);
  while (c != '\n') {ss << c; c = fgetc(matrixFile);}
  ss << std::endl;
  text_buf->append(ss.str().c_str());
  fn_updated = false;
}

/* Filler ' Do Nothing' Callback */
static void doNothingCallback(Fl_Widget*, void*) {}
static void textCallback(int, int, int, int, const char*, void*) {fn_updated = false;}

/* Method that Initializes the Main Window and all Objects */
static void initialize()
{
  /* Construct Main Window */
  main_window = new Fl_Double_Window(580, 630, "Graphical Timeline Builder");
  main_window->callback(doNothingCallback);
  /* Top Level Buttons */
  capture_button = new Fl_Button(10, 10, 180, 40, "Capture Nav Matrix");
  capture_button->callback((Fl_Callback *) captureNavMatrixCallback);
  done_button = new Fl_Button(200, 10, 155, 40, "Quit Recording");
  done_button->callback((Fl_Callback *) doneCallback);
  play_button = new Fl_Button(365, 10, 200, 40, "Play Current Timeline");
  play_button->callback((Fl_Callback *) playCallback);
  /* Save Funtionality */
  filename_field = new Fl_Input(140, 570, 275, 30, "Output Filename: ");
  save_button = new Fl_Button(430, 570, 130, 30, "Save Timeline");
  save_button->callback((Fl_Callback *) saveCallback);
  /* Timeline Input Box */
  text_edit = new Fl_Text_Editor(10, 90, 555, 470, NULL);
  text_buf = new Fl_Text_Buffer();
  text_edit->buffer(text_buf);
  text_buf->add_modify_callback(textCallback, text_edit);
  /* Second Level Buttons */
  orientation_button = new Fl_Check_Button(230, 60, 340, 20, " Only get the (x,y,z) position from Nav Matrix");
  orientation_button->type(FL_TOGGLE_BUTTON);
  delta_time_input= new Fl_Input(100, 60, 120, 20, "Delta Time: ");
  /* Set State and Exit */
  main_window->end();
  main_window->show();
  running = true;
  fn_updated = true;
}


// Main Execution Thread
int main() 
{
  initialize();
  while (Fl::wait() && running) {
    if(!main_window->shown()) {
      running = false;
    }
  }
  return 0 ;
}

