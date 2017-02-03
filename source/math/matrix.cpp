

#include "matrix.h"
#include "../platform.h"
#include "quaternion.h"
#include "vec4f.h"
#include "vec3f.h"
#include "../utils.h"

#define M(row,col)  m_matrix[col*4+row]
//#define M(row,col)  m_matrix[row*4+col]

void LogMat(const float* m)
{
//	Log("["<<m[0]<<","<<m[1]<<","<<m[2]<<","<<m[3]<<"]");
//	Log("["<<m[4]<<","<<m[5]<<","<<m[6]<<","<<m[7]<<"]");
//	Log("["<<m[8]<<","<<m[9]<<","<<m[10]<<","<<m[11]<<"]");
//	Log("["<<m[12]<<","<<m[13]<<","<<m[14]<<","<<m[15]<<"]");
//	Log(std::endl<<std::endl;
}

Matrix::Matrix()
{
	reset();
}

Matrix::Matrix(Vec4f a, Vec4f b, Vec4f c, Vec4f d)
{
//#define M(row,col)  m_matrix[col*4+row]

//#undef M
}

Matrix::~Matrix()
{
}

void Matrix::set( const float *matrix )
{
	memcpy( m_matrix, matrix, sizeof( float )*16 );
}

void Matrix::reset()
{
	memset( m_matrix, 0, sizeof( float )*16 );
	m_matrix[0] = m_matrix[5] = m_matrix[10] = m_matrix[15] = 1;
}

void Matrix::inverseRotateVect( float *pVect )
{
	float vec[3];

	vec[0] = pVect[0]*m_matrix[0]+pVect[1]*m_matrix[1]+pVect[2]*m_matrix[2];
	vec[1] = pVect[0]*m_matrix[4]+pVect[1]*m_matrix[5]+pVect[2]*m_matrix[6];
	vec[2] = pVect[0]*m_matrix[8]+pVect[1]*m_matrix[9]+pVect[2]*m_matrix[10];

	memcpy( pVect, vec, sizeof( float )*3 );
}

void Matrix::inverseTranslateVect( float *pVect )
{
	pVect[0] = pVect[0]-m_matrix[12];
	pVect[1] = pVect[1]-m_matrix[13];
	pVect[2] = pVect[2]-m_matrix[14];
}

