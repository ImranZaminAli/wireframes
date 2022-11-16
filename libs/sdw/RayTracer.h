#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "RayTriangleIntersection.h"
#include "DrawingWindow.h"
#include "ModelTriangle.h"
#include "Camera.h"
#include "CanvasPoint.h"
#define SCALE 500
class RayTracer {
	int width;
	int height;
	uint32_t black;
	std::vector<ModelTriangle>* triangles;
	Camera* camera;
	DrawingWindow* window;
	glm::vec3 getPossibleSolution(int index, glm::vec3 rayDir, glm::vec3 startPosition);
	//void getIntersection(glm::vec3& rayDir, RayTriangleIntersection& rayData, ModelTriangle& triangle);
	void getClosestIntersection(glm::vec3 rayDir, RayTriangleIntersection& rayData, glm::vec3 startPosition, bool first);
	bool checkValid(float u, float v, float t);
	void trace(CanvasPoint& point);
	glm::vec3 getRayDirection(CanvasPoint& point);
public:
	RayTracer(int windowWidth, int windowHeight);

	void drawRayTracedImage(DrawingWindow* window, std::vector<ModelTriangle>* triangles, Camera* camera);
};