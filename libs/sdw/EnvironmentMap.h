#pragma once

#include "TextureMap.h"
#include "Colour.h"
#include <glm/glm.hpp>

class EnvironmentMap {
	TextureMap posX, posY, posZ, negX, negY, negZ;

	enum Side {PosX, PosY, PosZ, NegX, NegY, NegZ};

public:
	EnvironmentMap();
	Colour getEnvironmentPixelColour(glm::vec3 ray);
};