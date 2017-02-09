
#include "polygon.h"
#include "math3d.h"
#include "../tool/rendertopo.h"
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
	const double MATCH_FACTOR = 0.9999;		// Used to cover up the error in floating point
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
/*
*VVVVVVVVVVVVVVVVVVVVVVVV
*/

/*		  		
*  Triangle-Triangle Overlap Test Routines				
*  July, 2002                                                          
*  Updated December 2003                                                
*                                                                       
*  This file contains C implementation of algorithms for                
*  performing two and three-dimensional triangle-triangle intersection test 
*  The algorithms and underlying theory are described in                    
*                                                                           
* "Fast and Robust Triangle-Triangle Overlap Test 
*  Using Orientation Predicates"  P. Guigue - O. Devillers
*                                                 
*  Journal of Graphics Tools, 8(1), 2003                                    
*                                                                           
*  Several geometric predicates are defined.  Their parameters are all      
*  points.  Each point is an array of two or three real precision         
*  floating point numbers. The geometric predicates implemented in          
*  this file are:                                                            
*                                                                           
*    int tri_tri_overlap_test_3d(p1,q1,r1,p2,q2,r2)                         
*    int tri_tri_overlap_test_2d(p1,q1,r1,p2,q2,r2)                         
*                                                                           
*    int tri_tri_intersection_test_3d(p1,q1,r1,p2,q2,r2,
*                                     coplanar,source,target)               
*                                                                           
*       is a version that computes the segment of intersection when            
*       the triangles overlap (and are not coplanar)                        
*                                                                           
*    each function returns 1 if the triangles (including their              
*    boundary) intersect, otherwise 0                                       
*                                                                           
*                                                                           
*  Other information are available from the Web page                        
*  http:<i>//www.acm.org/jgt/papers/GuigueDevillers03/                         
*                                                                           
*/

#define real float

// modified by Aaron to better detect coplanarity

#define ZERO_TEST(x)  (x == 0)
//#define ZERO_TEST(x)  ((x) > -0.001 && (x) < .001)

//# include "../system/Precision.h"

/* function prototype */

int tri_tri_overlap_test_3d(real p1[3], real q1[3], real r1[3], 
							real p2[3], real q2[3], real r2[3]);


int coplanar_tri_tri3d(real  p1[3], real  q1[3], real  r1[3],
					   real  p2[3], real  q2[3], real  r2[3],
					   real  N1[3], real  N2[3]);


int tri_tri_overlap_test_2d(real p1[2], real q1[2], real r1[2], 
							real p2[2], real q2[2], real r2[2]);


int tri_tri_intersection_test_3d(real p1[3], real q1[3], real r1[3], 
								 real p2[3], real q2[3], real r2[3],
								 int * coplanar, 
								 real source[3],real target[3]);

/* coplanar returns whether the triangles are coplanar  
*  source and target are the endpoints of the segment of 
*  intersection if it exists) 
*/


/* some 3D macros */

#define CROSS(dest,v1,v2)                       \
	dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
	dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
	dest[2]=v1[0]*v2[1]-v1[1]*v2[0];

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])



#define SUB(dest,v1,v2) dest[0]=v1[0]-v2[0]; \
	dest[1]=v1[1]-v2[1]; \
	dest[2]=v1[2]-v2[2]; 


#define SCALAR(dest,alpha,v) dest[0] = alpha * v[0]; \
	dest[1] = alpha * v[1]; \
	dest[2] = alpha * v[2];



#define CHECK_MIN_MAX(p1,q1,r1,p2,q2,r2) {\
	SUB(v1,p2,q1)\
	SUB(v2,p1,q1)\
	CROSS(N1,v1,v2)\
	SUB(v1,q2,q1)\
	if (DOT(v1,N1) > 0.0f) return 0;\
	SUB(v1,p2,p1)\
	SUB(v2,r1,p1)\
	CROSS(N1,v1,v2)\
	SUB(v1,r2,p1) \
	if (DOT(v1,N1) > 0.0f) return 0;\
  else return 1; }



