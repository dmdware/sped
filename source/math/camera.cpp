
#include "camera.h"
#include "math3d.h"
#include "physics.h"

Camera g_cam;

Vec3f Camera::up2()		
{
	return Normalize( Cross( m_strafe, m_view - m_pos ) );
}

float Camera::yaw()
{
	return m_orientv.y;
}

float Camera::pitch()
{
	return m_orientv.x;
}

void Camera::calcyaw()
{
	Vec3f d = m_view - m_pos;
	m_orientv.y = GetYaw(d.x, d.z);
}

void Camera::calcpitch()
{
	Vec3f d = m_view - m_pos;
	float lateral = Magnitude(Vec3f(d.x, 0, d.z));
	m_orientv.x = RADTODEG( atan2(d.y, lateral) );
}

void Camera::calcroll()
{
	Vec3f d = m_view - m_pos;
	float lateral = Magnitude(Vec3f(d.x, 0, d.z));
	m_orientv.x = RADTODEG( atan2(d.y, lateral) );
}

void Camera::frameupd()
{
	m_pos = m_pos + m_vel;
	m_view = m_view + m_vel;

	//Vec3f vNew = m_pos + m_vel * g_drawfrinterval;
	//MoveTo(vNew);
}

Vec3f Camera::lookpos()
{
	return m_pos;

	/*
	if(g_viewmode == FIRSTPERSON)
		return m_pos;
	else
	{
		Vec3f d = Normalize(m_view - m_pos);
		Vec3f vLine[2];
		vLine[0] = m_pos;
		vLine[1] = m_pos - d * 64.0f;

		vLine[1] = g_edmap.TraceRay(m_pos, vLine[1]);

		CEntity* e;
		int localE = g_player[g_localP].entity;
		for(int i=0; i<ENTITIES; i++)
		{
			e = &g_entity[i];

			if(!e->on)
				continue;

			if(localE == i)
				continue;

			vLine[1] = e->TraceRay(vLine);
		}

		return vLine[1];
	}*/
}

Camera::Camera()
{
	m_pos	= Vec3f(0.0, 0.0, 0.0);
	m_view	= Vec3f(0.0, -1.0, 1.0);
	m_up	= Vec3f(0.0, 1.0, 0.0);

	m_grounded = false;

	m_orientv.x = 0;
	m_orientv.y = 0;

	calcstrafe();
}

Camera::Camera(float posx, float posy, float posz, float viewx, float viewy, float viewz, float upx, float upy, float upz)
{
	position(posx, posy, posz, viewx, viewy, viewz, upx, upy, upz);
}

void Camera::position(float posx, float posy, float posz, float viewx, float viewy, float viewz, float upx, float upy, float upz)
{
	m_pos = Vec3f(posx, posy, posz);
	m_view  = Vec3f(viewx, viewy, viewz);
	m_up = Vec3f(upx, upy, upz);

	calcstrafe();
	calcyaw();
	calcpitch();
}

void Camera::rotatebymoUseS(int dx, int dy)
{
	if( (dx == 0) && (dy == 0) ) return;

	// Get the direction the mouse moved in, but bring the number down to a reasonable amount
	float angleY = (float)( -dx ) * MOUSE_SENSITIVITY;
	float angleZ = (float)( -dy ) * MOUSE_SENSITIVITY;

	static float lastRotX = 0.0f; 
 	lastRotX = m_orientv.x; // We store off the currentRotX and will use it in when the angle is capped
	
	// Here we keep track of the current rotation (for up and down) so that
	// we can restrict the camera from doing a full 360 loop.
	m_orientv.x += angleZ;
 
	// If the current rotation (in radians) is greater than 1.0, we want to cap it.
	if(m_orientv.x > 1.0f)     
	{
		m_orientv.x = 1.0f;
		
		// Rotate by remaining angle if there is any
		if(lastRotX != 1.0f) 
		{
			// To find the axis we need to rotate around for up and down
			// movements, we need to get a perpendicular vector from the
			// camera's view vector and up vector.  This will be the axis.
			// Before using the axis, it's a good idea to normalize it first.
			Vec3f vAxis = Cross(m_view - m_pos, m_up);
			vAxis = Normalize(vAxis);
				
			// rotate the camera by the remaining angle (1.0f - lastRotX)
			rotateview( 1.0f - lastRotX, vAxis.x, vAxis.y, vAxis.z);
		}
	}
	// Check if the rotation is below -1.0, if so we want to make sure it doesn't continue
	else if(m_orientv.x < -1.0f)
	{
		m_orientv.x = -1.0f;
		
		// Rotate by the remaining angle if there is any
		if(lastRotX != -1.0f)
		{
			// To find the axis we need to rotate around for up and down
			// movements, we need to get a perpendicular vector from the
			// camera's view vector and up vector.  This will be the axis.
			// Before using the axis, it's a good idea to normalize it first.
			Vec3f vAxis = Cross(m_view - m_pos, m_up);
			vAxis = Normalize(vAxis);
			
			// rotate the camera by ( -1.0f - lastRotX)
			rotateview( -1.0f - lastRotX, vAxis.x, vAxis.y, vAxis.z);
		}
	}
	// Otherwise, we can rotate the view around our position
	else 
	{	
		// To find the axis we need to rotate around for up and down
		// movements, we need to get a perpendicular vector from the
		// camera's view vector and up vector.  This will be the axis.
		// Before using the axis, it's a good idea to normalize it first.
		Vec3f vAxis = Cross(m_view - m_pos, m_up);
		vAxis = Normalize(vAxis);
	
		// Rotate around our perpendicular axis
		rotateview(angleZ, vAxis.x, vAxis.y, vAxis.z);
	}

	// Always rotate the camera around the y-axis
	rotateview(angleY, 0, 1, 0);
	calcstrafe();
	calcyaw();
	//calcpitch();
}

