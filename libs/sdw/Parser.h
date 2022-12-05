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

using namespace std;

class Parser {
	const float scale = .17f;
	const float maxColour = 255.0f;
	std::vector<glm::vec3> vertexNormals;
	unordered_map<string, Colour> colours;
	string nextLine;
	
public:
	std::vector<glm::vec3> vertices;
	std::vector<ModelTriangle> triangles;
    //std::vector<std::vector<ModelTriangle>> trianglesSplit;
	Parser();

};
