#include <btBulletDynamicsCommon.h>
#ifdef TEST_SERIALIZATION
#include <LinearMath/btSerializer.h>
#endif //TEST_SERIALIZATION

#include <stdio.h> //printf debugging
#include <string>
#include <vector>
#include <fstream>

#include "Environment.h"
#include "utils.h"
#include "World.h"

using namespace std;


//reads in the data from the file, and converts it to an object in the simulation
Environment::Environment(string filename) {

	ifstream dataIn(filename.c_str());

	string input;
	getLine(dataIn, input);


	//creates a convex hull for each polygon in the shape
	while(input == "polygon") {

		btConvexHullShape* convex = new btConvexHullShape();
		getLine(dataIn, input);
		while(input != "polygon") {

			if(input == "") {
				break;
			}
			btVector3 point = getPoint(input);
			convex->addPoint(point);
			getLine(dataIn, input);

		}

		btTransform transform;
		transform.setIdentity();
		
		btDefaultMotionState* myMotionState = new btDefaultMotionState(transform);

		btRigidBody::btRigidBodyConstructionInfo cInfo(0,myMotionState,convex,btVector3(0,0,0));

		btRigidBody* body = new btRigidBody(cInfo);
		body->setContactProcessingThreshold(BT_LARGE_FLOAT);

		shapes.push_back(convex);
		bodies.push_back(body);

	}

}

Environment::~Environment() {

	for(int i = 0; i < shapes.size(); i++) {
		delete shapes[i];
	}

}

void Environment::addToWorld(btDynamicsWorld* world) {

	for(int i = 0; i < bodies.size(); i++)
		world->addRigidBody(bodies[i]);

}

void Environment::scale(btScalar scaleFactor) {

	for(int i = 0; i < shapes.size(); i++) {

		btConvexHullShape* newShape = new btConvexHullShape();

		for(int j = 0; j < shapes[i]->getNumPoints(); j++) {

			btVector3 newPoint = (shapes[i]->getUnscaledPoints())[j];
			newPoint *= scaleFactor;
			newShape->addPoint(newPoint);

		}

		btTransform transform;
		transform.setIdentity();

		btDefaultMotionState* myMotionState = new btDefaultMotionState(transform);

		btRigidBody::btRigidBodyConstructionInfo cInfo(0,myMotionState,newShape,btVector3(0,0,0));

		btRigidBody* body = new btRigidBody(cInfo);
		body->setContactProcessingThreshold(BT_LARGE_FLOAT);
		
		delete shapes[i];
		shapes[i] = newShape;
		delete bodies[i];
		bodies[i] = body;

	}

}