void Matrix::postmult( const Matrix& matrix )	//no longer used for light matrix concatenation (and skeletal animation?)
{
	float newMatrix[16];

#if 0

	const float *m1 = m_matrix, *m2 = matrix.m_matrix;

	newMatrix[0] = m1[0]*m2[0] + m1[4]*m2[1] + m1[8]*m2[2];
	newMatrix[1] = m1[1]*m2[0] + m1[5]*m2[1] + m1[9]*m2[2];
	newMatrix[2] = m1[2]*m2[0] + m1[6]*m2[1] + m1[10]*m2[2];
	newMatrix[3] = 0;

	newMatrix[4] = m1[0]*m2[4] + m1[4]*m2[5] + m1[8]*m2[6];
	newMatrix[5] = m1[1]*m2[4] + m1[5]*m2[5] + m1[9]*m2[6];
	newMatrix[6] = m1[2]*m2[4] + m1[6]*m2[5] + m1[10]*m2[6];
	newMatrix[7] = 0;

	newMatrix[8] = m1[0]*m2[8] + m1[4]*m2[9] + m1[8]*m2[10];
	newMatrix[9] = m1[1]*m2[8] + m1[5]*m2[9] + m1[9]*m2[10];
	newMatrix[10] = m1[2]*m2[8] + m1[6]*m2[9] + m1[10]*m2[10];
	newMatrix[11] = 0;

	newMatrix[12] = m1[0]*m2[12] + m1[4]*m2[13] + m1[8]*m2[14] + m1[12];
	newMatrix[13] = m1[1]*m2[12] + m1[5]*m2[13] + m1[9]*m2[14] + m1[13];
	newMatrix[14] = m1[2]*m2[12] + m1[6]*m2[13] + m1[10]*m2[14] + m1[14];
	newMatrix[15] = 1;

#else

	const float *a = m_matrix, *b = matrix.m_matrix;

	newMatrix[0]  = a[0] * b[0]  + a[4] * b[1]  + a[8] * b[2]   + a[12] * b[3];
	newMatrix[1]  = a[1] * b[0]  + a[5] * b[1]  + a[9] * b[2]   + a[13] * b[3];
	newMatrix[2]  = a[2] * b[0]  + a[6] * b[1]  + a[10] * b[2]  + a[14] * b[3];
	newMatrix[3]  = a[3] * b[0]  + a[7] * b[1]  + a[11] * b[2]  + a[15] * b[3];

	newMatrix[4]  = a[0] * b[4]  + a[4] * b[5]  + a[8] * b[6]   + a[12] * b[7];
	newMatrix[5]  = a[1] * b[4]  + a[5] * b[5]  + a[9] * b[6]   + a[13] * b[7];
	newMatrix[6]  = a[2] * b[4]  + a[6] * b[5]  + a[10] * b[6]  + a[14] * b[7];
	newMatrix[7]  = a[3] * b[4]  + a[7] * b[5]  + a[11] * b[6]  + a[15] * b[7];

	newMatrix[8]  = a[0] * b[8]  + a[4] * b[9]  + a[8] * b[10]  + a[12] * b[11];
	newMatrix[9]  = a[1] * b[8]  + a[5] * b[9]  + a[9] * b[10]  + a[13] * b[11];
	newMatrix[10] = a[2] * b[8]  + a[6] * b[9]  + a[10] * b[10] + a[14] * b[11];
	newMatrix[11] = a[3] * b[8]  + a[7] * b[9]  + a[11] * b[10] + a[15] * b[11];

	newMatrix[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14]  + a[12] * b[15];
	newMatrix[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14]  + a[13] * b[15];
	newMatrix[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15];
	newMatrix[15] = a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15];

#endif

	set( newMatrix );
}

void Matrix::postmult2( const Matrix& matrix )	//used only for light matrix (and skeletal animation?), doesn't work with perspective projection because that is not an affine transformation
{
	float newMatrix[16];

#if 1

	const float *m1 = m_matrix, *m2 = matrix.m_matrix;

	newMatrix[0] = m1[0]*m2[0] + m1[4]*m2[1] + m1[8]*m2[2];
	newMatrix[1] = m1[1]*m2[0] + m1[5]*m2[1] + m1[9]*m2[2];
	newMatrix[2] = m1[2]*m2[0] + m1[6]*m2[1] + m1[10]*m2[2];
	newMatrix[3] = 0;

	newMatrix[4] = m1[0]*m2[4] + m1[4]*m2[5] + m1[8]*m2[6];
	newMatrix[5] = m1[1]*m2[4] + m1[5]*m2[5] + m1[9]*m2[6];
	newMatrix[6] = m1[2]*m2[4] + m1[6]*m2[5] + m1[10]*m2[6];
	newMatrix[7] = 0;

	newMatrix[8] = m1[0]*m2[8] + m1[4]*m2[9] + m1[8]*m2[10];
	newMatrix[9] = m1[1]*m2[8] + m1[5]*m2[9] + m1[9]*m2[10];
	newMatrix[10] = m1[2]*m2[8] + m1[6]*m2[9] + m1[10]*m2[10];
	newMatrix[11] = 0;

	newMatrix[12] = m1[0]*m2[12] + m1[4]*m2[13] + m1[8]*m2[14] + m1[12];
	newMatrix[13] = m1[1]*m2[12] + m1[5]*m2[13] + m1[9]*m2[14] + m1[13];
	newMatrix[14] = m1[2]*m2[12] + m1[6]*m2[13] + m1[10]*m2[14] + m1[14];
	newMatrix[15] = 1;

#else

	const float *a = m_matrix, *b = matrix.m_matrix;

	newMatrix[0]  = a[0] * b[0]  + a[4] * b[1]  + a[8] * b[2]   + a[12] * b[3];
	newMatrix[1]  = a[1] * b[0]  + a[5] * b[1]  + a[9] * b[2]   + a[13] * b[3];
	newMatrix[2]  = a[2] * b[0]  + a[6] * b[1]  + a[10] * b[2]  + a[14] * b[3];
	newMatrix[3]  = a[3] * b[0]  + a[7] * b[1]  + a[11] * b[2]  + a[15] * b[3];

	newMatrix[4]  = a[0] * b[4]  + a[4] * b[5]  + a[8] * b[6]   + a[12] * b[7];
	newMatrix[5]  = a[1] * b[4]  + a[5] * b[5]  + a[9] * b[6]   + a[13] * b[7];
	newMatrix[6]  = a[2] * b[4]  + a[6] * b[5]  + a[10] * b[6]  + a[14] * b[7];
	newMatrix[7]  = a[3] * b[4]  + a[7] * b[5]  + a[11] * b[6]  + a[15] * b[7];

	newMatrix[8]  = a[0] * b[8]  + a[4] * b[9]  + a[8] * b[10]  + a[12] * b[11];
	newMatrix[9]  = a[1] * b[8]  + a[5] * b[9]  + a[9] * b[10]  + a[13] * b[11];
	newMatrix[10] = a[2] * b[8]  + a[6] * b[9]  + a[10] * b[10] + a[14] * b[11];
	newMatrix[11] = a[3] * b[8]  + a[7] * b[9]  + a[11] * b[10] + a[15] * b[11];

	newMatrix[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14]  + a[12] * b[15];
	newMatrix[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14]  + a[13] * b[15];
	newMatrix[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15];
	newMatrix[15] = a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15];

