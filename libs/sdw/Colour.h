#pragma once

#include <iostream>
#include <glm/glm.hpp>

struct Colour {
	std::string name;
	int red{};
	int green{};
	int blue{};
	bool textured;
	bool mirror;
	bool glass;
	bool environment;
	bool bumped;
	float rf;
	uint32_t black;
	Colour();
	Colour(int r, int g, int b);
	Colour(std::string n, int r, int g, int b);
    uint32_t getArbg();
    uint32_t getArbg(float intensity);
};

std::ostream &operator<<(std::ostream &os, const Colour &colour);
