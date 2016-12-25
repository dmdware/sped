




#include "math3d.h"
#include "../utils.h"
#include "physics.h"
#include "../window.h"
#include "../gui/gui.h"
#include "vec3f.h"
#include "camera.h"
#include "quaternion.h"
#include "vec4f.h"
#include "matrix.h"
#include "plane.h"
#include "../window.h"

float Clipf(float n, float lower, float upper)
{
  return fmax(lower, fmin(n, upper));
}

int Clipi(int n, int lower, int upper)
{
  return imax(lower, imin(n, upper));
}

Vec3f VMin(float minf, Vec3f v)
{
	v.x = fmin(minf, v.x);
	v.y = fmin(minf, v.y);
	v.z = fmin(minf, v.z);
	return v;
}

float Magnitude(Vec3f vNormal)
{
	return (float)sqrt( (vNormal.x * vNormal.x) + (vNormal.y * vNormal.y) + (vNormal.z * vNormal.z) );
}
float Magnitude(Vec2f vNormal)
{
	return (float)sqrt( (vNormal.x * vNormal.x) + (vNormal.y * vNormal.y) );
}

float Magnitude2(Vec3f vNormal)
{
	return (vNormal.x * vNormal.x) + (vNormal.y * vNormal.y) + (vNormal.z * vNormal.z);
}

float Magnitude3(Vec3f vNormal)
{
	return (vNormal.x * vNormal.x) + (vNormal.z * vNormal.z);
}

Vec3f Normalize(Vec3f vNormal)
{
	float magnitude = Magnitude(vNormal);

	vNormal.x /= magnitude;
	vNormal.y /= magnitude;
	vNormal.z /= magnitude;

	return vNormal;
}

Vec3f Cross(Vec3f vVector1, Vec3f vVector2)
{
	Vec3f vNormal;

	vNormal.x = ((vVector1.y * vVector2.z) - (vVector1.z * vVector2.y));
	vNormal.y = ((vVector1.z * vVector2.x) - (vVector1.x * vVector2.z));
	vNormal.z = ((vVector1.x * vVector2.y) - (vVector1.y * vVector2.x));

	return vNormal;
}


Vec3f Vector(Vec3f vPoint1, Vec3f vPoint2)
{
	Vec3f vVector;

	vVector.x = vPoint1.x - vPoint2.x;
	vVector.y = vPoint1.y - vPoint2.y;
	vVector.z = vPoint1.z - vPoint2.z;

	return vVector;
}

// Clockwise
Vec3f Normal(Vec3f vTriangle[])
{
	Vec3f vVector1 = Vector(vTriangle[2], vTriangle[0]);
	Vec3f vVector2 = Vector(vTriangle[1], vTriangle[0]);

	Vec3f vNormal = Cross(vVector1, vVector2);

	vNormal = Normalize(vNormal);

	return vNormal;
}

// Counter-clockwise
Vec3f Normal2(Vec3f vTriangle[])
{
	Vec3f vVector1 = Vector(vTriangle[2], vTriangle[0]);
	Vec3f vVector2 = Vector(vTriangle[1], vTriangle[0]);

	Vec3f vNormal = Cross(vVector2, vVector1);

	vNormal = Normalize(vNormal);

	return vNormal;
}

bool IntersectedPlane(Vec3f vPoly[], Vec3f vLine[], Vec3f &vNormal, float &originDistance)
{
	float distance1=0, distance2=0;						// The distances from the 2 points of the line from the plane

	vNormal = Normal(vPoly);

	// Let's find the distance our plane is from the origin.  We can find this value
	// from the normal to the plane (polygon) and any point that lies on that plane (Any vertice)
	originDistance = PlaneDistance(vNormal, vPoly[0]);

	// Get the distance from point1 from the plane using: Ax + By + Cz + D = (The distance from the plane)

	distance1 = ((vNormal.x * vLine[0].x)  +					// Ax +
		         (vNormal.y * vLine[0].y)  +					// Bx +
				 (vNormal.z * vLine[0].z)) + originDistance;	// Cz + D

	// Get the distance from point2 from the plane using Ax + By + Cz + D = (The distance from the plane)

	distance2 = ((vNormal.x * vLine[1].x)  +					// Ax +
		         (vNormal.y * vLine[1].y)  +					// Bx +
				 (vNormal.z * vLine[1].z)) + originDistance;	// Cz + D

	// Now that we have 2 distances from the plane, if we times them together we either
	// get a positive or negative number.  If it's a negative number, that means we collided!
	// This is because the 2 points must be on either side of the plane (IE. -1 * 1 = -1).

	if(distance1 * distance2 >= 0)			// Check to see if both point's distances are both negative or both positive
	   return false;

	return true;
}

