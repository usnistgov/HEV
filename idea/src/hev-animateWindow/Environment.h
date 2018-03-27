#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <btBulletDynamicsCommon.h>
#include <LinearMath/btAlignedObjectArray.h>

#include <string>

using namespace std;


class Environment {

	private:
		btAlignedObjectArray<btRigidBody*> bodies;
		btAlignedObjectArray<btConvexHullShape*> shapes;

	public:
		Environment(std::string filename);
		~Environment();

		void addToWorld(btDynamicsWorld* world);
		void scale(btScalar scaleFactor);

};

#endif //ENVIRONMENT_H
