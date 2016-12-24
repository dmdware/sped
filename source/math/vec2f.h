

#ifndef VEC2F_H
#define VEC2F_H

class Vec2f
{
public:
	float x, y;

	Vec2f() { x = y = 0; }

	Vec2f(float X, float Y)
	{
		x = X; y = Y;
	}
};

#endif