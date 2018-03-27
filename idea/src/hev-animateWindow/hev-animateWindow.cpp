// use ""s around include files in your local directory
// just include the source into the main program to get the dependencies right

#include "Convex.cpp"
#include "Environment.cpp"
#include "World.cpp"

// use <>s around include files from libraries
#include <btBulletDynamicsCommon.h>
#include <unistd.h>
#include <climits>

int main(int argc,char** argv)
{
	World collisionWorld;

	int c;
	int frontPadding = 0;
	int animationLength = -1;
	int numAnimatedFrames = 100;
	float timeStep = .1;
	bool externalInertias = false;
	
	while((c = getopt(argc, argv, "a:l:m:t:f:s:e:u:")) != -1) { // getopt will return the argument it founds in argv specified in the string. It returns -1 when all the arguments are parsed.
		switch(c) {
			case 'a':
				collisionWorld.setAngDamping(atof(optarg));
				break;
			case 'l':
				collisionWorld.setLinDamping(atof(optarg));
				break;
			case 'm':
				externalInertias = true;
				break;
			case 't':
				collisionWorld.setTimestep(atof(optarg));
				timeStep = atof(optarg);
				break;
			case 'f':
				numAnimatedFrames = atoi(optarg);
				break;
			case 's':
				frontPadding = atoi(optarg);
				break;
			case 'e':
				animationLength = atoi(optarg);
				break;
			case 'u':
				collisionWorld.setUnits(atof(optarg));
				break;
		}
	}

	if(argc - optind < 3) {
		printf("ERROR: Incorrect command line arguments\n");
		printf("Usage: %s [options] [inputDir] [inputFile] [envFile]\n", argv[0]);
		return 0;
	}

	string inputDirectory = argv[optind];
	string inputFile = argv[optind+1];
	string environmentFile = argv[optind+2];

	ofstream fout("frames.iris");

	inputDirectory += "/";

	ifstream fin((inputDirectory + inputFile).c_str()); // we open shards.dat (or whatever the name given to the file describing the shards)
	string input;

	fout << "GROUP initFrame\n"; // we write in frames.iris
	// Kelso, 8/10/12- SEQ files are not supported in IRIS, using a GROUP instead, which needs to be animated with hev-animate
	//fout << "SEQ times " << .1 << "\n\n\n";
	fout << "GROUP times\n\n\n";
	
	// btVector3 is a class that is defined in /local/HEV/external/bulletphysics/bullet-2.76/src/LinearMath/btVector3.h
	btVector3 minSize(FLT_MAX, FLT_MAX, FLT_MAX), maxSize(0,0,0); // Specify a x,y,z value for the vectors minSize and maxSize

	for(int i = 0; fin.good(); i++) { // While we are reading something from shards.dat
		getLine(fin, input);      // We get a line from shards.dat (shards.dat contains the list of the savg shards and their initial velocity)
		if(input.size() == 0)
			break;

		int spaceLoc = input.find(' '); // We look for the first space in the line (that corresponds to the end of the savg shard file name, as a line in shards.dat is as follow :
		                                // shard-00.savg 0 -10 0)
		string filename = inputDirectory + input.substr(0, spaceLoc); // we have now in filename the whole path to the savg shard file (inputDirectory/shard-00.savg)
		btVector3 velocity = getPoint(input.substr(spaceLoc));        // put into the vector called velocity the corresponding value (0 -10 0)

		Convex* shard = new Convex(filename, velocity, externalInertias); // Define a pointer to a Convex Body. The convex body will contain the convex hull shape of the shard as well as all the 
		                                                                  // relevant information concerning this object (mass, centroid, ...)

		minSize.setMin(shard->getSize());
		maxSize.setMax(shard->getSize());

		collisionWorld.addBody(shard);

		fout << "LOAD shard" << i << " " << filename << "\n";
		fout << "ADDCHILD shard" << i << " initFrame\n";
	}
	btScalar min = minSize[minSize.minAxis()];
	btScalar max = maxSize[maxSize.maxAxis()];
	//printf("min : %f max : %f\n",min,max);
	btScalar scaleFactor = sqrt((max-min)*(max-min)/((6-.1)*(6-.1))); //scales the points so that all the objects lie around bullet's optimal .1 - 6 range
	//printf("Scale Factor: %f\n", scaleFactor);
	//scaleFactor = 1;
	collisionWorld.setScale(scaleFactor);

	Environment* landscape = new Environment(inputDirectory + environmentFile); // set the landscape (room.savg) and define a convex hull around each of its polygon
	collisionWorld.addBody(landscape);                                          // add the landscape to the object collisionWorld (add all the convex hulls of the environment to the world convexes)

	fin.close();

	collisionWorld.initPhysics(); // creates the simulation world and adds the objects

	fout << "\n\n";

	int frames;
	for(frames = 0; frames < frontPadding; frames++) {
		fout << "GROUP time" << frames << "\n";
		fout << "ADDCHILD time" << frames << " times\n";
		fout << "ADDCHILD initFrame time" << frames << "\n\n";
	}


	int nextStop = frontPadding + numAnimatedFrames;

	for(; frames < nextStop && collisionWorld.isActive(); frames++) {
		collisionWorld.stepWorld();
		collisionWorld.stepWorld();
		collisionWorld.stepWorld();
		collisionWorld.stepWorld();
		collisionWorld.writeFrame(frames, fout);
	}

	for(int lastFrame = frames-1; frames < animationLength; frames++) {
		fout << "GROUP time" << frames << "\n";
		fout << "ADDCHILD time" << frames << " times\n";
		fout << "ADDCHILD frame" << lastFrame << " time" << frames << "\n\n";
	}
	fout << "\n\nRETURN times\n";

	fout.close();

	return 0;
}