void Camera::rotateabout(Vec3f center, float rad, float x, float y, float z)
{
	m_view = RotateAround(m_view, center, rad, x, y, z);
	m_pos = RotateAround(m_pos, center, rad, x, y, z);
	m_up = Rotate(m_up, rad, x, y, z);
	
	calcstrafe();
	calcyaw();
}

void Camera::grounded(bool ground)
{
	m_grounded = ground;

	if(m_grounded && m_vel.y < 0.0f)
		m_vel.y = 0.0f;
}

void Camera::limithvel(float vLimit)
{
	Vec3f hVel = Vec3f(m_vel.x, 0, m_vel.z);
	float vVel = Magnitude( hVel );

	if(vVel <= vLimit)
		return;

	hVel = hVel * vLimit / vVel;

	m_vel.x = hVel.x;
	m_vel.z = hVel.z;
}

void Camera::rotateview(float angle, float x, float y, float z)
{
	Vec3f vNewView;

	Vec3f vView = m_view - m_pos;		

	float cosTheta = (float)cos(angle);
	float sinTheta = (float)sin(angle);

	vNewView.x  = (cosTheta + (1 - cosTheta) * x * x)		* vView.x;
	vNewView.x += ((1 - cosTheta) * x * y - z * sinTheta)	* vView.y;
	vNewView.x += ((1 - cosTheta) * x * z + y * sinTheta)	* vView.z;

	vNewView.y  = ((1 - cosTheta) * x * y + z * sinTheta)	* vView.x;
	vNewView.y += (cosTheta + (1 - cosTheta) * y * y)		* vView.y;
	vNewView.y += ((1 - cosTheta) * y * z - x * sinTheta)	* vView.z;

	vNewView.z  = ((1 - cosTheta) * x * z - y * sinTheta)	* vView.x;
	vNewView.z += ((1 - cosTheta) * y * z + x * sinTheta)	* vView.y;
	vNewView.z += (cosTheta + (1 - cosTheta) * z * z)		* vView.z;

	m_view = m_pos + vNewView;
	calcyaw();
	calcstrafe();
}

void Camera::lookat(Vec3f at)
{
	m_view = at;
	calcyaw();
	calcstrafe();
}

/*
void Camera::Strafe(float speed)
{
	m_vel.x += m_strafe.x * speed;
	m_vel.z += m_strafe.z * speed;
}

void Camera::Move(float speed)
{
	Vec3f vVector = m_view - m_pos;

	vVector = Normalize(vVector);

	m_vel.x += vVector.x * speed;
	m_vel.z += vVector.z * speed;
}
*/

void Camera::strafe(float speed)
{
	m_pos.x += m_strafe.x * speed;
	m_pos.z += m_strafe.z * speed;

	m_view.x += m_strafe.x * speed;
	m_view.z += m_strafe.z * speed;
}

void Camera::move(float speed)
{
	Vec3f vVector = m_view - m_pos;

	vVector = Normalize(vVector);

	m_pos.x += vVector.x * speed;
	m_pos.z += vVector.z * speed;
	m_view.x += vVector.x * speed;
	m_view.z += vVector.z * speed;
}

//bool g_debug2 = false;

void Camera::accelerate(float speed)
{
	Vec3f vVector = m_view - m_pos;

	vVector = Normalize(vVector);
	
	m_vel.x += vVector.x * speed;
	m_vel.z += vVector.z * speed;
}

void Camera::accelstrafe(float speed)
{
	Vec3f vVector = Normalize(m_strafe);
	
	m_vel.x += vVector.x * speed;
	m_vel.z += vVector.z * speed;
}

void Camera::accelrise(float speed)
{
	Vec3f vVector = Normalize(m_up);
	
	m_vel.y += vVector.y * speed;
}

void Camera::friction()
{
	m_vel.x *= INVFRICTION;
	m_vel.z *= INVFRICTION;
}

void Camera::friction2()
{
	m_vel.x *= INVFRICTION;
	m_vel.y *= INVFRICTION;
	m_vel.z *= INVFRICTION;
}

/*
void Camera::Rise(float speed)
{
	Vec3f vVector = m_up;

	vVector = Normalize(vVector);

	m_vel.y += vVector.y * speed;
}		
*/

void Camera::rise(float speed)
{
	Vec3f up = Normalize(m_up);

	m_pos.y += up.y * speed;
	m_view.y += up.y * speed; 
}	

void Camera::move(Vec3f delta)
{
	m_pos = m_pos + delta;
	m_view = m_view + delta;
}

void Camera::moveto(Vec3f newpos)
{
	Vec3f delta = newpos - m_pos;
	//Move(delta);
	m_view = m_view + delta;
	m_pos = newpos;
}

void Camera::stop()
{
	m_vel = Vec3f(0, 0, 0);
}

void Camera::calcstrafe() 
{
	Vec3f vCross = Cross(Normalize(m_view - m_pos), m_up);
	m_strafe = Normalize(vCross);
}
