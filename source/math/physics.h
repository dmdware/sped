


#define EPSILON			0.03125f //0.1f	//1.2f
#define	CLOSE_EPSILON	0.1f
#define FRICTION		1.5f
#define INVFRICTION		(1.0f/FRICTION)

class Vec3f;
class Triangle;

bool TriBoxOverlap(Vec3f vPos, Vec3f vMin, Vec3f vMax, Triangle tri);
bool TriBoxOverlap(Vec3f vCenter, Vec3f vRadius, Triangle tri);
bool TriBoxOverlap2(Vec3f vScaleDown, Vec3f vCenter, Triangle tri);