This example demonstrates how to set a pivot point and as well as odd
behavior when the RESET button in hev-navigationControl is pressed.

Start the demo.

There is a cube at -.4 .4 0.

A pivot point is initialized at -.4 .4 0

The usual track ball rotations rotates the cube around the expected
pivot point.

Bring up hev-navigationControl (nav under MCP). Click the RESET button.

Now rotations are not about the pivot point. It seems to have lost
the pivot point. 

You can click the Pivot MCP button to restore the pivot point.

Its not clear if this is the proper behavior or a bug.

Notes: On the desktop, jump mode is best.

       Turn on echo to see what control commands are being sent.

June 2, 2015
Steve
