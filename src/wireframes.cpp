#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <glm/glm.hpp>
#include <CanvasPoint.h>
#include <CanvasTriangle.h>
#include <Colour.h>
#include <ctime>
#include <Parser.h>
#include <array>
// 320 240
#define WIDTH 320
#define HEIGHT 240
#define SCALE 300

using namespace std;

array<array<float, WIDTH>, HEIGHT> buffer {};

vector<float> interpolateSingleFloats(float from, float to, size_t numberOfValues){
	vector<float> result;
	float increment = (to - from)/(numberOfValues-1);
	
	for(size_t i = 0; i < numberOfValues; i++){
		result.push_back(from);
		from += increment;
	}

	return result;
}

vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, size_t numberOfValues){ // todo run, test and debug this function
	vector<glm::vec3> result;
	for(int i = 0; i < numberOfValues; i++){
		glm::vec3 v = glm::vec3(0,0,0);
		result.push_back(v);
	}
	
	for(int i = 0; i < 3; i++){
		vector<float> temp;
		temp = interpolateSingleFloats(from[i], to[i], numberOfValues);
		for(int j = 0; j < temp.size(); j++){
			result[j][i] = temp[j];
		}
	}
	
	return result;
}


// write a line of pixels to the window
void write(DrawingWindow &window, vector<glm::vec3>* shades, int x, int y, size_t numberOfValues, bool vertical){
	vector<float> coords = interpolateSingleFloats(0, (vertical? HEIGHT : WIDTH), numberOfValues);
	int index = 1;
	
	int end = (vertical ? HEIGHT : WIDTH);
	for(size_t i = 0; i < end; i++){
		glm::vec3 value;
		value = (*shades)[index-1];
		uint32_t colour = (255 << 24) + (int(value[0]) << 16) + (int(value[1]) << 8) + int(value[2]);
		if (vertical) window.setPixelColour(x, i, colour);
		else window.setPixelColour(i, y, colour);
			
		
		if(i > coords[index]) index++;
	}
}

void makeBorders(DrawingWindow &window, glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 botLeft, glm::vec3 botRight, vector<glm::vec3>* rightBorder, vector<glm::vec3>* leftBorder, size_t numberOfValues){
	*rightBorder = interpolateThreeElementValues(topRight, botRight, numberOfValues);
	write(window, rightBorder, window.width -1, 0, numberOfValues, true);
	*leftBorder = interpolateThreeElementValues(topLeft, botLeft, numberOfValues);
	write(window, leftBorder, 0,0, numberOfValues, true);
	
	
}

bool checkAndUpdateBuffer(float x, float y, float depth){
    float reciprocal = 1/depth;
    if(buffer[y][x] == 0 || reciprocal > buffer[y][x]){
        buffer[y][x] = reciprocal;
        return true;
    }

    return false;
}

uint32_t getArgb(Colour colour) {
    uint32_t argb = (255 << 24) + (int(colour.red) << 16) + (int(colour.green) << 8) + (int(colour.blue));
    return argb;
}

void getLineVariables(float &numberOfSteps, float &xStep, float &yStep, CanvasPoint start, CanvasPoint finish){
	float xDiff = finish.x - start.x;
	float yDiff = finish.y - start.y;
	numberOfSteps = max(abs(xDiff), abs(yDiff));
	xStep = xDiff / numberOfSteps;
	yStep = yDiff / numberOfSteps;
}

void line(DrawingWindow &window, CanvasPoint start, CanvasPoint finish, Colour colour){
	float numberOfSteps, xStep, yStep;
	getLineVariables(numberOfSteps, xStep, yStep, start, finish);
	for(float i = 0.0f; i < numberOfSteps; i++){
		float x = start.x + xStep * i;
		float y = start.y + yStep * i;
		uint32_t argb = getArgb(colour);

		window.setPixelColour(ceil(x), floor(y), argb);
	}
	
}

void randomColour(Colour &colour){
	int colours[3];
	for(int i = 0; i < 3; i++){
		colours[i] = rand() % 256;
	}
	colour = Colour(colours[0], colours[1], colours[2]);
}

void drawStrokedTriangle(DrawingWindow &window, CanvasTriangle tri, Colour colour){
	//Colour colour = Colour(255,255,255);
	//randomColour(colour);
	
	line(window, tri.v0(), tri.v1(), colour);
	line(window, tri.v1(), tri.v2(), colour);
	line(window, tri.v0(), tri.v2(), colour);
}



void orderTriPoints(CanvasTriangle &tri){
	
	if(tri.v1().y < tri.v0().y){
		swap(tri.v0(), tri.v1());
	}
	if(tri.v2().y < tri.v1().y){
		swap(tri.v1(), tri.v2());
	}
	if(tri.v1().y < tri.v0().y){
		swap(tri.v0(), tri.v1());
	}
	
}