/* Permutation in a canonical form of T2's vertices */

#define TRI_TRI_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2) { \
	if (dp2 > 0.0f) { \
	if (dq2 > 0.0f) CHECK_MIN_MAX(p1,r1,q1,r2,p2,q2) \
	 else if (dr2 > 0.0f) CHECK_MIN_MAX(p1,r1,q1,q2,r2,p2)\
	 else CHECK_MIN_MAX(p1,q1,r1,p2,q2,r2) }\
  else if (dp2 < 0.0f) { \
  if (dq2 < 0.0f) CHECK_MIN_MAX(p1,q1,r1,r2,p2,q2)\
	else if (dr2 < 0.0f) CHECK_MIN_MAX(p1,q1,r1,q2,r2,p2)\
	else CHECK_MIN_MAX(p1,r1,q1,p2,q2,r2)\
} else { \
	if (dq2 < 0.0f) { \
	if (dr2 >= 0.0f)  CHECK_MIN_MAX(p1,r1,q1,q2,r2,p2)\
	  else CHECK_MIN_MAX(p1,q1,r1,p2,q2,r2)\
	} \
	else if (dq2 > 0.0f) { \
	if (dr2 > 0.0f) CHECK_MIN_MAX(p1,r1,q1,p2,q2,r2)\
	  else  CHECK_MIN_MAX(p1,q1,r1,q2,r2,p2)\
} \
	else  { \
	if (dr2 > 0.0f) CHECK_MIN_MAX(p1,q1,r1,r2,p2,q2)\
	  else if (dr2 < 0.0f) CHECK_MIN_MAX(p1,r1,q1,r2,p2,q2)\
	  else return coplanar_tri_tri3d(p1,q1,r1,p2,q2,r2,N1,N2);\
}}}



/*
*
*  Three-dimensional Triangle-Triangle Overlap Test
*
*/


int tri_tri_overlap_test_3d(real p1[3], real q1[3], real r1[3], 

							real p2[3], real q2[3], real r2[3])
{
	real dp1, dq1, dr1, dp2, dq2, dr2;
	real v1[3], v2[3];
	real N1[3], N2[3]; 

	/* Compute distance signs  of p1, q1 and r1 to the plane of
	triangle(p2,q2,r2) */


	SUB(v1,p2,r2)
		SUB(v2,q2,r2)
		CROSS(N2,v1,v2)

		SUB(v1,p1,r2)
		dp1 = DOT(v1,N2);
	SUB(v1,q1,r2)
		dq1 = DOT(v1,N2);
	SUB(v1,r1,r2)
		dr1 = DOT(v1,N2);

	if (((dp1 * dq1) > 0.0f) && ((dp1 * dr1) > 0.0f))  return 0; 

	/* Compute distance signs  of p2, q2 and r2 to the plane of
	triangle(p1,q1,r1) */


	SUB(v1,q1,p1)
		SUB(v2,r1,p1)
		CROSS(N1,v1,v2)

		SUB(v1,p2,r1)
		dp2 = DOT(v1,N1);
	SUB(v1,q2,r1)
		dq2 = DOT(v1,N1);
	SUB(v1,r2,r1)
		dr2 = DOT(v1,N1);

	if (((dp2 * dq2) > 0.0f) && ((dp2 * dr2) > 0.0f)) return 0;

	/* Permutation in a canonical form of T1's vertices */




	if (dp1 > 0.0f) {
		if (dq1 > 0.0f) TRI_TRI_3D(r1,p1,q1,p2,r2,q2,dp2,dr2,dq2)
		else if (dr1 > 0.0f) TRI_TRI_3D(q1,r1,p1,p2,r2,q2,dp2,dr2,dq2)	
		else TRI_TRI_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2)
	} else if (dp1 < 0.0f) {
		if (dq1 < 0.0f) TRI_TRI_3D(r1,p1,q1,p2,q2,r2,dp2,dq2,dr2)
		else if (dr1 < 0.0f) TRI_TRI_3D(q1,r1,p1,p2,q2,r2,dp2,dq2,dr2)
		else TRI_TRI_3D(p1,q1,r1,p2,r2,q2,dp2,dr2,dq2)
	} else {
		if (dq1 < 0.0f) {
			if (dr1 >= 0.0f) TRI_TRI_3D(q1,r1,p1,p2,r2,q2,dp2,dr2,dq2)
			else TRI_TRI_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2)
		}
		else if (dq1 > 0.0f) {
			if (dr1 > 0.0f) TRI_TRI_3D(p1,q1,r1,p2,r2,q2,dp2,dr2,dq2)
			else TRI_TRI_3D(q1,r1,p1,p2,q2,r2,dp2,dq2,dr2)
		}
		else  {
			if (dr1 > 0.0f) TRI_TRI_3D(r1,p1,q1,p2,q2,r2,dp2,dq2,dr2)
			else if (dr1 < 0.0f) TRI_TRI_3D(r1,p1,q1,p2,r2,q2,dp2,dr2,dq2)
			else return coplanar_tri_tri3d(p1,q1,r1,p2,q2,r2,N1,N2);
		}
		}
};



