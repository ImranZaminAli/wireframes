#include "EnvironmentMap.h"

EnvironmentMap::EnvironmentMap() {
	//std::string root = "C:\\Users\\izami\\Documents\\UoBYr3\\wireframes\\";
	/*posX = TextureMap("C:\\Users\\izami\\Documents\\UoBYr3\\wireframes\\textured-cornell-box.mtl\\posx");
	posX = TextureMap("C:\\Users\\izami\\Documents\\UoBYr3\\wireframes\\textured-cornell-box.mtl\\posy");
	posX = TextureMap("C:\\Users\\izami\\Documents\\UoBYr3\\wireframes\\textured-cornell-box.mtl\\posz");
	posX = TextureMap("C:\\Users\\izami\\Documents\\UoBYr3\\wireframes\\textured-cornell-box.mtl\\negx");
	posX = TextureMap("C:\\Users\\izami\\Documents\\UoBYr3\\wireframes\\textured-cornell-box.mtl\\po");
	posX = TextureMap("C:\\Users\\izami\\Documents\\UoBYr3\\wireframes\\textured-cornell-box.mtl\\posx");*/
    std::string root = "";
	posX = TextureMap(root + "posy.ppm");
	posY = TextureMap(root + "posy.ppm");
	posZ = TextureMap(root + "posz.ppm");
	negX = TextureMap(root + "negx.ppm");
	negY = TextureMap(root + "negy.ppm");
	negZ = TextureMap(root + "negz.ppm");
}

Colour EnvironmentMap::getEnvironmentPixelColour(glm::vec3 ray) {
	Side side;
	float absX = std::fabs(ray.x), absY = std::fabs(ray.y), absZ = std::fabs(ray.z);
	float maxAxis, uc, vc;
	glm::vec2 coords;

	if (absX >= absY && absX >= absZ) {
		maxAxis = absX;
		uc = ray.x >= 0 ? -ray.z : ray.z;
		vc = ray.y;
		side = ray.x >= 0 ? Side::PosX : Side::NegX;
	}
	else if (absY >= absX && absY >= absZ) {
		maxAxis = absY;
		uc = ray.x;
		vc = ray.y >= 0 ? -ray.z : ray.z;
		side = ray.y >= 0 ? Side::PosY : Side::NegY;
	}
	else {
		maxAxis = absZ;
		uc = ray.z >= 0 ? ray.x : -ray.x;
		vc = ray.y;
		side = ray.z >= 0 ? Side::PosZ : Side::NegZ;
	}

	coords[0] = 0.5f * (uc / maxAxis + 1.0f);
	coords[1] = 0.5f * (vc / maxAxis + 1.0f);
	//if (coords[0] > 1)
		std::cout << "u: " << coords[0] << " v: " << coords[1] << std::endl;
	//if (coords[1] > 1)
	//	std::cout << "v : " << coords[1] << std::endl;
	//if (absX >= absY && absX >= absZ) {
	//	//maxAxis = absX;
	//	side = ray.x >= 0 ? Side::PosX : Side::NegX;
	//	coords[0] = (ray.y/absX+1.0f)*0.5f;
	//	coords[1] = (ray.z/ absX +1.0f)*0.5f;
	//}
	//else if (absY >= absX && absY >= absZ) {
	//	//maxAxis = absY;
	//	side = ray.y >= 0 ? Side::PosY : Side::NegY;
	//	coords[0] = (ray.x / absY + 1.0f) * 0.5f;
	//	coords[1] = (ray.z / absY + 1.0f) * 0.5f;
	//}
	//else {
	//	//maxAxis = absZ;
	//	side = ray.z >= 0 ? Side::PosZ : Side::NegZ;
	//	coords[0] = (ray.x / absZ + 1.0f) * 0.5f;
	//	coords[1] = (ray.y / absZ + 1.0f) * 0.5f;
	//}
	//std::cout << coords[0] << " " << coords[1] << std::endl;
	switch (side) {
	case Side::PosX:
		return posX.getPixelColour(coords[0] * (posX.width), coords[1] * (posX.height-1));
		break;
	case Side::PosY:
		return posY.getPixelColour(coords[0] * (posY.width), coords[1] * (posY.height-1)); 
		break;
	case Side::PosZ:
		return posZ.getPixelColour(coords[0] * (posZ.width), coords[1] * (posZ.height-1)); 
		break;
	case Side::NegX:
		return negX.getPixelColour(coords[0] * (negX.width), coords[1] * (negX.height-1)); 
		break;
	case Side::NegY:
		return negY.getPixelColour(coords[0] * (negY.width), coords[1] * (negY.height-1)); 
		break;
	case Side::NegZ:
		return negZ.getPixelColour(coords[0] * (negZ.width), coords[1] * (negZ.height-1)); 
		break;
	default:
		break;
	}
}