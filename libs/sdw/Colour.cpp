#include "Colour.h"
#include <utility>

Colour::Colour() { black = 0x00FFFFFF; };
Colour::Colour(int r, int g, int b) : red(r), green(g), blue(b) { black = 0x00FFFFFF; }
Colour::Colour(std::string n, int r, int g, int b) :
		name(std::move(n)),
		red(r), green(g), blue(b) {
	black = 0x00FFFFFF;
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