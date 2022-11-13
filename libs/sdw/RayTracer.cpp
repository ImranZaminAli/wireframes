#include "RayTracer.h"

RayTracer::RayTracer(int windowWidth, int windowHeight) {
	width = windowWidth;
	height = windowHeight;
	std::vector<ModelTriangle> tri = std::vector<ModelTriangle>();
	

}

glm::vec3 RayTracer::getRayDirection(CanvasPoint& point) {
	glm::vec3 rayDir;
	float x, y, z;
	z = -(*camera).focalLength;

	x = ((point.x - (width / 2)) / SCALE) * -z;
	y = ((point.y - (height / 2)) / SCALE) * z;

	rayDir = glm::vec3(x, y, z) * glm::inverse((*camera).orientation);
	return rayDir;
}

void RayTracer::getIntersection(glm::vec3& rayDir, RayTriangleIntersection& rayData, ModelTriangle& triangle) {
	glm::vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
	glm::vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
	glm::vec3 SPVector = (*camera).position - triangle.vertices[0];
	glm::mat3 DEMatrix(-rayDir, e0, e1);
	glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;

	bool checkU, checkV, checkSum;
	checkU = (possibleSolution[1] >= 0.0) && (possibleSolution[1] <= 1.0);
	checkV = (possibleSolution[2] >= 0.0) && (possibleSolution[2] <= 1.0);
	checkSum = possibleSolution[1] + possibleSolution[2] <= 1.0;

	

	if (!(checkU && checkV && checkSum))
		rayData.distanceFromCamera = INFINITY;
	else
		rayData.distanceFromCamera = possibleSolution[0];

}

void RayTracer::trace(CanvasPoint& point) {
	RayTriangleIntersection closestRay = RayTriangleIntersection::RayTriangleIntersection();
	closestRay.distanceFromCamera = INFINITY;
	RayTriangleIntersection rayData;
	for (int i = 0; i < (*triangles).size(); i++) {
		//std::cout << i << std::endl;
		glm::vec3 rayDir = getRayDirection(point);
		rayData = RayTriangleIntersection(glm::vec3(point.x, point.y, point.depth), INFINITY, (*triangles)[i], i);
		getIntersection(rayDir, rayData, (*triangles)[i]);
		if (rayData.distanceFromCamera < closestRay.distanceFromCamera)
			closestRay = rayData;
	}

	if (closestRay.distanceFromCamera == INFINITY)
		(*window).setPixelColour(point.x, point.y, 0xFF000000);
	else
		(*window).setPixelColour(point.x, point.y, closestRay.intersectedTriangle.colour.getArbg());

	
}

void RayTracer::drawRayTracedImage(DrawingWindow* window, std::vector<ModelTriangle>* triangles, Camera* camera) {
	this->window = window;
	this->triangles = triangles;
	this->camera = camera;
	
	
	for (int i = 0; i < height; i++){
		for (int j = 0; j < width; j++) {
			trace(CanvasPoint(j,i, (*camera).focalLength));
		}
	}
}