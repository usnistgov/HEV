#ifndef _CONVEX_H_
#define _CONVEX_H_

#include "btBulletDynamicsCommon.h"
#include <vector>
#include <string>
#include <fstream>

using namespace std;

//this retrieves the first three numbers stored in [input] and stores them in [p]
static btVector3 getPoint(string input) {

        btVector3 point;

        input = input.substr(input.find_first_not_of(" "));
        int spaceLoc = input.find(' '); //the location of the delimiting space
        string xLoc = input.substr(0, spaceLoc);
        input = input.substr(spaceLoc);
        input = input.substr(input.find_first_not_of(" "));
        spaceLoc = input.find(' ');
        string yLoc = input.substr(0, spaceLoc);
        string zLoc = input.substr(spaceLoc);
        point.setX(atof(xLoc.c_str()));
        point.setY(atof(yLoc.c_str()));
        point.setZ(atof(zLoc.c_str()));

        return point;

}

//gets a line from a savg file
static void getLine(istream& input, string& write) {

	write = "";
	if(!input.good())
		return;
	getline(input, write);


	int found = write.find("#");
	write = write.substr(0, found);

	found = write.find_first_not_of(" \t\n\v\f\r");
	if(found != string::npos)
		write = write.substr(found, write.size());
	else {
		getLine(input, write);
		return;
	}

	found = write.find_last_not_of(" \t\n\v\f\r");
	if(found != string::npos)
		write = write.substr(0, found + 1);

}

static void getMassProps(string filename, btVector3& centroid, float& mass, btMatrix3x3& inertiaTensor) {

	ifstream input(filename.c_str());

	input >> mass;
	input >> centroid[0] >> centroid[1] >> centroid[2];
	input >> inertiaTensor[0][0] >> inertiaTensor[0][1] >> inertiaTensor[0][2];
	input >> inertiaTensor[1][0] >> inertiaTensor[1][1] >> inertiaTensor[1][2];
	input >> inertiaTensor[2][0] >> inertiaTensor[2][1] >> inertiaTensor[2][2];

}

