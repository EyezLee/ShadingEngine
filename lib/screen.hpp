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
	vector<vector<Vector3f>> pixels;
	vector<vector<float>> dBuffer;
	Vector3f bgCol;

	ScreenData(int xRes, int yRes, Vector3f _bgCol)
	{
		width = xRes;
		height = yRes;
		bgCol = _bgCol;
		for(int y = 0; y < yRes; y++)
		{
			vector<Vector3f> pxlRow;
			vector<float> dptRow;
			for(int x = 0; x < xRes; x++)
			{
				pxlRow.push_back(bgCol);
				dptRow.push_back(3.402823e+38);
			}
			pixels.push_back(pxlRow);
			dBuffer.push_back(dptRow);
		}
	}
};
#endif