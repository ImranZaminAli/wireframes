#include "RayTracer.h"

RayTracer::RayTracer(int windowWidth, int windowHeight) {
	width = windowWidth;
	height = windowHeight;
	lightPoint = glm::vec3(0, 0.4, 0.2f);
	//lightPoint = glm::vec3(0, 0.4, 0);
	black = 0xFF000000;
	sourceStrength = 50.0f;
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

glm::vec3 RayTracer::getPossibleSolution(int index, glm::vec3 rayDir, glm::vec3 startPosition) {
	
	glm::vec3 e0 = (*triangles)[index].vertices[1] - (*triangles)[index].vertices[0];
	glm::vec3 e1 = (*triangles)[index].vertices[2] - (*triangles)[index].vertices[0];
	glm::vec3 SPVector = startPosition - (*triangles)[index].vertices[0];
	glm::mat3 DEMatrix(-rayDir, e0, e1);
	glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
	return possibleSolution;
}

bool RayTracer::checkValid(float u, float v, float t) {
	bool checkU, checkV, checkSum;

	checkU = (u >= 0.0f) && (u <= 1.0f);
	checkV = (v >= 0.0f) && (v <= 1.0f);
	checkSum = u + v <= 1.0f;

	return checkU && checkV && checkSum && t > 0.001;
}

glm::vec3 calculateIntersection(ModelTriangle triangle, float u, float v) {
	glm::vec3 point = triangle.vertices[0];
	
	point += (triangle.vertices[1] - triangle.vertices[0]) * u;
	point += (triangle.vertices[2] - triangle.vertices[0]) * v;
	//point[2] *= -1;
	return point;
}

void RayTracer::getClosestIntersection(glm::vec3 rayDir, RayTriangleIntersection& rayData,glm::vec3 startPosition, bool first) {
	rayData.distanceFromCamera = INFINITY;
	
	for (int i = 0; i < triangles->size(); i++) {
		glm::vec3 possibleSolution = getPossibleSolution(i, rayDir, startPosition);
		bool valid = checkValid(possibleSolution[1], possibleSolution[2], possibleSolution[0]);
					
		if (valid && possibleSolution[0] < rayData.distanceFromCamera) {
			
			rayData.distanceFromCamera = possibleSolution[0];
			rayData.intersectedTriangle = (*triangles)[i];
			rayData.triangleIndex = i;
			

			rayData.intersectionPoint = calculateIntersection((*triangles)[i], possibleSolution[1], possibleSolution[2]);
		}
	}

	
}

float RayTracer::calculateIntensity(float distance, glm::vec3 rayDir, int triIndex){
	float dotProduct = glm::dot(rayDir, (*triangles)[triIndex].normal);
	//std::cout << dotProduct << std::endl;
    float intensity = (sourceStrength * dotProduct) / (4 * M_PI * distance * distance);
	
	return glm::clamp(intensity, 0.07f,1.0f);
}

float RayTracer::calculateIntensity(glm::vec3 cameraRay, glm::vec3 shadowRay, ModelTriangle& tri) {
	glm::vec3 reflectedRay = glm::normalize(shadowRay - 2.0f * tri.normal * glm::dot(shadowRay, tri.normal));
	float intensity = glm::dot(reflectedRay, cameraRay);
	return std::powf(intensity, 20.0f);
}

float RayTracer::calculateIntensity(float distance, glm::vec3 cameraRay, glm::vec3 shadowRay, ModelTriangle& tri) {
	glm::vec3 reflectedRay = glm::normalize(shadowRay - 2.0f * tri.normal * glm::dot(shadowRay, tri.normal));
	float spectralVal = std::powf(glm::dot(reflectedRay, cameraRay), 64);
	float incidenceVal = glm::dot(shadowRay, tri.normal);
	float intensity = (sourceStrength * incidenceVal ) / (4 * M_PI * distance * distance);

	return glm::clamp(intensity + spectralVal, 0.0f, 1.0f);
}

void RayTracer::trace(CanvasPoint& point) {
	RayTriangleIntersection rayData;
	glm::vec3 cameraRay = getRayDirection(point);
	getClosestIntersection(cameraRay, rayData, camera->position, true);
	//if (rayData.distanceFromCamera == INFINITY)
	//	window->setPixelColour(point.x, point.y, black);
	//else if (rayData.triangleIndex == 0 || rayData.triangleIndex == 1)
	//	window->setPixelColour(point.x, point.y, rayData.intersectedTriangle.colour.getArbg());
	//else {
	//	
	//	rayDir = glm::normalize(rayData.intersectionPoint - lightPoint);
	//	float dist = glm::distance(lightPoint, rayData.intersectionPoint);
	//	
	//	RayTriangleIntersection shadowData;
	//	getClosestIntersection(rayDir, shadowData, lightPoint, false);
	//			
	//	/*if (shadowData.distanceFromCamera < dist && rayData.triangleIndex != shadowData.triangleIndex) {
	//		window->setPixelColour(point.x, point.y, black);
	//	}
	//	else {
	//	    float intensity = calculateIntensity(rayData.distanceFromCamera, glm::normalize(rayDir), rayData.triangleIndex);
	//		window->setPixelColour(point.x, point.y, shadowData.intersectedTriangle.colour.getArbg(intensity));
	//	}*/
	//	
	//}

	if (rayData.distanceFromCamera == INFINITY) {
		window->setPixelColour(point.x, point.y, black);
	}
	else {
		glm::vec3 shadowRay = glm::normalize(rayData.intersectionPoint - lightPoint);
		float dist = glm::distance(lightPoint, rayData.intersectionPoint);
		//RayTriangleIntersection shadowData;
		//getClosestIntersection(rayDir, shadowData, lightPoint, false);
		//float intensity = calculateIntensity(rayData.distanceFromCamera, shadowRay, rayData.triangleIndex);
		//float intensity = calculateIntensity(glm::normalize(cameraRay), shadowRay, (*triangles)[rayData.triangleIndex]);
		float intensity = calculateIntensity(rayData.distanceFromCamera, cameraRay, shadowRay, (*triangles)[rayData.triangleIndex]);
		window->setPixelColour(point.x, point.y, rayData.intersectedTriangle.colour.getArbg(intensity));
	}
}

void RayTracer::drawRayTracedImage(DrawingWindow* window, std::vector<ModelTriangle>* triangles, Camera* camera) {
	this->window = window;
	this->triangles = triangles;
	this->camera = camera;
		
	for (int i = 0; i < height; i++){
		for (int j = 0; j < width; j++) {
		    CanvasPoint point = CanvasPoint(j,i, camera->focalLength);
			trace(point);
		}
	}

}