//#define MATH_DEBUG

// http://en.wikipedia.org/wiki/Line-plane_intersection
bool Intersection(Vec3f l0, Vec3f l, Plane3f p, Vec3f& inter)
{
	float denom = Dot(l, p.m_normal);

#ifdef MATH_DEBUG
	Log("intersection denom="<<denom<<" (plane=n("<<p.m_normal.x<<","<<p.m_normal.y<<","<<p.m_normal.z<<")d="<<p.m_d<<") (line=("<<l.x<<","<<l.y<<","<<l.z<<"))");
	
#endif

	if(fabs(denom) <= EPSILON/2.0f)
	//if(denom == 0.0f)
		return false;

	Vec3f p0 = PointOnPlane(p);

#ifdef MATH_DEBUG
	Log("intersection pointonplane="<<p0.x<<","<<p0.y<<","<<p0.z<<std::endl;
	
#endif

	float num = Dot(p0 - l0, p.m_normal);

	inter = l0 + l * (num/denom);
	return true;
}

float Dot(Vec3f vVector1, Vec3f vVector2)
{
	return ( (vVector1.x * vVector2.x) + (vVector1.y * vVector2.y) + (vVector1.z * vVector2.z) );
}
float Dot(Vec2f vVector1, Vec2f vVector2)
{
	return ( (vVector1.x * vVector2.x) + (vVector1.y * vVector2.y) );
}

double AngleBetweenVectors(Vec3f Vector1, Vec3f Vector2)
{
	float dotProduct = Dot(Vector1, Vector2);
	float vectorsMagnitude = Magnitude(Vector1) * Magnitude(Vector2) ;
	double angle = acos( dotProduct / vectorsMagnitude );

	if(_isnan(angle))
		return 0;

	return( angle );
}

bool WithinYaw(Camera* c, Vec3f p, float angle)
{
	Vec3f d = p - c->m_pos;
	float yaw = GetYaw(d.x, d.z);
	float yaw2 = yaw - DEGTORAD(360.0f);
	float yaw3 = yaw + DEGTORAD(360.0f);

	if(fabs(c->yaw() - yaw) <= angle || fabs(c->yaw() - yaw2) <= angle || fabs(c->yaw() - yaw3) <= angle)
		return true;

	return false;
}

float DYaw(Camera* c, Vec3f p)
{
	Vec3f d = p - c->m_pos;
	float yaw = GetYaw(d.x, d.z);
	float yaw2 = yaw - DEGTORAD(360.0f);
	float yaw3 = yaw + DEGTORAD(360.0f);

	float dyaw = yaw - c->yaw();
	float dyaw2 = yaw2 - c->yaw();
	float dyaw3 = yaw3 - c->yaw();

	float mindyaw = dyaw;

	if(fabs(dyaw2) < fabs(mindyaw))
		mindyaw = dyaw2;

	if(fabs(dyaw3) < fabs(mindyaw))
		mindyaw = dyaw3;

	return mindyaw;
}

Vec3f IntersectionPoint(Vec3f vNormal, Vec3f vLine[], double distance)
{
	Vec3f vPoint;
	Vec3f vLineDir;
	double Numerator = 0.0, Denominator = 0.0, dist = 0.0;

	// Here comes the confusing part.  We need to find the 3D point that is actually
	// on the plane.  Here are some steps to do that:

	// 1)  First we need to get the vector of our line, Then normalize it so it's a length of 1
	vLineDir = Vector(vLine[1], vLine[0]);		// Get the Vector of the line
	vLineDir = Normalize(vLineDir);				// Normalize the lines vector


	// 2) Use the plane equation (distance = Ax + By + Cz + D) to find the distance from one of our points to the plane.
	//    Here I just chose a arbitrary point as the point to find that distance.  You notice we negate that
	//    distance.  We negate the distance because we want to eventually go BACKWARDS from our point to the plane.
	//    By doing this is will basically bring us back to the plane to find our intersection point.
	Numerator = - (vNormal.x * vLine[0].x +		// Use the plane equation with the normal and the line
				   vNormal.y * vLine[0].y +
				   vNormal.z * vLine[0].z + distance);

	// 3) If we take the dot product between our line vector and the normal of the polygon,
	//    this will give us the cosine of the angle between the 2 (since they are both normalized - length 1).
	//    We will then divide our Numerator by this value to find the offset towards the plane from our arbitrary point.
	Denominator = Dot(vNormal, vLineDir);		// Get the dot product of the line's vector and the normal of the plane

	// Since we are using division, we need to make sure we don't get a divide by zero error
	// If we do get a 0, that means that there are INFINATE points because the the line is
	// on the plane (the normal is perpendicular to the line - (Normal.Vector = 0)).
	// In this case, we should just return any point on the line.

	if( Denominator == 0.0)						// Check so we don't divide by zero
		return vLine[0];						// Return an arbitrary point on the line

	// We divide the (distance from the point to the plane) by (the dot product)
	// to get the distance (dist) that we need to move from our arbitrary point.  We need
	// to then times this distance (dist) by our line's vector (direction).  When you times
	// a scalar (single number) by a vector you move along that vector.  That is what we are
	// doing.  We are moving from our arbitrary point we chose from the line BACK to the plane
	// along the lines vector.  It seems logical to just get the numerator, which is the distance
	// from the point to the line, and then just move back that much along the line's vector.
	// Well, the distance from the plane means the SHORTEST distance.  What about in the case that
	// the line is almost parallel with the polygon, but doesn't actually intersect it until half
	// way down the line's length.  The distance from the plane is short, but the distance from
	// the actual intersection point is pretty long.  If we divide the distance by the dot product
	// of our line vector and the normal of the plane, we get the correct length.  Cool huh?

	dist = Numerator / Denominator;				// Divide to get the multiplying (percentage) factor

	// Now, like we said above, we times the dist by the vector, then add our arbitrary point.
	// This essentially moves the point along the vector to a certain distance.  This now gives
	// us the intersection point.  Yay!

	vPoint.x = (float)(vLine[0].x + (vLineDir.x * dist));
	vPoint.y = (float)(vLine[0].y + (vLineDir.y * dist));
	vPoint.z = (float)(vLine[0].z + (vLineDir.z * dist));

	return vPoint;								// Return the intersection point
}

Vec3f Rotate(Vec3f v, float rad, float x, float y, float z)
{
	Vec3f newV;
	float cosTheta = (float)cos(rad);
	float sinTheta = (float)sin(rad);

	newV.x  = (cosTheta + (1 - cosTheta) * x * x)		* v.x;
	newV.x += ((1 - cosTheta) * x * y - z * sinTheta)	* v.y;
	newV.x += ((1 - cosTheta) * x * z + y * sinTheta)	* v.z;

	newV.y  = ((1 - cosTheta) * x * y + z * sinTheta)	* v.x;
	newV.y += (cosTheta + (1 - cosTheta) * y * y)		* v.y;
	newV.y += ((1 - cosTheta) * y * z - x * sinTheta)	* v.z;

	newV.z  = ((1 - cosTheta) * x * z - y * sinTheta)	* v.x;
	newV.z += ((1 - cosTheta) * y * z + x * sinTheta)	* v.y;
	newV.z += (cosTheta + (1 - cosTheta) * z * z)		* v.z;

	return newV;
}

Vec3f RotateAround(Vec3f v, Vec3f around, float rad, float x, float y, float z)
{
	v = v - around;
	v = Rotate(v, rad, x, y, z);
	v = around + v;
	return v;
}

float GetYaw(float dx, float dz)
{
	return RADTODEG( atan2(dx, dz) );
}

/*
// http://webglfactory.blogspot.ca/2011/06/how-to-create-view-matrix.html
Matrix lookAt(Vec3f eye, Vec3f target, Vec3f up)
{
         Vec3f vz= Normalize(eye - target);
         Vec3f vx = normalize(crossProduct(up, vz));
         // vy doesn't need to be normalized because it's a cross
         // product of 2 normalized vectors
         Vec3f vy = crossProduct(vz, vx);
         Matrix inverseViewMatrix = Matrix(Vec4f(vx, 0), Vec4f(vy, 0), Vec4f(vz, 0), Vec4f(eye, 1));
         return inverseViewMatrix.getInverse();
}
*/

Matrix LookAt(float eyex, float eyey, float eyez,
               float centerx, float centery, float centerz,
               float upx, float upy, float upz)
{
    float m[16];
    float x[3], y[3], z[3];
    float mag;

    /* Make rotation matrix */

    /* Z vector */
    z[0] = eyex - centerx;
    z[1] = eyey - centery;
    z[2] = eyez - centerz;
    mag = sqrtf(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
    if (mag) {          /* mpichler, 19950515 */
        z[0] /= mag;
        z[1] /= mag;
        z[2] /= mag;
    }

    /* Y vector */
    y[0] = upx;
    y[1] = upy;
    y[2] = upz;

    /* X vector = Y cross Z */
    x[0] = y[1] * z[2] - y[2] * z[1];
    x[1] = -y[0] * z[2] + y[2] * z[0];
    x[2] = y[0] * z[1] - y[1] * z[0];

    /* Recompute Y = Z cross X */
    y[0] = z[1] * x[2] - z[2] * x[1];
    y[1] = -z[0] * x[2] + z[2] * x[0];
    y[2] = z[0] * x[1] - z[1] * x[0];

    /* mpichler, 19950515 */
    /* cross product gives area of parallelogram, which is < 1.0 for
     * non-perpendicular unit-length vectors; so normalize x, y here
     */
#if 1
    mag = sqrtf(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
    if (mag) {
        x[0] /= mag;
        x[1] /= mag;
        x[2] /= mag;
    }

    mag = sqrtf(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
    if (mag) {
        y[0] /= mag;
        y[1] /= mag;
        y[2] /= mag;
    }
#endif

#define M(row,col)  m[col*4+row]
    M(0, 0) = x[0];
    M(0, 1) = x[1];
    M(0, 2) = x[2];
    M(0, 3) = 0.0;
    M(1, 0) = y[0];
    M(1, 1) = y[1];
    M(1, 2) = y[2];
    M(1, 3) = 0.0;
    M(2, 0) = z[0];
    M(2, 1) = z[1];
    M(2, 2) = z[2];
    M(2, 3) = 0.0;
    M(3, 0) = 0.0;
    M(3, 1) = 0.0;
    M(3, 2) = 0.0;
    M(3, 3) = 1.0;
#undef M
    //glMultMatrixf(m);
    Matrix mat;
    mat.set(m);

    /* Translate Eye to Origin */
    //glTranslatef(-eyex, -eyey, -eyez);
    Matrix mat2;
    float trans[] = {-eyex, -eyey, -eyez};
    mat2.setTranslation(trans);

    //mat.postmult(mat2);
    mat.postmult(mat2);

    return mat;
}

#if 0
Matrix LookAtOrtho(float eyex, float eyey, float eyez,
               float centerx, float centery, float centerz,
               float upx, float upy, float upz)
{
    float m[16];
    Vec3f x, y, z;
    float mag;

    /* Make rotation matrix */

    /* Z vector */
    z = Vec3f(eyex, eyey, eyez) - Vec3f(centerx, centery, centerz);
    mag = sqrtf(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
    if (mag) {          /* mpichler, 19950515 */
        z[0] /= mag;
        z[1] /= mag;
        z[2] /= mag;
    }

    /* Y vector */
    y = Vec3f(upx, upy, upz);

    //Vec3f vCross = Cross(m_view - m_pos, m_up);
    /* X vector = Y cross Z */
    x = Normalize(Cross(y, z));

	// return Normalize( Cross( m_strafe, m_view - m_pos ) );
    /* Recompute Y = Z cross X */
	y = Normalize(Cross(z, x));

    /* mpichler, 19950515 */
    /* cross product gives area of parallelogram, which is < 1.0 for
     * non-perpendicular unit-length vectors; so normalize x, y here
     */
#if 1
    mag = sqrtf(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
    if (mag) {
        x[0] /= mag;
        x[1] /= mag;
        x[2] /= mag;
    }

    mag = sqrtf(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
    if (mag) {
        y[0] /= mag;
        y[1] /= mag;
        y[2] /= mag;
    }
#endif

#define M(row,col)  m[col*4+row]
//#define M(row,col)  m[row*4+col]
    M(0, 0) = x.x;
    M(0, 1) = x.y;
    M(0, 2) = x.z;
    M(0, 3) = 0.0;
    //M(0, 3) = eyex;
    M(1, 0) = y.x;
    M(1, 1) = y.y;
    M(1, 2) = y.z;
    M(1, 3) = 0.0;
    //M(1, 3) = eyey;
    M(2, 0) = z.x;
    M(2, 1) = z.y;
    M(2, 2) = z.z;
    M(2, 3) = 0.0;
    M(3, 0) = 0.0;
    M(3, 1) = 0.0;
    M(3, 2) = 0.0;
    //M(3, 2) = eyez;
    M(3, 3) = 1.0;
    //M(3, 0) = -eyex;
    //M(3, 1) = -eyey;
    //M(3, 2) = -eyez;
    //M(3, 3) = 1.0;
#undef M
    //glMultMatrixf(m);
    Matrix mat;
    mat.set(m);

    /* Translate Eye to Origin */
    //glTranslatef(-eyex, -eyey, -eyez);
    Matrix mat2;
    float trans[] = {-eyex, -eyey, -eyez};
    //float trans[] = {eyex, eyey, eyez};
    mat2.setTranslation(trans);
	//mat2.inverseTranslateVect(trans);

	//Matrix mat;
    //float trans[] = {-eyex, -eyey, -eyez};
    //mat.setTranslation(trans);
	//Matrix mat2;
	//mat2.set(m);
/*
	Log("------------------------");
	Log("LookAt ");
	Log("eye="<<eyex<<","<<eyey<<","<<eyez<<" center="<<centerx<<","<<centery<<","<<centerz<<" up="<<upx<<","<<upy<<","<<upz<<std::endl;
	for(int i=0; i<16; i++)
		Log("before translation["<<i<<"] = "<<m[i]<<std::endl;
	for(int i=0; i<16; i++)
		Log("the translation["<<i<<"] = "<<mat2.m_matrix[i]<<std::endl;*/

    //mat.postmult(mat2);
    mat.postmult2(mat2);
    /*
	for(int i=0; i<16; i++)
		Log("final view matrix, after translation["<<i<<"] = "<<mat.m_matrix[i]<<std::endl;
	Log("------------------------");

	
	*/

    return mat;
}
#endif

// http://www.songho.ca/opengl/gl_projectionmatrix.html
// http://www.scratchapixel.com/lessons/3d-advanced-lessons/perspective-and-orthographic-projection-matrix/orthographic-projection/
// http://www.opengl.org/discussion_boards/showthread.php/172280-Constructing-an-orthographic-matrix-for-2D-drawing
Matrix OrthoProj(float l, float r, float t, float b, float n, float f)
{
    float m[16];

#define M(row,col)  m[col*4+row]
    M(0, 0) = 2 / (r - l);
    M(0, 1) = 0;
    M(0, 2) = 0;
    M(0, 3) = 0;

    M(1, 0) = 0;
    M(1, 1) = 2 / (t - b);
    M(1, 2) = 0;
    M(1, 3) = 0;

    M(2, 0) = 0;
    M(2, 1) = 0;
    //M(2, 2) = -1 / (f - n);
    M(2, 2) = -2 / (f - n);
    M(2, 3) = 0;

    M(3, 0) = -(r + l) / (r - l);
    M(3, 1) = -(t + b) / (t - b);
    //M(3, 2) = -n / (f - n);
    M(3, 2) = -(f + n) / (f - n);
    M(3, 3) = 1;
#undef M

    Matrix mat;
    mat.set(m);
	/*
	Log("------------------------");
	Log("set orthographic matrix ");
	Log("left="<<l<<" right="<<r<<" top="<<t<<" bottom="<<b<<" near="<<n<<" far="<<f<<std::endl;
	for(int i=0; i<16; i++)
		Log("m["<<i<<"] = "<<m[i]<<std::endl;
	Log("------------------------");
	*/

	return mat;
}

// http://www.opengl.org/discussion_boards/showthread.php/172280-Constructing-an-orthographic-matrix-for-2D-drawing
Matrix setorthographicmat2(float l, float r, float t, float b, float n, float f)
{
    float orthomatrix[16];
	/*
	        orthomatrix[0].x = 2.0/(right-left);
            orthomatrix[0].y = 0;
            orthomatrix[0].z = 0;
            orthomatrix[0].w = 0;

            orthomatrix[1].x = 0;
            orthomatrix[1].y = 2.0/(top-bottom);
            orthomatrix[1].z = 0;
            orthomatrix[1].w = 0;

            orthomatrix[2].x = 0;
            orthomatrix[2].y = 0;
            orthomatrix[2].z = 2.0/(Zfar-Znear);
            orthomatrix[2].w = 0;

            orthomatrix[3].x = -(right+left)/(right-left);
            orthomatrix[3].y = -(top+bottom)/(top-bottom);
            orthomatrix[3].z = -(Zfar+Znear)/(Zfar-Znear);
            orthomatrix[3].w = 1;
			*/
	Matrix mat;
	mat.set(orthomatrix);

	return mat;
}


/*
Matrix setperspectivepmat(float Near, float Far, float fov)
{
    GLfloat m[16];
	for(int i=0; i<16; i++)
		m[i] = 0;

    float scale = float(1) / tan(DEGTORAD(fov * 0.5));
#define M(row,col)  m[col*4+row]
    M(0, 0) = M(1, 1) = scale;
    M(2, 2) = - Far / (Far - Near);
    M(3, 2) = - Far * Near / (Far - Near);
    M(2, 3) = - 1;
    M(3, 3) = 0;
#undef M

	Matrix mat;
	mat.set(m);
}*/

#define PI_OVER_360		(M_PI/360.0f)
Matrix PerspProj(float fov, float aspect, float znear, float zfar)
{
	float m[16];

  float xymax = znear * tan(fov * PI_OVER_360);
  float ymin = -xymax;
  float xmin = -xymax;

  float width = xymax - xmin;
  float height = xymax - ymin;

  float depth = zfar - znear;
  float q = -(zfar + znear) / depth;
  float qn = -2 * (zfar * znear) / depth;

  float w = 2 * znear / width;
  w = w / aspect;
  float h = 2 * znear / height;

  m[0]  = w;
  m[1]  = 0;
  m[2]  = 0;
  m[3]  = 0;

  m[4]  = 0;
  m[5]  = h;
  m[6]  = 0;
  m[7]  = 0;

  m[8]  = 0;
  m[9]  = 0;
  m[10] = q;
  m[11] = -1;

  m[12] = 0;
  m[13] = 0;
  m[14] = qn;
  m[15] = 0;

  Matrix mat;
  mat.set(m);
  return mat;
}

Vec4f ScreenPos(Matrix* mvp, Vec3f vec, float width, float height, bool persp)
{
	Vec4f screenpos;
	screenpos.x = vec.x;
	screenpos.y = vec.y;
	screenpos.z = vec.z;
	screenpos.w = 1;

	screenpos.transform(*mvp);

	if(persp)	//.w is not always exactly 1 and doesn't give pixel-perfect tiles
	{
		//only needed for perspective projection
		screenpos = screenpos / screenpos.w;
	}

#if 0
	if(!persp)
	{
		Log(".w="<<screenpos.w<<std::endl;
	}
#endif

	screenpos.x = (screenpos.x * 0.5f + 0.5f) * width;
	screenpos.y = (-screenpos.y * 0.5f + 0.5f) * height;

	return screenpos;
}

Vec3f OnNear(int x, int y, int width, int height, Vec3f posvec, Vec3f sidevec, Vec3f upvec)
{
	/*
	float halfWidth = g_width / 2.0f;
	float halfHeight = g_height / 2.0f;

	float ratioX = (x - halfWidth) / halfWidth;

	float ratioY = -(y - halfHeight) / halfHeight;

	Vec3f direction = Normalize( g_cam.m_view - g_cam.m_pos );

	float Hnear = 2 * tan( DEGTORAD(FIELD_OF_VIEW) / 2) * MIN_DISTANCE;
	float Wnear = Hnear * g_width/g_height;

	return ( direction * MIN_DISTANCE + g_cam.m_strafe * ratioX * Wnear/2.0f + g_cam.up2() * ratioY * Hnear/2.0f );
	*/

	float halfWidth = (float)width / 2.0f;
	float halfHeight = (float)height / 2.0f;

	float ratiox = (x - halfWidth) / halfWidth;

	float ratioy = -(y - halfHeight) / halfHeight;

	//Vec3f direction = Normalize( g_cam.m_view - g_cam.m_pos );

	//-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom
	//PROJ_RIGHT*aspect/g_zoom + PROJ_RIGHT*aspect/g_zoom = 2.0f*PROJ_RIGHT*aspect/g_zoom
	//

	float aspect = fabsf((float)width / (float)height);
	float Wnear = PROJ_RIGHT * aspect / g_zoom;
	float Hnear = PROJ_RIGHT / g_zoom;

	//return ( g_cam.m_pos + g_cam.m_strafe * ratioX * Wnear + g_cam.up2() * ratioY * Hnear );
	Vec3f result = posvec + sidevec * ratiox * Wnear + upvec * ratioy * Hnear;

	//Log("--------------");
	//Log("onnear x,y="<<x<<","<<y<<" w,h="<<width<<","<<height<<" halfw,h="<<halfWidth<<","<<halfHeight<<" ratiox,y="<<ratiox<<","<<ratioy<<" aspect="<<aspect<<" W,Hnear="<<Wnear<<","<<Hnear<<std::endl;
	//Log("onnear resultx,y,z="<<result.x<<","<<result.y<<","<<result.z<<std::endl;
	//Log("--------------");

	return posvec + sidevec * ratiox * Wnear + upvec * ratioy * Hnear;
}

Vec3f OnNearPersp(int x, int y, int width, int height, Vec3f posvec, Vec3f sidevec, Vec3f upvec, Vec3f viewdir, float fov, float mind)
{
	viewdir = Normalize(viewdir);

	float halfWidth = (float)width / 2.0f;
	float halfHeight = (float)height / 2.0f;

	float ratiox = (x - halfWidth) / halfWidth;
	float ratioy = -(y - halfHeight) / halfHeight;

	float aspect = fabsf((float)width / (float)height);
	float Hnear = 2 * tan( DEGTORAD(fov) / 2) * mind;
	float Wnear = Hnear * aspect;

	//return ( g_cam.m_pos + g_cam.m_strafe * ratioX * Wnear + g_cam.up2() * ratioY * Hnear );
	Vec3f result = viewdir * mind + posvec + sidevec * ratiox * Wnear + upvec * ratioy * Hnear;

	//Log("--------------");
	//Log("onnear x,y="<<x<<","<<y<<" w,h="<<width<<","<<height<<" halfw,h="<<halfWidth<<","<<halfHeight<<" ratiox,y="<<ratiox<<","<<ratioy<<" aspect="<<aspect<<" W,Hnear="<<Wnear<<","<<Hnear<<std::endl;
	//Log("onnear resultx,y,z="<<result.x<<","<<result.y<<","<<result.z<<std::endl;
	//Log("--------------");

	return result;
}

Vec3f ScreenPerspRay(int x, int y, int width, int height, Vec3f posvec, Vec3f sidevec, Vec3f upvec, Vec3f viewdir, float fov)
{
	float halfWidth = (float)width / 2.0f;
	float halfHeight = (float)height / 2.0f;

	float ratiox = (x - halfWidth) / halfWidth;

	float ratioy = -(y - halfHeight) / halfHeight;

	float aspect = fabsf((float)width / (float)height);
	float Hnear = 2.0f * tan( DEGTORAD(fov) / 2.0f );
	float Wnear = Hnear * aspect;

	return Normalize(viewdir) + sidevec * ratiox * Wnear/2.0f + upvec * ratioy * Hnear/2.0f;

#if 0
	float halfWidth = g_width / 2.0f;
	float halfHeight = g_height / 2.0f;

	float ratioX = (x - halfWidth) / halfWidth;

	float ratioY = -(y - halfHeight) / halfHeight;

	Vec3f direction = Normalize( g_cam.m_view - g_cam.m_pos );

	float Hnear = 2 * tan( DEGTORAD(FIELD_OF_VIEW) / 2) * MIN_DISTANCE;
	float Wnear = Hnear * g_width/g_height;

	return ( direction * MIN_DISTANCE + g_cam.m_strafe * ratioX * Wnear/2.0f + g_cam.up2() * ratioY * Hnear/2.0f );
#endif
}

float Snap(float base, float value)
{
	int count = value / base;

	return count * base;
}

float SnapNearest(float base, float value)
{
	int count = (value + base/2.0f) / base;

	return count * base;
}
