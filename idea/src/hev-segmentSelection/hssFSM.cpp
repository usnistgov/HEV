
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dtk.h>
#include "hev-segmentSelection.h"
#include "hssDBHL.h"
#include "hssCmd.h"
#include "hssOps.h"




///////////////////////////////////////////////////////////////////


/*


OK, I'm going to construct a simple finite state machine to handle
the user interactions.  This is not going to be a general purpose FSM;
it will not be driven off of some sort of table of transitions.  The
transitions will be encoded directly into the code; thus it will be
very specific to the application. 

Each state will be represented by a function.  Each state function
will contain its own event processing loop.  When the function
returns, its return value will indicate the next state.  Each state
function is responsible for figuring out which events it wants to
pay attention to.  If a state receives an event that it doesn't 
handle, then that event will be lost.

Events will be gathered through a separate little subsystem.

Highlighting will also have a little subsystem.

Maybe there should be a State class, but I don't see the advantage.
Similarly for events.


Here are the events:

   These events are derived directly from the devices
   (actually from the shared memory segs that represent
   the devices) :

      LBD - left button goes from up to down
      LBU - left button goes from down to up
      RBD - right button goes from up to down
      RBU - right button goes from down to up

      move      - cursor changes position

      (We need to recall that because the buttons are polled, we can only
       detect button up and down events based on periodic observations of
       the shared memory segments.)

      (We say "left" and "right" buttons, but these are regarded 
       as virtual devices that could be any button or other means of
       setting the shared memory.)

   These events are derived from the device data in
   conjunction with the data that describes the current
   segments and their pickable regions:

      move near - cursor pos moves into hot spot 
      move far  - cursor pos moves out of current hot spot 

   (I use the term "hot spot" to refer to a region in space
   within which an object is can be picked.  Typically the
   position of the 3D cursor will be compared to hot spots
   to determine highlighting an picking.)


   (Events are also affected by the "commands" that come in to the program.
    So I intend to handle these these commands within the same subsystem 
    that reports events. )
  

States:

  Far          - not near an end pt; no highlight; none pickable;
                     ---> INITIAL STATE

  Create-Drag  - user setting first point of segment

  Drag         - editing position of an end pt

  Near         - near an end pt; highlight; pt is pickable

  Near-Delete  - near an edit point, during delete operation

  Delete       - near an edit point, waiting for confirmation



Far is the initial state.


Transitions:

  begin state     event       action                         end state

  Far             LBD         start new seg: attach first    Create-Drag
                              end marker to wand

  Create-Drag     move        drag 1st end marker            Create-Drag

  Create-Drag     LBU         set position of first          Drag
                              end marker, add line and
                              2nd end marker attached
                              to wand; 2nd end pt is
                              current

  Drag            LBD         no action                      Drag


  Drag            LBU         set new pos of current obj     Far

  Drag            move        drag current obj               Drag

  Drag            RBD         delete current segment         Far

  Near            move near   redo highlights if nec.        Near

  Near            move far    unhighlight                    Far

  Near            LBD         set current obj to drag        Drag

  Near            RBD         highlight curr seg to delete   Near-Delete

  Near-Delete     move far    unhighlight                    Far

  Near-Delete     RBU         set current seg to delete      Delete

  Far             move near   redo highlights                Near

  Delete          RBD         no action                      Delete

  Delete          RBU         delete current segment         Far

  Delete          move far    unhighlight                    Far



*/

//////////////////////////////////////////////////////////////////
//
// Now the event stuff

//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////

enum EventType
	{
	ET_LBD,
	ET_LBU,
	ET_RBD,
	ET_RBU,
	ET_MOVE,
	ET_NEAR,
	ET_FAR
	} ;
	


typedef struct event
	{
	EventType type;
	} HSSEvent;


enum EventCategory
	{
	EC_BUTTON = 0x1,
	EC_MOVE = 0x2,
	EC_NEARFAR = 0x4
	} ;




static int PrevLeftButton = 0;
static int PrevRightButton = 1;
static float PrevCursorLoc[3] = { 0.0, 0.0, 0.0 };
static bool PrevNear = false;

static unsigned long USleepDuration = 100000;

void setSleep (unsigned long uslp)
        {
        USleepDuration = uslp;
        } // end of setSleep

