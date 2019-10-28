#ifndef SCENE_H
#define SCENE_H 

#include <Eigen>
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>
#include <iostream>

using namespace Eigen;
using namespace std;

struct model_mat
{
	string label;
	Matrix4f mat;
};

struct material_data
{
	string label;
	Vector3f ambient;
	Vector3f diffuse;
	Vector3f specular;
	float shininess;
};

struct obj_src
{
	vector<Vector4f> v;
	vector<Vector4f> n;
	vector<Vector3f> f;
	vector<Vector3f> nf;
};

struct obj_world
{
	vector<Vector4f> v;
	vector<Vector4f> n;
	vector<Vector3f> f;
	vector<Vector3f> nf;
	material_data material;
};

struct transform_data
{
	Matrix4f camera;
	Matrix4f projection;
};

struct light_data
{
	Vector3f position;
	Vector3f color;
	float attenuation;
};

namespace Scene
{
	class SceneData
	{
		public:
			transform_data transformData;
			vector<model_mat> modelMat;
			vector<material_data> materialData;
			vector<light_data> lightData;

			unordered_map<string, obj_src> objSrc;
			vector<obj_world> objWorld;

			Vector3f camPos;

			void WorldSpace(SceneData &scene);
	};

	Vector2f NDCtoScreen(int xRes, int yRes, Vector3f Vndc)
	{
		float x = (Vndc[0] + 1) * (xRes / 2);
		float y = yRes - (Vndc[1] + 1) * (yRes / 2);
		return Vector2f((int) x, (int) y);
	}

	Vector3f WorldtoNDC(transform_data &trans, Vector4f v)
	{
		Matrix4f camMat = trans.camera;
		Matrix4f projMat = trans.projection;
		Vector4f vCam = projMat * camMat * v;
		Vector3f Vndc = Vector3f(vCam[0] / vCam[3], vCam[1] / vCam[3], vCam[2] / vCam[3]);
		return Vndc;
	}

	void SceneData::WorldSpace(SceneData &scene)
	{
		for(int i = 0; i < scene.modelMat.size(); i++)
		{
			obj_world objWorld;
			unordered_map<string, obj_src>::iterator it;
			model_mat currentModelMat = scene.modelMat[i];
			it = scene.objSrc.find(currentModelMat.label);
			for(int j = 0; j < it->second.v.size(); j++)
			{
				Vector4f vertWorld = currentModelMat.mat * it->second.v[j];
				objWorld.v.push_back(vertWorld);
			}
			// transform normal to world space
			Matrix4f model_inv = currentModelMat.mat.inverse();
			Matrix4f model_norm = model_inv.transpose();
			for(int k = 0; k < it->second.n.size(); k++)
			{
				Vector4f normWorld = model_norm * it->second.n[k];
				objWorld.n.push_back(normWorld.normalized());
			}
			objWorld.f = it->second.f;
			objWorld.nf = it->second.nf;
			objWorld.material = scene.materialData[i];
			scene.objWorld.push_back(objWorld);
		}
	}
}
#endif
