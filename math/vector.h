#ifndef _VECTOR_H
#define _VECTOR_H

#include <math.h>

class Vector {
public:
	Vector()
	{
	}
	
	Vector(const float x, const float y, const float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}
	
	~Vector()
	{
	}
	
	inline Vector operator+(const Vector v) const
	{
		return Vector(this->x + v.x, this->y + v.y, this->z + v.z);
	}
	
	inline Vector &operator+=(const Vector v)
	{
		this->x += v.x;
		this->y += v.y;
		this->z += v.z;

		return *this;
	}
	
	inline Vector operator-(const Vector v) const
	{
		return Vector(this->x - v.x, this->y - v.y, this->z - v.z);
	}
	
	inline Vector &operator-=(const Vector v)
	{
		this->x -= v.x;
		this->y -= v.y;
		this->z -= v.z;

		return *this;
	}
	
	inline Vector operator*(const float s) const
	{
		return Vector(this->x * s, this->y * s, this->z * s);
	}
	
	inline Vector &operator*=(const float s)
	{
		this->x *= s;
		this->y *= s;
		this->z *= s;

		return *this;
	}
	
	inline float operator*(const Vector v) const
	{
		return (this->x * v.x + this->y * v.y + this->z * v.z) /
			(float)sqrt((this->x * this->x + this->y * this->y + this->z * this->z) *
			     (v.x * v.x + v.y * v.y + v.z * v.z));
	}
	
	inline Vector operator/(const float s) const
	{
		return *this * (1.0f / s);
	}
	
	inline Vector &operator/=(const float s)
	{
		*this *= (1.0f / s);
		return *this;
	}
	
	inline Vector cross_product(const Vector v) const
	{
		return Vector(this->y * v.z - this->z * v.y,
			      this->z * v.x - this->x * v.z,
			      this->x * v.y - this->y * v.x);
	}
	
	inline float magnitude() const
	{
		return sqrt(this->x * this->x +
			    this->y * this->y +
			    this->z * this->z);
	}
	
	inline Vector &normalize()
	{
		*this *= (1.0f / this->magnitude());
		return *this;
	}

	float x, y, z;
};

#endif /* defined(_VECTOR_H) */
