#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <cmath>

#include "Scene.h"
#include "Camera.h"
#include "Color.h"
#include "Mesh.h"
#include "Rotation.h"
#include "Scaling.h"
#include "Translation.h"
#include "Triangle.h"
#include "Vec3.h"
#include "tinyxml2.h"
#include "Helpers.h"

using namespace tinyxml2;
using namespace std;

/*
	Transformations, clipping, culling, rasterization are done here.
	You may define helper functions.
*/
Matrix4 calculateCameraMatrix(Camera *camera)
{
    double translation[4][4] = {
		{1, 0, 0, -(camera->pos.x)},
        {0, 1, 0, -(camera->pos.y)},
        {0, 0, 1, -(camera->pos.z)},
        {0, 0, 0, 1}
	};
    double rotation[4][4] = {
		{camera->u.x, camera->u.y, camera->u.z, 0},
    	{camera->v.x, camera->v.y, camera->v.z, 0},
        {camera->w.x, camera->w.y, camera->w.z, 0},
    	{0, 0, 0, 1}
	};
    return multiplyMatrixWithMatrix(Matrix4(rotation), Matrix4(translation));
}
Matrix4 calculateProjectionMatrix(Camera *camera)
{
    if (camera->projectionType) { //perspective
        double perspective[4][4] = {
			{(2*camera->near) / (camera->right - camera->left), 0, (camera->right + camera->left) / (camera->right - camera->left), 0},
        	{0, (2*camera->near) / (camera->top - camera->bottom), (camera->top + camera->bottom) / (camera->top - camera->bottom), 0},
            {0, 0, -((camera->far + camera->near) / (camera->far - camera->near)), -((2*camera->far*camera->near) / (camera->far - camera->near))},
            {0, 0, -1, 0}
		};
        return Matrix4(perspective);
    }
    else { //orthogonal
        double orthogonal[4][4] = {
			{2/(camera->right - camera->left), 0, 0, -((camera->right + camera->left) / (camera->right - camera->left))},
            {0, 2/(camera->top - camera->bottom), 0, -((camera->top + camera->bottom) / (camera->top - camera->bottom))},
            {0, 0, -(2/(camera->far - camera->near)), -((camera->far + camera->near) / (camera->far - camera->near))},
            {0, 0, 0, 1}
		};
        return Matrix4(orthogonal);
    }
}
Matrix4 calculateViewportMatrix(Camera *camera)
{
	double viewport[4][4] = {
		{camera->horRes/2.0, 0, 0, (camera->horRes-1)/2.0},
    	{0, camera->verRes/2.0, 0, (camera->verRes-1)/2.0},
        {0, 0, 0.5, 0.5},
        {0, 0, 0, 1}
	};
    return Matrix4(viewport);
}
Matrix4 Scene::calculateTransformMatrix(Camera * camera, Mesh* mesh)
{
    Matrix4 result = getIdentityMatrix();
    for (int i = 0; i < mesh->numberOfTransformations; i++) {
        if (mesh->transformationTypes[i] == 't') {
            Translation * t = translations[mesh->transformationIds[i]-1];
            double translation[4][4] = {
				{1,0,0,t->tx},
				{0,1,0,t->ty},
				{0,0,1,t->tz},
				{0,0,0,1}
			};
            result = multiplyMatrixWithMatrix(Matrix4(translation), result);
        }
        else if (mesh->transformationTypes[i] == 's') {
            Scaling * s = scalings[mesh->transformationIds[i]-1];
            double scaling[4][4] = {
				{s->sx,0,0,0},
                {0,s->sy,0,0},
				{0,0,s->sz,0},
				{0,0,0,1}
			};
            result = multiplyMatrixWithMatrix(scaling, result);
        }
        else if (mesh->transformationTypes[i] == 'r') {
            Rotation * r = rotations[mesh->transformationIds[i]-1];
            // Set minimum base to 0, swap other two and negate one of them 
			Vec3 u = Vec3(r->ux, r->uy, r->uz, 1);
			Vec3 v;
			Vec3 w;
            double minBase = min(min(ABS(r->ux), ABS(r->uy)), ABS(r->uz));
            if (minBase == ABS(r->ux))
                v = Vec3(0, -r->uz, r->uy, 1);
            else if (minBase == ABS(r->uy))
                v = Vec3(-r->uz, 0, r->ux, 1);
            else if (minBase == ABS(r->uz))
                v = Vec3(-r->uy, r->ux, 0, 1);
            w = crossProductVec3(u, v);
            v = normalizeVec3(v);
            w = normalizeVec3(w);
            double M[4][4] = {
				{u.x,u.y,u.z,0},
				{v.x,v.y,v.z,0},
				{w.x,w.y,w.z,0},
				{0,0,0,1}
			};
            double M_T[4][4] = {		
				{u.x,v.x,w.x,0},
				{u.y,v.y,w.y,0},
				{u.z,v.z,w.z,0},
				{0,0,0,1}
			};
            double rotateM[4][4] = {
				{1,0,0,0},
                {0,cos(r->angle * M_PI/180),-sin(r->angle * M_PI/180),0},
                {0,sin(r->angle * M_PI/180),cos(r->angle * M_PI/180),0},
                {0,0,0,1}
			};
            Matrix4 rot_w2c = multiplyMatrixWithMatrix(Matrix4(rotateM), Matrix4(M));
            Matrix4 c2w = multiplyMatrixWithMatrix(Matrix4(M_T), rot_w2c);
            result = multiplyMatrixWithMatrix(c2w, result);
        }
    }
    return result;
}



