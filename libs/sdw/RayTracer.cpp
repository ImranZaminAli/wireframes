#include "RayTracer.h"

RayTracer::RayTracer(int windowWidth, int windowHeight) {
	width = windowWidth;
	height = windowHeight;
	std::cout << "here\n";
	//lightPoint = glm::vec3(0, 0.1, 0.0);
	//lightPoint = glm::vec3(0.2f, 0.7f, 0.3f);
	//lightPoint = glm::vec3(0,0.4,2);
	//lightPoint = glm::vec3(0.2f, 1.2f, 2.0f);
	lightPoint = glm::vec3(0.0f, 0.35f, 0.0f);
	//lightPoint = glm::vec3(0.0f, 0.2f, 0.5f);
	black = 0xFF000000;
	sourceStrength = 2.0f;
	maxBounces = 20;
    incidentStrength = 1.3;
    proximityStrength = 2.5f;
	//textureMap = TextureMap("C:\\Users\\izami\\Documents\\UoBYr3\\wireframes\\metalTexture.ppm");
	//normalMap = TextureMap("C:\\Users\\izami\\Documents\\UoBYr3\\wireframes\\metalNorm.ppm");
    normalMap = TextureMap("brickwall_normal.ppm");
    textureMap = TextureMap("brickwall.ppm");
	//envMap = TextureMap("C:\\Users\\izami\\Documents\\UoBYr3\\wireframes\\spacebox.ppm");
    envMap = TextureMap("spacebox.ppm");

	for (int y = 0; y < envMap.height; y++) {
		std::vector<uint32_t> rowVals;
		for (int x = 0; x < envMap.width; x++) {
			rowVals.push_back(envMap.pixels.at(y * envMap.width + x));
		}
		map.push_back(rowVals);
	}
}