static void
getEvents 
  (
  int eventMask,              // OR together EventCategorys
  std::vector<HSSEvent> &evList  // list of events in requested categories
  )
	{

	// This routine is taking over the functions performed inside
	// the event loop originally implemented in main.


	// usleep (2000000);
	// usleep (30000);  // 1/33 sec
	// usleep (12500);  // 1/80 sec
	// usleep (8333);  // 1/120 sec
	usleep (USleepDuration);

	processCommands ();

	evList.clear ();

	// if ( ! Active )
	if ( ! active () )
		{
		return;
		}

	trackInCoords ();


	if (eventMask & EC_BUTTON)
		{
		// look at shared memory segments for buttons and 
		// report changes

		unsigned char lb, rb;

		CreateButtonShm->read (&lb);


		if (lb != PrevLeftButton)
			{
			static HSSEvent ev;
			ev.type = lb ? ET_LBD : ET_LBU;
			evList.push_back (ev);
			PrevLeftButton = lb;
			if (Debug)
				{
				fprintf (stderr, "Got event: left button %s\n",
					lb ? "DOWN" : "UP" );
				}
			}
		
		DeleteButtonShm->read (&rb);
		if (rb != PrevRightButton)
			{
			static HSSEvent ev;
			ev.type = rb ? ET_RBD : ET_RBU;
			evList.push_back (ev);
			PrevRightButton = rb;
			if (Debug)
				{
				fprintf (stderr, "Got event: right button %s\n",
					rb ? "DOWN" : "UP" );
				}
			}
		
		}   // end of if EC_BUTTON


	if ( (eventMask & EC_MOVE) || (eventMask & EC_NEARFAR) )
		{
		// get newCursorPos

		float newCursorLoc[3];

		getCursorLocation (newCursorLoc);

		if (eventMask | EC_MOVE)
			{
			// look at shared memory segment for 3D cursor; report
			// event if it has changed 

			if ( 
                             (PrevCursorLoc[0] != newCursorLoc[0]) ||
                             (PrevCursorLoc[1] != newCursorLoc[1]) ||
                             (PrevCursorLoc[2] != newCursorLoc[2])    )
				{
				memcpy (PrevCursorLoc, newCursorLoc, 
							3*sizeof(float));
				static HSSEvent ev;
				ev.type = ET_MOVE;
				evList.push_back (ev);
				if (Debug)
					{
					fprintf (stderr, 
					 "Got event move: %f %f %f\n", 
						PrevCursorLoc[0],
						PrevCursorLoc[1],
						PrevCursorLoc[2] );
					}
				}
			}  // end of if EC_MOVE


		if (eventMask & EC_NEARFAR)
			{
			// check if current cursor position is in same near/far 
			// proximity as last report.  If not, report change.

			bool near;
			// Note that checkCursorNearFar handles all of the 
			// highlighting.
			checkCursorNearFar (near);

			if (PrevNear != near)
				{
				PrevNear = near;
				static HSSEvent ev;
				ev.type = near ? ET_NEAR : ET_FAR ;
				evList.push_back (ev);
				if (Debug)
					{
					fprintf (stderr, 
					  "Got event near/far : %s\n", 
					  near ? "NEAR" : "FAR");
					}
						
				}

			}  // end of if EC_NEARFAR
		}

// fprintf (stderr, "inside getEvents: evList.size() %d\n", evList.size());

	return;
	}  // end of getEvents


static int 
findEvent (EventType evType, std::vector<HSSEvent> evList)
	{

	for (int i = 0; i < evList.size(); i++)
		{
		if (evType == evList[i].type)
			{
			return i;
			}
		}

	return -1;
	}  // end of findEvent

void
getStoredCursorLocation (float pos[3])
	{
	// JGH set last detected cursor position

	memcpy (pos, PrevCursorLoc, 3*sizeof(float));

	}  // end of getStoredCursorLocation



enum HSSState
	{
	FAR,
	CREATE_DRAG,
	DRAG,
	NEAR,
	NEAR_DELETE,
	DELETE
	} ;