int coplanar_tri_tri3d(real p1[3], real q1[3], real r1[3],
					   real p2[3], real q2[3], real r2[3],
					   real normal_1[3], real normal_2[3]){

						   real P1[2],Q1[2],R1[2];
						   real P2[2],Q2[2],R2[2];

						   real n_x, n_y, n_z;

						   n_x = ((normal_1[0]<0)?-normal_1[0]:normal_1[0]);
						   n_y = ((normal_1[1]<0)?-normal_1[1]:normal_1[1]);
						   n_z = ((normal_1[2]<0)?-normal_1[2]:normal_1[2]);


						   /* Projection of the triangles in 3D onto 2D such that the area of
						   the projection is maximized. */


						   if (( n_x > n_z ) && ( n_x >= n_y )) {
							   // Project onto plane YZ

							   P1[0] = q1[2]; P1[1] = q1[1];
							   Q1[0] = p1[2]; Q1[1] = p1[1];
							   R1[0] = r1[2]; R1[1] = r1[1]; 

							   P2[0] = q2[2]; P2[1] = q2[1];
							   Q2[0] = p2[2]; Q2[1] = p2[1];
							   R2[0] = r2[2]; R2[1] = r2[1]; 

						   } else if (( n_y > n_z ) && ( n_y >= n_x )) {
							   // Project onto plane XZ

							   P1[0] = q1[0]; P1[1] = q1[2];
							   Q1[0] = p1[0]; Q1[1] = p1[2];
							   R1[0] = r1[0]; R1[1] = r1[2]; 

							   P2[0] = q2[0]; P2[1] = q2[2];
							   Q2[0] = p2[0]; Q2[1] = p2[2];
							   R2[0] = r2[0]; R2[1] = r2[2]; 

						   } else {
							   // Project onto plane XY

							   P1[0] = p1[0]; P1[1] = p1[1]; 
							   Q1[0] = q1[0]; Q1[1] = q1[1]; 
							   R1[0] = r1[0]; R1[1] = r1[1]; 

							   P2[0] = p2[0]; P2[1] = p2[1]; 
							   Q2[0] = q2[0]; Q2[1] = q2[1]; 
							   R2[0] = r2[0]; R2[1] = r2[1]; 
						   }

						   return tri_tri_overlap_test_2d(P1,Q1,R1,P2,Q2,R2);

};



/*
*                                                                
*  Three-dimensional Triangle-Triangle Intersection              
*
*/

/*
This macro is called when the triangles surely intersect
It constructs the segment of intersection of the two triangles
if they are not coplanar.
*/