#endif

	set( newMatrix );
}

void Matrix::setTranslation( const float *translation )
{
#if 0 //asssimp

//#define M(row,col)  m_matrix[col*4+row]
////#define M(row,col)  m_matrix[row*4+col]
    M(0,0) = 1.0f; M(0,1) = 0.0f; M(0,2) = 0.0f; M(0,3) = x;
    M(1,0) = 0.0f; M(1,1) = 1.0f; M(1,2) = 0.0f; M(1,3) = y;
    M(2,0) = 0.0f; M(2,1) = 0.0f; M(2,2) = 1.0f; M(2,3) = z;
    M(3,0) = 0.0f; M(3,1) = 0.0f; M(3,2) = 0.0f; M(3,3) = 1.0f;
#endif

#if 0
	m_matrix[0] = m_matrix[5] =  m_matrix[10] = m_matrix[15] = 1.0;
    m_matrix[1] = m_matrix[2] = m_matrix[3] = m_matrix[4] = 0.0;
    m_matrix[6] = m_matrix[7] = m_matrix[8] = m_matrix[9] = 0.0;
    m_matrix[11] = 0.0;

#elif 1

	m_matrix[12] = translation[0];
	m_matrix[13] = translation[1];
	m_matrix[14] = translation[2];

#elif 0
=
/*
// http://stackoverflow.com/questions/13293469/why-does-my-translation-matrix-needs-to-be-transposed
1, 0, 0, 0
0, 1, 0, 0
0, 0, 1, 0
x, y, z, 1
*/
	M(0,0) = 1;
	M(0,1) = 0;
	M(0,2) = 0;
	M(0,3) = translation[0];
	M(1,0) = 0;
	M(1,1) = 1;
	M(1,2) = 0;
	M(1,3) = translation[1];
	M(2,0) = 0;
	M(2,1) = 0;
	M(2,2) = 1;
	M(2,3) = translation[2];
	M(3,0) = 0;
	M(3,1) = 0;
	M(3,2) = 0;
	M(3,3) = 1;

#endif
}

void Matrix::setInverseTranslation( const float *translation )
{
	m_matrix[12] = -translation[0];
	m_matrix[13] = -translation[1];
	m_matrix[14] = -translation[2];
}

