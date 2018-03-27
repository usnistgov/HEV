#include "World.h"
#include "utils.h"

#include <stdio.h> //printf debugging
#include <string>
#include <vector>
#include <fstream>

//moves the simulation by one timestep
void World::stepWorld() {

	stillRunning = false; //ensures that the simulation ends when no objects are moving
	//writes the data to the file
	for(int i = 0; i < m_convexes.size() ; i++) {
		//tests whether the simulation is over or not	
		if(!m_convexes[i]->isSleeping()) {
			stillRunning = true;
			break;
		}
	}
	
	m_dynamicsWorld->stepSimulation(timeStep,1);
}

void World::writeFrame(int frame, std::ofstream& output) {
	output << "GROUP frame" << frame << endl;
	
	for(int i = 0; i < m_convexes.size(); i++) {
		btVector3 centralTranslation = m_convexes[i]->getCentralTranslation() / scaleFactor;
		btQuaternion rotation = m_convexes[i]->getRotation();
		btVector3 finalTranslation = m_convexes[i]->getFinalTranslation() / scaleFactor;
		
		//this translates the shard so that it rotates around it's center of gravity at the origin. Without this, the
		//convex would rotate around the origin, and move in a circle
		output << "SCS shard" << i << "-" << frame << "ToRotationCenter ";
		output << centralTranslation.getX() << " " << centralTranslation.getY() << " " << centralTranslation.getZ() << endl;


		output << "SCSQ shard" << i << "-" << frame << "Rotate 0 0 0 ";

		//bullet provides quaternions in the form <x y z> w, where <x y z> is a unit vector around which the shape rotates, and w is the angle of rotation
		//savgfly uses the correct quaternion, where given a unit axis <x y z> and an angle w, the quaternion is sin(w/2)*<x y z> cos(w/2)
		btVector3 axis = rotation.getAxis();
		float sin1 = sin(rotation.getAngle()/2);
		float cos1 = cos(rotation.getAngle()/2);
		if (frame == 0)
			output << "0 0 0 1" << endl;
		else
			output << axis.getX()*sin1 << " " << axis.getY()*sin1 << " " << axis.getZ()*sin1 << " " << cos1 << endl;

		//moves the convex to the correct place
		output << "SCS shard" << i << "-" << frame << "FinalLocation ";
		
		if (frame == 0)
			output << -centralTranslation.getX() << " " << -centralTranslation.getY() << " " << -centralTranslation.getZ() << endl;
		else
			output << finalTranslation.getX() << " " << finalTranslation.getY() << " " << finalTranslation.getZ() << endl;

		output << "ON shard" << i << " shard" << i << "-" << frame << "ToRotationCenter" << endl;
		output << "ON shard" << i << "-" << frame << "ToRotationCenter shard" << i << "-" << frame << "Rotate" << endl;
		output << "ON shard" << i << "-" << frame << "Rotate shard" << i << "-" << frame << "FinalLocation" << endl;
		output << "ON shard" << i << "-" << frame << "FinalLocation frame" << frame << endl;
	}

	output << "ADDCHILD frame" << frame << " times\n\n";
}

//creates the simulation world and adds the objects
void World::initPhysics() {
	///collision configuration contains default setup for memory, collision setup
	m_collisionConfiguration = new btDefaultCollisionConfiguration();
	//m_collisionConfiguration->setConvexConvexMultipointIterations();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	m_dispatcher = new	btCollisionDispatcher(m_collisionConfiguration);

	m_broadphase = new btDbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* sol = new btSequentialImpulseConstraintSolver;
	m_solver = sol;

	m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher,m_broadphase,m_solver,m_collisionConfiguration);
	
	m_dynamicsWorld->setGravity(btVector3(0,0,-10));

	for(int j = 0; j < m_convexes.size(); j++) {
		m_convexes[j]->scale(scaleFactor);
		m_convexes[j]->addToWorld(m_dynamicsWorld);
	}
	for(int j = 0; j < m_environment.size(); j++) {
		m_environment[j]->scale(scaleFactor);
		m_environment[j]->addToWorld(m_dynamicsWorld);
	}
}

//destroys all objects	
void World::exitPhysics() {

	//cleanup in the reverse order of creation/initialization


	//remove the rigidbodies from the dynamics world and delete them
	int i;
	for (i=m_dynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--)
	{
		btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		m_dynamicsWorld->removeCollisionObject( obj );
		delete obj;
	}

	for(int j = 0; j < m_convexes.size(); j++) {
		Convex* convex = m_convexes[j];
		delete convex;
	}
	for(int j = 0; j < m_environment.size(); j++) {
		Environment* convex = m_environment[j];
		delete convex;
	}

	delete m_dynamicsWorld;

	delete m_solver;

	delete m_broadphase;

	delete m_dispatcher;

	delete m_collisionConfiguration;

}