#define CONSTRUCT_INTERSECTION(p1,q1,r1,p2,q2,r2) { \
	SUB(v1,q1,p1) \
	SUB(v2,r2,p1) \
	CROSS(N,v1,v2) \
	SUB(v,p2,p1) \
	if (DOT(v,N) > 0.0f) {\
	SUB(v1,r1,p1) \
	CROSS(N,v1,v2) \
	if (DOT(v,N) <= 0.0f) { \
	SUB(v2,q2,p1) \
	CROSS(N,v1,v2) \
	if (DOT(v,N) > 0.0f) { \
	SUB(v1,p1,p2) \
	SUB(v2,p1,r1) \
	alpha = DOT(v1,N2) / DOT(v2,N2); \
	SCALAR(v1,alpha,v2) \
	SUB(source,p1,v1) \
	SUB(v1,p2,p1) \
	SUB(v2,p2,r2) \
	alpha = DOT(v1,N1) / DOT(v2,N1); \
	SCALAR(v1,alpha,v2) \
	SUB(target,p2,v1) \
	return 1; \
	} else { \
	SUB(v1,p2,p1) \
	SUB(v2,p2,q2) \
	alpha = DOT(v1,N1) / DOT(v2,N1); \
	SCALAR(v1,alpha,v2) \
	SUB(source,p2,v1) \
	SUB(v1,p2,p1) \
	SUB(v2,p2,r2) \
	alpha = DOT(v1,N1) / DOT(v2,N1); \
	SCALAR(v1,alpha,v2) \
	SUB(target,p2,v1) \
	return 1; \
	} \
	} else { \
	return 0; \
	} \
	} else { \
	SUB(v2,q2,p1) \
	CROSS(N,v1,v2) \
	if (DOT(v,N) < 0.0f) { \
	return 0; \
	} else { \
	SUB(v1,r1,p1) \
	CROSS(N,v1,v2) \
	if (DOT(v,N) >= 0.0f) { \
	SUB(v1,p1,p2) \
	SUB(v2,p1,r1) \
	alpha = DOT(v1,N2) / DOT(v2,N2); \
	SCALAR(v1,alpha,v2) \
	SUB(source,p1,v1) \
	SUB(v1,p1,p2) \
	SUB(v2,p1,q1) \
	alpha = DOT(v1,N2) / DOT(v2,N2); \
	SCALAR(v1,alpha,v2) \
	SUB(target,p1,v1) \
	return 1; \
	} else { \
	SUB(v1,p2,p1) \
	SUB(v2,p2,q2) \
	alpha = DOT(v1,N1) / DOT(v2,N1); \
	SCALAR(v1,alpha,v2) \
	SUB(source,p2,v1) \
	SUB(v1,p1,p2) \
	SUB(v2,p1,q1) \
	alpha = DOT(v1,N2) / DOT(v2,N2); \
	SCALAR(v1,alpha,v2) \
	SUB(target,p1,v1) \
	return 1; \
	}}}} 



#define TRI_TRI_INTER_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2) { \
	if (dp2 > 0.0f) { \
	if (dq2 > 0.0f) CONSTRUCT_INTERSECTION(p1,r1,q1,r2,p2,q2) \
	 else if (dr2 > 0.0f) CONSTRUCT_INTERSECTION(p1,r1,q1,q2,r2,p2)\
	 else CONSTRUCT_INTERSECTION(p1,q1,r1,p2,q2,r2) }\
  else if (dp2 < 0.0f) { \
  if (dq2 < 0.0f) CONSTRUCT_INTERSECTION(p1,q1,r1,r2,p2,q2)\
	else if (dr2 < 0.0f) CONSTRUCT_INTERSECTION(p1,q1,r1,q2,r2,p2)\
	else CONSTRUCT_INTERSECTION(p1,r1,q1,p2,q2,r2)\
} else { \
	if (dq2 < 0.0f) { \
	if (dr2 >= 0.0f)  CONSTRUCT_INTERSECTION(p1,r1,q1,q2,r2,p2)\
	  else CONSTRUCT_INTERSECTION(p1,q1,r1,p2,q2,r2)\
	} \
	else if (dq2 > 0.0f) { \
	if (dr2 > 0.0f) CONSTRUCT_INTERSECTION(p1,r1,q1,p2,q2,r2)\
	  else  CONSTRUCT_INTERSECTION(p1,q1,r1,q2,r2,p2)\
} \
	else  { \
	if (dr2 > 0.0f) CONSTRUCT_INTERSECTION(p1,q1,r1,r2,p2,q2)\
	  else if (dr2 < 0.0f) CONSTRUCT_INTERSECTION(p1,r1,q1,r2,p2,q2)\
	  else { \
	  *coplanar = 1; \
	  return coplanar_tri_tri3d(p1,q1,r1,p2,q2,r2,N1,N2);\
} \
}} }


