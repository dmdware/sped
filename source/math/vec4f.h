

#ifndef VEC4F_H
#define VEC4F_H

class Vec3f;

class Matrix;

class Vec4f
{
public:
	float x, y, z, w;

	Vec4f() {}

	Vec4f(float X, float Y, float Z, float W) 
	{ 
		x = X; y = Y; z = Z; w = W;
	}

	Vec4f(const float* values)
	{
		set(values);
	}

	Vec4f(const Vec3f V, const float W);

	bool operator==(const Vec4f vVector)
	{
		if(x == vVector.x && y == vVector.y && z == vVector.z && w == vVector.w)
			return true;

		return false;
	}

	bool operator!=(const Vec4f vVector)
	{
		if(x == vVector.x && y == vVector.y && z == vVector.z && w == vVector.w)
			return false;

		return true;
	}

	Vec4f operator+(const Vec4f vVector)
	{
		return Vec4f(vVector.x + x, vVector.y + y, vVector.z + z, vVector.w + w);
	}

	Vec4f operator-(const Vec4f vVector)
	{
		return Vec4f(x - vVector.x, y - vVector.y, z - vVector.z, w - vVector.w);
	}
	
	Vec4f operator*(const float num)
	{
		return Vec4f(x * num, y * num, z * num, w);
	}
	
	Vec4f operator*(const Vec4f v)
	{
		return Vec4f(x * v.x, y * v.y, z * v.z, w);
	}

	Vec4f operator/(const float num)
	{
		return Vec4f(x / num, y / num, z / num, w);
	}

	inline void set(const float* values)
	{
		x = values[0];
		y = values[1];
		z = values[2];
		w = values[3];
	}

	void transform(const Matrix& m);
	void transform3(const Matrix& m);					
};

#endif