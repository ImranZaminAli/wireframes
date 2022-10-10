#pragma once

#include <glm/glm.hpp>
#include <utility>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include "ModelTriangle.h"
#include "Utils.h"
#include "Colour.h"


class Parser {
	const float scale = 0.17;
	const float maxColour = 255.0f;
	
public:
	std::vector<glm::vec3> vertices;
	std::vector<ModelTriangle> triangles;
	Parser();

};
