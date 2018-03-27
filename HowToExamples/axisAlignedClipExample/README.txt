Notes on axisAlignedClip.mcp - How to restrict clipping tool to axis alignment
Steve Satterfield, 7/17/2013

Intro
-----

This directory contains an example of how to use axisAlignedClip.mcp
(installed in the general data directory).

axisAlignedClip.mcp is a temporary solution for adding axis aligned
clipping capability (axisAlignedClip.mcp) to the current clip tool. It
is a short term "kludge" installed on top of the current clipping
tool.  The capability should be added and integrated into the
installed clip.mcp.


Build/run
---------

To build this demo:
                   hev-moo grid.iris pcl.iris > moo.iris

To run this demo:
		   ./rundemo



Bug or Feature?
---------------

axisAlignedClip.mcp is added to an irisfly application at run
time. Since its not fully integrated into hev-relativeMove and
hev-clip there is specific run time sequence that should be followed.

Also the axis alignment constraints are aligned with the physical
planes (wall/floor) of the CAVE. If you change HPR of the navigation,
the clipping tool remains fixed. This behavior may be considered a
"bug" or a "feature".



What to Click & When
--------------------

When running this demo or other, there is a specific sequence that
should be followed. The clipping too must be launched and activated to
ensure all the underlying scenegraph and running processes are in place
prior to adding in the axis aligned capability.

Specifically:
	
	Click "clip", select windshield and perform some clipping -
	press and hold left wand button to activate clipping

	Click "idea" -> "nav".

	On navigation menu, click "RESET"

	Click "AAclip", then "ON/OFF", then "XZ" 
	The clipping plane won't change until you activate with
	left wand button - it should then snap into an axis
	position. Its movements will then be restricted XYZ, but HPR
	remains fixed.


	At this point is ready. Using the "AAclip" menu, the other
	axis alignments can be selected.

	AAclip can be turned off and the clipping tool will again be
	HPR from wand.

	I have noticed occasionally it will spew error messages to
	the console. It will usually recover and the expected
	functionality returns.

	Remember the RESET on the navigation button is the data gets
	rotated in any way.

	All the other clipping shapes can be used in axis aligned
	mode.


Implementation
--------------

axisAlignedClip.mcp is implemented as a undocumented feature of
hev-relativeMove command. The implementations consists of a few lines
to code in hev-relativeMove.cpp (marked with "sgs") and the addition
of axisAlignedClip.mcp.

Again, the plan is that this temporary solution and should be replaced
by implementing the functionality into the production clip.mcp tool.
John Hagedorn implemented the clip tool and when he has time will work
on this update.


---
End
---