void Matrix::setScale( const float *scale )
{
	//reset();

	m_matrix[0] = scale[0];
	m_matrix[5] = scale[1];
	m_matrix[10] = scale[2];
	m_matrix[15] = 1;
}

void Matrix::setRotationDegrees( const float *angles )
{
	float vec[3];
	vec[0] = ( float )( angles[0]*180.0/M_PI );
	vec[1] = ( float )( angles[1]*180.0/M_PI );
	vec[2] = ( float )( angles[2]*180.0/M_PI );
	setRotationRadians( vec );
}

void Matrix::setInverseRotationDegrees( const float *angles )
{
	float vec[3];
	vec[0] = ( float )( angles[0]*180.0/M_PI );
	vec[1] = ( float )( angles[1]*180.0/M_PI );
	vec[2] = ( float )( angles[2]*180.0/M_PI );
	setInverseRotationRadians( vec );
}

void Matrix::setRotationRadians( const float *angles )
{
	double cr = cos( angles[0] );
	double sr = sin( angles[0] );
	double cp = cos( angles[1] );
	double sp = sin( angles[1] );
	double cy = cos( angles[2] );
	double sy = sin( angles[2] );

	m_matrix[0] = ( float )( cp*cy );
	m_matrix[1] = ( float )( cp*sy );
	m_matrix[2] = ( float )( -sp );

	double srsp = sr*sp;
	double crsp = cr*sp;

	m_matrix[4] = ( float )( srsp*cy-cr*sy );
	m_matrix[5] = ( float )( srsp*sy+cr*cy );
	m_matrix[6] = ( float )( sr*cp );

	m_matrix[8] = ( float )( crsp*cy+sr*sy );
	m_matrix[9] = ( float )( crsp*sy-sr*cy );
	m_matrix[10] = ( float )( cr*cp );
}

void Matrix::setInverseRotationRadians( const float *angles )
{
	double cr = cos( angles[0] );
	double sr = sin( angles[0] );
	double cp = cos( angles[1] );
	double sp = sin( angles[1] );
	double cy = cos( angles[2] );
	double sy = sin( angles[2] );

	m_matrix[0] = ( float )( cp*cy );
	m_matrix[4] = ( float )( cp*sy );
	m_matrix[8] = ( float )( -sp );

	double srsp = sr*sp;
	double crsp = cr*sp;

	m_matrix[1] = ( float )( srsp*cy-cr*sy );
	m_matrix[5] = ( float )( srsp*sy+cr*cy );
	m_matrix[9] = ( float )( sr*cp );

	m_matrix[2] = ( float )( crsp*cy+sr*sy );
	m_matrix[6] = ( float )( crsp*sy-sr*cy );
	m_matrix[10] = ( float )( cr*cp );
}

void Matrix::setRotationQuaternion( const Quaternion& quat )
{
	m_matrix[0] = ( float )( 1.0 - 2.0*quat[1]*quat[1] - 2.0*quat[2]*quat[2] );
	m_matrix[1] = ( float )( 2.0*quat[0]*quat[1] + 2.0*quat[3]*quat[2] );
	m_matrix[2] = ( float )( 2.0*quat[0]*quat[2] - 2.0*quat[3]*quat[1] );

	m_matrix[4] = ( float )( 2.0*quat[0]*quat[1] - 2.0*quat[3]*quat[2] );
	m_matrix[5] = ( float )( 1.0 - 2.0*quat[0]*quat[0] - 2.0*quat[2]*quat[2] );
	m_matrix[6] = ( float )( 2.0*quat[1]*quat[2] + 2.0*quat[3]*quat[0] );

	m_matrix[8] = ( float )( 2.0*quat[0]*quat[2] + 2.0*quat[3]*quat[1] );
	m_matrix[9] = ( float )( 2.0*quat[1]*quat[2] - 2.0*quat[3]*quat[0] );
	m_matrix[10] = ( float )( 1.0 - 2.0*quat[0]*quat[0] - 2.0*quat[1]*quat[1] );
}



