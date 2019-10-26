#include <scene.hpp>
#include <screen.hpp>
#include <parser.hpp>
#include <render.hpp>

#include <iostream>
#include <string>

using namespace std;

Scene::SceneData scene;
ScreenData screen;

void Render(int width, int height, int mode);

int main(int argc, char *argv[])
{
	parser::ParseData(argv[1], scene);
	scene.WorldSpace(scene);
	screen.Init(atoi(argv[2]), atoi(argv[3]), screen);
	Render(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
	// PPM
	cout << "P3" << endl;
	cout << atoi(argv[2]) << " " << atoi(argv[3]) << endl;
	cout << 255 << endl; 

	for(int y = 0; y < screen.pixels.size(); y++)
	{
		for(int x = 0; x < screen.pixels[y].size(); x++)
		{
			Vector3f p = screen.pixels[y][x];
			cout << p[0] << " " << p[1] << " " << p[2] << endl;
		}
	}
	return 0;
}

void Render(int width, int height, int mode)
{
	for(int i = 0; i < scene.objWorld.size(); i++)
	{
		obj_world obj = scene.objWorld[i];
		for(int j = 0; j < obj.f.size(); j++)
		{	
			Vector3f thisFaceV = obj.f[j];
			Vector3f thisFaceN = obj.nf[j];
			int indexV0 = thisFaceV[0];
			int indexV1 = thisFaceV[1];
			int indexV2 = thisFaceV[2];
			//cout << "face: " << indexV0 << ", " << indexV1 << ", " << indexV2 << endl;
			Vector4f v0 = obj.v[indexV0];
			Vector4f v1 = obj.v[indexV1];
			Vector4f v2 = obj.v[indexV2];
			//cout << "v: " << "v0" << v0 << "\n" << "v1" << v1 << "\n" << "v2" << v2 << endl;
			int indexN0 = thisFaceN[0];
			int indexN1 = thisFaceN[1];
			int indexN2 = thisFaceN[2];
			//cout << "norm: " << indexN0 << ", " << indexN1 << ", " << indexN2 << endl;
			Vector4f n0 = obj.n[indexN0];
			Vector4f n1 = obj.n[indexN1];
			Vector4f n2 = obj.n[indexN2];
			//cout << "n: " << "n0" << n0 << "\n" << "n1" << n1 << "\n" << "n2" << n2 << endl;
			if(mode == 0)
				render::PhongShading(v0, v1, v2, n0, n1, n2, scene.camPos, obj.material, scene.lightData, scene.transformData, width, height, screen.pixels, screen.dBuffer);
			else if(mode == 1)
			render::GouraudShading(v0, v1, v2, n0, n1, n2, scene.camPos, obj.material, scene.lightData, scene.transformData, width, height, screen.pixels, screen.dBuffer);
			else if(mode == 2)
			render::FlatShading(v0, v1, v2, n0, n1, n2, scene.camPos, obj.material, scene.lightData, scene.transformData, width, height, screen.pixels, screen.dBuffer);

		}
	}
}