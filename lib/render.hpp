#include <iostream>
#include <Eigen>
#include <scene.hpp>
#include <screen.hpp>
#include <cmath>

using namespace std;
using namespace Eigen;

namespace render
{
	void DrawLine(Vector2f p0, Vector2f p1, vector<vector<Vector3f>> &pixels, Vector3f vertCol)
	{
		int x0 = p0[0];
		int y0 = p0[1];
		int x1 = p1[0];
		int y1 = p1[1];
		int dx = x1 - x0;
		int dy = y1 - y0;
		float e = 0; // distance to true y on y
		int xPos = x0;
		int yPos = y0;
		if(dy > 0 && dx > 0)
		{
			float m = dy / dx;
			if(m < 1)
			{
				for(int xPos = x0; xPos < x1; xPos++)
				{
					pixels[yPos][xPos] = vertCol;
					if(2 * (e + dy) < dx)
						e = e + dy;
					else
					{
						e = e + dy - dx;
						yPos += 1;
					}
				}
			}
			else
			{
				for(int yPos = y0; yPos < y1; yPos++)
				{
					pixels[yPos][xPos] = vertCol;
					if(2 * (e + dx) > dy)
					{
						e = e + dx - dy;
						xPos += 1;
					}
					else
						e = e + dx;
				}
			}
		}
		else if(dy < 0 && dx < 0)
		{
			float m = dy / dx;
			if(m < 1)
			{
				for(int xPos = x0; xPos > x1; xPos--)
				{
					pixels[yPos][xPos] = vertCol;
					if(2 * (e - dy) > -dx)
					{
						e = e - dy + dx;
						yPos -=1;
					}
					else
						e = e - dy;
				}
			}
			else
			{
				for(int yPos = y0; yPos > y1; yPos--)
				{
					pixels[yPos][xPos] = vertCol;
					if(2 * (e - dx) > -dy)
					{
						e = e - dx + dy;
						xPos -= 1;
					}
					else
						e = e - dx;
				}
			}
		}
		else if(dy < 0 && dx > 0)
		{
			float m = dy / dx;
			if(m > -1)
			{
				for(int xPos = x0; xPos < x1; xPos++)
				{
					pixels[yPos][xPos] = vertCol;
					if(2 * (e + dy) < -dx)
					{
						e = e + dy + dx;
						yPos -= 1;
					}
					else
						e = e + dy;
				}
			}
			else
			{
				for(int yPos = y0; yPos > y1; yPos--)
				{
					pixels[yPos][xPos] = vertCol;
					if(2 * (e - dx) > dy)
						e = e - dx;
					else
					{
						e = e - dx - dy;
						xPos += 1;
					}
				}
			}
		}
		else if(dy > 0 && dx < 0)
		{
			float m = dy / dx;
			if(m > -1)
			{
				for(int xPos = x0; xPos > x1; xPos--)
				{
					pixels[yPos][xPos] = vertCol;
					if(2 * (e - dy) > dx)
						e = e - dy;
					else
					{
						e = e - dy - dx; 
						yPos += 1;
					}
				}
			}
			else
			{
				for(int yPos = y0; yPos < y1; yPos++)
				{
					pixels[yPos][xPos] = vertCol;
					if(2 * (e + dx) < -dy)
					{
						e = e + dx + dy;
						xPos -= 1;
					}
					else
						e = e + dx;
				}
			}
		}
		else if(dx == 0 && dy != 0)
		{
			for(int step = 0; step < dy; step++)
			{
				pixels[yPos][xPos] = vertCol;
				if(dy < 0)
					yPos -= 1;
				if(dy > 0)
					yPos += 1;
			}
		}
		else if(dx != 0 && dy == 0)
		{
			for(int step = 0; step < dx; step++)
			{
				pixels[yPos][xPos] = vertCol;
				if(dx < 0)
					xPos -= 1;
				if(dx > 0)
					xPos += 1;
			}
		}
	}
	float ComputeAlpha(int xa, int ya, int xb, int yb, int xc, int yc, int x, int y)
	{
		float Aa = (yb-yc)*x+(xc-xb)*y+xb*yc-xc*yb; 
		float A =  xa * yb + xb * yc + xc * ya - xa * yc - xb * ya - xc * yb;
		return (Aa / A);
	}

	float ComputeBeta(int xa, int ya, int xb, int yb, int xc, int yc, int x, int y)
	{
		float Ab = xa * y + x * yc + xc * ya - xa * yc - x * ya - xc * y;
		float A =  xa * yb + xb * yc + xc * ya - xa * yc - xb * ya - xc * yb;
		return (Ab / A);
	}
	float ComputeGamma(int xa, int ya, int xb, int yb, int xc, int yc, int x, int y)
	{
		float Ac = ((ya-yb)*x+(xb-xa)*y+xa*yb-xb*ya);
		float A =  xa * yb + xb * yc + xc * ya - xa * yc - xb * ya - xc * yb;
		return (Ac / A);
	}