static HSSState farState ()
	{

	if (Debug)
		{
		fprintf (stderr, "\n\n-------------------  Entered farState ()\n");
		}

	// make sure all highlighting is off
	unhighlight ();
	PrevNear = false;


	while (1)
		{

  		std::vector<HSSEvent> evList; // events in requested categories
		getEvents (EC_NEARFAR | EC_BUTTON, evList);

		int iEv;


		// We want to process a NEAR event first so that we
		// get the transition before we do the button events

// fprintf (stderr, "evList.size() %d\n", evList.size());

#if 0
fprintf (stderr, "evList[0].type %d    ET_LBD %d   ET_NEAR %d\n", 
evList[0].type,
ET_LBD,
ET_NEAR);
#endif

		if ( (iEv = findEvent (ET_NEAR, evList)) >= 0)
			{
			return NEAR;
			}

		if ( (iEv = findEvent (ET_LBD, evList)) >= 0)
			{
			return CREATE_DRAG;
			}


		}


	}  // end of farState;




static HSSState createDragState ()
	{

	if (Debug)
		{
		fprintf (stderr, "\n\n-------------------  Entered createDragState ()\n");
		}


	// unhighlight everything
	unhighlight ();

	// see if there is space to create a new segment
	int iSeg = getFreeSegIndex ();
	if (iSeg < 0)
		{
		// if we're trying to create a new segment, but we're beyond
		// the max number of segments, then revert to default state.
		return FAR;
		}

	// Rather than creating a new segment at this point, I'm just
	// going to highlight the cursor to indicate that the first point
	// is being placed.
	highlightCursor ();


	while (1)
		{

  		std::vector<HSSEvent> evList;  // events in requested categories
		getEvents (EC_BUTTON | EC_MOVE, evList);

		int iEv;

		// We want to process a LBU event first so that we
		// get that transition before we do the move events
		if ( (iEv = findEvent (ET_LBU, evList)) >= 0)
			{
			unhighlightCursor ();
			unhighlight ();


			// create a new segment with both points at the
			// current cursor location.

			float seg[2][3];
			getStoredCursorLocation (seg[0]);
			memcpy (seg[1], seg[0], 3*sizeof(float));

			if (setSeg (iSeg, seg))
				{
				return FAR;
				}

			// highlight the second point, which will then be
			// the one that gets dragged.
			highlight (iSeg, 1, HL_DRAG);

			return DRAG;
			}


		if ( (iEv = findEvent (ET_MOVE, evList)) >= 0)
			{
			// I don't think there is anything that we need 
			// to do to do the move
			// no change of state
			}
		}

	}  // end of createDragState;



static HSSState nearState ()
	{

	if (Debug)
		{
		fprintf (stderr, "\n\n-------------------  Entered nearState ()\n");
		}


	// Note that highlighting is done under getEvents when the NEARFAR
	// events are called for.

	while (1)
		{

  		std::vector<HSSEvent> evList;  // events in requested categories
		getEvents (EC_BUTTON | EC_MOVE | EC_NEARFAR, evList);

		int iEv;

		if ( (iEv = findEvent (ET_LBD, evList)) >= 0)
			{
			return DRAG;
			}

		if ( (iEv = findEvent (ET_FAR, evList)) >= 0)
			{
			return FAR;
			}

		if ( (iEv = findEvent (ET_NEAR, evList)) >= 0)
			{
			// we don't have to redo highlighting because 
			// getEvents does it
			// no change of state
			}

		if ( (iEv = findEvent (ET_RBD, evList)) >= 0)
			{
			return NEAR_DELETE;
			}

		}  // end of while (1)


	
	}  // end of nearState;

