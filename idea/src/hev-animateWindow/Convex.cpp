#include "Convex.h"
#include "utils.h"

#include <stdio.h> //printf debugging
#include <string>
#include <vector>
#include <fstream>
#include <set>

//reads in the data from the file, and converts it to an object in the simulation
Convex::Convex(std::string filename, btVector3 velocity, bool externalInertias) {
	initVel = velocity;

	ifstream pointInput(filename.c_str()); //the input file that contains the locations of all the other files
	string input; //the input

	getLine(pointInput, input);
	for(int i = 0; i < input.size(); i++)
		input[i] = tolower(input[i]);

	vector<vector<btVector3> > data; //all the point data

	//stores the convex data in an array
	while(input.find("polygon") != -1) { // while input contains "polygon"
		getLine(pointInput, input);  // we store the next line in input (that will correspond to the 3 coordinates of the first point of the polygon

		vector<btVector3> polygon;
		do {
			if(input == "") {
				input = "escape";
				break;
			}

			polygon.push_back(getPoint(input)); // we store the point of the polygon in polygon

			getLine(pointInput, input);        // we get the next line of the file
			for(int i = 0; i < input.size(); i++)
				input[i] = tolower(input[i]);

		} while(pointInput.good() && input.find("polygon") == -1); // we do that while we are not to the end of the file or the end of the current polygon
		data.push_back(polygon); // once we found all the points of the polygon we store it into data
	}
	// at this point data contains all the polygons defined in the shard.savg file
	pointInput.close();

	btScalar mass = 0.;                           //the mass of the convex
	centroid = btVector3(0, 0, 0);                //the centroid of the convex
	btMatrix3x3 inertiaTensor(0,0,0,0,0,0,0,0,0); //the inertia tensor of the convex

	//extracts the mass properties, either from a specified file, or from the shards themselves
	if(externalInertias) {
		string massFilename = filename.substr(0, filename.find_last_of('.')) + ".dat"; // filename is the name of the shard.savg file
		getMassProps(massFilename, centroid, mass, inertiaTensor);
	}
	else
		getMassProps(data, centroid, mass, inertiaTensor); // compute the centroid, mass and inertiaTensor of the shard (represented by all its polygon in data)

	inertiaTensor.diagonalize(rotation, .00001, 20); // diagonalize the matrix inertiaTensor
	btVector3 principalInertia(inertiaTensor[0][0], inertiaTensor[1][1], inertiaTensor[2][2]); //the principal moments of inertia of the convex

	btVector3 aabbMin = data[0][0]; //the minimum values of the convex
	btVector3 aabbMax = data[0][0]; //the maximum values of the convex
	btMatrix3x3 invRot = rotation.transpose();

	convexHull = new btConvexHullShape(); //the convex hull shape
	//translates the convex so that the centroid is at the origin, and the original coordinate frame diagonalizes the tensor
	for(int i = 0; i < data.size(); i++)  {
		for(int j = 0; j < data[i].size(); j++) {
			aabbMin.setMin(data[i][j]);
			aabbMax.setMax(data[i][j]);
			data[i][j] -= centroid;
			data[i][j] = btVector3(invRot[0].dot(data[i][j]), invRot[1].dot(data[i][j]), invRot[2].dot(data[i][j]));
			convexHull->addPoint(data[i][j]);
		}
	}	

	btTransform transform; //transforms the convex to it's starting location
	transform.setIdentity();
	transform.setOrigin(centroid);
	transform.setBasis(rotation);

	btVector3 localInertia;
	convexHull->calculateLocalInertia(mass, localInertia);
	
	//creates the rigid body
	btDefaultMotionState* myMotionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, convexHull, localInertia);
	rigidBody = new btRigidBody(cInfo);
	rigidBody->setContactProcessingThreshold(BT_LARGE_FLOAT);
	rigidBody->setWorldTransform(transform);

	size = aabbMax-aabbMin;
	//sets the continuous collection properties
	btScalar threshold = size[size.minAxis()];
	rigidBody->setCcdMotionThreshold(threshold/2);
	rigidBody->setCcdSweptSphereRadius(threshold * .2);
}

Convex::~Convex() {

	delete convexHull;

}

bool Convex::isSleeping() {

	return rigidBody->wantsSleeping();

}

void Convex::addToWorld(btDynamicsWorld* world) {

	world->addRigidBody(rigidBody);
	rigidBody->setLinearVelocity(initVel);

	//printf("%f %f %f\n", initVel[0], initVel[1], initVel[2]);

}

void Convex::scale(btScalar scaleFactor) {

	//scale collision shapes
	btConvexHullShape* newShape = new btConvexHullShape();
	set<btVector3> uniquePoints;

	for(int j = 0; j < convexHull->getNumPoints(); j++) {

		btVector3 newPoint = (convexHull->getUnscaledPoints())[j];
		uniquePoints.insert(newPoint);

	}

	for(set<btVector3>::iterator i = uniquePoints.begin(); i != uniquePoints.end(); i++) {

		newShape->addPoint((*i) * scaleFactor);

	}
	//scale position
	centroid *= scaleFactor;
	//scale velocity
	initVel *= scaleFactor;
	//scale linear sleep threshold
	//scale inertia
	btVector3 localInertia;
	btVector3 inertia = rigidBody->getInvInertiaDiagLocal();
	localInertia.setValue(inertia.x() != btScalar(0.0) ? btScalar(1.0) / inertia.x(): btScalar(0.0),
			      inertia.y() != btScalar(0.0) ? btScalar(1.0) / inertia.y(): btScalar(0.0),
			      inertia.z() != btScalar(0.0) ? btScalar(1.0) / inertia.z(): btScalar(0.0));
	localInertia = scaleFactor * scaleFactor * scaleFactor * scaleFactor * scaleFactor * localInertia;
	//printf("local: %e %e %e\n", localInertia[0], localInertia[1], localInertia[2]);
	//scale mass
	btScalar mass = 1 / rigidBody->getInvMass();
	mass = scaleFactor * scaleFactor * scaleFactor * mass;
//	printf("mass: %e\n", mass);
	//scale size
	size *= scaleFactor;

	btTransform transform; //transforms the convex to it's starting location
	transform.setIdentity();
	transform.setOrigin(centroid);
	transform.setBasis(rotation);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, newShape, localInertia);
	btRigidBody* body = new btRigidBody(cInfo);
	body->setContactProcessingThreshold(BT_LARGE_FLOAT);
	body->setWorldTransform(transform);

	//sets the continuous collection properties
	btScalar threshold = size[size.minAxis()];
	body->setCcdMotionThreshold(threshold/2);
	body->setCcdSweptSphereRadius(threshold * .2);

	delete convexHull;
	convexHull = newShape;
	delete rigidBody;
	rigidBody = body;


}

btVector3 Convex::getSize() {

	return size;

}

btVector3 Convex::getCentralTranslation() {

	return centroid * -1;

}

btQuaternion Convex::getRotation() {

	btQuaternion quaternion;
	rotation.transpose().getRotation(quaternion);
	quaternion = rigidBody->getOrientation() * quaternion;
	return quaternion;

}

btVector3 Convex::getFinalTranslation() {

	return rigidBody->getCenterOfMassPosition();

}
