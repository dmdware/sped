
#include "polygon.h"
#include "math3d.h"
#include "../utils.h"

Polyg::Polyg()
{
	m_drawoutva = NULL;
}

Polyg::~Polyg()
{
	freeva();
}

Polyg::Polyg(const Polyg& original)
{
	*this = original;
}

Polyg& Polyg::operator=(const Polyg& original)
{
	m_drawoutva = NULL;
	m_edv = original.m_edv;
	makeva();

	return *this;
}

void Polyg::makeva()
{
	freeva();
	m_drawoutva = new Vec3f[m_edv.size()];
	int i=0;
	for(std::list<Vec3f>::iterator j=m_edv.begin(); j!=m_edv.end(); i++, j++)
		m_drawoutva[i] = *j;
}

void Polyg::freeva()
{
	if(m_drawoutva)
	{
		delete [] m_drawoutva;
		m_drawoutva = NULL;
	}
}

bool InsidePolygon(Vec3f vIntersection, Vec3f Poly[], long verticeCount)
{
	const double MATCH_FACTOR = 0.999;		// Used to cover up the error in floating point
	double Angle = 0.0;						// Initialize the angle
	Vec3f vA, vB;						// Create temp vectors
	
	// Just because we intersected the plane, doesn't mean we were anywhere near the polygon.
	// This functions checks our intersection point to make sure it is inside of the polygon.
	// This is another tough function to grasp at first, but let me try and explain.
	// It's a brilliant method really, what it does is create triangles within the polygon
	// from the intersection point.  It then adds up the inner angle of each of those triangles.
	// If the angles together add up to 360 degrees (or 2 * PI in radians) then we are inside!
	// If the angle is under that value, we must be outside of polygon.  To further
	// understand why this works, take a pencil and draw a perfect triangle.  Draw a dot in
	// the middle of the triangle.  Now, from that dot, draw a line to each of the vertices.
	// Now, we have 3 triangles within that triangle right?  Now, we know that if we add up
	// all of the angles in a triangle we get 180° right?  Well, that is kinda what we are doing,
	// but the inverse of that.  Say your triangle is an equilateral triangle, so add up the angles
	// and you will get 180° degree angles.  60 + 60 + 60 is 360°.

	for (int i = 0; i < verticeCount; i++)		// Go in a circle to each vertex and get the angle between
	{	
		vA = Vector(Poly[i], vIntersection);	// Subtract the intersection point from the current vertex
												// Subtract the point from the next vertex
		vB = Vector(Poly[(i + 1) % verticeCount], vIntersection);
												
		Angle += AngleBetweenVectors(vA, vB);	// Find the angle between the 2 vectors and add them all up as we go along
	}

	// Now that we have the total angles added up, we need to check if they add up to 360 degrees.
	// Since we are using the dot product, we are working in radians, so we check if the angles
	// equals 2*PI.  We defined PI in math3d.h.  You will notice that we use a MATCH_FACTOR
	// in conjunction with our desired degree.  This is because of the inaccuracy when working
	// with floating point numbers.  It usually won't always be perfectly 2 * PI, so we need
	// to use a little twiddling.  I use .9999, but you can change this to fit your own desired accuracy.

	if(Angle >= (MATCH_FACTOR * (2.0 * M_PI)) )	// If the angle is greater than 2 PI, (360 degrees)
		return true;							// The point is inside of the polygon

	//fprintf(g_applog, "MATCH ANgle%f >= %f",
	//	(float)Angle,
	//	(float)(MATCH_FACTOR * (2.0 * M_PI)));

	return false;								// If you get here, it obviously wasn't inside the polygon, so Return FALSE
}

/*! @param PIP Point-in-Plane3f */
bool testRayThruTriangle( Vec3f tri[3], Vec3f line[2], Vec3f *PIP)
{  
	Vec3f n, IntersectPos;

	Vec3f P1 = tri[0];
	Vec3f P2 = tri[1];
	Vec3f P3 = tri[2];

//Vec3f PIP = tri[0];

	Vec3f R1 = line[0];
	Vec3f R2 = line[1];

	// Find Triangle Normal
	n = Cross( P2 - P1, P3 - P1 );
	n = Normalize(n); // not really needed?  Vec3f does this with cross.
	//n = Normalize( Normal(tri) );

	// Find distance from LP1 and LP2 to the plane defined by the triangle
	float Dist1 = Dot(R1-P1, n );
	float Dist2 = Dot(R2-P1, n );

	if ( (Dist1 * Dist2) >= 0.0f) { 
		//SFLog(@"no cross"); 
		return false; 
	} // line doesn't cross the triangle.

	///if ( Dist1 == Dist2) { 
		//SFLog(@"parallel"); 
	//	return false; 
	//} // line and plane are parallel

	// Find point on the line that intersects with the plane
	IntersectPos = R1 + (R2-R1) * ( -Dist1/(Dist2-Dist1) );

	// Find if the interesection point lies inside the triangle by testing it against all edges
	Vec3f vTest;

	vTest = Cross( n, P2-P1 );
	if ( Dot( vTest, IntersectPos-P1) < 0.01f ) { 
		//SFLog(@"no intersect P2-P1"); 
		return false; 
	}

	vTest = Cross( n, P3-P2 );
	if ( Dot( vTest, IntersectPos-P2) < 0.01f ) { 
		//SFLog(@"no intersect P3-P2"); 
		return false; 
	}

	vTest = Cross( n, P1-P3 );
	if ( Dot( vTest, IntersectPos-P1) < 0.01f ) { 
		//SFLog(@"no intersect P1-P3"); 
		return false; 
	}

	//NSLog(@"Intersects at ( %f, %f )", IntersectPos.x(), IntersectPos.y());

	if(PIP)
		*PIP = IntersectPos;

	return true;
}

bool IntersectedPolygon(Vec3f vPoly[], Vec3f vLine[], int verticeCount, Vec3f* vIntersection)
{
	//return testRayThruTriangle(vPoly, vLine, vIntersection);

	Vec3f vNormal;// = {0};
	float originDistance = 0;

	Vec3f vPoly2[3];
	vPoly2[0] = vPoly[1];
	vPoly2[1] = vPoly[0];
	vPoly2[2] = vPoly[2];

	// First we check to see if our line intersected the plane.  If this isn't true
	// there is no need to go on, so return false immediately.
	// We pass in address of vNormal and originDistance so we only calculate it once

									 // Reference   // Reference
	if(IntersectedPlane(vPoly, vLine,   vNormal,   originDistance) //||
		//IntersectedPlane(vPoly2, vLine,   vNormal,   originDistance)
		)
		goto next;

	return false;

next:
	// Now that we have our normal and distance passed back from IntersectedPlane(), 
	// we can use it to calculate the intersection point.  The intersection point
	// is the point that actually is ON the plane.  It is between the line.  We need
	// this point test next, if we are inside the polygon.  To get the I-Point, we
	// give our function the normal of the plan, the points of the line, and the originDistance.

	Vec3f vTemp = IntersectionPoint(vNormal, vLine, originDistance);

	// Now that we have the intersection point, we need to test if it's inside the polygon.
	// To do this, we pass in :
	// (our intersection point, the polygon, and the number of vertices our polygon has)

	if(InsidePolygon(vTemp, vPoly, verticeCount) //||
		//InsidePolygon(vTemp, vPoly2, 3)
		)
	{
		if(vIntersection != NULL)
			(*vIntersection) = vTemp;

		return true;
	}

	return false;
}