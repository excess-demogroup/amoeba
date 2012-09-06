#ifndef _MATRIX_H
#define _MATRIX_H

#include <stdio.h>
#include "math/vector.h"

typedef float MatrixGL[16];
typedef float Matrix3DS[12];

class Matrix{
public:	
	float matrix[4][4];

	Matrix(){ Identity(); }
	
	Matrix(const Matrix3DS &m) {
		Identity();
		matrix[0][0] = m[0]; matrix[0][1] = m[1 ];  matrix[0][2] = m[2 ]; matrix[0][3] = 0;
		matrix[1][0] = m[3]; matrix[1][1] = m[4 ];  matrix[1][2] = m[5 ]; matrix[1][3] = 0;
		matrix[2][0] = m[6]; matrix[2][1] = m[7 ];  matrix[2][2] = m[8 ]; matrix[2][3] = 0;
		matrix[3][0] = m[9]; matrix[3][1] = m[10];  matrix[3][2] = m[11]; matrix[3][3] = 1;
		swap();
	}
	Matrix(const MatrixGL &m) {
		Identity();
		matrix[0][0] = m[ 0]; matrix[1][0] = m[ 1];  matrix[2][0] = m[ 2]; matrix[3][0] = m[ 3];
		matrix[0][1] = m[ 4]; matrix[1][1] = m[ 5];  matrix[2][1] = m[ 6]; matrix[3][1] = m[ 7];
		matrix[0][2] = m[ 8]; matrix[1][2] = m[ 9];  matrix[2][2] = m[10]; matrix[3][2] = m[11];
		matrix[0][3] = m[12]; matrix[1][3] = m[13];  matrix[2][3] = m[14]; matrix[3][3] = m[15];
	}

	/* special thanks to Farbrausch ;-) */
	Matrix(const float x, const float y, const float z, const float w)
	{
		/*
		      ( 1-yy-zz , xy+wz   , xz-wy   )
		  T = ( xy-wz   , 1-xx-zz , yz+wx   )
		      ( xz+wy   , yz-wx   , 1-xx-yy )
		 */
		float fx2, fy2, fz2, fwx, fwy, fwz, fxx, fxy, fxz, fyy, fyz, fzz;

		fx2 = x + x;    fy2 = y + y;    fz2 = z + z;
		fwx = w * fx2;  fwy = w * fy2;  fwz = w * fz2;
		fxx = x * fx2;  fxy = x * fy2;  fxz = x * fz2;
		fyy = y * fy2;  fyz = y * fz2;  fzz = z * fz2;

		matrix[0][0] = (float)1.0 - (fyy + fzz);
		matrix[0][1] = (float)fxy + fwz;
		matrix[0][2] = (float)fxz - fwy;
		matrix[0][3] = 0.0f;
		
		matrix[1][0] = (float)fxy - fwz;
		matrix[1][1] = (float)1.0f - (fxx + fzz);
		matrix[1][2] = (float)fyz + fwx;
		matrix[1][3] = 0.0f;
		
		matrix[2][0] = (float)fxz + fwy;
		matrix[2][1] = (float)fyz - fwx;
		matrix[2][2] = (float)1.0f - (fxx + fyy);
		matrix[2][3] = 0.0f;

		matrix[3][0] = 0.0f;
		matrix[3][1] = 0.0f;
		matrix[3][2] = 0.0f;
		matrix[3][3] = 1.0f;
	}

	inline Matrix operator *=(const Matrix &a) {
		Matrix temp;
		int i;
		for (i=0; i<3; i++)
			for (int j=0; j<3; j++)
				temp.matrix[i][j] =	(matrix[i][0]*a.matrix[0][j])+
							(matrix[i][1]*a.matrix[1][j])+
							(matrix[i][2]*a.matrix[2][j]);
		for (i=0; i<3; i++)
			for (int j=0; j<3; j++)
				matrix[i][j] = temp.matrix[i][j];
		return temp;
	}

	inline Matrix operator +=(const Matrix &a) {
		Matrix temp;
		for (int i=0; i<3; i++)
			for (int j=0; j<3; j++)
				temp.matrix[i][j] = matrix[i][j] + a.matrix[i][j];
		return temp;
	}

	inline Matrix operator*(const Matrix &a) const {
		Matrix temp;
		for (int i=0; i<3; i++)
			for (int j=0; j<3; j++)
				temp.matrix[i][j] =	(matrix[i][0]*a.matrix[0][j])+
							(matrix[i][1]*a.matrix[1][j])+
							(matrix[i][2]*a.matrix[2][j]);
		return temp;
	}