	void FillPixel(int x, int y, vector<vector<Vector3f>> &pixels, Vector3f c)
	{
		pixels[y][x] = c;
	}

	Vector3f Lighting(Vector3f v, Vector3f n, material_data &mat, vector<light_data> &lightData, Vector3f e)
	{
		Vector3f diffuseCol = mat.diffuse;
		Vector3f ambientCol = mat.ambient;
		Vector3f specularCol = mat.specular;
		float shininess = mat.shininess;

		Vector3f diffuseSum = Vector3f(0, 0, 0);
		Vector3f specularSum = Vector3f(0, 0, 0);

		Vector3f eDir = e - v;
		eDir = eDir.normalized();

		for(int i = 0; i < lightData.size(); i++)
		{
			light_data currLit = lightData[i];
			Vector3f lPos = currLit.position;
			Vector3f lCol = currLit.color;
			Vector3f lDir = (lPos - v).normalized();
			float atten = currLit.attenuation;
			lCol *= 1 / (1 + atten * (lPos - v).squaredNorm());

			float a = 0;
			float d = n.dot(lDir);
			Vector3f lDiffuse = lCol * max(a, d);
			diffuseSum += lDiffuse;

			Vector3f h = lDir + eDir;
			float s = n.dot(h.normalized());
			Vector3f lSpecular = lCol * pow(max(a, s), shininess);
			specularSum += lSpecular;
		}
		float max = 1;
		float r = ambientCol[0] + diffuseCol[0] * diffuseSum[0] + specularCol[0] * specularSum[0];
		float g = ambientCol[1] + diffuseCol[1] * diffuseSum[1] + specularCol[1] * specularSum[1];
		float b = ambientCol[2] + diffuseCol[2] * diffuseSum[2] + specularCol[2] * specularSum[2];
		r = std::min(max, r);
		g = std::min(max, g);
		b = std::min(max, b);
		Vector3f output = Vector3f(r, g, b);
		return output;
	}

	void RasterTriangle(Vector3f Va_ndc, Vector3f Vb_ndc, Vector3f Vc_ndc, Vector3f aCol, Vector3f bCol, Vector3f cCol, ScreenData &scr)
	{
		// backface culling
		Vector3f direction = (Vc_ndc - Vb_ndc).cross(Va_ndc - Vb_ndc);
		if(direction[2] < 0)
			return;
		Vector2f Pa = Scene::NDCtoScreen(scr.width, scr.height, Va_ndc);
		Vector2f Pb = Scene::NDCtoScreen(scr.width, scr.height, Vb_ndc);
		Vector2f Pc = Scene::NDCtoScreen(scr.width, scr.height, Vc_ndc);

		int xMin = std::min(std::min(Pa[0], Pb[0]), Pc[0]);
		int xMax = std::max(std::max(Pa[0], Pb[0]), Pc[0]);
		int yMin = std::min(std::min(Pa[1], Pb[1]), Pc[1]);
		int yMax = std::max(std::max(Pa[1], Pb[1]), Pc[1]);

		for(int y = yMin; y <= yMax; y++)
		{
			for(int x = xMin; x <= xMax; x++)
			{
				float alpha = ComputeAlpha(Pa[0], Pa[1], Pb[0], Pb[1], Pc[0], Pc[1], x, y);
				float beta = ComputeBeta(Pa[0], Pa[1], Pb[0], Pb[1], Pc[0], Pc[1], x, y);
				float gamma = ComputeGamma(Pa[0], Pa[1], Pb[0], Pb[1], Pc[0], Pc[1], x, y);
				if(alpha >= 0 && alpha <= 1 && beta >= 0 && beta <= 1 && gamma >= 0 && gamma <= 1)
				{
					Vector3f pos = alpha * Va_ndc + beta * Vb_ndc + gamma * Vc_ndc;
					if(pos[0] > -1 && pos[0] < 1 && pos[1] > -1 && pos[1] < 1 && pos[2] > -1 && pos[2] < 1) // within crop cube
					{
						// check depth buffer
						float depth = pos[2];
						if(depth < scr.dBuffer[y][x])
						{
							scr.dBuffer[y][x] = depth;
							float r = alpha * aCol[0] + beta * bCol[0] + gamma * cCol[0];
							float g = alpha * aCol[1] + beta * bCol[1] + gamma * cCol[1];
							float b = alpha * aCol[2] + beta * bCol[2] + gamma * cCol[2];
							r = round(r * 255);
							g = round(g * 255);
							b = round(b * 255);
							Vector3f color = Vector3f(r, g, b);
							FillPixel(x, y, scr.pixels, color);
						}
					}
				}
			}
		}
	}
	void GouraudShading(Vector4f Va, Vector4f Vb, Vector4f Vc, Vector4f Na, Vector4f Nb, Vector4f Nc, material_data &material, Scene::SceneData &scn, ScreenData &scr)
	{
		// calculate lighting in worldspace
		Vector3f normA = Vector3f(Na[0], Na[1], Na[2]).normalized();
		Vector3f normB = Vector3f(Nb[0], Nb[1], Nb[2]).normalized();
		Vector3f normC = Vector3f(Nc[0], Nc[1], Nc[2]).normalized();
		Vector3f Va_world = Vector3f(Va[0], Va[1], Va[2]);
		Vector3f Vb_world = Vector3f(Vb[0], Vb[1], Vb[2]);
		Vector3f Vc_world = Vector3f(Vc[0], Vc[1], Vc[2]);

		Vector3f Va_ndc = Scene::WorldtoNDC(scn.transformData, Va);
		Vector3f Vb_ndc = Scene::WorldtoNDC(scn.transformData, Vb);
		Vector3f Vc_ndc = Scene::WorldtoNDC(scn.transformData, Vc);

		Vector3f aCol = Lighting(Va_world, normA, material, scn.lightData, scn.camPos);
		Vector3f bCol = Lighting(Vb_world, normB, material, scn.lightData, scn.camPos);
		Vector3f cCol = Lighting(Vc_world, normC, material, scn.lightData, scn.camPos);

		RasterTriangle(Va_ndc, Vb_ndc, Vc_ndc, aCol, bCol, cCol, scr);
	}