/*
The following version computes the segment of intersection of the
two triangles if it exists. 
coplanar returns whether the triangles are coplanar
source and target are the endpoints of the line segment of intersection 
*/

int tri_tri_intersection_test_3d(real p1[3], real q1[3], real r1[3], 
								 real p2[3], real q2[3], real r2[3],
								 int * coplanar, 
								 real source[3], real target[3] )

{
	real dp1, dq1, dr1, dp2, dq2, dr2;
	real v1[3], v2[3], v[3];
	real N1[3], N2[3], N[3];
	real alpha;

	// Compute distance signs  of p1, q1 and r1 
	// to the plane of triangle(p2,q2,r2)


	SUB(v1,p2,r2)
		SUB(v2,q2,r2)
		CROSS(N2,v1,v2)

		SUB(v1,p1,r2)
		dp1 = DOT(v1,N2);
	SUB(v1,q1,r2)
		dq1 = DOT(v1,N2);
	SUB(v1,r1,r2)
		dr1 = DOT(v1,N2);

	if (((dp1 * dq1) > 0.0f) && ((dp1 * dr1) > 0.0f))  return 0; 
	//if (((dp1 * dq1) >= -0.000f) && ((dp1 * dr1) >= -0.000f))  return 0; //ed

	// Compute distance signs  of p2, q2 and r2 
	// to the plane of triangle(p1,q1,r1)


	SUB(v1,q1,p1)
		SUB(v2,r1,p1)
		CROSS(N1,v1,v2)

		SUB(v1,p2,r1)
		dp2 = DOT(v1,N1);
	SUB(v1,q2,r1)
		dq2 = DOT(v1,N1);
	SUB(v1,r2,r1)
		dr2 = DOT(v1,N1);

	if (((dp2 * dq2) > 0.0f) && ((dp2 * dr2) > 0.0f)) return 0;
	//if (((dp2 * dq2) >= -0.000f) && ((dp2 * dr2) >= -0.000f)) return 0;	//ed

	// Permutation in a canonical form of T1's vertices


	//  printf("d1 = [%f %f %f], d2 = [%f %f %f]\n", dp1, dq1, dr1, dp2, dq2, dr2);
	/*
	// added by Aaron
	if (ZERO_TEST(dp1) || ZERO_TEST(dq1) ||ZERO_TEST(dr1) ||ZERO_TEST(dp2) ||ZERO_TEST(dq2) ||ZERO_TEST(dr2))
	{
	coplanar = 1;
	return 0;
	}
	*/


	if (dp1 > 0.0f) {
		if (dq1 > 0.0f) TRI_TRI_INTER_3D(r1,p1,q1,p2,r2,q2,dp2,dr2,dq2)
		else if (dr1 > 0.0f) TRI_TRI_INTER_3D(q1,r1,p1,p2,r2,q2,dp2,dr2,dq2)

		else TRI_TRI_INTER_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2)
	} else if (dp1 < 0.0f) {
		if (dq1 < 0.0f) TRI_TRI_INTER_3D(r1,p1,q1,p2,q2,r2,dp2,dq2,dr2)
		else if (dr1 < 0.0f) TRI_TRI_INTER_3D(q1,r1,p1,p2,q2,r2,dp2,dq2,dr2)
		else TRI_TRI_INTER_3D(p1,q1,r1,p2,r2,q2,dp2,dr2,dq2)
	} else {
		if (dq1 < 0.0f) {
			if (dr1 >= 0.0f) TRI_TRI_INTER_3D(q1,r1,p1,p2,r2,q2,dp2,dr2,dq2)
			else TRI_TRI_INTER_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2)
		}
		else if (dq1 > 0.0f) {
			if (dr1 > 0.0f) TRI_TRI_INTER_3D(p1,q1,r1,p2,r2,q2,dp2,dr2,dq2)
			else TRI_TRI_INTER_3D(q1,r1,p1,p2,q2,r2,dp2,dq2,dr2)
		}
		else  {
			if (dr1 > 0.0f) TRI_TRI_INTER_3D(r1,p1,q1,p2,q2,r2,dp2,dq2,dr2)
			else if (dr1 < 0.0f) TRI_TRI_INTER_3D(r1,p1,q1,p2,r2,q2,dp2,dr2,dq2)
			else {
				// triangles are co-planar

				*coplanar = 1;
				return coplanar_tri_tri3d(p1,q1,r1,p2,q2,r2,N1,N2);
			}
		}
		}
};