void Matrix::InitScaleTransform(float ScaleX, float ScaleY, float ScaleZ)
{
    M(0,0) = ScaleX; M(0,1) = 0.0f;   M(0,2) = 0.0f;   M(0,3) = 0.0f;
    M(1,0) = 0.0f;   M(1,1) = ScaleY; M(1,2) = 0.0f;   M(1,3) = 0.0f;
    M(2,0) = 0.0f;   M(2,1) = 0.0f;   M(2,2) = ScaleZ; M(2,3) = 0.0f;
    M(3,0) = 0.0f;   M(3,1) = 0.0f;   M(3,2) = 0.0f;   M(3,3) = 1.0f;
}

void Matrix::InitRotateTransform(float RotateX, float RotateY, float RotateZ)
{
    Matrix rx, ry, rz;

    const float x = ToRadian(RotateX);
    const float y = ToRadian(RotateY);
    const float z = ToRadian(RotateZ);

    rx.M(0,0) = 1.0f; rx.M(0,1) = 0.0f   ; rx.M(0,2) = 0.0f    ; rx.M(0,3) = 0.0f;
    rx.M(1,0) = 0.0f; rx.M(1,1) = cosf(x); rx.M(1,2) = -sinf(x); rx.M(1,3) = 0.0f;
    rx.M(2,0) = 0.0f; rx.M(2,1) = sinf(x); rx.M(2,2) = cosf(x) ; rx.M(2,3) = 0.0f;
    rx.M(3,0) = 0.0f; rx.M(3,1) = 0.0f   ; rx.M(3,2) = 0.0f    ; rx.M(3,3) = 1.0f;

    ry.M(0,0) = cosf(y); ry.M(0,1) = 0.0f; ry.M(0,2) = -sinf(y); ry.M(0,3) = 0.0f;
    ry.M(1,0) = 0.0f   ; ry.M(1,1) = 1.0f; ry.M(1,2) = 0.0f    ; ry.M(1,3) = 0.0f;
    ry.M(2,0) = sinf(y); ry.M(2,1) = 0.0f; ry.M(2,2) = cosf(y) ; ry.M(2,3) = 0.0f;
    ry.M(3,0) = 0.0f   ; ry.M(3,1) = 0.0f; ry.M(3,2) = 0.0f    ; ry.M(3,3) = 1.0f;

    rz.M(0,0) = cosf(z); rz.M(0,1) = -sinf(z); rz.M(0,2) = 0.0f; rz.M(0,3) = 0.0f;
    rz.M(1,0) = sinf(z); rz.M(1,1) = cosf(z) ; rz.M(1,2) = 0.0f; rz.M(1,3) = 0.0f;
    rz.M(2,0) = 0.0f   ; rz.M(2,1) = 0.0f    ; rz.M(2,2) = 1.0f; rz.M(2,3) = 0.0f;
    rz.M(3,0) = 0.0f   ; rz.M(3,1) = 0.0f    ; rz.M(3,2) = 0.0f; rz.M(3,3) = 1.0f;

    *this = rz * ry * rx;
}

void Matrix::InitTranslationTransform(float x, float y, float z)
{
    M(0,0) = 1.0f; M(0,1) = 0.0f; M(0,2) = 0.0f; M(0,3) = x;
    M(1,0) = 0.0f; M(1,1) = 1.0f; M(1,2) = 0.0f; M(1,3) = y;
    M(2,0) = 0.0f; M(2,1) = 0.0f; M(2,2) = 1.0f; M(2,3) = z;
    M(3,0) = 0.0f; M(3,1) = 0.0f; M(3,2) = 0.0f; M(3,3) = 1.0f;
}


void Matrix::InitCameraTransform(const Vec3f& Target, const Vec3f& Up)
{
    Vec3f N = Target;
    N.Normalize();
    Vec3f U = Up;
    U.Normalize();
    U = U.Cross(N);
    Vec3f V = N.Cross(U);

    M(0,0) = U.x;   M(0,1) = U.y;   M(0,2) = U.z;   M(0,3) = 0.0f;
    M(1,0) = V.x;   M(1,1) = V.y;   M(1,2) = V.z;   M(1,3) = 0.0f;
    M(2,0) = N.x;   M(2,1) = N.y;   M(2,2) = N.z;   M(2,3) = 0.0f;
    M(3,0) = 0.0f;  M(3,1) = 0.0f;  M(3,2) = 0.0f;  M(3,3) = 1.0f;
}

