#include "RayTracer.h"

RayTracer::RayTracer(int windowWidth, int windowHeight) {
	width = windowWidth;
	height = windowHeight;
	//lightPoint = glm::vec3(0, 0.1, 0.0);
	lightPoint = glm::vec3(0.0f, 0.4f, 0.0f);
	//lightPoint = glm::vec3(0, 0.4, 0);
	black = 0xFF000000;
	sourceStrength = 2.0f;
	textureMap = TextureMap("C:\\Users\\izami\\Documents\\UoBYr3\\Graphics\\wireframes\\texture.ppm");
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

	return checkU && checkV && checkSum && t > 0.0000001;
}

glm::vec3 RayTracer::calculateIntersection(ModelTriangle triangle, glm::vec3& pointNormal, float u, float v) {
	glm::vec3 point = triangle.vertices[0];
	
	point += (triangle.vertices[1] - triangle.vertices[0]) * u;
	point += (triangle.vertices[2] - triangle.vertices[0]) * v;
	pointNormal = triangle.vertexNormals[0];
	pointNormal += (triangle.vertexNormals[1] - triangle.vertexNormals[0]) * u;
	pointNormal += (triangle.vertexNormals[2] - triangle.vertexNormals[0]) * v;
	//pointNormal = glm::normalize(pointNormal);
	//point[2] *= -1;
	return point;
}

void RayTracer::getClosestIntersection(glm::vec3 rayDir, RayTriangleIntersection& rayData, glm::vec3& pointNormal, glm::vec3 startPosition, bool first) {
	rayData.distanceFromCamera = INFINITY;
	for (int i = 0; i < triangles->size(); i++) {
		glm::vec3 possibleSolution = getPossibleSolution(i, rayDir, startPosition);
		bool valid = checkValid(possibleSolution[1], possibleSolution[2], possibleSolution[0]);
					
		if (valid && possibleSolution[0] < rayData.distanceFromCamera) {
			
			rayData.distanceFromCamera = possibleSolution[0];
			rayData.intersectedTriangle = (*triangles)[i];
			rayData.triangleIndex = i;
			
			
			rayData.intersectionPoint = calculateIntersection((*triangles)[i], pointNormal, possibleSolution[1], possibleSolution[2]);
			rayData.u = possibleSolution[1];
			rayData.v = possibleSolution[2];
		}
	}
}

float RayTracer::calculateIntensity(float distance, glm::vec3 rayDir, ModelTriangle& tri){
	float dotProduct = glm::dot(rayDir, tri.normal);
    float intensity = (sourceStrength * dotProduct) / (4 * M_PI * distance * distance);
	
	return glm::clamp(intensity, 0.07f,1.0f);
}

float RayTracer::calculateIntensity(glm::vec3 shadowRay, glm::vec3 cameraRay, ModelTriangle& tri) {
	glm::vec3 reflectedRay = glm::normalize(shadowRay - 2.0f * tri.normal * glm::dot(shadowRay, tri.normal));
	float intensity = glm::dot(reflectedRay, cameraRay);
	return glm::clamp(std::powf(intensity, 256.0f), 0.0f, 1.0f);
}

float RayTracer::calculateIntensity(float distance, glm::vec3 cameraRay, glm::vec3 shadowRay, glm::vec3 viewRay, ModelTriangle& tri, glm::vec3 pointNormal) {
	glm::vec3 reflectedRay = glm::normalize(shadowRay - 2.0f * pointNormal * glm::dot(shadowRay, -pointNormal));
	float specularIntensity = std::powf(glm::dot(reflectedRay, viewRay), 256);
	
	float inidenceAngle = glm::dot(shadowRay, pointNormal);
	float proximityStrength = 1.5f;
	float proximityIntensity = 1/ (4 * M_PI * distance * distance);
	float incidentStrength = 1.6f;
	float specularStrength = 10.0f;
	float diffuseIntensity = fmaxf(proximityIntensity * proximityStrength, incidentStrength * inidenceAngle);
	return glm::clamp(specularIntensity * specularStrength + incidentStrength * inidenceAngle + proximityIntensity * proximityStrength, 0.0f, 1.0f);
	//return glm::clamp(specularIntensity * specularStrength, 0.0f, 1.0f);
}