glm::vec3 RayTracer::getRayDirection(CanvasPoint& point) {
	glm::vec3 rayDir;
	float x, y, z;
	z = -(*camera).focalLength;

	x = ((point.x - (width / 2)) / 700) * -z;
	y = ((point.y - (height / 2)) / 700) * z;

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

	if (triangle.colour.bumped) {
		//std::cout << "bumping\n";
		std::vector<glm::vec2> texturePoints;
		//std::cout << triangle.texturePoints[1] << std::endl;
		texturePoints.push_back(glm::vec2(triangle.texturePoints[0].x, triangle.texturePoints[0].y));
		texturePoints.push_back(glm::vec2(triangle.texturePoints[1].x, triangle.texturePoints[1].y));
		texturePoints.push_back(glm::vec2(triangle.texturePoints[2].x, triangle.texturePoints[2].y));
		glm::vec2 coord = texturePoints[0];
		coord += -(texturePoints[1] - texturePoints[0]) * u * (float)textureMap.width;
		coord += -(texturePoints[2] - texturePoints[0]) * v * (float)textureMap.height;
		int x = floorf(coord[0]);
		int y = floorf(coord[1]);

		Colour normal = normalMap.getPixelColour(x, y);

		pointNormal = glm::vec3(normal.red, normal.green, normal.blue);
		pointNormal *= -1;
	}
	else {
		pointNormal = triangle.vertexNormals[0];
		pointNormal += (triangle.vertexNormals[1] - triangle.vertexNormals[0]) * u;
		pointNormal += (triangle.vertexNormals[2] - triangle.vertexNormals[0]) * v;
	}
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

float RayTracer::calculateIntensity(float distance, glm::vec3 cameraRay, glm::vec3 shadowRay, glm::vec3 viewRay, ModelTriangle& tri, glm::vec3 pointNormal, bool& specular) {
	//glm::vec3 reflectedRay = glm::normalize(shadowRay - 2.0f * pointNormal * glm::dot(shadowRay,pointNormal));
	glm::vec3 reflectedRay = glm::reflect(glm::normalize(shadowRay), pointNormal);
	float specularIntensity = powf(std::fmaxf(0.0f,glm::dot(reflectedRay, viewRay)), 160);
	
	float inidenceAngle = glm::dot(shadowRay, pointNormal);
	//float proximityStrength = 3.0f; //1.5
	float proximityIntensity = 1/ (4 * M_PI * distance * distance);
	//float proximityIntensity = 1.0f;
	//float incidentStrength = 3.2f; // 1.6
	float specularStrength = 1.2f;
	//float diffuseIntensity = fmaxf(proximityIntensity * proximityStrength, incidentStrength * inidenceAngle);

	float diffuse = fmaxf(incidentStrength * inidenceAngle, proximityIntensity * proximityStrength);
    //final = fmaxf(final, specularIntensity * specularStrength);
    /*if (final < specularStrength * specularIntensity){
        final = specularStrength * specularIntensity;
        specular = true;
    }*/
    return fmaxf(diffuse , specularStrength * specularIntensity);
}

glm::vec3 RayTracer::refract(glm::vec3& rayDir, glm::vec3 n, float rf) {
	float cosi = glm::clamp(glm::dot(glm::normalize(rayDir), n), -1.0f, 1.0f);
	float etai = 1, etat = rf;
	glm::vec3 norm = n;
	if (cosi < 0.0f) {
		cosi *= -1;
		
	}
	else {
		std::swap(etai, etat);
		norm *= -1;
	}

	float eta = etai / etat;
	float k = 1 - eta * eta * (1 - cosi * cosi);
	return k < 0 ? glm::vec3(0, 0, 0) : eta * rayDir + (eta * cosi - sqrtf(k)) * norm;
}

float RayTracer::fresnel(glm::vec3& rayDir, glm::vec3 n, float rf) {
	float cosi = glm::clamp(glm::dot(rayDir, n), -1.0f, 1.0f);
	float etai = 1;
	float etat = rf;
	if (cosi > 0) 
		std::swap(etai, etat);
	float sint = (etai / etat) * sqrtf(std::max(0.f, 1 - cosi * cosi));
	if (sint >= 1) {  return 1.0f; }
		//return 1.0f;
	//std::cout << "na\n";
	float cost = sqrtf(std::fmaxf(0.0f, 1 - sint * sint));
	cosi = fabsf(cosi);
	float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
	float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
	return (Rs * Rs + Rp * Rp) / 2;	
}

std::pair<Colour, float> RayTracer::trace(glm::vec3& rayDir, glm::vec3 start, glm::vec3 lightPos, int bounce, bool debug, bool specular) {

	if (bounce > maxBounces) {
		std::cout << "acheived\n";
		return std::make_pair(Colour(255, 255, 255), 1.0f);
	}
	RayTriangleIntersection rayData;
	glm::vec3 pointNormal = glm::vec3(0, 0, 0);
	getClosestIntersection(rayDir, rayData, pointNormal, start, true);

	if (rayData.distanceFromCamera == INFINITY) {
		//return std::make_pair(Colour(0,0,0), 0.0f);
        rayDir = glm::normalize(rayDir);
        float m = 2.0f * sqrt( pow(rayDir.x, 2) + pow(rayDir.y , 2) + pow(rayDir.z + 1, 2));
        float u = ((rayDir.x / m) + 0.5f) * envMap.width;
        float v = ((rayDir.y / m) + 0.5f) * envMap.height;

        if (u > envMap.width - 1) u = envMap.width - 1;
        if (v > envMap.height - 1) v = envMap.height - 1;
        if (v < 0) v = 0;
        if (u < 0) u = 0;

        uint32_t argb = map[v][u];
        uint32_t red = (argb & 0x00FF0000) >> 16;
        uint32_t green = (argb & 0x0000FF00) >> 8;
        uint32_t blue = (argb & 0x000000FF);
        return std::make_pair(Colour(red,blue,green), 1.0f);
	}
	else if (rayData.intersectedTriangle.colour.mirror) {
		glm::vec3 reflectRay = glm::normalize(glm::reflect(glm::normalize(rayDir), pointNormal));
		return trace((reflectRay), rayData.intersectionPoint, lightPos, bounce + 1, debug, false);
	}
	else if (rayData.intersectedTriangle.colour.glass) {
		rayDir = glm::normalize(rayDir);
		float kr = fresnel(rayDir, pointNormal, 1.5);
		std::pair<Colour, float> refractValues;
		if (kr < 1.0f) {
			glm::vec3 refractRay = glm::normalize(refract(rayDir, pointNormal, 1.5));
			refractValues = trace((refractRay), rayData.intersectionPoint, lightPos, bounce + 1, debug, false);
		}
		glm::vec3 reflectRay = glm::normalize(glm::reflect(rayDir, pointNormal));
		std::pair<Colour, float> reflectValues = trace((reflectRay), rayData.intersectionPoint, lightPos, bounce + 1, debug, false);
		Colour finalColour = refractValues.first;
		finalColour.red *= (1-kr); finalColour.green *= (1- kr); finalColour.blue *= (1-kr);
		finalColour.red += reflectValues.first.red * (kr);
		finalColour.green += reflectValues.first.green * (kr);
		finalColour.blue += reflectValues.first.blue * (kr);
		if (finalColour.red > 255.0f) finalColour.red = 255.0f;
		if (finalColour.green > 255.0f) finalColour.green = 255.0f;
		if (finalColour.blue > 255.0f) finalColour.blue = 255.0f;
		if(debug) std::cout << std::hex << finalColour.getArbg() << std::endl;
		return std::make_pair(finalColour, (refractValues.second * (1-kr)+ reflectValues.second * kr));
		
	}
	else if (rayData.intersectedTriangle.colour.environment) {
		glm::vec3 reflectRay = glm::reflect(rayDir, pointNormal);

		float m = 2.0f * sqrt( pow(reflectRay.x, 2) + pow(reflectRay.y , 2) + pow(reflectRay.z + 1, 2));

		float u = ((reflectRay.x / m) + 0.5f) * envMap.width;
		float v = ((reflectRay.y / m) + 0.5f) * envMap.height;

		if (u > envMap.width - 1) u = envMap.width - 1;
		if (v > envMap.height - 1) v = envMap.height - 1;
		if (v < 0) v = 0;
		if (u < 0) u = 0;
		//return std::make_pair(envMap.getPixelColour(u,v), 1.0f);
		//std::vector<std::vector<uint32_t>> map;
		Colour white = Colour(255,255,255);
		//std::cout << u << " " << v << std::endl;
		//std::cout << map.size() << " " << map[0].size() << std::endl;
		uint32_t argb = map[v][u];
		uint32_t red = (argb & 0x00FF0000) >> 16;
		uint32_t green = (argb & 0x0000FF00) >> 8;
		uint32_t blue = (argb & 0x000000FF);
		return std::make_pair(Colour(red,blue,green), 1.0f);
		//return std::make_pair(envMap.getEnvironmentPixelColour(reflectray), 1.0f);
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

			glm::vec3 normal = pointNormal;
			
			intensity = calculateIntensity(dist, glm::normalize(rayDir), shadowRay, viewRay, rayData.intersectedTriangle, pointNormal, specular);
		}
        Colour white = Colour(255,255,255);
		if(rayData.intersectedTriangle.colour.textured){
			std::vector<glm::vec2> texturePoints;
			texturePoints.push_back(glm::vec2(rayData.intersectedTriangle.texturePoints[0].x, rayData.intersectedTriangle.texturePoints[0].y));
			texturePoints.push_back(glm::vec2(rayData.intersectedTriangle.texturePoints[1].x, rayData.intersectedTriangle.texturePoints[1].y));
			texturePoints.push_back(glm::vec2(rayData.intersectedTriangle.texturePoints[2].x, rayData.intersectedTriangle.texturePoints[2].y));
			glm::vec2 coord = texturePoints[0];
			coord += -(texturePoints[1] - texturePoints[0]) * rayData.u * (float) textureMap.width;
			coord += -(texturePoints[2] - texturePoints[0]) * rayData.v * (float) textureMap.height;
			int x = floorf(coord[0]);
			int y = floorf(coord[1]);
			

			return std::make_pair(textureMap.getPixelColour(x,y), intensity);

		}
		else {
			return std::make_pair(rayData.intersectedTriangle.colour, intensity);
		}
	}
}

