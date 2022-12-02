#include "Colour.h"
#include <utility>

Colour::Colour() { black = 0x00FFFFFF; textured = false; mirror = false; glass = false; rf = 1.0f; };
Colour::Colour(int r, int g, int b) : red(r), green(g), blue(b) { black = 0x00FFFFFF; textured = false; mirror = false; glass = false; rf = 1.0f; }
Colour::Colour(std::string n, int r, int g, int b) :
		name(std::move(n)),
		red(r), green(g), blue(b) {
	black = 0x00FFFFFF;
	textured = false;
	mirror = false;
	glass = false;
	rf = 1.0f;
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
    return (255 << 24) + (int(red * intensity) << 16) + (int(green * intensity) << 8) + (int(blue * intensity));
}
