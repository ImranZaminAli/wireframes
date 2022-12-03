#include "RayTracer.h"

RayTracer::RayTracer(int windowWidth, int windowHeight) {
	width = windowWidth;
	height = windowHeight;
	//lightPoint = glm::vec3(0, 0.1, 0.0);
	//lightPoint = glm::vec3(0.2f, 0.7f, 0.3f);
	//lightPoint = glm::vec3(0,0.4,2);
	lightPoint = glm::vec3(0.2f, 1.2f, 2.0f);
	//lightPoint = glm::vec3(0, 0.4, 0);
	black = 0xFF000000;
	sourceStrength = 2.0f;
	maxBounces = 10;
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
	pointNormal = glm::normalize(pointNormal);
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

void RayTracer::checkForBlockedLight(glm::vec3 rayDir, RayTriangleIntersection& rayData, glm::vec3& pointNormal, glm::vec3 startPosition) {
	rayData.distanceFromCamera = INFINITY;
	for (int i = 0; i < triangles->size(); i++) {
		/*if (rayData.distanceFromCamera != INFINITY)
			break;*/
		glm::vec3 possibleSolution = getPossibleSolution(i, rayDir, startPosition);
		bool valid = checkValid(possibleSolution[1], possibleSolution[2], possibleSolution[0]);

		if (valid && !(*triangles)[i].colour.glass && possibleSolution[0] < rayData.distanceFromCamera) {

			rayData.distanceFromCamera = possibleSolution[0];
			rayData.intersectedTriangle = (*triangles)[i];
			rayData.triangleIndex = i;


			rayData.intersectionPoint = calculateIntersection((*triangles)[i], pointNormal, possibleSolution[1], possibleSolution[2]);
			rayData.u = possibleSolution[1];
			rayData.v = possibleSolution[2];
		}
	}
}

glm::vec3 RayTracer::getReflectedRay(glm::vec3& incidentRay, glm::vec3& normal) {
	//return glm::normalize(incidentRay - 2.0f * normal * glm::dot(incidentRay, -normal));
	return 2.0f * glm::dot(normal, incidentRay) * normal - incidentRay;
}

float RayTracer::calculateIntensity(float distance, glm::vec3 cameraRay, glm::vec3 shadowRay, glm::vec3 viewRay, ModelTriangle& tri, glm::vec3 pointNormal) {
	glm::vec3 reflectedRay = glm::normalize(shadowRay - 2.0f * pointNormal * glm::dot(shadowRay,pointNormal));
	//glm::vec3 reflectedRay = getReflectedRay(shadowRay, pointNormal);
	float specularIntensity = std::powf(std::fmaxf(0.0f,glm::dot(reflectedRay, viewRay)), 64);
	
	float inidenceAngle = glm::dot(shadowRay, pointNormal);
	float proximityStrength = 2.2f; //1.5
	float proximityIntensity = 1/ (4 * M_PI * distance * distance);
	//float proximityIntensity = 1.0f;
	float incidentStrength = 6.0f; // 1.6
	float specularStrength = 5.0f;
	//float diffuseIntensity = fmaxf(proximityIntensity * proximityStrength, incidentStrength * inidenceAngle);
	return glm::clamp(specularIntensity * specularStrength + 1.8f * incidentStrength * inidenceAngle * proximityIntensity * proximityStrength, 0.0f, 1.0f);
}

std::pair<Colour, float> RayTracer::trace(glm::vec3& rayDir, glm::vec3 start, glm::vec3 lightPos, int bounce) {
	if (bounce > maxBounces) {
		//std::cout << "acheived\n";
		return std::make_pair(Colour(0, 0, 0), 1.0f);
	}
	RayTriangleIntersection rayData;
	glm::vec3 pointNormal = glm::vec3(0, 0, 0);
	getClosestIntersection(rayDir, rayData, pointNormal, start, true);
	if (rayData.distanceFromCamera == INFINITY) {
		//if (bounce > 0) std::cout << "wall";
		return std::make_pair(Colour(0,0,0), 0.0f);
	}
	else if (rayData.intersectedTriangle.colour.mirror) {
		//glm::vec3 reflectRay = -getReflectedRay(glm::normalize(rayDir),pointNormal);
		glm::vec3 reflectRay = glm::reflect(glm::normalize(rayDir), pointNormal);
		//std::cout << rayDir[0] << " " << rayDir[1] << " " << rayDir[2] << " " << reflectRay[0] << " " << reflectRay[1] << " " << reflectRay[2] << std::endl;
		return trace(glm::normalize(reflectRay), rayData.intersectionPoint, lightPos, bounce + 1);
	}
	else if (rayData.intersectedTriangle.colour.glass) {
		/*glm::vec3 nRefr = rayData.intersectedTriangle.normal;
		float cosi = glm::clamp(glm::dot(glm::normalize(rayDir), glm::normalize(nRefr)), -1.0f, 1.0f);
		float etai = 1;
		float etat = rayData.intersectedTriangle.colour.rf;
		float kr;
		if (cosi > 0) std::swap(etai, etat);
		float sint = etai / etat * std::sqrtf(std::fmaxf(0.0f, 1 - cosi * cosi));
		if (sint >= 1) {
			kr = 1;
		}
		else {
			float cost = std::sqrtf(std::fmaxf(0.0f, 1 - sint * sint));
			cosi = std::fabsf(cosi);
			float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
			float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
			kr = (Rs * Rs + Rp * Rp) / 2;
		}

		std::pair<Colour, float> refractionValue;
		if (kr < 1) {
			
		}*/
	}
	else {
		
		glm::vec3 shadowRay = glm::normalize(rayData.intersectionPoint - lightPos);
		float dist = glm::distance(lightPos, rayData.intersectionPoint);
		RayTriangleIntersection shadowData;
		checkForBlockedLight(shadowRay, shadowData, pointNormal, lightPos);
		float intensity;
		if (shadowData.distanceFromCamera > 0.0001 && rayData.triangleIndex != shadowData.triangleIndex)
			intensity = 0;
		else {
			glm::vec3 viewRay = glm::normalize(camera->position - rayData.intersectionPoint);
			intensity = calculateIntensity(dist, glm::normalize(rayDir), shadowRay, viewRay, (*triangles)[rayData.triangleIndex], pointNormal);
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
			uint32_t argb = 0xFF000000;
			uint32_t red = (textureMap.pixels[index] & 0x00FF0000) >> 16;
			uint32_t green = (textureMap.pixels[index] & 0x0000FF00) >> 8;
			uint32_t blue = (textureMap.pixels[index] & 0x000000FF);
			
			return std::make_pair(Colour(red, green, blue), intensity);

		}
		else {
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
				glm::vec3 rayDir = getRayDirection(point);
				std::pair<Colour, float> colourVal = trace(glm::normalize(rayDir), camera->position, lights[i], 0);
				colour = colourVal.first;
				intensity += colourVal.second;
			}
			window->setPixelColour(j,i,colour.getArbg(std::fmaxf(intensity/lights.size(), 0.2f)));
		}
	}

}
