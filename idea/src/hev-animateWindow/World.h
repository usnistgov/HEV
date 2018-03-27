//This class represents the world all the physics occurs in

#ifndef WORLD_H
#define WORLD_H

#include <LinearMath/btAlignedObjectArray.h>
#include <btBulletDynamicsCommon.h>

#include <fstream>
#include <string>

#include "Convex.h"
#include "Environment.h"

class World {


	btDynamicsWorld* m_dynamicsWorld;	

	btBroadphaseInterface* m_broadphase;

	btCollisionDispatcher* m_dispatcher;

	btConstraintSolver* m_solver;

	btDefaultCollisionConfiguration* m_collisionConfiguration;

	btAlignedObjectArray<Convex*> m_convexes;

	btAlignedObjectArray<Environment*> m_environment;

	btScalar angDamping; //the angular damping factor
	btScalar linDamping; //the linear damping factor
	bool externalMassProps; //whether to extract external mass properties
	btScalar timeStep; //the amount of time between each simulation step
	btScalar units; //the length of each savg unit in meters
	bool stillRunning; //whether the simulation is active or not
	btScalar scaleFactor; //the normalization scale factor

	public:

	World()
	{
		angDamping = linDamping = -1.0;
		externalMassProps = false;
		timeStep = 1.0/10;
		stillRunning = true;
		scaleFactor = 1;
		units = 1;

	}

	bool useExternalMassProps() {
		externalMassProps = true;
	}
	void setAngDamping(btScalar damping) {
		angDamping = damping;
	}
	void setLinDamping(btScalar damping) {
		linDamping = damping;
	}
	void setTimestep(float step) {
		timeStep = step;
	}
	void setUnits(btScalar u) {
		units = u;
	}
	void setScale(btScalar scale) {
		scaleFactor = scale;
	}
	bool isActive() {
		return stillRunning;
	}

	void stepWorld();

	void addBody(Convex* shard) {

		m_convexes.push_back(shard);

	}

	void addBody(Environment* room) {

		m_environment.push_back(room);

	}

	void writeFrame(int frames, ofstream& fileOut);

	void exitPhysics();
	virtual ~World() {
		exitPhysics();
	}
	void initPhysics();

};

#endif //BASIC_DEMO_H