Vec4 perspectiveDivide(Vec4 v)
{
	return Vec4(v.x/v.t, v.y/v.t, v.z/v.t, 1, v.colorId);
}
bool visible(double den, double num, double &tE, double &tL)
{
	double t = num / den;
    if (den > 0) {
        if (t > tL)
            return false;
        else if (t > tE)
            tE = t;
    }
    else if (den < 0) {
        if (t < tE)
            return false;
        else if (t < tL)
            tL = t;
    }
    else if (num > 0) {
        return false;
    }
    return true;
}
bool liangBarksy(Color &c0, Vec4 &v0, Color &c1, Vec4 &v1)
{	
	double dx = v1.x - v0.x;
	double dy = v1.y - v0.y;
	double dz = v1.z - v0.z;
	
	double tE = 0, tL = 1;
	
	bool visibility=false;

	if(visible(dx,-v0.x-1,tE,tL))
	if(visible(-dx,v0.x-1,tE,tL))
	if(visible(dy,-v0.y-1,tE,tL))
	if(visible(-dy,v0.y-1,tE,tL))
	if(visible(dz,-v0.z-1,tE,tL))
	if(visible(-dz,v0.z-1,tE,tL))
		visibility=true;

	if(visibility)
	{
		Color dc = c1 - c0;
        if (tL < 1) {
            v1.x = v0.x + (dx * tL);
            v1.y = v0.y + (dy * tL);
            v1.z = v0.z + (dz * tL);
            c1 = c0 + (dc * tL);
        }
        if (tE > 0) {
            v0.x = v0.x + (dx * tE);
            v0.y = v0.y + (dy * tE);
            v0.z = v0.z + (dz * tE);
            c0 = c0 + (dc * tE);
        }
	}
	return visibility;
}
void lineRasterization(vector< vector<Color> > &image, Color c0, Vec4 v0, Color c1, Vec4 v1)
{
    double dx = v1.x - v0.x;
    double dy = v1.y - v0.y;
	// There are two main cases first is m E [-1,1] and second is m E [1,inf]
	// In both cases, there are 2 cases of points in the viewport screen if we call the point has smaller x as x0
	// case 1:
				// x1
		// x0
		// case1.1 
			// x0	// x1
	// case 2:
		// x0
				// x1
		//case 2.1
			// x0
			// x1

	// Swap the values to make v0 has smaller x
	if(ABS(dy) <= ABS(dx))
	{
		if(v1.x < v0.x)
		{
			swap(v0, v1);
			swap(c0, c1);
		}
		int y0 = v0.y, y1 = v1.y;
		int x0 = v0.x, x1 = v1.x;
		// case 1:
		if(y0 <= y1)
		{
			int y = y0;
			int d = (y0 - y1) + 0.5 * (x1 - x0);
			Color c = c0;
			Color dc = (c1 - c0) / (x1 - x0);
			for(int x=x0; x<=x1; x++)	// increment x as iterator
			{
				image[x][y] = c.round_color();
				if(d < 0)
				{
					y++;							// increment y if the condition holds
					d += (y0 - y1) + (x1 - x0);
				}
				else 
					d += y0 - y1;
				c = c + dc;
			}
		}
		// case 2:
		else
		{
			int y = y0;
			int d = (y0 - y1) - 0.5 * (x1 - x0);
			Color c = c0;
			Color dc = (c1-c0) / (x1 - x0);
			for(int x=x0; x<=x1; x++)
			{
				image[x][y] = c.round_color();
				if(d > 0)
				{
					y--;		// decrement y
					d += (y0 - y1) - (x1 - x0);
				}
				else
					d += y0 - y1;
				c = c + dc;
			}
		}
	}
	// In slope > 1 case, we need to swap the values of x and y
	else if(ABS(dy) > ABS(dx))
	{
		if(v1.y < v0.y)
		{
			swap(v0, v1);
			swap(c0, c1);
		}
		int y0 = v0.y, y1 = v1.y;
		int x0 = v0.x, x1 = v1.x;
		// case 1:
		if(x0 < x1)
		{
			int x = x0;
			int d = (x0 - x1) + 0.5 * (y1 - y0);
			Color c = c0;
			Color dc = (c1 - c0) / (y1 - y0);
			for(int y=y0; y<=y1; y++)	// increment x as iterator
			{
				image[x][y] = c.round_color();
				if(d < 0)
				{
					x++;							// increment x if the condition holds
					d += (x0 - x1) + (y1 - y0);
				}
				else 
					d += x0 - x1;
				c = c + dc;
			}
		}
		// case 2:
		else
		{
			int x = x0;
			int d = (x0 - x1) - 0.5 * (y1 - y0);
			Color c = c0;
			Color dc = (c1-c0) / (y1 - y0);
			for(int y=y0; y<=y1; y++)
			{
				image[x][y] = c.round_color();
				if(d >= 0)
				{
					x--;		// decrement x
					d += (x0 - x1) - (y1 - y0);
				}
				else
					d += x0 - x1;
				c = c + dc;
			}
		}
	}

}
double f_func(int x, int y, int x0, int y0, int x1, int y1)
{
	return x * (y0 - y1) + y*(x1 - x0) + x0*y1 - x1*y0;
}
void triangleRasterization(vector< vector<Color> > &image, Camera *camera, Vec4 &v0, Vec4 &v1, Vec4 &v2, Color &c0, Color &c1, Color &c2)
{
	int xMin = min(v0.x, min(v1.x, v2.x));
        xMin = xMin < 0 ? 0:xMin;
//	    xMin = xMin <= camera->horRes-1 ? xMin:camera->horRes-1;

	int xMax = max(v0.x, max(v1.x, v2.x));
        xMax = xMax > camera->horRes-1 ? camera->horRes-1:xMax;
//        xMax = xMax < 0 ? 0:xMax;

	int yMin = min(v0.y, min(v1.y, v2.y));
        yMin = yMin < 0 ? 0:yMin;
//	    yMin = yMin <= camera->verRes-1 ? yMin:camera->verRes-1;

	int yMax = max(v0.y, max(v1.y, v2.y));
        yMax = yMax > camera->verRes-1 ? camera->verRes-1:yMax;
//        yMax = yMax < 0 ? 0:yMax;

	double a, b, g;
	for(int y=yMin; y<yMax; y++)
	{
		for(int x=xMin; x<xMax; x++)
		{
			a = f_func(x, y, v1.x, v1.y, v2.x, v2.y) / f_func(v0.x, v0.y, v1.x, v1.y, v2.x, v2.y);	// f_12(x,y) / f_12(x0, y0)
			b = f_func(x, y, v2.x, v2.y, v0.x, v0.y) / f_func(v1.x, v1.y, v2.x, v2.y, v0.x, v0.y);	// f_20(x,y) / f_20(x1, y1)
			g = f_func(x, y, v0.x, v0.y, v1.x, v1.y) / f_func(v2.x, v2.y, v0.x, v0.y, v1.x, v1.y);	// f_01(x,y) / f_01(x2, y2)
			if(a>=0 && b>=0 && g>=0)
			{
				Color c = c0*a + c1*b + c2*g;
				image[x][y] = c.round_color();
			}
		}
	}
}
void Scene::forwardRenderingPipeline(Camera *camera)
{
    Matrix4 camera_matrix = calculateCameraMatrix(camera);
    Matrix4 projection_matrix = calculateProjectionMatrix(camera);
    Matrix4 viewport_matrix = calculateViewportMatrix(camera);

	Matrix4 proj_cam = multiplyMatrixWithMatrix(projection_matrix, camera_matrix);

	for (int i = 0; i<meshes.size(); i++)
	{
		Mesh* curmesh = meshes[i];
		bool wireframe = !curmesh->type;		// true for wireframe, false for solid

		Matrix4 transform_matrix = calculateTransformMatrix(camera, curmesh);
		Matrix4 proj_cam_trans = multiplyMatrixWithMatrix(proj_cam, transform_matrix);
		for(int j = 0; j<curmesh->triangles.size(); j++)
		{
			Triangle *curtriangle = &curmesh->triangles[j];
			// Vertices of triangle
            Vec3 *first = vertices[curtriangle->getFirstVertexId()-1];
            Vec3 *second = vertices[curtriangle->getSecondVertexId()-1];
            Vec3 *third = vertices[curtriangle->getThirdVertexId()-1];

			Vec4 v0 = multiplyMatrixWithVec4(proj_cam_trans, Vec4(first->x, first->y, first->z, 1, first->colorId));
			Vec4 v1 = multiplyMatrixWithVec4(proj_cam_trans, Vec4(second->x, second->y, second->z, 1, second->colorId));
			Vec4 v2 = multiplyMatrixWithVec4(proj_cam_trans, Vec4(third->x, third->y, third->z, 1, third->colorId));
			// Multiplying up to perspective divide is finished
			// check if backface culling enabled
			bool bfc = 	
				dotProductVec3(
					normalizeVec3(
						crossProductVec3(
							subtractVec3(Vec3(v1.x, v1.y, v1.z, v1.colorId), Vec3(v0.x, v0.y, v0.z, v0.colorId)),
							subtractVec3(Vec3(v2.x, v2.y, v2.z, v2.colorId), Vec3(v0.x, v0.y, v0.z, v0.colorId))
						)
					),
					Vec3(v0.x, v0.y, v0.z, v0.colorId)
				) < 0;
			// projcam * first ..
			if (cullingEnabled && bfc) //kardeş burası da tamam
			{
				continue;
			}
			//extract colors
			Color color_v0 = *colorsOfVertices[v0.colorId-1];
            Color color_v1 = *colorsOfVertices[v1.colorId-1];
            Color color_v2 = *colorsOfVertices[v2.colorId-1];
			// clipping
			if(wireframe)
			{
				// perspective divide
				Vec4 v0_p = perspectiveDivide(v0);
				Vec4 v1_p = perspectiveDivide(v1);
				Vec4 v2_p = perspectiveDivide(v2);

				//cloning vec4 and colors
				Vec4 v0_p_2 = Vec4(v0_p);
				Vec4 v1_p_2 = Vec4(v1_p);
				Vec4 v2_p_2 = Vec4(v2_p);
				Color color_v0_2 = Color(color_v0);
				Color color_v1_2 = Color(color_v1);
				Color color_v2_2 = Color(color_v2);
				
				//clipping
				bool line_0_1 = liangBarksy(color_v0, v0_p, color_v1, v1_p);
				bool line_1_2 = liangBarksy(color_v1_2, v1_p_2, color_v2, v2_p);
				bool line_2_0 = liangBarksy(color_v2_2, v2_p_2, color_v0_2, v0_p_2);

				//viewport multiplication
				Vec4 v0_vp = multiplyMatrixWithVec4(viewport_matrix, v0_p);
				Vec4 v1_vp = multiplyMatrixWithVec4(viewport_matrix, v1_p);
				Vec4 v2_vp = multiplyMatrixWithVec4(viewport_matrix, v2_p);
				Vec4 v0_vp_2 = multiplyMatrixWithVec4(viewport_matrix, v0_p_2);
				Vec4 v1_vp_2 = multiplyMatrixWithVec4(viewport_matrix, v1_p_2);
				Vec4 v2_vp_2 = multiplyMatrixWithVec4(viewport_matrix, v2_p_2);

				//rasterize line
				if(line_0_1)
					lineRasterization(image, color_v0, v0_vp, color_v1, v1_vp);		// Rasterize through v0 to v1
				if(line_1_2)
					lineRasterization(image, color_v1_2, v1_vp_2, color_v2, v2_vp);		// Rasterize through v1 to v2
				if(line_2_0)
					lineRasterization(image, color_v2_2, v2_vp_2, color_v0_2, v0_vp_2);		// Rasterize through v2 to v0
			}
			else //solid
			{
				// perspective divide and viewport
				Vec4 v0_vp = multiplyMatrixWithVec4(viewport_matrix, perspectiveDivide(v0));
				Vec4 v1_vp = multiplyMatrixWithVec4(viewport_matrix, perspectiveDivide(v1));
				Vec4 v2_vp = multiplyMatrixWithVec4(viewport_matrix, perspectiveDivide(v2));

//                v0_vp.x = v0_vp.x > camera->horRes-1 ? camera->horRes-1 : v0_vp.x;
//                v0_vp.y = v0_vp.y > camera->verRes-1 ? camera->verRes-1 : v0_vp.y;
//                v0_vp.x = v0_vp.x < 0 ? 0 : v0_vp.x;
//                v0_vp.y = v0_vp.y < 0 ? 0 : v0_vp.y;
//
//                v1_vp.x = v1_vp.x > camera->horRes-1 ? camera->horRes-1 : v1_vp.x;
//                v1_vp.y = v1_vp.y > camera->verRes-1 ? camera->verRes-1 : v1_vp.y;
//                v1_vp.x = v1_vp.x < 0 ? 0 : v1_vp.x;
//                v1_vp.y = v1_vp.y < 0 ? 0 : v1_vp.y;
//
//                v2_vp.x = v2_vp.x > camera->horRes-1 ? camera->horRes-1 : v2_vp.x;
//                v2_vp.y = v2_vp.y > camera->verRes-1 ? camera->verRes-1 : v2_vp.y;
//                v2_vp.x = v2_vp.x < 0 ? 0 : v2_vp.x;
//                v2_vp.y = v2_vp.y < 0 ? 0 : v2_vp.y;
                triangleRasterization(image, camera, v0_vp, v1_vp, v2_vp, color_v0, color_v1, color_v2);
			}
		}
	}
}