void RayTracer::drawRayTracedImage(DrawingWindow* window, std::vector<ModelTriangle>* triangles, Camera* camera, int start, int finish) {
	this->window = window;
	this->triangles = triangles;
	this->camera = camera;
	std::vector<glm::vec3> lights;
	lights.push_back(lightPoint);
    float offset = 0.05f;
    /*lights.push_back(lightPoint + glm::vec3(offset, 0,0));
    lights.push_back(lightPoint + glm::vec3(-offset, 0,0));
    lights.push_back(lightPoint + glm::vec3(0,0,offset));
    lights.push_back(lightPoint + glm::vec3(0,0,-offset));
    lights.push_back(lightPoint + glm::vec3(offset * 2.0f, 0,0));
    lights.push_back(lightPoint + glm::vec3(-offset* 2.0f, 0,0));
    lights.push_back(lightPoint + glm::vec3(0,0,offset* 2.0f));
    lights.push_back(lightPoint + glm::vec3(0,0,-offset* 2.0f));*/


    //lights.push_back(lightPoint + glm::vec3(0,offset,0));
    //lights.push_back(lightPoint + glm::vec3(0,-offset,0));

    /*lights.push_back(lightPoint + glm::vec3(offset,0,offset));
    lights.push_back(lightPoint + glm::vec3(-offset,0,offset));
    lights.push_back(lightPoint + glm::vec3(offset,0,-offset));
    lights.push_back(lightPoint + glm::vec3(-offset,0,offset));*/

    /*lights.push_back(lightPoint + glm::vec3(0,offset,offset));
    lights.push_back(lightPoint + glm::vec3(0,-offset,offset));
    lights.push_back(lightPoint + glm::vec3(0,offset,-offset));
    lights.push_back(lightPoint + glm::vec3(0,-offset,offset));

    lights.push_back(lightPoint + glm::vec3(offset,offset,0));
    lights.push_back(lightPoint + glm::vec3(-offset,offset,0));
    lights.push_back(lightPoint + glm::vec3(offset,-offset,0));
    lights.push_back(lightPoint + glm::vec3(-offset,offset,0));*/
	//lights.push_back(lights[0] + glm::vec3(0.15, 0.0, 0.0));
	//lights.push_back(lights[1] + glm::vec3(0.15, 0.0, 0.0));
	
	for (int i = start; i < finish; i++){
		for (int j = 0; j < width; j++) {
			Colour colour = Colour(0, 0, 0);
			float intensity = 0;
		    CanvasPoint point = CanvasPoint(j,i, camera->focalLength);
			for (int i = 0; i < lights.size(); i++) {
				glm::vec3 rayDir = glm::normalize(getRayDirection(point));
                bool specular = false;
				std::pair<Colour, float> colourVal = trace((rayDir), camera->position, lights[i], 0, false, specular);
				colour = colourVal.first;
				intensity += colourVal.second;
			}
			window->setPixelColour(j,i,colour.getArbg(std::fmaxf( intensity/lights.size(), 0.23f)));
		}
	}

}