void Matrix::InitPersProjTransform(const PersProjInfo& p)
{
    const float ar         = p.Width / p.Height;
    const float zRange     = p.zNear - p.zFar;
    const float tanHalfFOV = tanf(ToRadian(p.FOV / 2.0f));

    M(0,0) = 1.0f/(tanHalfFOV * ar); M(0,1) = 0.0f;            M(0,2) = 0.0f;            M(0,3) = 0.0;
    M(1,0) = 0.0f;                   M(1,1) = 1.0f/tanHalfFOV; M(1,2) = 0.0f;            M(1,3) = 0.0;
    M(2,0) = 0.0f;                   M(2,1) = 0.0f;            M(2,2) = (-p.zNear - p.zFar)/zRange ; M(2,3) = 2.0f*p.zFar*p.zNear/zRange;
    M(3,0) = 0.0f;                   M(3,1) = 0.0f;            M(3,2) = 1.0f;            M(3,3) = 0.0;
}


float Matrix::Determinant() const
{
	return M(0,0)*M(1,1)*M(2,2)*M(3,3) - M(0,0)*M(1,1)*M(2,3)*M(3,2) + M(0,0)*M(1,2)*M(2,3)*M(3,1) - M(0,0)*M(1,2)*M(2,1)*M(3,3)
		+ M(0,0)*M(1,3)*M(2,1)*M(3,2) - M(0,0)*M(1,3)*M(2,2)*M(3,1) - M(0,1)*M(1,2)*M(2,3)*M(3,0) + M(0,1)*M(1,2)*M(2,0)*M(3,3)
		- M(0,1)*M(1,3)*M(2,0)*M(3,2) + M(0,1)*M(1,3)*M(2,2)*M(3,0) - M(0,1)*M(1,0)*M(2,2)*M(3,3) + M(0,1)*M(1,0)*M(2,3)*M(3,2)
		+ M(0,2)*M(1,3)*M(2,0)*M(3,1) - M(0,2)*M(1,3)*M(2,1)*M(3,0) + M(0,2)*M(1,0)*M(2,1)*M(3,3) - M(0,2)*M(1,0)*M(2,3)*M(3,1)
		+ M(0,2)*M(1,1)*M(2,3)*M(3,0) - M(0,2)*M(1,1)*M(2,0)*M(3,3) - M(0,3)*M(1,0)*M(2,1)*M(3,2) + M(0,3)*M(1,0)*M(2,2)*M(3,1)
		- M(0,3)*M(1,1)*M(2,2)*M(3,0) + M(0,3)*M(1,1)*M(2,0)*M(3,2) - M(0,3)*M(1,2)*M(2,0)*M(3,1) + M(0,3)*M(1,2)*M(2,1)*M(3,0);
}