	void FlatShading(Vector4f Va, Vector4f Vb, Vector4f Vc, Vector4f Na, Vector4f Nb, Vector4f Nc, material_data &material, Scene::SceneData &scn, ScreenData &scr)
	{
		Vector3f normA = Vector3f(Na[0], Na[1], Na[2]).normalized();
		Vector3f normB = Vector3f(Nb[0], Nb[1], Nb[2]).normalized();
		Vector3f normC = Vector3f(Nc[0], Nc[1], Nc[2]).normalized();		
		Vector3f Va_world = Vector3f(Va[0], Va[1], Va[2]);
		Vector3f Vb_world = Vector3f(Vb[0], Vb[1], Vb[2]);
		Vector3f Vc_world = Vector3f(Vc[0], Vc[1], Vc[2]);

		Vector3f Va_ndc = Scene::WorldtoNDC(scn.transformData, Va);
		Vector3f Vb_ndc = Scene::WorldtoNDC(scn.transformData, Vb);
		Vector3f Vc_ndc = Scene::WorldtoNDC(scn.transformData, Vc);

		Vector3f aCol = Lighting(Va_world, normA, material, scn.lightData, scn.camPos);
		Vector3f bCol = Lighting(Vb_world, normB, material, scn.lightData, scn.camPos);
		Vector3f cCol = Lighting(Vc_world, normC, material, scn.lightData, scn.camPos);
		
		Vector3f colorAvg = (aCol + bCol + cCol) / 3;

		RasterTriangle(Va_ndc, Vb_ndc, Vc_ndc, colorAvg, colorAvg, colorAvg, scr);	
	}