static HSSState dragState ()
	{

	if (Debug)
		{
		fprintf (stderr, "\n\n-------------------  Entered dragState ()\n");
		}


	// Look at what's being highlighted
	int iSeg, whichPt;
	int hlType;
	getCurrentHL (iSeg, whichPt, hlType);

	if (hlType == HL_NONE)
		{
		unhighlight ();
		return FAR;
		}

	// make sure highlighting is correct
	if (hlType != HL_DRAG)
		{
		highlight (iSeg, whichPt, HL_DRAG);
		}

	float anchorPos[3];
	getStoredCursorLocation (anchorPos);

	float segStartPos[2][3];
	getSeg (iSeg, segStartPos);

	// Should we make cursor invisible?


	while (1)
		{

  		std::vector<HSSEvent> evList;  // events in requested categories
		getEvents (EC_BUTTON | EC_MOVE, evList);

		int iEv;

		if ( (iEv = findEvent (ET_LBU, evList)) >= 0)
			{
			unhighlight ();
			// or let the far state do the unhighlight?
			return FAR;
			}

		if ( (iEv = findEvent (ET_RBD, evList)) >= 0)
			{
			// The idea here is to let the user easily
			// delete a segment that was inadvertantly created
			// with an accidental LBD when in FAR state.
			// When that happens, you move into DRAG state 
			// typically with all buttons up.  So the Right button
			// can be used to signal that you want to delete the
			// current seg.
			unhighlight();
			// delete the curr segment
			deleteSeg (iSeg);
			return FAR;
			}

		if ( (iEv = findEvent (ET_MOVE, evList)) >= 0)
			{
			float seg[2][3];
			
			// drag the thing
			for (int i = 0; i < 3; i++)
				{
				float cLoc[3];
				getStoredCursorLocation (cLoc);
				float relativeMove = cLoc[i] - anchorPos[i];
				if (whichPt == 2)
					{
					seg[0][i] = 
					   segStartPos[0][i] + relativeMove;
					seg[1][i] = 
					   segStartPos[1][i] + relativeMove;
					}

				else
					{
					seg[whichPt][i] = 
					 segStartPos[whichPt][i] + relativeMove;
					}
				}

			if (whichPt == 2)
				{
				setSeg (iSeg, seg);
				}
			else
				{
				setSegEndPt (iSeg, whichPt, seg[whichPt]);
				}
			// no change of state
			}

		}  // end of while (1)

	}  // end of dragState;

static HSSState nearDeleteState ()
	{

	if (Debug)
		{
		fprintf (stderr, "\n\n-------------------  Entered nearDeleteState ()\n");
		}


	int iSeg, whichPt;
	int hlType;
	getCurrentHL (iSeg, whichPt, hlType);

	if (hlType == HL_NONE)
		{
		// we shouldn't get here, but if we do, kick out to FAR
		fprintf (stderr, 
		  "%s: Internal error in nearDeleteState.\n", ProgName);
		unhighlight ();
		return FAR;
		}

	// make sure highlighting is correct for deletion
	unhighlight ();
	highlight (iSeg, 2, HL_DELETE);
	

	while (1)
		{

  		std::vector<HSSEvent> evList;  // events in requested categories
		getEvents (EC_BUTTON | EC_NEARFAR, evList);

		int iEv;

		if ( (iEv = findEvent (ET_FAR, evList)) >= 0)
			{
			unhighlight ();
			return FAR;
			}

		if ( (iEv = findEvent (ET_RBU, evList)) >= 0)
			{
			return DELETE;
			}

		} // end of while (1)



	}  // end of nearDeleteState;

static HSSState deleteState ()
	{

	if (Debug)
		{
		fprintf (stderr, "\n\n-------------------  Entered deleteState ()\n");
		}



	// Make sure that we are highlighting for deletion

	int iSeg, whichPt;
	int hlType;
	getCurrentHL (iSeg, whichPt, hlType);

	if (hlType != HL_DELETE)
		{
		// we shouldn't get here, but if we do, kick out to FAR
		fprintf (stderr, 
		  "%s: Internal error in deleteState.\n", ProgName);
		unhighlight ();
		return FAR;
		}


	while (1)
		{

  		std::vector<HSSEvent> evList;  // events in requested categories
		getEvents (EC_BUTTON | EC_NEARFAR, evList);

		int iEv;

		if ( (iEv = findEvent (ET_FAR, evList)) >= 0)
			{
			unhighlight ();
			return FAR;
			}

		if ( (iEv = findEvent (ET_RBU, evList)) >= 0)
			{
			unhighlight ();
			deleteSeg (iSeg);
			return FAR;
			}

		} // end of while (1)

	
	}  // end of deleteState;



void
runFiniteStateMachine ()
	{

	HSSState currentState = FAR;
	HSSState nextState;

	while (1)
		{
		switch (currentState)
			{
			case FAR:
				nextState = farState ();
				break;

			case CREATE_DRAG:
				nextState = createDragState ();
				break;

			case DRAG:
				nextState = dragState ();
				break;

			case NEAR:
				nextState = nearState ();
				break;

			case NEAR_DELETE:
				nextState = nearDeleteState ();
				break;

			case DELETE:
				nextState = deleteState ();
				break;

			default:
				fprintf (stderr, 
				  "%s: Internal error - bad state %d.\n", 
				  ProgName, currentState);
				nextState = currentState;

			}


		currentState = nextState;

		}  // end of while (1)


	}  // end of runFiniteStateMachine

	