/*
*
*  Two dimensional Triangle-Triangle Overlap Test    
*
*/


/* some 2D macros */

#define ORIENT_2D(a, b, c)  ((a[0]-c[0])*(b[1]-c[1])-(a[1]-c[1])*(b[0]-c[0]))


#define INTERSECTION_TEST_VERTEX(P1, Q1, R1, P2, Q2, R2) {\
	if (ORIENT_2D(R2,P2,Q1) >= 0.0f)\
	if (ORIENT_2D(R2,Q2,Q1) <= 0.0f)\
	if (ORIENT_2D(P1,P2,Q1) > 0.0f) {\
	if (ORIENT_2D(P1,Q2,Q1) <= 0.0f) return 1; \
	else return 0;} else {\
	if (ORIENT_2D(P1,P2,R1) >= 0.0f)\
	if (ORIENT_2D(Q1,R1,P2) >= 0.0f) return 1; \
	  else return 0;\
	else return 0;}\
	else \
	if (ORIENT_2D(P1,Q2,Q1) <= 0.0f)\
	if (ORIENT_2D(R2,Q2,R1) <= 0.0f)\
	if (ORIENT_2D(Q1,R1,Q2) >= 0.0f) return 1; \
	  else return 0;\
	else return 0;\
	  else return 0;\
  else\
  if (ORIENT_2D(R2,P2,R1) >= 0.0f) \
  if (ORIENT_2D(Q1,R1,R2) >= 0.0f)\
  if (ORIENT_2D(P1,P2,R1) >= 0.0f) return 1;\
	else return 0;\
	  else \
	  if (ORIENT_2D(Q1,R1,Q2) >= 0.0f) {\
	  if (ORIENT_2D(R2,R1,Q2) >= 0.0f) return 1; \
	  else return 0; }\
	else return 0; \
	else  return 0; \
};



#define INTERSECTION_TEST_EDGE(P1, Q1, R1, P2, Q2, R2) { \
	if (ORIENT_2D(R2,P2,Q1) >= 0.0f) {\
	if (ORIENT_2D(P1,P2,Q1) >= 0.0f) { \
	if (ORIENT_2D(P1,Q1,R2) >= 0.0f) return 1; \
		else return 0;} else { \
		if (ORIENT_2D(Q1,R1,P2) >= 0.0f){ \
		if (ORIENT_2D(R1,P1,P2) >= 0.0f) return 1; else return 0;} \
	  else return 0; } \
	} else {\
	if (ORIENT_2D(R2,P2,R1) >= 0.0f) {\
	if (ORIENT_2D(P1,P2,R1) >= 0.0f) {\
	if (ORIENT_2D(P1,R1,R2) >= 0.0f) return 1;  \
	else {\
	if (ORIENT_2D(Q1,R1,R2) >= 0.0f) return 1; else return 0;}}\
	  else  return 0; }\
	else return 0; }}