//calculates the centroid and mass of the convex in the data matrix
static void getMassProps(vector<vector<btVector3> >& data, btVector3& centroid, float& mass, btMatrix3x3& inertiaTensor) {

		btScalar density = 1;
		mass = 0;
                //calculate centroid of the convex by splitting the polyhedron into tetrahedrons and finding the weighted average of their centroids
		for(int i = 0; i < data.size(); i++) {
			btVector3 point1 = data[0][0];
			btVector3 point2 = data[i][0];
			btVector3 point3 = data[i][1];
			for(int j = 0; j < data[i].size(); j++) {
				btVector3 point4 = data[i][j];
				btScalar tetradMass = btMatrix3x3(point2.getX()-point1.getX(), point3.getX()-point1.getX(), point4.getX()-point1.getX(), point2.getY()-point1.getY(), point3.getY()-point1.getY(), point4.getY()-point1.getY(), point2.getZ()-point1.getZ(), point3.getZ()-point1.getZ(), point4.getZ()-point1.getZ()).determinant() / 6;
				if(tetradMass < 0)
					tetradMass *= -1;
				mass += tetradMass;
				centroid += (point1+point2+point3+point4)/4*tetradMass;
				point3 = point4;
			}
		}
		centroid = centroid / mass;
		mass *= density;

	                //printf("centroid: %e %e %e\n", centroid.getX(), centroid.getY(), centroid.getZ());

		//calculates the inertia tensor by splittin the polyhedron into tetrahedons around the centroid, and finding the sum of their inertia tensors
		inertiaTensor.setValue(0,0,0,0,0,0,0,0,0);
                for(int i = 0; i < data.size(); i++) {

			btVector3 point1a = centroid;
			btVector3 point2a = data[i][0];
			btVector3 point3a = data[i][1];
			for(int j = 2; j < data[i].size(); j++) {

				btVector3 point4a = data[i][j];

				btVector3 point1 = point1a - centroid;
				btVector3 point2 = point2a - centroid;
				btVector3 point3 = point3a - centroid;
				btVector3 point4 = point4a - centroid;

				//calculate inertia for this tetrad
				btMatrix3x3 jacobian(point2.getX()-point1.getX(), point3.getX()-point1.getX(), point4.getX()-point1.getX(), point2.getY()-point1.getY(), point3.getY()-point1.getY(), point4.getY()-point1.getY(), point2.getZ()-point1.getZ(), point3.getZ()-point1.getZ(), point4.getZ()-point1.getZ());
				btScalar det = jacobian.determinant();
				if(det < 0)
					det *= -1.0;


				btScalar xSum = point1.getX()*point1.getX() + point1.getX()*point2.getX() + point1.getX()*point3.getX() + point1.getX()*point4.getX() + point2.getX()*point2.getX() + point2.getX()*point3.getX() + point2.getX()*point4.getX() + point3.getX()*point3.getX() + point3.getX()*point4.getX() + point4.getX()*point4.getX();
				btScalar ySum = point1.getY()*point1.getY() + point1.getY()*point2.getY() + point1.getY()*point3.getY() + point1.getY()*point4.getY() + point2.getY()*point2.getY() + point2.getY()*point3.getY() + point2.getY()*point4.getY() + point3.getY()*point3.getY() + point3.getY()*point4.getY() + point4.getY()*point4.getY();
				btScalar zSum = point1.getZ()*point1.getZ() + point1.getZ()*point2.getZ() + point1.getZ()*point3.getZ() + point1.getZ()*point4.getZ() + point2.getZ()*point2.getZ() + point2.getZ()*point3.getZ() + point2.getZ()*point4.getZ() + point3.getZ()*point3.getZ() + point3.getZ()*point4.getZ() + point4.getZ()*point4.getZ();

                                //printf("(xSum, ySum, zSum): %f %f %f\n", xSum, ySum, zSum);

				btScalar a = density * det * (ySum + zSum) / 60.0;
				btScalar b = density * det * (xSum + zSum) / 60.0;
				btScalar c = density * det * (xSum + ySum) / 60.0;
				btScalar ap = 2.0*point1.getY()*point1.getZ() + point2.getY()*point1.getZ() + point3.getY()*point1.getZ() + point4.getY()*point1.getZ() + point1.getY()*point2.getZ() + 2.0*point2.getY()*point2.getZ() + point3.getY()*point2.getZ() + point4.getY()*point2.getZ() + point1.getY()*point3.getZ() + point2.getY()*point3.getZ() + 2.0*point3.getY()*point3.getZ() + point4.getY()*point3.getZ() + point1.getY()*point4.getZ() + point2.getY()*point4.getZ() + point3.getY()*point4.getZ() + 2.0*point4.getY()*point4.getZ();
				btScalar cp = 2.0*point1.getY()*point1.getX() + point2.getY()*point1.getX() + point3.getY()*point1.getX() + point4.getY()*point1.getX() + point1.getY()*point2.getX() + 2.0*point2.getY()*point2.getX() + point3.getY()*point2.getX() + point4.getY()*point2.getX() + point1.getY()*point3.getX() + point2.getY()*point3.getX() + 2.0*point3.getY()*point3.getX() + point4.getY()*point3.getX() + point1.getY()*point4.getX() + point2.getY()*point4.getX() + point3.getY()*point4.getX() + 2.0*point4.getY()*point4.getX();
				btScalar bp = 2.0*point1.getX()*point1.getZ() + point2.getX()*point1.getZ() + point3.getX()*point1.getZ() + point4.getX()*point1.getZ() + point1.getX()*point2.getZ() + 2.0*point2.getX()*point2.getZ() + point3.getX()*point2.getZ() + point4.getX()*point2.getZ() + point1.getX()*point3.getZ() + point2.getX()*point3.getZ() + 2.0*point3.getX()*point3.getZ() + point4.getX()*point3.getZ() + point1.getX()*point4.getZ() + point2.getX()*point4.getZ() + point3.getX()*point4.getZ() + 2.0*point4.getX()*point4.getZ();
					
				ap = density * det * ap / 120.0;
				bp = density * det * bp / 120.0;
				cp = density * det * cp / 120.0;

				btMatrix3x3 tetradTensor(a, bp*-1, cp*-1, bp*-1, b, ap*-1, cp*-1, ap*-1, c);

				inertiaTensor[0][0] += tetradTensor[0][0];
				inertiaTensor[1][1] += tetradTensor[1][1];
				inertiaTensor[2][2] += tetradTensor[2][2];

				inertiaTensor[1][0] += tetradTensor[1][0];
				inertiaTensor[2][0] += tetradTensor[2][0];
				inertiaTensor[2][1] += tetradTensor[2][1];

				point3a = point4a;

			}

                }

		inertiaTensor[0][1] = inertiaTensor[1][0];
		inertiaTensor[0][2] = inertiaTensor[2][0];
		inertiaTensor[1][2] = inertiaTensor[2][1];

}

#endif
