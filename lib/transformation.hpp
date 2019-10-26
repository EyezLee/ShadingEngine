#include <Eigen>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

using namespace std;
using namespace Eigen;

namespace transformation
{
	// basic mat generation
	Matrix4f Translate(float x, float y, float z)
	{
		Matrix4f t;
		t << 1, 0, 0, x, 
			 0, 1, 0, y,
			 0, 0, 1, z,
			 0, 0, 0, 1;
		return t;
	}

	Matrix4f Rotate(float x, float y, float z, float r)
	{
		Matrix4f R;

		R << x*x+(1-x*x)*cos(r), x*y*(1-cos(r))-z*sin(r), x*z*(1-cos(r))+y*sin(r), 0,
			 y*x*(1-cos(r))+z*sin(r), y*y+(1-y*y)*cos(r), y*z*(1-cos(r))-x*sin(r), 0,
			 z*y*(1-cos(r))-y*sin(r), z*y*(1-cos(r))+x*sin(r), z*z+(1-z*z)*cos(r), 0,
			 0, 0, 0, 1;
		return R;
	}

	Matrix4f Scale(float x, float y, float z)
	{
		Matrix4f s;
		s << x, 0, 0, 0,
			 0, y, 0, 0,
			 0, 0, z, 0,
			 0, 0, 0, 1;
		return s;
	}

	Matrix4f CameraMat(float xPos, float yPos, float zPos, float xRot, float yRot, float zRot, float angle)
	{
		Matrix4f Tc = Translate(xPos, yPos, zPos);
		Matrix4f Rc = Rotate(xRot, yRot, zRot, angle);
		Matrix4f cam = Rc * Tc;
		return cam.inverse();
	}

	Matrix4f ProjectionMat(float n, float f, float l, float r, float t, float b)
	{
		Matrix4f p;
		p << 2*n/(r-l), 0, (r+l)/(r-l), 0,
			 0, 2*n/(t-b),  (t+b)/(t-b), 0,
			 0, 0, -(f+n)/(f-n), -2*f*n/(f-n),
			 0, 0, -1, 0;
		return p;
	}

	Matrix4f ModelMat(vector<Matrix4f> mat)
	{
		Matrix4f model = mat[0];
		for(int i = 1; i < mat.size(); i++)
		{
			model = mat[i] * model;
		}
		return model;
	}
}