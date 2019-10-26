#include <iostream>
#include <fstream>
#include <Eigen>
#include <string>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <transformation.hpp>
#include <scene.hpp>
#include <screen.hpp>

using namespace std;
using namespace Eigen;
using namespace transformation;

namespace parser
{

	Matrix4f ParseCamera(ifstream &file, Vector3f &camPos)
	{
		float camX, camY, camZ, rotX, rotY, rotZ, angle;
		string label;
		string line;
		istringstream iss;
		getline(file, line);
		if(line.substr(0, line.find(" ")) == "position")
		{
			iss.str(line);
			iss >> label >> camX >> camY >> camZ;
			camPos = Vector3f(camX, camY, camZ);
			line.clear();
			iss.clear();
		}
		getline(file, line);
		if(line.substr(0, line.find(" ")) == "orientation")
		{
			iss.str(line);
			iss >> label >> rotX >> rotY >> rotZ >> angle;
			line.clear();
			iss.clear();
		}
		return CameraMat(camX, camY, camZ, rotX, rotY, rotZ, angle);
	}

	float ProjParserHelper(ifstream &file, string keyword)
	{
		float v;
		string label;
		string line;
		istringstream iss;
		getline(file, line);
		if(line.substr(0, line.find(" ")) == keyword)
		{
			iss.str(line);
			iss >> label >> v;
		}
		iss.clear();
		line.clear();
		return v;
	}

	Matrix4f ParseProjection(ifstream &file)
	{
		float n, f, l, r, t, b;
		n = ProjParserHelper(file, "near");
		f = ProjParserHelper(file, "far");
		l = ProjParserHelper(file, "left");
		r = ProjParserHelper(file, "right");
		t = ProjParserHelper(file, "top");
		b = ProjParserHelper(file, "bottom");
		return ProjectionMat(n, f, l, r, t, b);
	}

	obj_src ParseObjFile(string path)
	{
		obj_src obj;
		// 0 index of v
		obj.v.push_back(Vector4f(0, 0, 0, 0));
		obj.n.push_back(Vector4f(0, 0, 0, 0));
		ifstream objFile;
		objFile.open(path, ios::in);
		if(!objFile)
		{
			cout << "Can't open file: " << path << endl;
		}
		string objLine, type, symbol;
		istringstream objIss;
		float x, y, z, nx, ny, nz;
		while(getline(objFile, objLine))
		{
			objIss.str(objLine);
			string prefix = objLine.substr(0, objLine.find(" "));
			if(prefix == "v")
			{
				objIss >> type >> x >> y >> z;
				obj.v.push_back(Vector4f(x, y, z, 1));
			}
			if(prefix == "vn")
			{
				objIss >> type >> x >> y >> z;
				obj.n.push_back(Vector4f(x, y, z, 1));
			}
			if(prefix == "f")
			{
				size_t pos = objLine.find("//");
				while(pos != string::npos)
				{
					objLine.replace(pos, 2, " ");
					pos = objLine.find("//", pos + 1);
				}
				objIss.str(objLine);
				objIss >> type >> x >> nx >> y >> ny >> z >> nz;
				obj.f.push_back(Vector3f(x, y, z));
				obj.nf.push_back(Vector3f(nx, ny, nz));
			}
			objIss.clear();
		}
		objFile.close();
		return obj;
	}

	void ParseObj(ifstream &file, string dir, unordered_map<string, obj_src> &map)
	{
		string line;
		istringstream iss;
		string label, fileName;
		while(getline(file, line))
		{
			if(line.length() != 0)
			{
				iss.str(line);
				iss >> label >> fileName;
				string path = dir + "/" + fileName;
				map[label] = ParseObjFile(path);
				iss.clear();
			}
			else
				break;
		}
	}

	model_mat ParseModelMat(ifstream &file, string key)
	{
		string line;
		istringstream iss;
		model_mat _mat;
		_mat.label = key;
		vector<Matrix4f> _m;
		while(getline(file, line))
		{
			if(line.size() > 0)
			{
				float x, y, z, a;
				string type;
				iss.str(line);
				string prefix = line.substr(0, line.find(" "));
				if(prefix == "t")
				{
					iss >> type >> x >> y >> z;
					_m.push_back(transformation::Translate(x, y, z));
				}
				if(prefix == "r")
				{
					iss >> type >> x >> y >> z >> a;
					Vector3f rot = Vector3f(x, y, z).normalized();// normalize rotation axes
					_m.push_back(transformation::Rotate(rot[0], rot[1], rot[2], a));
				}
				if(prefix == "s")
				{
					iss >> type >> x >> y >> z;
					_m.push_back(transformation::Scale(x, y, z));
				}
				iss.clear();
			}
			else if(line.size() == 0)
			{
				_mat.mat = transformation::ModelMat(_m);
				break;
			}
		}
		if(file.eof())
		{
			_mat.mat = transformation::ModelMat(_m);
		}
		return _mat;
	}

