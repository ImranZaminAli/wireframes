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
#include <Camera.h>
#include <Rasteriser.h>
#include <Enums.cpp>
#include <RayTracer.h>
// 320 240
#define WIDTH 400
#define HEIGHT 400
#define SCALE 300

using namespace std;

//array<array<float, WIDTH>, HEIGHT> buffer {};
Camera camera = Camera();
Rasteriser rasteriser = Rasteriser();
Parser parser = Parser();
RayTracer rayTracer = RayTracer(WIDTH, HEIGHT);
array<array<float, WIDTH>, HEIGHT> buffer{};
DrawMode mode = DrawMode::wireframe;


vector<float> interpolateSingleFloats(float from, float to, size_t numberOfValues){
	vector<float> result;
	float increment = (to - from)/(numberOfValues-1);
	
	for(size_t i = 0; i < numberOfValues; i++){
		result.push_back(from);
		from += increment;
	}

	return result;
}

vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, size_t numberOfValues){
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

void randomColour(Colour &colour){
	int colours[3];
	for(int i = 0; i < 3; i++){
		colours[i] = rand() % 256;
	}
	colour = Colour(colours[0], colours[1], colours[2]);
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


CanvasPoint getCanvasIntersectionPoint(glm::vec3 vertexPos) {
	float x, y, depth;
    glm::vec3 intersection = (vertexPos - camera.position) * camera.orientation;
    x = -camera.focalLength * SCALE * (intersection[0] / intersection[2]) + (WIDTH / 2);
    y = camera.focalLength * SCALE * (intersection[1] / intersection[2]) + (HEIGHT / 2);

    depth = camera.focalLength * (intersection[2]);
    return CanvasPoint(x, y, depth);

}

void drawTriangle(DrawingWindow& window, ModelTriangle& tri, array<array<float, HEIGHT>, WIDTH>& buffer) {
	CanvasTriangle triCan = CanvasTriangle(getCanvasIntersectionPoint(tri.vertices[0]), getCanvasIntersectionPoint(tri.vertices[1]), getCanvasIntersectionPoint(tri.vertices[2]));
    orderTriPoints(triCan);
	if(mode == DrawMode::fill)
		rasteriser.drawFilledTriangle(window, triCan, tri.colour, buffer);
    rasteriser.drawStrokedTriangle(window, triCan, tri.colour, buffer);
}

void draw(DrawingWindow &window){
    //array<array<float, WIDTH>, HEIGHT> buffer {};

	/*switch (mode)
	{
	case wireframe:
		for (int i = 0; i < parser.triangles.size(); i++) {
			
		}
		break;
	case fill:
		for (int i = 0; i < parser.triangles.size(); i++) {
			drawTriangle(window, parser.triangles[i], buffer);
		}
		break;
	case rayTrace:
		break;
	default:
		break;
	}*/

	if (mode == DrawMode::wireframe || mode == DrawMode::fill) {
		for (int i = 0; i < parser.triangles.size(); i++) {
			drawTriangle(window, parser.triangles[i], buffer);
		}
	}
	else {
		rayTracer.drawRayTracedImage(&window, &parser.triangles, &camera);
	}

    
}

void emptyBuffer() {
	for (int i = 0; i < HEIGHT; i++){
		for (int j = 0; j < WIDTH; j++){
			buffer[j][i] = 0;
		}
	}
}

void cycleMode() {
	if (mode == DrawMode::wireframe)
		mode = DrawMode::fill;
	else if (mode == DrawMode::fill)
		mode = DrawMode::rayTrace;
	else
		mode = DrawMode::wireframe;
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
    float stepSize = 0.2;
    float angle = glm::radians(2.5);
    window.clearPixels();
    if (event.type == SDL_KEYDOWN) {
		emptyBuffer();
		if (event.key.keysym.sym == SDLK_LEFT) {camera.moveCamera(Direction::right, -stepSize);}
		else if (event.key.keysym.sym == SDLK_RIGHT) {camera.moveCamera(Direction::right, stepSize); }
		else if (event.key.keysym.sym == SDLK_UP) {camera.moveCamera(Direction::up, stepSize); }
		else if (event.key.keysym.sym == SDLK_DOWN) {camera.moveCamera(Direction::up, -stepSize); }
        else if (event.key.keysym.sym == SDLK_w) {camera.moveCamera(Direction::forwards, stepSize); }
		else if (event.key.keysym.sym == SDLK_s) {camera.moveCamera(Direction::forwards, -stepSize);}
        else if (event.key.keysym.sym == SDLK_a) {camera.moveCamera(Direction::rotateX, angle);}
		else if (event.key.keysym.sym == SDLK_d) {camera.moveCamera(Direction::rotateY, angle);}
		else if (event.key.keysym.sym == SDLK_p) { cycleMode(); }
		//camera.lookAt();
        
        //draw(window);
		//renderOption? draw(window) : rayTracer.drawRayTracedImage(&window, &parser.triangles, &camera);
		draw(window);
		window.renderFrame();
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
	//draw(window);

	//array<array<float, WIDTH>, HEIGHT> buffer{};
	//for (int i = 0; i < parser.triangles.size(); i++) {
	//	cout << i << endl;
	//	drawTriangle(window, parser.triangles[i], buffer);
	//}
	//bool method = false;
	draw(window);
	//rayTracer.drawRayTracedImage(&window, &parser.triangles, &camera);
	//draw(window);

	window.renderFrame();
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		//window.renderFrame();
	}


	return 0;

}
