// uniform parameters under user control
uniform float spotSize;		// size of spot
uniform float spotCutoff;	// stress cutoff for spot display
uniform float stressCutoff;	// stress cutoff for rock display
uniform float stressScale;	// control rock color
uniform float keyframe;         // frame to highlight (-1 to track frames)

uniform float showspots;	// show closest point spots?
uniform float showcolor;	// show stress with color?
uniform float showtext;		// show stress value text?

// values passed in by application
uniform float osg_FrameTime;	// frame counter provided by OpenSceneGraph
uniform int frame;		// simulation frame

