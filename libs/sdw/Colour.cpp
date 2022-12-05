#include "Colour.h"
#include <utility>

Colour::Colour() { black = 0x00FFFFFF; textured = false; mirror = false; glass = false; rf = 1.0f; environment = false; bumped = false; };
Colour::Colour(int r, int g, int b) : red(r), green(g), blue(b) { black = 0x00FFFFFF; textured = false; mirror = false; glass = false; rf = 1.0f; environment = false; bumped = false; }
Colour::Colour(std::string n, int r, int g, int b) :
		name(std::move(n)),
		red(r), green(g), blue(b) {
	black = 0x00FFFFFF;
	textured = false;
	mirror = false;
	glass = false;
	rf = 1.0f;
	environment = false;
	bumped = false;
}
std::ostream &operator<<(std::ostream &os, const Colour &colour) {
	os << colour.name << " ["
	   << colour.red << ", "
	   << colour.green << ", "
	   << colour.blue << "]";
	return os;
}

uint32_t Colour::getArbg() {
    return (255 << 24) + (int(red) << 16) + (int(green) << 8) + (int(blue));
}

uint32_t Colour::getArbg(float intensity) {
    float r = red * intensity;
    float g = green * intensity;
    float b = blue * intensity;
    if(r > 255)
        r = 255;
    if(g > 255)
        g = 255;
    if(b > 255)
        b = 255;
    return (255 << 24) + (int(r) << 16) + (int(g) << 8) + (int(b));
}
