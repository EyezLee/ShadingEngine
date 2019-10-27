#include <scene.hpp>
#include <screen.hpp>
#include <parser.hpp>
#include <render.hpp>

#include <iostream>
#include <string>

using namespace std;

Scene::SceneData scene;

void Render(int width, int height, int mode);
void PPM(int w, int h, ScreenData &scr);

int main(int argc, char *argv[])
{
	int width = atoi(argv[2]);
	int height = atoi(argv[3]);
	int mode = atoi(argv[4]);
	Vector3f bgCol = Vector3f(0, 0, 0);
	ScreenData screen(width, height, bgCol);
	// load scene data
	parser::ParseData(argv[1], scene);
	// transfer to worldspace
	scene.WorldSpace(scene);

	// render
	render::Render(width, height, mode, scene, screen);
	// printout picture
	PPM(width, height, screen);

	return 0;
}

void PPM(int w, int h, ScreenData &scr)
{
	cout << "P3" << endl;
	cout << w << " " << h << endl;
	cout << 255 << endl; 

	for(int y = 0; y < h; y++)
	{
		for(int x = 0; x < w; x++)
		{
			Vector3f p = scr.pixels[y][x];
			cout << p[0] << " " << p[1] << " " << p[2] << endl;
		}
	}
}
