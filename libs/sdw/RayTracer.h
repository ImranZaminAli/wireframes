#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "RayTriangleIntersection.h"
#include "DrawingWindow.h"
#include "ModelTriangle.h"
#include "Camera.h"
#include "CanvasPoint.h"
#define SCALE 400
class RayTracer {
	int width;
	int height;
	std::vector<ModelTriangle>* triangles;
	Camera* camera;
	DrawingWindow* window;
	void getIntersection(glm::vec3& rayDir, RayTriangleIntersection& rayData, ModelTriangle& triangle);
	void trace(CanvasPoint& point);
	glm::vec3 getRayDirection(CanvasPoint& point);
public:
	RayTracer(int windowWidth, int windowHeight);

	void drawRayTracedImage(DrawingWindow* window, std::vector<ModelTriangle>* triangles, Camera* camera);
};