CanvasPoint getScaledPoint(CanvasPoint a, CanvasPoint b, CanvasPoint c) {
	float scale = (float)(a.y - b.y) / (float)(a.y - c.y);
	float x = round((float)((c.x - a.x) * scale)) + a.x;
	b.x = x;
	return b;
}


void drawFilledTriangle(DrawingWindow& window, CanvasTriangle& tri, Colour& colour) {

	orderTriPoints(tri);
	CanvasPoint intersect = getScaledPoint(tri.v0(), tri.v1(), tri.v2());

		for (float y = tri.v0().y; y <= intersect.y; y++) {

			//float xStart = getScaledPoint(tri.v0(), CanvasPoint(0, y), tri.v2()).x;
			//float xEnd = getScaledPoint(tri.v0(), CanvasPoint(0, y), tri.v1()).x;
            CanvasPoint start = getScaledPoint(tri.v0(), CanvasPoint(0, y), tri.v2());
            CanvasPoint end = getScaledPoint(tri.v0(), CanvasPoint(0, y), tri.v1());
			if (end.x == INFINITY) {
				intersect = tri.v0();
				break;
			}

            line(window, start, end, colour);

			/*for (float i = min(xStart, xEnd); i <= max(xStart, xEnd); i++) {
				window.setPixelColour(round(i), round(y), argb);
			}*/
		}

		for (float y = intersect.y; y <= tri.v2().y; y++) {

			/*float xStart = getScaledPoint(tri.v0(), CanvasPoint(0, y), tri.v2()).x;
			float xEnd = getScaledPoint(tri.v1(), CanvasPoint(0, y), tri.v2()).x;

			for (float i = min(xStart,xEnd); i <= max(xStart, xEnd); i++) {
				window.setPixelColour(round(i), round(y), argb);
			}*/

            CanvasPoint start = getScaledPoint(tri.v0(), CanvasPoint(0, y), tri.v2());
            CanvasPoint end = getScaledPoint(tri.v1(), CanvasPoint(0, y), tri.v2());
            line(window, start, end, colour);
		}
}


void drawTriangle(DrawingWindow &window){
	CanvasTriangle tri;
	CanvasPoint points[3];
	for (int i = 0; i < 3; i++) {
		int x = rand() % window.width;
		int y = rand() % window.height;
		points[i] = CanvasPoint(x,y);
	}
	Colour colour;
	randomColour(colour);

	tri = CanvasTriangle(points[0], points[1], points[2]);
	drawFilledTriangle(window, tri, colour);
	//drawStrokedTriangle(window, tri);
}

void drawPoints(DrawingWindow& window) {
	CanvasPoint points[3];
	points[0] = CanvasPoint((104.569), (160.75));
	points[1] = CanvasPoint((164.355), (222.229));
	points[2] = CanvasPoint((104.566), (222.12));

	for (int i = 0; i < sizeof(points) / sizeof(points[0]); i++) {
		window.setPixelColour(points[i].x, points[i].y, 0xFFFF0000);
	}
}

CanvasPoint getCanvasIntersectionPoint(glm::vec3 vertexPos) {
	float x, y, depth;
    glm::vec3 cameraPos = glm::vec3(0,0,3);
    float focalLength = 2;
	x = -focalLength * SCALE * (vertexPos[0] / (vertexPos[2] - cameraPos[2])) + (WIDTH / 2);
	y = focalLength * SCALE * (vertexPos[1] / (vertexPos[2] - cameraPos[2])) + (HEIGHT / 2);
    depth = cameraPos[2] - vertexPos[2];
	//return CanvasPoint(round(x) , round(y));
	return CanvasPoint(x, y, depth);

}

void draw(DrawingWindow& window, ModelTriangle& tri) {
	CanvasTriangle triCan = CanvasTriangle(getCanvasIntersectionPoint(tri.vertices[0]), getCanvasIntersectionPoint(tri.vertices[1]), getCanvasIntersectionPoint(tri.vertices[2]));
	drawFilledTriangle(window, triCan, tri.colour);
    drawStrokedTriangle(window, triCan, tri.colour);
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
		else if (event.key.keysym.sym == SDLK_u) {drawTriangle(window); window.renderFrame();};
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}


int main(int argc, char *argv[]) {
	// uncomment line below for proper random
	//srand((unsigned int) time(NULL));

	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;
	Parser parser = Parser();
	for (int i = 0; i < parser.triangles.size(); i++) {
		draw(window, parser.triangles[i]);
	}
	window.renderFrame();
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		//window.renderFrame();
	}
}
