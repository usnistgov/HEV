/*
This class defines a convex body
*/

#ifndef CONVEX_H
#define CONVEX_H

#include <btBulletDynamicsCommon.h>
#include "LinearMath/btAlignedObjectArray.h"
#include <string>

class Convex {

	private:

		btRigidBody* rigidBody;
		btConvexHullShape* convexHull;
		btVector3 centroid;
		btMatrix3x3 rotation;
		btVector3 initVel;
		btVector3 size;

	public:

		Convex(std::string filename, btVector3 velocity, bool externalInertias);
		~Convex();

		bool isSleeping();
		btVector3 getSize();
		btVector3 getCentralTranslation();
		btQuaternion getRotation();
		btVector3 getFinalTranslation();

		void scale(btScalar scaleFactor);
		void addToWorld(btDynamicsWorld* world);	

};

#endif //CONVEX_H