	void PhongShading(Vector4f Va, Vector4f Vb, Vector4f Vc, Vector4f Na, Vector4f Nb, Vector4f Nc, material_data &material, Scene::SceneData &scn, ScreenData &scr)
	{
		Vector3f normA = Vector3f(Na[0], Na[1], Na[2]).normalized();
		Vector3f normB = Vector3f(Nb[0], Nb[1], Nb[2]).normalized();
		Vector3f normC = Vector3f(Nc[0], Nc[1], Nc[2]).normalized();		
		Vector3f Va_world = Vector3f(Va[0], Va[1], Va[2]);
		Vector3f Vb_world = Vector3f(Vb[0], Vb[1], Vb[2]);
		Vector3f Vc_world = Vector3f(Vc[0], Vc[1], Vc[2]);
		Vector3f Va_ndc = Scene::WorldtoNDC(scn.transformData, Va);
		Vector3f Vb_ndc = Scene::WorldtoNDC(scn.transformData, Vb);
		Vector3f Vc_ndc = Scene::WorldtoNDC(scn.transformData, Vc);
				// backface culling
		Vector3f direction = (Vc_ndc - Vb_ndc).cross(Va_ndc - Vb_ndc);
		if(direction[2] < 0)
			return;
		Vector2f Pa = Scene::NDCtoScreen(scr.width, scr.height, Va_ndc);
		Vector2f Pb = Scene::NDCtoScreen(scr.width, scr.height, Vb_ndc);
		Vector2f Pc = Scene::NDCtoScreen(scr.width, scr.height, Vc_ndc);

		int xMin = std::min(std::min(Pa[0], Pb[0]), Pc[0]);
		int xMax = std::max(std::max(Pa[0], Pb[0]), Pc[0]);
		int yMin = std::min(std::min(Pa[1], Pb[1]), Pc[1]);
		int yMax = std::max(std::max(Pa[1], Pb[1]), Pc[1]);

		for(int y = yMin; y <= yMax; y++)
		{
			for(int x = xMin; x <= xMax; x++)
			{
				float alpha = ComputeAlpha(Pa[0], Pa[1], Pb[0], Pb[1], Pc[0], Pc[1], x, y);
				float beta = ComputeBeta(Pa[0], Pa[1], Pb[0], Pb[1], Pc[0], Pc[1], x, y);
				float gamma = ComputeGamma(Pa[0], Pa[1], Pb[0], Pb[1], Pc[0], Pc[1], x, y);
				if(alpha >= 0 && alpha <= 1 && beta >= 0 && beta <= 1 && gamma >= 0 && gamma <= 1)
				{
					Vector3f pos = alpha * Va_ndc + beta * Vb_ndc + gamma * Vc_ndc;
					if(pos[0] > -1 && pos[0] < 1 && pos[1] > -1 && pos[1] < 1 && pos[2] > -1 && pos[2] < 1) // within crop cube
					{
						// check depth buffer
						float depth = pos[2];
						if(depth < scr.dBuffer[y][x])
						{
							scr.dBuffer[y][x] = depth;
							Vector3f v = alpha * Va_world + beta * Vb_world + gamma * Vc_world;
							Vector3f n = alpha * normA + beta * normB + gamma * normC;
							Vector3f color = Lighting(v, n, material, scn.lightData, scn.camPos);
							color = Vector3f(round(color[0] * 255), round(color[1] * 255), round(color[2] * 255));
							FillPixel(x, y, scr.pixels, color);
						}
					}
				}
			}
		}
	}

	void Wireframe(Vector4f Va, Vector4f Vb, Vector4f Vc, Scene::SceneData &scn, ScreenData &scr)
	{
		Vector3f Va_ndc = Scene::WorldtoNDC(scn.transformData, Va);
		Vector3f Vb_ndc = Scene::WorldtoNDC(scn.transformData, Vb);
		Vector3f Vc_ndc = Scene::WorldtoNDC(scn.transformData, Vc);
		Vector2f Pa = Scene::NDCtoScreen(scr.width, scr.height, Va_ndc);
		Vector2f Pb = Scene::NDCtoScreen(scr.width, scr.height, Vb_ndc);
		Vector2f Pc = Scene::NDCtoScreen(scr.width, scr.height, Vc_ndc);
		int w = scr.width;
		int h = scr.height;
		if(Pa[0] < w &&Pa[0] > 0 && Pa[1] < h &&Pa[1] > 0&& Pb[0] > 0&&Pb[0] < w && Pb[1] > 0 && Pb[1] < h && Pc[0] > 0&&Pc[0] < w && Pc[1] < h && Pc[1] > 0)  // within crop cube
		{
			//cout << "check" << endl;
			DrawLine(Pa, Pb, scr.pixels, Vector3f(255, 255, 0));
			DrawLine(Pb, Pc, scr.pixels, Vector3f(255, 255, 0));
			DrawLine(Pc, Pa, scr.pixels, Vector3f(255, 255, 0));
		}
	}

	void Render(int width, int height, int mode, Scene::SceneData &scn, ScreenData &scr)
	{
		for(int i = 0; i < scn.objWorld.size(); i++)
		{
			obj_world obj = scn.objWorld[i];
			for(int j = 0; j < obj.f.size(); j++)
			{	
				Vector3f thisFaceV = obj.f[j];
				Vector3f thisFaceN = obj.nf[j];
				int indexV0 = thisFaceV[0];
				int indexV1 = thisFaceV[1];
				int indexV2 = thisFaceV[2];
				Vector4f v0 = obj.v[indexV0];
				Vector4f v1 = obj.v[indexV1];
				Vector4f v2 = obj.v[indexV2];
				int indexN0 = thisFaceN[0];
				int indexN1 = thisFaceN[1];
				int indexN2 = thisFaceN[2];
				Vector4f n0 = obj.n[indexN0];
				Vector4f n1 = obj.n[indexN1];
				Vector4f n2 = obj.n[indexN2];

				if(mode == 0)
					PhongShading(v0, v1, v2, n0, n1, n2, obj.material, scn, scr);
				else if(mode == 1)
					GouraudShading(v0, v1, v2, n0, n1, n2, obj.material, scn, scr);
				else if(mode == 2)
					FlatShading(v0, v1, v2, n0, n1, n2, obj.material, scn, scr);
				else if(mode == 3)
					Wireframe(v0, v1, v2, scn, scr);
			}
		}
	}
}