

#ifndef VEC3F_H
#define VEC3F_H

class Matrix;

class Vec3f
{
public:
	float x, y, z;

	Vec3f()
	{
		x = y = z = 0;
	}

	Vec3f(float X, float Y, float Z)
	{
		x = X;
		y = Y;
		z = Z;
	}

	Vec3f(const float* values)
	{
		set(values);
	}

	bool operator==(const Vec3f vVector) const
	{
		if(x == vVector.x && y == vVector.y && z == vVector.z)
			return true;

		return false;
	}

	bool operator!=(const Vec3f vVector) const
	{
		if(x == vVector.x && y == vVector.y && z == vVector.z)
			return false;

		return true;
	}

	Vec3f operator+(const Vec3f vVector) const
	{
		return Vec3f(vVector.x + x, vVector.y + y, vVector.z + z);
	}

	Vec3f operator-(const Vec3f vVector) const
	{
		return Vec3f(x - vVector.x, y - vVector.y, z - vVector.z);
	}

	Vec3f operator*(const float num) const
	{
		return Vec3f(x * num, y * num, z * num);
	}

	Vec3f operator*(const Vec3f v) const
	{
		return Vec3f(x * v.x, y * v.y, z * v.z);
	}

	Vec3f operator/(const float num) const
	{
		return Vec3f(x / num, y / num, z / num);
	}

	float& operator[](int i)
	{
		return (float&)*(((float*)(this))+i);
	}

	inline void set(const float* values)
	{
		x = values[0];
		y = values[1];
		z = values[2];
	}

	void transform(const Matrix& m);
	void transform3(const Matrix& m);


    Vec3f& operator+=(const Vec3f& r)
    {
        x += r.x;
        y += r.y;
        z += r.z;

        return *this;
    }

    Vec3f& operator-=(const Vec3f& r)
    {
        x -= r.x;
        y -= r.y;
        z -= r.z;

        return *this;
    }

    Vec3f& operator*=(float f)
    {
        x *= f;
        y *= f;
        z *= f;

        return *this;
    }

    Vec3f Cross(const Vec3f& v) const;

    Vec3f& Normalize();

    void Rotate(float Angle, const Vec3f& Axis);
};

bool Close(Vec3f a, Vec3f b);

#endif