/*
	Parses XML file
*/
Scene::Scene(const char *xmlPath)
{
	const char *str;
	XMLDocument xmlDoc;
	XMLElement *pElement;

	xmlDoc.LoadFile(xmlPath);

	XMLNode *pRoot = xmlDoc.FirstChild();

	// read background color
	pElement = pRoot->FirstChildElement("BackgroundColor");
	str = pElement->GetText();
	sscanf(str, "%lf %lf %lf", &backgroundColor.r, &backgroundColor.g, &backgroundColor.b);

	// read culling
	pElement = pRoot->FirstChildElement("Culling");
	if (pElement != NULL) {
		str = pElement->GetText();
		
		if (strcmp(str, "enabled") == 0) {
			cullingEnabled = true;
		}
		else {
			cullingEnabled = false;
		}
	}

	// read cameras
	pElement = pRoot->FirstChildElement("Cameras");
	XMLElement *pCamera = pElement->FirstChildElement("Camera");
	XMLElement *camElement;
	while (pCamera != NULL)
	{
		Camera *cam = new Camera();

		pCamera->QueryIntAttribute("id", &cam->cameraId);

		// read projection type
		str = pCamera->Attribute("type");

		if (strcmp(str, "orthographic") == 0) {
			cam->projectionType = 0;
		}
		else {
			cam->projectionType = 1;
		}

		camElement = pCamera->FirstChildElement("Position");
		str = camElement->GetText();
		sscanf(str, "%lf %lf %lf", &cam->pos.x, &cam->pos.y, &cam->pos.z);

		camElement = pCamera->FirstChildElement("Gaze");
		str = camElement->GetText();
		sscanf(str, "%lf %lf %lf", &cam->gaze.x, &cam->gaze.y, &cam->gaze.z);

		camElement = pCamera->FirstChildElement("Up");
		str = camElement->GetText();
		sscanf(str, "%lf %lf %lf", &cam->v.x, &cam->v.y, &cam->v.z);

		cam->gaze = normalizeVec3(cam->gaze);
		cam->u = crossProductVec3(cam->gaze, cam->v);
		cam->u = normalizeVec3(cam->u);

		cam->w = inverseVec3(cam->gaze);
		cam->v = crossProductVec3(cam->u, cam->gaze);
		cam->v = normalizeVec3(cam->v);

		camElement = pCamera->FirstChildElement("ImagePlane");
		str = camElement->GetText();
		sscanf(str, "%lf %lf %lf %lf %lf %lf %d %d",
			   &cam->left, &cam->right, &cam->bottom, &cam->top,
			   &cam->near, &cam->far, &cam->horRes, &cam->verRes);

		camElement = pCamera->FirstChildElement("OutputName");
		str = camElement->GetText();
		cam->outputFileName = string(str);

		cameras.push_back(cam);

		pCamera = pCamera->NextSiblingElement("Camera");
	}

	// read vertices
	pElement = pRoot->FirstChildElement("Vertices");
	XMLElement *pVertex = pElement->FirstChildElement("Vertex");
	int vertexId = 1;

	while (pVertex != NULL)
	{
		Vec3 *vertex = new Vec3();
		Color *color = new Color();

		vertex->colorId = vertexId;

		str = pVertex->Attribute("position");
		sscanf(str, "%lf %lf %lf", &vertex->x, &vertex->y, &vertex->z);

		str = pVertex->Attribute("color");
		sscanf(str, "%lf %lf %lf", &color->r, &color->g, &color->b);

		vertices.push_back(vertex);
		colorsOfVertices.push_back(color);

		pVertex = pVertex->NextSiblingElement("Vertex");

		vertexId++;
	}

	// read translations
	pElement = pRoot->FirstChildElement("Translations");
	XMLElement *pTranslation = pElement->FirstChildElement("Translation");
	while (pTranslation != NULL)
	{
		Translation *translation = new Translation();

		pTranslation->QueryIntAttribute("id", &translation->translationId);

		str = pTranslation->Attribute("value");
		sscanf(str, "%lf %lf %lf", &translation->tx, &translation->ty, &translation->tz);

		translations.push_back(translation);

		pTranslation = pTranslation->NextSiblingElement("Translation");
	}

	// read scalings
	pElement = pRoot->FirstChildElement("Scalings");
	XMLElement *pScaling = pElement->FirstChildElement("Scaling");
	while (pScaling != NULL)
	{
		Scaling *scaling = new Scaling();

		pScaling->QueryIntAttribute("id", &scaling->scalingId);
		str = pScaling->Attribute("value");
		sscanf(str, "%lf %lf %lf", &scaling->sx, &scaling->sy, &scaling->sz);

		scalings.push_back(scaling);

		pScaling = pScaling->NextSiblingElement("Scaling");
	}

	// read rotations
	pElement = pRoot->FirstChildElement("Rotations");
	XMLElement *pRotation = pElement->FirstChildElement("Rotation");
	while (pRotation != NULL)
	{
		Rotation *rotation = new Rotation();

		pRotation->QueryIntAttribute("id", &rotation->rotationId);
		str = pRotation->Attribute("value");
		sscanf(str, "%lf %lf %lf %lf", &rotation->angle, &rotation->ux, &rotation->uy, &rotation->uz);

		rotations.push_back(rotation);

		pRotation = pRotation->NextSiblingElement("Rotation");
	}

	// read meshes
	pElement = pRoot->FirstChildElement("Meshes");

	XMLElement *pMesh = pElement->FirstChildElement("Mesh");
	XMLElement *meshElement;
	while (pMesh != NULL)
	{
		Mesh *mesh = new Mesh();

		pMesh->QueryIntAttribute("id", &mesh->meshId);

		// read projection type
		str = pMesh->Attribute("type");

		if (strcmp(str, "wireframe") == 0) {
			mesh->type = 0;
		}
		else {
			mesh->type = 1;
		}

		// read mesh transformations
		XMLElement *pTransformations = pMesh->FirstChildElement("Transformations");
		XMLElement *pTransformation = pTransformations->FirstChildElement("Transformation");

		while (pTransformation != NULL)
		{
			char transformationType;
			int transformationId;

			str = pTransformation->GetText();
			sscanf(str, "%c %d", &transformationType, &transformationId);

			mesh->transformationTypes.push_back(transformationType);
			mesh->transformationIds.push_back(transformationId);

			pTransformation = pTransformation->NextSiblingElement("Transformation");
		}

		mesh->numberOfTransformations = mesh->transformationIds.size();

		// read mesh faces
		char *row;
		char *clone_str;
		int v1, v2, v3;
		XMLElement *pFaces = pMesh->FirstChildElement("Faces");
        str = pFaces->GetText();
		clone_str = strdup(str);

		row = strtok(clone_str, "\n");
		while (row != NULL)
		{
			int result = sscanf(row, "%d %d %d", &v1, &v2, &v3);
			
			if (result != EOF) {
				mesh->triangles.push_back(Triangle(v1, v2, v3));
			}
			row = strtok(NULL, "\n");
		}
		mesh->numberOfTriangles = mesh->triangles.size();
		meshes.push_back(mesh);

		pMesh = pMesh->NextSiblingElement("Mesh");
	}
}