std::pair<Colour, float> RayTracer::trace(CanvasPoint& point, glm::vec3 lightPos) {
	RayTriangleIntersection rayData;
	glm::vec3 cameraRay = getRayDirection(point);
	glm::vec3 pointNormal = glm::vec3(0, 0, 0);
	getClosestIntersection(cameraRay, rayData, pointNormal, camera->position, true);
		if (rayData.distanceFromCamera == INFINITY) {
			//window->setPixelColour(point.x, point.y, black);
			return std::make_pair(Colour(0,0,0), 0.0f);
		}
	else {
		glm::vec3 shadowRay = glm::normalize(rayData.intersectionPoint - lightPos);
		float dist = glm::distance(lightPos, rayData.intersectionPoint);
		RayTriangleIntersection shadowData;
		getClosestIntersection(shadowRay, shadowData, pointNormal, lightPos, false);
		float intensity;
		if (shadowData.distanceFromCamera != 0 && rayData.triangleIndex != shadowData.triangleIndex)
			intensity = 0;
		else {
			glm::vec3 viewRay = glm::normalize(camera->position - rayData.intersectionPoint);
			intensity = calculateIntensity(dist, glm::normalize(cameraRay), shadowRay, viewRay, (*triangles)[rayData.triangleIndex], pointNormal);
		}
		
		if(rayData.intersectedTriangle.colour.textured){
			std::vector<glm::vec2> texturePoints;
			texturePoints.push_back(glm::vec2(rayData.intersectedTriangle.texturePoints[0].x, rayData.intersectedTriangle.texturePoints[0].y));
			texturePoints.push_back(glm::vec2(rayData.intersectedTriangle.texturePoints[1].x, rayData.intersectedTriangle.texturePoints[1].y));
			texturePoints.push_back(glm::vec2(rayData.intersectedTriangle.texturePoints[2].x, rayData.intersectedTriangle.texturePoints[2].y));
			glm::vec2 coord = texturePoints[0];
			coord += -(texturePoints[1] - texturePoints[0]) * rayData.u * (float) textureMap.width;
			coord += -(texturePoints[2] - texturePoints[0]) * rayData.v * (float) textureMap.height;
			int x = std::floorf(coord[0]);
			int y = std::floorf(coord[1]);
			long index = textureMap.width * y + x;
			//float brightness = std::max(intensity, 0.2f);
			uint32_t argb = 0xFF000000;
			uint32_t red = (textureMap.pixels[index] & 0x00FF0000) >> 16;
			uint32_t green = (textureMap.pixels[index] & 0x0000FF00) >> 8;
			uint32_t blue = (textureMap.pixels[index] & 0x000000FF);
			//red *= brightness; green *= brightness; blue *= brightness;
			//red = red << 16; green = green << 8;
			//argb += red + green + blue;
			//window->setPixelColour(point.x, point.y, argb);
			return std::make_pair(Colour(red, green, blue), intensity);

		}
		else {
			//window->setPixelColour(point.x, point.y, rayData.intersectedTriangle.colour.getArbg(std::max(intensity, 0.2f)));
			/*float brightness = std::fmaxf(0.2f, intensity);
			rayData.intersectedTriangle.colour.red *= brightness;
			rayData.intersectedTriangle.colour.green *= brightness;
			rayData.intersectedTriangle.colour.blue *= brightness;*/
			return std::make_pair(rayData.intersectedTriangle.colour, intensity);
		}
	}
}

void RayTracer::drawRayTracedImage(DrawingWindow* window, std::vector<ModelTriangle>* triangles, Camera* camera) {
	this->window = window;
	this->triangles = triangles;
	this->camera = camera;
	std::vector<glm::vec3> lights;
	lights.push_back(lightPoint);
	//lights.push_back(lights[0] + glm::vec3(0.15, 0.0, 0.0));
	//lights.push_back(lights[1] + glm::vec3(0.15, 0.0, 0.0));
	
	for (int i = 0; i < height; i++){
		for (int j = 0; j < width; j++) {
			Colour colour = Colour(0, 0, 0);
			float intensity = 0;
		    CanvasPoint point = CanvasPoint(j,i, camera->focalLength);
			for (int i = 0; i < lights.size(); i++) {
				std::pair<Colour, float> colourVal = trace(point, lights[i]);
				colour = colourVal.first;
				intensity += colourVal.second;
			}
			window->setPixelColour(j,i,colour.getArbg(std::fmaxf(intensity/lights.size(), 0.2f)));
		}
	}

}