Matrix& Matrix::Inverse()
{
	// Compute the reciprocal determinant
	float det = Determinant();
	if(det == 0.0f)
	{
		// Matrix not invertible. Setting all elements to nan is not really
		// correct in a mathematical sense but it is easy to debug for the
		// programmer.
		/*const float nan = std::numeric_limits<float>::quiet_NaN();
		*this = Matrix(
			nan,nan,nan,nan,
			nan,nan,nan,nan,
			nan,nan,nan,nan,
			nan,nan,nan,nan);*/
        assert(0);
		return *this;
	}

	float invdet = 1.0f / det;

	Matrix res;
	res.M(0,0) = invdet  * (M(1,1) * (M(2,2) * M(3,3) - M(2,3) * M(3,2)) + M(1,2) * (M(2,3) * M(3,1) - M(2,1) * M(3,3)) + M(1,3) * (M(2,1) * M(3,2) - M(2,2) * M(3,1)));
	res.M(0,1) = -invdet * (M(0,1) * (M(2,2) * M(3,3) - M(2,3) * M(3,2)) + M(0,2) * (M(2,3) * M(3,1) - M(2,1) * M(3,3)) + M(0,3) * (M(2,1) * M(3,2) - M(2,2) * M(3,1)));
	res.M(0,2) = invdet  * (M(0,1) * (M(1,2) * M(3,3) - M(1,3) * M(3,2)) + M(0,2) * (M(1,3) * M(3,1) - M(1,1) * M(3,3)) + M(0,3) * (M(1,1) * M(3,2) - M(1,2) * M(3,1)));
	res.M(0,3) = -invdet * (M(0,1) * (M(1,2) * M(2,3) - M(1,3) * M(2,2)) + M(0,2) * (M(1,3) * M(2,1) - M(1,1) * M(2,3)) + M(0,3) * (M(1,1) * M(2,2) - M(1,2) * M(2,1)));
	res.M(1,0) = -invdet * (M(1,0) * (M(2,2) * M(3,3) - M(2,3) * M(3,2)) + M(1,2) * (M(2,3) * M(3,0) - M(2,0) * M(3,3)) + M(1,3) * (M(2,0) * M(3,2) - M(2,2) * M(3,0)));
	res.M(1,1) = invdet  * (M(0,0) * (M(2,2) * M(3,3) - M(2,3) * M(3,2)) + M(0,2) * (M(2,3) * M(3,0) - M(2,0) * M(3,3)) + M(0,3) * (M(2,0) * M(3,2) - M(2,2) * M(3,0)));
	res.M(1,2) = -invdet * (M(0,0) * (M(1,2) * M(3,3) - M(1,3) * M(3,2)) + M(0,2) * (M(1,3) * M(3,0) - M(1,0) * M(3,3)) + M(0,3) * (M(1,0) * M(3,2) - M(1,2) * M(3,0)));
	res.M(1,3) = invdet  * (M(0,0) * (M(1,2) * M(2,3) - M(1,3) * M(2,2)) + M(0,2) * (M(1,3) * M(2,0) - M(1,0) * M(2,3)) + M(0,3) * (M(1,0) * M(2,2) - M(1,2) * M(2,0)));
	res.M(2,0) = invdet  * (M(1,0) * (M(2,1) * M(3,3) - M(2,3) * M(3,1)) + M(1,1) * (M(2,3) * M(3,0) - M(2,0) * M(3,3)) + M(1,3) * (M(2,0) * M(3,1) - M(2,1) * M(3,0)));
	res.M(2,1) = -invdet * (M(0,0) * (M(2,1) * M(3,3) - M(2,3) * M(3,1)) + M(0,1) * (M(2,3) * M(3,0) - M(2,0) * M(3,3)) + M(0,3) * (M(2,0) * M(3,1) - M(2,1) * M(3,0)));
	res.M(2,2) = invdet  * (M(0,0) * (M(1,1) * M(3,3) - M(1,3) * M(3,1)) + M(0,1) * (M(1,3) * M(3,0) - M(1,0) * M(3,3)) + M(0,3) * (M(1,0) * M(3,1) - M(1,1) * M(3,0)));
	res.M(2,3) = -invdet * (M(0,0) * (M(1,1) * M(2,3) - M(1,3) * M(2,1)) + M(0,1) * (M(1,3) * M(2,0) - M(1,0) * M(2,3)) + M(0,3) * (M(1,0) * M(2,1) - M(1,1) * M(2,0)));
	res.M(3,0) = -invdet * (M(1,0) * (M(2,1) * M(3,2) - M(2,2) * M(3,1)) + M(1,1) * (M(2,2) * M(3,0) - M(2,0) * M(3,2)) + M(1,2) * (M(2,0) * M(3,1) - M(2,1) * M(3,0)));
	res.M(3,1) = invdet  * (M(0,0) * (M(2,1) * M(3,2) - M(2,2) * M(3,1)) + M(0,1) * (M(2,2) * M(3,0) - M(2,0) * M(3,2)) + M(0,2) * (M(2,0) * M(3,1) - M(2,1) * M(3,0)));
	res.M(3,2) = -invdet * (M(0,0) * (M(1,1) * M(3,2) - M(1,2) * M(3,1)) + M(0,1) * (M(1,2) * M(3,0) - M(1,0) * M(3,2)) + M(0,2) * (M(1,0) * M(3,1) - M(1,1) * M(3,0)));
	res.M(3,3) = invdet  * (M(0,0) * (M(1,1) * M(2,2) - M(1,2) * M(2,1)) + M(0,1) * (M(1,2) * M(2,0) - M(1,0) * M(2,2)) + M(0,2) * (M(1,0) * M(2,1) - M(1,1) * M(2,0)));
	*this = res;

	return *this;
}