	inline Vector operator*(const Vector &v) const {
		Vector temp;
		temp.x = (v.x*matrix[0][0])+(v.y*matrix[0][1])+(v.z*matrix[0][2])+matrix[0][3];
		temp.y = (v.x*matrix[1][0])+(v.y*matrix[1][1])+(v.z*matrix[1][2])+matrix[1][3];
		temp.z = (v.x*matrix[2][0])+(v.y*matrix[2][1])+(v.z*matrix[2][2])+matrix[2][3];
		return temp;
	}

	inline Matrix operator+(const Matrix &a) const {
		Matrix temp;
		for (int i=0; i<3; i++)
			for (int j=0; j<3; j++)
				temp.matrix[i][j] = matrix[i][j] + a.matrix[i][j];
		return temp;
	}

	inline void Identity() {
		matrix[0][0] = 1; matrix[1][0] = 0; matrix[2][0] = 0; matrix[3][0] = 0;
		matrix[0][1] = 0; matrix[1][1] = 1; matrix[2][1] = 0; matrix[3][1] = 0;
		matrix[0][2] = 0; matrix[1][2] = 0; matrix[2][2] = 1; matrix[3][2] = 0;
		matrix[0][3] = 0; matrix[1][3] = 0; matrix[2][3] = 0; matrix[3][3] = 1;
	}
	inline Matrix operator =(Matrix3DS &m) {
		return Matrix(m);
	}

	inline Matrix Invscale() {
		Matrix temp;
		
		for(int i=0; i<4; i++){
			float scale = (matrix[i][0] * matrix[i][0]) +
				      (matrix[i][1] * matrix[i][1]) +
				      (matrix[i][2] * matrix[i][2]);
			scale = 1.0f / scale;
			for( int j = 0; j < 4; j++ ) temp.matrix[i][j] = matrix[i][j] * scale;			
		}
		return temp;
	}

	inline void swap() {
		float tmp;
		int i;
		for (i = 0; i < 3; i++) {
			tmp = matrix[i][1];
			matrix[i][1] = matrix[i][2];
			matrix[i][2] = tmp;
		}
		for (i = 0; i < 3; i++) {
			tmp = matrix[1][i];
			matrix[1][i] = matrix[2][i];
			matrix[2][i] = tmp;
		}
		tmp = matrix[3][1]; matrix[3][1] = matrix[3][2]; matrix[3][2] = tmp;
	}

	inline Matrix transpose() {
		Matrix temp;
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				temp.matrix[i][j] = matrix[j][i];

		const float tx = matrix[0][3];
		const float ty = matrix[1][3];
		const float tz = matrix[2][3];
		
		temp.matrix[0][3] = -(temp.matrix[0][0] * tx +
				      temp.matrix[0][1] * ty +
				      temp.matrix[0][2] * tz);
		temp.matrix[1][3] = -(temp.matrix[1][0] * tx +
				      temp.matrix[1][1] * ty +
				      temp.matrix[1][2] * tz);
		temp.matrix[2][3] = -(temp.matrix[2][0] * tx +
				      temp.matrix[2][1] * ty +
				      temp.matrix[2][2] * tz);
		*this = temp;
		return *this;
	}
	
	void ToGL(float *temp) {
		temp[0 ] = matrix[0][0];
		temp[1 ] = matrix[0][1];
		temp[2 ] = matrix[0][2];
		temp[3 ] = matrix[0][3];
		temp[4 ] = matrix[1][0];
		temp[5 ] = matrix[1][1];
		temp[6 ] = matrix[1][2];
		temp[7 ] = matrix[1][3];
		temp[8 ] = matrix[2][0];
		temp[9 ] = matrix[2][1];
		temp[10] = matrix[2][2];
		temp[11] = matrix[2][3];
		temp[12] = matrix[3][0];
		temp[13] = matrix[3][1];
		temp[14] = matrix[2][2];
		temp[15] = matrix[3][3];
	}

	void print() {
		printf("%5.3f %5.3f %5.3f %5.3f\n", matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3]);
		printf("%5.3f %5.3f %5.3f %5.3f\n", matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3]);
		printf("%5.3f %5.3f %5.3f %5.3f\n", matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3]);
		printf("%5.3f %5.3f %5.3f %5.3f\n", matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]);
		printf("\n");
	}
};

#endif // _MATRIX_H
