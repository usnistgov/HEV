//// vertex to fragment communication
varying float silhouette;	// dot product to find silhouette (set
				// to 0 for solid rocks)
varying float stress;		// per-rock stress

varying vec2 Pn;		// coordinates for writing numbers
varying vec3 Pr;		// surface position in rock space
varying vec3 Po;		// surface position in global system

varying vec4 stressData[POINTARRAY];	// data on each closest point