#undef M

bool InvertMatrix(const float m[16], float invOut[16])
{
    float inv[16], det;
    int i;

    inv[0] = m[5]  * m[10] * m[15] - 
             m[5]  * m[11] * m[14] - 
             m[9]  * m[6]  * m[15] + 
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] - 
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + 
              m[4]  * m[11] * m[14] + 
              m[8]  * m[6]  * m[15] - 
              m[8]  * m[7]  * m[14] - 
              m[12] * m[6]  * m[11] + 
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - 
             m[4]  * m[11] * m[13] - 
             m[8]  * m[5] * m[15] + 
             m[8]  * m[7] * m[13] + 
             m[12] * m[5] * m[11] - 
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] + 
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] - 
               m[8]  * m[6] * m[13] - 
               m[12] * m[5] * m[10] + 
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] + 
              m[1]  * m[11] * m[14] + 
              m[9]  * m[2] * m[15] - 
              m[9]  * m[3] * m[14] - 
              m[13] * m[2] * m[11] + 
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] - 
             m[0]  * m[11] * m[14] - 
             m[8]  * m[2] * m[15] + 
             m[8]  * m[3] * m[14] + 
             m[12] * m[2] * m[11] - 
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] + 
              m[0]  * m[11] * m[13] + 
              m[8]  * m[1] * m[15] - 
              m[8]  * m[3] * m[13] - 
              m[12] * m[1] * m[11] + 
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] - 
              m[0]  * m[10] * m[13] - 
              m[8]  * m[1] * m[14] + 
              m[8]  * m[2] * m[13] + 
              m[12] * m[1] * m[10] - 
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] - 
             m[1]  * m[7] * m[14] - 
             m[5]  * m[2] * m[15] + 
             m[5]  * m[3] * m[14] + 
             m[13] * m[2] * m[7] - 
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] + 
              m[0]  * m[7] * m[14] + 
              m[4]  * m[2] * m[15] - 
              m[4]  * m[3] * m[14] - 
              m[12] * m[2] * m[7] + 
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] - 
              m[0]  * m[7] * m[13] - 
              m[4]  * m[1] * m[15] + 
              m[4]  * m[3] * m[13] + 
              m[12] * m[1] * m[7] - 
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] + 
               m[0]  * m[6] * m[13] + 
               m[4]  * m[1] * m[14] - 
               m[4]  * m[2] * m[13] - 
               m[12] * m[1] * m[6] + 
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + 
              m[1] * m[7] * m[10] + 
              m[5] * m[2] * m[11] - 
              m[5] * m[3] * m[10] - 
              m[9] * m[2] * m[7] + 
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - 
             m[0] * m[7] * m[10] - 
             m[4] * m[2] * m[11] + 
             m[4] * m[3] * m[10] + 
             m[8] * m[2] * m[7] - 
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + 
               m[0] * m[7] * m[9] + 
               m[4] * m[1] * m[11] - 
               m[4] * m[3] * m[9] - 
               m[8] * m[1] * m[7] + 
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - 
              m[0] * m[6] * m[9] - 
              m[4] * m[1] * m[10] + 
              m[4] * m[2] * m[9] + 
              m[8] * m[1] * m[6] - 
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return false;

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return true;
}
