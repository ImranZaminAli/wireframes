                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   #pragma once
#include <glm/glm.hpp>
#include <vector>
#include "RayTriangleIntersection.h"
#include "DrawingWindow.h"
#include "ModelTriangle.h"
#include "Camera.h"
#include "CanvasPoint.h"
#include "TextureMap.h"

#define SCALE 700
class RayTracer {
	int width;
	int height;
	int maxBounces;
	uint32_t black;
	glm::vec3 lightPoint;
	//int lights;
	std::vector<ModelTriangle>* triangles;
	float sourceStrength;
	Camera* camera;
	DrawingWindow* window;
	glm::vec3 getPossibleSolution(int index, glm::vec3 rayDir, glm::vec3 startPosition);
	//glm::vec3 getPossibleSolution(int index, glm::vec3 rayDir, glm::vec3 startPosition);
	//void getIntersection(glm::vec3& rayDir, RayTriangleIntersection& rayData, ModelTriangle& triangle);
	void getClosestIntersection(glm::vec3 rayDir, RayTriangleIntersection& rayData, glm::vec3& pointNormal, glm::vec3 startPosition, bool first);
	bool checkValid(float u, float v, float t);
	glm::vec3 calculateIntersection(ModelTriangle triangle, glm::vec3& pointNormal, float u, float v);
	std::pair<Colour, float> trace(glm::vec3& rayDir, glm::vec3 start, glm::vec3 lightPos, int bounce);
	float calculateIntensity(float distance, glm::vec3 cameraRay, glm::vec3 shadowRay, glm::vec3 viewRay, ModelTriangle& tri, glm::vec3 pointNormal);
	TextureMap textureMap;
	glm::vec3 getRayDirection(CanvasPoint& point);
	glm::vec3 getReflectedRay(glm::vec3& incidentRay, glm::vec3& normal);
public:
	RayTracer(int windowWidth, int windowHeight);

	void drawRayTracedImage(DrawingWindow* window, std::vector<ModelTriangle>* triangles, Camera* camera);
};
