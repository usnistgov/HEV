#ifndef matrix_glsl
#define matrix_glsl

// handy matrix math functions
//
// Externally usable functions:
//    mat2 adjoint(mat2 M);
//    mat3 adjoint(mat3 M);
//    mat4 adjoint(mat4 M);
// Adjoint of 2x2 to 4x4 matrix M

mat2 adjoint(mat2 M) 
{
    return mat2(M[1][1],-M[0][1],-M[1][0],M[0][0]);
}

mat2 inverse(mat2 M)
{
    return adjoint(M)/determinant(M);
}

mat3 adjoint(mat3 M)
{
    return transpose(mat3(M[1]^M[2], M[2]^M[0], M[0]^M[1]));
}

mat4 adjoint(mat4 M)
{
    // 2x2 submatrices
    mat2 m00 = mat2(M[0][0],M[1][0],M[0][1],M[1][1]);
    mat2 m10 = mat2(M[2][0],M[3][0],M[2][1],M[3][1]);
    mat2 m01 = mat2(M[0][2],M[1][2],M[0][3],M[1][3]);
    mat2 m11 = mat2(M[2][2],M[3][2],M[2][3],M[3][3]);

    // compute adjoint by Strassen's algorithm
    mat2 t = adjoint(m11);
    mat2 i00 = adjoint(m00 - m10*t*m01);
    mat2 i10 = -i00 * (m10*t);
    mat2 i01 = -(t*m01) * i00;
    mat2 i11 = t + i01*m10*t;

    // stuff back into 4x4 form
    return mat4(i00[0][0],i00[0][1],i01[0][0],i01[0][1],
		i00[1][0],i00[1][1],i01[1][0],i01[1][1],
		i10[0][0],i10[0][1],i11[0][0],i11[0][1],
		i10[1][0],i10[1][1],i11[1][0],i11[1][1]);
}

#endif
