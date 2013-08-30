// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic\Types\Matrix4.h"
#include "Generic\Math\Math.h"
#include <string.h>

Matrix4::Matrix4()
{
	memset(Elements, 0, sizeof(float) * 16);
}

Matrix4::Matrix4(float c11, float c12, float c13, float c14,
				 float c21, float c22, float c23, float c24,
				 float c31, float c32, float c33, float c34,
				 float c41, float c42, float c43, float c44)
{
	Elements[0] = c11;
	Elements[1] = c12;
	Elements[2] = c13;
	Elements[3] = c14;
	Elements[4] = c21;
	Elements[5] = c22;
	Elements[6] = c23;
	Elements[7] = c24;
	Elements[8] = c31;
	Elements[9] = c32;
	Elements[10] = c33;
	Elements[11] = c34;
	Elements[12] = c41;
	Elements[13] = c42;
	Elements[14] = c43;
	Elements[15] = c44;
}

void Matrix4::SetColumn(int column, float e1, float e2, float e3, float e4)
{
	Elements[(column * 4)]	   = e1;
	Elements[(column * 4) + 1] = e2;
	Elements[(column * 4) + 2] = e3;
	Elements[(column * 4) + 3] = e4;
}

void Matrix4::SetRow(int column, float e1, float e2, float e3, float e4)
{
	Elements[(column * 0)] = e1;
	Elements[(column * 1)] = e2;
	Elements[(column * 2)] = e3;
	Elements[(column * 3)] = e4;
}

Matrix4 Matrix4::operator*(const Matrix4 a) const
{
	const float* m = Elements;
	const float* n = a.Elements;

    return Matrix4(m[0]*n[0]  + m[1]*n[4]  + m[2]*n[8]  + m[3]*n[12],   m[0]*n[1]  + m[1]*n[5]  + m[2]*n[9]  + m[3]*n[13],   m[0]*n[2]  + m[1]*n[6]  + m[2]*n[10]  + m[3]*n[14],   m[0]*n[3]  + m[1]*n[7]  + m[2]*n[11]  + m[3]*n[15],
                   m[4]*n[0]  + m[5]*n[4]  + m[6]*n[8]  + m[7]*n[12],   m[4]*n[1]  + m[5]*n[5]  + m[6]*n[9]  + m[7]*n[13],   m[4]*n[2]  + m[5]*n[6]  + m[6]*n[10]  + m[7]*n[14],   m[4]*n[3]  + m[5]*n[7]  + m[6]*n[11]  + m[7]*n[15],
                   m[8]*n[0]  + m[9]*n[4]  + m[10]*n[8] + m[11]*n[12],  m[8]*n[1]  + m[9]*n[5]  + m[10]*n[9] + m[11]*n[13],  m[8]*n[2]  + m[9]*n[6]  + m[10]*n[10] + m[11]*n[14],  m[8]*n[3]  + m[9]*n[7]  + m[10]*n[11] + m[11]*n[15],
                   m[12]*n[0] + m[13]*n[4] + m[14]*n[8] + m[15]*n[12],  m[12]*n[1] + m[13]*n[5] + m[14]*n[9] + m[15]*n[13],  m[12]*n[2] + m[13]*n[6] + m[14]*n[10] + m[15]*n[14],  m[12]*n[3] + m[13]*n[7] + m[14]*n[11] + m[15]*n[15]);
}

Matrix4 Matrix4::Identity()
{
	Matrix4 mat;

	mat.SetColumn(0, 1, 0, 0, 0);
	mat.SetColumn(1, 0, 1, 0, 0);
	mat.SetColumn(2, 0, 0, 1, 0);
	mat.SetColumn(3, 0, 0, 0, 1);
	
	return mat;
}

Matrix4 Matrix4::Frustum(float l, float r, float b, float t, float n, float f)
{
	float X = (2 * n) / (r - l);
	float Y = (2 * n) / (t - b);

	float A = (r + l) / (r - l);
	float B = (t + b) / (t - b);
	float C = -(f + n) / (f - n);
	float D = -(2 * f * n) / (f - n);

	Matrix4 mat;
	mat.SetColumn(0, X, 0, A, 0);
	mat.SetColumn(1, 0, Y, B, 0);
	mat.SetColumn(2, 0, 0, C, D);
	mat.SetColumn(3, 0, 0, -1, 0);
	return mat;
}

Matrix4 Matrix4::Perspective(float fov, float aspect_ratio, float z_near, float z_far)
{
	float xmin, xmax, ymin, ymax;

	ymax = z_near * tan(fov * PI / 360.0);
	ymin = -ymax;
	xmin = ymin * aspect_ratio;
	xmax = ymax * aspect_ratio;

	return Frustum(xmin, xmax, ymin, ymax, z_near, z_far);
}	

Matrix4 Matrix4::LookAt(Vector3 eye, Vector3 center, Vector3 up)
{
	Vector3 z = (eye - center).Normalize();
	Vector3 x = z.Cross(up).Normalize();
	Vector3 y = z.Cross(x);

	Matrix4 m1;
	m1.SetColumn(0, x.X, x.Y, x.Z, 0);
	m1.SetColumn(1, y.X, y.Y, y.Z, 0);
	m1.SetColumn(2, z.X, z.Y, z.Z, 0);
	m1.SetColumn(3, 0,   0,   0,   1);

	Matrix4 m2;
	m2.SetColumn(0, 1, 0, 0, 0);
	m2.SetColumn(1, 0, 1, 0, 0);
	m2.SetColumn(2, 0, 0, 1, 0);
	m2.SetColumn(3, -eye.X, -eye.Y, -eye.Z, 1);

	return m1 * m2;
}
