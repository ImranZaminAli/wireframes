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
// 320 240
#define WIDTH 1000
#define HEIGHT 1000
#define SCALE 400

using namespace std;

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
		uint32_t argb = (255 << 24) + (int(colour.red) << 16) + (int(colour.blue) << 8) + (int(colour.green));
		window.setPixelColour(round(x), round(y), argb); 
	}
	
}

void randomColour(Colour &colour){
	int colours[3];
	for(int i = 0; i < 3; i++){
		colours[i] = rand() % 256;
	}
	colour = Colour(colours[0], colours[1], colours[2]);
}

void drawStrokedTriangle(DrawingWindow &window, CanvasTriangle tri){
	Colour colour = Colour(255,255,255);
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
	//CanvasPoint point = CanvasPoint(x, b.y);
	return b;
}

uint32_t getArgb(Colour colour) {
	uint32_t argb = (255 << 24) + (int(colour.red) << 16) + (int(colour.green) << 8) + (int(colour.blue));
	return argb;
}

void drawFilledTriangle(DrawingWindow& window, CanvasTriangle& tri, Colour& colour) {
	uint32_t argb = getArgb(colour);
	orderTriPoints(tri);
	CanvasPoint intersect = getScaledPoint(tri.v0(), tri.v1(), tri.v2());
	//int sign = (intersect.x > tri.v1().x) ? -1 : 1;
	
		cout << "here3\n";
		for (float y = tri.v0().y; y <= intersect.y; y++) {

			float xStart = getScaledPoint(tri.v0(), CanvasPoint(0, y), tri.v2()).x;
			cout << xStart << endl;
			float xEnd = getScaledPoint(tri.v0(), CanvasPoint(0, y), tri.v1()).x;
			//cout << ""
			if (xEnd == INFINITY) {
				intersect = tri.v0();
				break;
			}
			/*for (int i = 0; i <= abs(xStart - xEnd); i++) {
				window.setPixelColour(xStart + i * sign, y, argb);
			}*/

			for (float i = min(xStart, xEnd); i <= max(xStart, xEnd); i++) {
				window.setPixelColour(round(i), round(y), argb);
			}
		}

		for (float y = intersect.y; y <= tri.v2().y; y++) {

			float xStart = getScaledPoint(tri.v0(), CanvasPoint(0, y), tri.v2()).x;
			float xEnd = getScaledPoint(tri.v1(), CanvasPoint(0, y), tri.v2()).x;
			/*for (int i = 0; i <= abs(xStart - xEnd); i++) {
				window.setPixelColour(xStart + i * sign, y, argb);
			}*/

			for (float i = min(xStart,xEnd); i <= max(xStart, xEnd); i++) {
				window.setPixelColour(round(i), round(y), argb);
			}
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
	//// (104.569, 160.75, 0) 1(164.355, 220.229, 0) 1(104.566, 222.512, 0) 1
	// (181.591, 28.8966, 0) 1(137.354, 24.708, 0) 1(182.577, 24.708, 0) 1
	/*points[0] = CanvasPoint(round(104.569), round(160.75));
	points[1] = CanvasPoint(round(164.355), round(220.229));
	points[2] = CanvasPoint(round(104.566), round(222.512));*/
	Colour colour;
	randomColour(colour);


	/*points[0] = CanvasPoint(round(104.569), round(160.75));
	points[1] = CanvasPoint(round(164.355), round(222.229));
	points[2] = CanvasPoint(round(104.566), round(222.12));*/

	/*points[0] = CanvasPoint((104.569), (160.75));
	points[1] = CanvasPoint((164.355), (222.229));
	points[2] = CanvasPoint((104.566), (222.12));*/

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

CanvasPoint getCanvasIntersectionPoint(glm::vec3 vertexPos, glm::vec3 cameraPos, float focalLength) {
	float x, y;

	x = -focalLength * SCALE * (vertexPos[0] / (vertexPos[2] - cameraPos[2])) + (WIDTH / 2);
	y = focalLength * SCALE * (vertexPos[1] / (vertexPos[2] - cameraPos[2])) + (HEIGHT / 2);

	//return CanvasPoint(round(x) , round(y));
	return CanvasPoint(x, y);

}

CanvasPoint getScaledCoord(glm::vec3 vertex) {
	glm::vec3 cameraPosition = glm::vec3(0, 0, 3);
	float focalLength = 2;
	CanvasPoint point = getCanvasIntersectionPoint(vertex, cameraPosition, focalLength);

	return point;
}

void draw(DrawingWindow& window, ModelTriangle& tri, int i) {
	// get the equivalent canvas points in a canvasTriangle
	CanvasTriangle triCan = CanvasTriangle(getScaledCoord(tri.vertices[0]), getScaledCoord(tri.vertices[1]), getScaledCoord(tri.vertices[2]));
	// draw 

	//drawStrokedTriangle(window, triCan);
	//cout << triCan << endl;
	if (i == 5) {
		//drawFilledTriangle(window, triCan, Colour(200,200,200));
	}
	//if(i == 1)
	drawFilledTriangle(window, triCan, tri.colour);
}

void draw(DrawingWindow& window, ModelTriangle& tri) {
	CanvasTriangle triCan = CanvasTriangle(getScaledCoord(tri.vertices[0]), getScaledCoord(tri.vertices[1]), getScaledCoord(tri.vertices[2]));
	drawFilledTriangle(window, triCan, tri.colour);
}

void draw(DrawingWindow &window){
	
	CanvasPoint topLeft = CanvasPoint(0.0f,0.0f);
	CanvasPoint mid = CanvasPoint((float) window.width / 2.0f, (float) window.height/2.0f);
	CanvasPoint botMid = CanvasPoint((float) window.width / 2.0f, (float) window.height - 1.0f);
	CanvasPoint midLeft = CanvasPoint(0, (float) window.height/2.0f);
	CanvasPoint cp = CanvasPoint((float) window.width * 2.0f / 3.0f, (float) window.height/2.0f);
	Colour colour = Colour(255,255,255);
	line(window, topLeft, mid, colour);
	line(window, mid, botMid, colour);
	line(window, midLeft, cp, colour);
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
		//cout << parser.triangles[i] <<  " " << parser.triangles[i].colour << endl;
		//cout << parser.triangles[i].colour << endl;
		draw(window, parser.triangles[i], i);
	}
	//draw(window, parser.triangles[14]);
	//drawTriangle(window);
	//drawTriangle(window);
	window.renderFrame();
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		//window.renderFrame();
	}
}
