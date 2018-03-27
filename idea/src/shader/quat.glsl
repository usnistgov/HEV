#ifndef quat_glsl
#define quat_glsl

// handy shader math functions
//
// Externally usable functions:
//    vec3 QRotate(vec4 Q, vec3 V);
// Rotate vector V by unit quaternion Q. 
// More efficient for a single vector rotation
//
//    vec3 QsRotate(vec4 Q, vec3 V);
// Rotate vector V by non-unit quaternion Qs. 
// equivalent to dot(Q,Q) * QRotate(normalize(Q), V)
//
//    mat3 QMatrix(vec4 Q)
// Return 3x3 rotation matrix for unit quaternion Q.
// To rotate two or more vectors, it is more efficient to generate
// the matrix then use matrix multiplies for each vector.
//
//    mat3 QsMatrix(vec4 Q)
// Return 3x3 rotation and scaling matrix for non-unit quaternion Q.
// Rotates by normalize(Q) and scales by dot(Q,Q)


// rotate vector by a unit quaternion
// ignoring vectorization & operation fusion: 18 *'s, 12 +'s
vec3 QRotate(vec4 Q, vec3 V) {
    return V + 2.*cross(Q.xyz, Q.w*V + cross(Q.xyz,V));
}

// rotate vector by a non-unit quaternion
// ignoring vectorization & operation fusion: 21 *'s, 14 +'s
vec3 QsRotate(vec4 Q, vec3 V) {
    return dot(Q,Q)*V + 2.*cross(Q.xyz, Q.w*V + cross(Q.xyz,V));
}

// create a 3x3 rotation matrix for a unit quaternion
// ignoring vectorization & operation fusion:
//   constructing matrix: 12 *'s, 12 +'s
//   using on a vector: 9 *'s, 6 +'s
mat3 QMatrix(vec4 Q)
{
    // build matrix from quaternion
    vec3 q = 2.*Q.xyz;
    float xx=q.x*Q.x, xy=q.x*Q.y, xz=q.x*Q.z, xw=q.x*Q.w;
    float yy=q.y*Q.y, yz=q.y*Q.z, yw=q.y*Q.w;
    float zz=q.z*Q.z, zw=q.z*Q.w;
    return mat3(1.-yy-zz, xy+zw, xz-yw,
		xy-zw, 1.-xx-zz, yz+xw,
		xz+yw, yz-xw, 1.-xx-yy);
}

// create a 3x3 rotation matrix for a non-unit quaternion
// ignoring vectorization & operation fusion:
//   constructing matrix: 15 *'s, 14 +'s
//   using on a vector: 9 *'s, 6 +'s
mat3 QsMatrix(vec4 Q)
{
    // build matrix from quaternion
    vec3 q = 2.*Q.xyz;
    float L2 = dot(Q,Q);
    float xx=q.x*Q.x, xy=q.x*Q.y, xz=q.x*Q.z, xw=q.x*Q.w;
    float yy=q.y*Q.y, yz=q.y*Q.z, yw=q.y*Q.w;
    float zz=q.z*Q.z, zw=q.z*Q.w;
    return mat3(L2-yy-zz, xy+zw, xz-yw,
		xy-zw, L2-xx-zz, yz+xw,
		xz+yw, yz-xw, L2-xx-yy);
}

// multiply two quaternions
// QRotate(Q1,QRotate(Q2,V)) = QRotate(QMultiply(Q1,Q2),V)
vec4 QMultiply(vec4 Q1, vec4 Q2)
{
    return vec4(Q1.w*Q2.xyz + Q2.w*Q1.xyz + cross(Q1.xyz,Q2.xyz),
		Q1.w*Q2.w - dot(Q1.xyz,Q2.xyz));
}

#endif