int ccw_tri_tri_intersection_2d(real p1[2], real q1[2], real r1[2], 
								real p2[2], real q2[2], real r2[2]) {
									if ( ORIENT_2D(p2,q2,p1) >= 0.0f ) {
										if ( ORIENT_2D(q2,r2,p1) >= 0.0f ) {
											if ( ORIENT_2D(r2,p2,p1) >= 0.0f ) return 1;
											else INTERSECTION_TEST_EDGE(p1,q1,r1,p2,q2,r2)
										} else {  
											if ( ORIENT_2D(r2,p2,p1) >= 0.0f ) 
												INTERSECTION_TEST_EDGE(p1,q1,r1,r2,p2,q2)
											else INTERSECTION_TEST_VERTEX(p1,q1,r1,p2,q2,r2)}}
									else {
										if ( ORIENT_2D(q2,r2,p1) >= 0.0f ) {
											if ( ORIENT_2D(r2,p2,p1) >= 0.0f ) 
												INTERSECTION_TEST_EDGE(p1,q1,r1,q2,r2,p2)
											else  INTERSECTION_TEST_VERTEX(p1,q1,r1,q2,r2,p2)}
										else INTERSECTION_TEST_VERTEX(p1,q1,r1,r2,p2,q2)}
};


int tri_tri_overlap_test_2d(real p1[2], real q1[2], real r1[2], 
							real p2[2], real q2[2], real r2[2]) {
								if ( ORIENT_2D(p1,q1,r1) < 0.0f )
									if ( ORIENT_2D(p2,q2,r2) < 0.0f )
										return ccw_tri_tri_intersection_2d(p1,r1,q1,p2,r2,q2);
									else
										return ccw_tri_tri_intersection_2d(p1,r1,q1,p2,q2,r2);
								else
									if ( ORIENT_2D(p2,q2,r2) < 0.0f )
										return ccw_tri_tri_intersection_2d(p1,q1,r1,p2,r2,q2);
									else
										return ccw_tri_tri_intersection_2d(p1,q1,r1,p2,q2,r2);

};


/*
* ^^^^^^^^^^^^^^^^^^^^
*/
bool TriTri(Vec3f *t1, Vec3f *t2)
{
	int coplanar = 0;
	Vec3f interline[2];

	if(tri_tri_intersection_test_3d((float*)&t1[0], (float*)&t1[1], (float*)&t1[2], 
		(float*)&t2[0], (float*)&t2[1], (float*)&t2[2],
		&coplanar, 
		(float*)&interline[0], (float*)&interline[1] ))
	{

		int samec=0;

		for(int vi=0; vi<3; ++vi)
		{
			if(Magnitude(t1[vi] - interline[0]) <= CLOSEPOSF ||
				Magnitude(t1[vi] - interline[1]) <= CLOSEPOSF)
				samec++;
		}

		if(samec>=2)
			return false;

		samec=0;

		for(int vi=0; vi<3; ++vi)
		{
			if(Magnitude(t2[vi] - interline[0]) <= CLOSEPOSF ||
				Magnitude(t2[vi] - interline[1]) <= CLOSEPOSF)
				samec++;
		}

		if(samec>=2)
			return false;

		return true;
	}
	if(coplanar)
		return false;

	return false;
#if 0
	Vec3f v0 = tr[1] - tr[0],
		v1 = tr[2] - tr[0],
		v2 = ir - tr[0];

	// do bounds test for each position
	double f00 = Dot( v0, v0 );
	double f01 = Dot( v0, v1 );
	double f11 = Dot( v1, v1 );

	double f02 = Dot( v0, v2 );
	double f12 = Dot( v1, v2 );

	// Compute barycentric coordinates
	double invDenom = 1 / ( f00 * f11 - f01 * f01 );
	if(ISNAN(invDenom))
		invDenom = 1;
	double fU = ( f11 * f02 - f01 * f12 ) * invDenom;
	double fV = ( f00 * f12 - f01 * f02 ) * invDenom;

	// Check if point is in triangle
	//if( ( fU >= 0.0 ) && ( fV >= 0.0 ) && ( fU + fV <= 1.0 ) )
	//	goto dotex;
	//continue;

	//dotex:

	Vec2f txcf = txc[0] * (1 - fU - fV) + 
		txc[1] * (fU) + 
		txc[2] * (fV);
#endif

	return false;
}