/*
	Initializes image with background color
*/
void Scene::initializeImage(Camera *camera)
{
	if (this->image.empty())
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			vector<Color> rowOfColors;

			for (int j = 0; j < camera->verRes; j++)
			{
				rowOfColors.push_back(this->backgroundColor);
			}

			this->image.push_back(rowOfColors);
		}
	}
	else
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			for (int j = 0; j < camera->verRes; j++)
			{
				this->image[i][j].r = this->backgroundColor.r;
				this->image[i][j].g = this->backgroundColor.g;
				this->image[i][j].b = this->backgroundColor.b;
			}
		}
	}
}

/*
	If given value is less than 0, converts value to 0.
	If given value is more than 255, converts value to 255.
	Otherwise returns value itself.
*/
int Scene::makeBetweenZeroAnd255(double value)
{
	if (value >= 255.0)
		return 255;
	if (value <= 0.0)
		return 0;
	return (int)(value);
}

/*
	Writes contents of image (Color**) into a PPM file.
*/
void Scene::writeImageToPPMFile(Camera *camera)
{
	ofstream fout;

	fout.open(camera->outputFileName.c_str());

	fout << "P3" << endl;
	fout << "# " << camera->outputFileName << endl;
	fout << camera->horRes << " " << camera->verRes << endl;
	fout << "255" << endl;

	for (int j = camera->verRes - 1; j >= 0; j--)
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			fout << makeBetweenZeroAnd255(this->image[i][j].r) << " "
				 << makeBetweenZeroAnd255(this->image[i][j].g) << " "
				 << makeBetweenZeroAnd255(this->image[i][j].b) << " ";
		}
		fout << endl;
	}
	fout.close();
}

/*
	Converts PPM image in given path to PNG file, by calling ImageMagick's 'convert' command.
	os_type == 1 		-> Ubuntu
	os_type == 2 		-> Windows
	os_type == other	-> No conversion
*/
void Scene::convertPPMToPNG(string ppmFileName, int osType)
{
	string command;

	// call command on Ubuntu
	if (osType == 1)
	{
		command = "convert " + ppmFileName + " " + ppmFileName + ".png";
		system(command.c_str());
	}

	// call command on Windows
	else if (osType == 2)
	{
		command = "magick convert " + ppmFileName + " " + ppmFileName + ".png";
		system(command.c_str());
	}

	// default action - don't do conversion
	else
	{
	}
}