	material_data ParseMaterial(ifstream &file, string key)
	{
		material_data material;
		int propertyNum = 4;
		string line;
		string label;
		Vector3f val;
		float sVal;
		for(int i = 0; i < propertyNum; i++)
		{
			getline(file, line);
			string hint = line.substr(0, line.find(" "));
			istringstream iss(line);
			if(hint == "ambient")
			{
				iss >> label >> val[0] >> val[1] >> val[2];
				material.ambient = val;
			}
			if(hint == "diffuse")
			{
				iss >> label >> val[0] >> val[1] >> val[2];
				material.diffuse = val;
			}
			if(hint == "specular")
			{
				iss >> label >> val[0] >> val[1] >> val[2];
				material.specular = val;
			}
			if(hint == "shininess")
			{
				iss >> label >> sVal;
				material.shininess = sVal;
			}
			iss.clear();
		}
		return material;
	}

	void ParseShadingnMotel(ifstream &file, vector<model_mat> &modelData, vector<material_data> &materialData)
	{
		string line;
		istringstream iss;
		while(getline(file, line))
		{
			int spaceCount = 0;
			for(int i = 0; i < line.size(); i++)
			{
				if(line[i] == ' ')
					spaceCount++;
			}
			if(line.size() > 0 && spaceCount == 0)
			{
				string key;
				iss.str(line);
				iss >> key;
				iss.clear();
				materialData.push_back(ParseMaterial(file, key));
				modelData.push_back(ParseModelMat(file, key));
			}	
		}
	}

	void ParseLight(ifstream &file, string &firstLine, vector<light_data> &light)
	{
		light_data firstLit;
		string line;
		string label;
		firstLine.erase(remove(firstLine.begin(), firstLine.end(), ','), firstLine.end());
		istringstream iss(firstLine);
		iss >> label >> firstLit.position[0] >> firstLit.position[1] >> firstLit.position[2] 
		>> firstLit.color[0] >> firstLit.color[1] >> firstLit.color[2] >> firstLit.attenuation;
		light.push_back(firstLit);
		//cout << firstLit.position << '\n' << firstLit.color << '\n' << firstLit.attenuation << endl;			
		iss.clear();

		while(getline(file, line))
		{
			if(line.size() != 0)
			{
				line.erase(remove(line.begin(), line.end(), ','), line.end());
				light_data thisLit;
				iss.str(line);
				iss >> label >> thisLit.position[0] >> thisLit.position[1] >> thisLit.position[2] 
				>> thisLit.color[0] >> thisLit.color[1] >> thisLit.color[2] >> thisLit.attenuation;	
				//cout << thisLit.position << '\n' << thisLit.color << '\n' << thisLit.attenuation << endl;			
				iss.clear();
				light.push_back(thisLit);
			}
			else
			{
				//getline(file, line);
				break;
			}
		}
	}

	void ParseData(char* file, Scene::SceneData &scene)
	{
		ifstream inFile;
		inFile.open(file);
		string dir = file;
		dir = dir.substr(0, dir.find("/"));
		string line;
		if(!inFile)
		{
			cout << "Can't open scene file!" << endl;
		}
		
		while(getline(inFile, line))
		{
			string hint = line.substr(0, line.find(" "));
			//cout << "hint: " << hint << endl;
			if(hint == "camera:")
			{
				scene.transformData.camera = ParseCamera(inFile, scene.camPos);
				scene.transformData.projection = ParseProjection(inFile);
				getline(inFile, line);
				continue;
			}
			if(hint == "light")
			{
				ParseLight(inFile, line, scene.lightData);
				continue;
			}
			if(hint == "objects:")
			{
				ParseObj(inFile, dir, scene.objSrc);
			}
			// obj model mat
			ParseShadingnMotel(inFile, scene.modelMat, scene.materialData);
		} 
		inFile.close();
	}
}