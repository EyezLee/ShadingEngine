#ifndef SCREEN_H
#define SCREEN_H 

#include <Eigen>
#include <vector>
#include <string>
#include <scene.hpp>

using namespace Eigen;
using namespace std;
struct tempObj
{
	vector<Vector2f> v;
	vector<Vector3f> f;
};

struct ScreenData
{
	vector<tempObj> temp;
	int width;
	int height;
	Vector3f bgColor = Vector3f(0, 0, 0);
	Vector3f wireColor = Vector3f(255, 255, 0);
	vector<vector<Vector3f>> pixels;
	vector<vector<float>> dBuffer;
	void Init(int xRes, int yRes, ScreenData &scr);
};


void ScreenData::Init(int xRes, int yRes, ScreenData &scr)
{
	for(int y = 0; y < yRes; y++)
	{
		vector<Vector3f> row;
		vector<float> depth;
		for(int x = 0; x < xRes; x++)
		{
			row.push_back(scr.bgColor);
			depth.push_back(3.402823e+38);
		}
		scr.pixels.push_back(row);
		scr.dBuffer.push_back(depth);
	}
	scr.width = xRes;
	scr.height = yRes;
}
#endif