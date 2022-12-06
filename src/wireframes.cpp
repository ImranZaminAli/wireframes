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
#include <TexturePoint.h>
#include <unistd.h>
#include <thread>

// 320 240
#define WIDTH 640
#define HEIGHT 480
#define SCALE 300

using namespace std;

//array<array<float, WIDTH>, HEIGHT> buffer {};
Camera camera = Camera();
Rasteriser rasteriser = Rasteriser();
Parser parser = Parser();
RayTracer rayTracer = RayTracer(WIDTH, HEIGHT);
array<array<float, 640>, 480> buffer{};
DrawMode mode = DrawMode::rayTrace;
DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

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
void write( vector<glm::vec3>* shades, int x, int y, size_t numberOfValues, bool vertical){
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

void makeBorders(glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 botLeft, glm::vec3 botRight, vector<glm::vec3>* rightBorder, vector<glm::vec3>* leftBorder, size_t numberOfValues){
	*rightBorder = interpolateThreeElementValues(topRight, botRight, numberOfValues);
	write( rightBorder, window.width -1, 0, numberOfValues, true);
	*leftBorder = interpolateThreeElementValues(topLeft, botLeft, numberOfValues);
	write( leftBorder, 0,0, numberOfValues, true);
	
	
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

void drawTriangle(ModelTriangle& tri, array<array<float, WIDTH>, HEIGHT>& buffer) {
	CanvasTriangle triCan = CanvasTriangle(getCanvasIntersectionPoint(tri.vertices[0]), getCanvasIntersectionPoint(tri.vertices[1]), getCanvasIntersectionPoint(tri.vertices[2]));
    orderTriPoints(triCan);
	if(mode == DrawMode::fill)
		rasteriser.drawFilledTriangle(window,  triCan, tri.colour, buffer);
    rasteriser.drawStrokedTriangle(window, triCan, tri.colour, buffer);
}

void draw(){

	if (mode == DrawMode::wireframe || mode == DrawMode::fill) {
		for (int i = 0; i < parser.triangles.size(); i++) {
			drawTriangle(parser.triangles[i], buffer);
		}
	}
	else {
		rayTracer.drawRayTracedImage(&window, &parser.triangles, &camera, 0, HEIGHT);
	}

}

void call( float start, float finish){
    rayTracer.drawRayTracedImage(&window, &parser.triangles, &camera, (int) start, (int) finish);
}

void draw(vector<float>& indexes){

    if (mode == DrawMode::wireframe || mode == DrawMode::fill) {
        for (int i = 0; i < parser.triangles.size(); i++) {
            drawTriangle(parser.triangles[i], buffer);
        }
    }
    else {
        /*thread one (rayTracer.drawRayTracedImage(), &window, &parser.triangles, &camera, indexes[0], indexes[1]);
        thread two (rayTracer.drawRayTracedImage(), &window, &parser.triangles, &camera, indexes[1], indexes[2]);
        thread three (rayTracer.drawRayTracedImage(), &window, &parser.triangles, &camera, indexes[2], indexes[3]);
        thread four (rayTracer.drawRayTracedImage(), &window, &parser.triangles, &camera, indexes[3], indexes[4]);
        thread five (rayTracer.drawRayTracedImage(), &window, &parser.triangles, &camera, indexes[4], indexes[5]);
        thread six (rayTracer.drawRayTracedImage(), &window, &parser.triangles, &camera, indexes[5], indexes[6]);*/
        thread one (call,  indexes[0], indexes[1]);
        thread two (call,  indexes[1], indexes[2]);
        thread three (call,indexes[2], indexes[3]);
        thread four (call,  indexes[3], indexes[4]);
        thread five (call,  indexes[4], indexes[5]);
        thread six (call, indexes[5], indexes[6]);


        one.join();
        two.join();
        three.join();
        four.join();
        five.join();
        six.join();
        //rayTracer.drawRayTracedImage(&window, &parser.triangles, &camera);
    }

}

void emptyBuffer() {
	for (int i = 0; i < buffer.size(); i++){
		for (int j = 0; j < buffer[i].size(); j++){
			buffer[i][j] = 0;
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

void handleEvent(SDL_Event event) {
    float stepSize = 0.2;
    float angle = glm::radians(3.0f);
    window.clearPixels();
    if (event.type == SDL_KEYDOWN) {
		emptyBuffer();
		if (event.key.keysym.sym == SDLK_LEFT) {camera.moveCamera(Direction::right, -stepSize);}
		else if (event.key.keysym.sym == SDLK_RIGHT) {camera.moveCamera(Direction::right, stepSize); }
		else if (event.key.keysym.sym == SDLK_UP) {camera.moveCamera(Direction::up, stepSize/2); }
		else if (event.key.keysym.sym == SDLK_DOWN) {camera.moveCamera(Direction::up, -stepSize/2); }
        else if (event.key.keysym.sym == SDLK_w) {camera.moveCamera(Direction::forwards, stepSize); }
		else if (event.key.keysym.sym == SDLK_s) {camera.moveCamera(Direction::forwards, -stepSize);}
        else if (event.key.keysym.sym == SDLK_a) {camera.moveCamera(Direction::rotateX, angle/2);}
        else if (event.key.keysym.sym == SDLK_q) {camera.moveCamera(Direction::rotateX, -angle/2);}
        else if (event.key.keysym.sym == SDLK_e) {camera.moveCamera(Direction::rotateY, -angle);}
		else if (event.key.keysym.sym == SDLK_d) {camera.moveCamera(Direction::rotateY, angle);}
		else if (event.key.keysym.sym == SDLK_p) { cycleMode(); }
        else if (event.key.keysym.sym == SDLK_c) {rayTracer.proximityStrength += 0.2f; cout << "proximity: " << rayTracer.proximityStrength << endl;}
        else if (event.key.keysym.sym == SDLK_v) {rayTracer.proximityStrength -= 0.2f; cout << "proximity: " << rayTracer.proximityStrength << endl;}
        else if (event.key.keysym.sym == SDLK_b) {rayTracer.incidentStrength += 0.2f; cout << "incident: " << rayTracer.incidentStrength << endl;}
        else if (event.key.keysym.sym == SDLK_n) {rayTracer.incidentStrength -= 0.2f; cout << "incident: " << rayTracer.incidentStrength << endl;}
		//camera.lookAt();
        //draw(window);
		//renderOption? draw(window) : rayTracer.drawRayTracedImage(&window, &parser.triangles, &camera);
		draw();
		window.renderFrame();
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		//CanvasPoint point = CanvasPoint(event.button.x, event.button.y, camera.focalLength);
		//rayTracer.trace(rayTracer.getRayDirection(point), camera.position, rayTracer.lightPoint, 0, true);
        cout << "saving\n";
		window.savePPM("output1.ppm");
		cout << "saved\n";
	}
}


int main(int argc, char *argv[]) {
	// uncomment line below for proper random
	//srand((unsigned int) time(NULL));

	SDL_Event event;
    int pauseTime = 25/2;
    vector<float> index = interpolateSingleFloats(0.0f, (float) HEIGHT, 7);
    /// rasteriser
    /*
    for(float i = 0; i < (int) (360/3) + pauseTime + pauseTime; i++){
        draw();
        window.renderFrame();
        buffer[0][0] = 0;

        if(i < pauseTime){
            window.savePPM("frames/rasteriser/" + std::to_string((int) i) + ".ppm");
        }
        else if(i >= pauseTime && i <= 120 + pauseTime) {
            cout << "breaks here\n";
            float angle = glm::radians(3.0);
            camera.moveCamera(Direction::rotateY, angle);
            window.savePPM("frames/rasteriser/" + std::to_string((int) i) + ".ppm");



        }
        else{
            window.savePPM("frames/rasteriser/" + std::to_string((int) i) + ".ppm");
        }
        if(i*3 == 180)
            mode = DrawMode::fill;

        cout << "finished frame: " << i << endl;
        window.clearPixels();
        emptyBuffer();
    }

    mode = DrawMode::rayTrace;*/

    /// lighting
    //cout << 0.4 / 0.02 << endl;
    /*mode = DrawMode::rayTrace;
    int lightMoved = (int) (0.4/0.01);
    draw();
    window.renderFrame();
    for(int i = 0; i < pauseTime + pauseTime + lightMoved; i++){
        

        if(i >= pauseTime && i < pauseTime + lightMoved){
            window.clearPixels();
            rayTracer.lightPoint.x += 0.01;
            draw( index);
            window.renderFrame();
        }
        window.savePPM("frames/shadows/" + std::to_string((int) i) + ".ppm");
        cout << "finished frame: " << i << endl;
    }*/
    /*for(int i = 0; i < )
    for(int i = ; i <= (int) (0.4/0.01) + 25; i++){
        draw(window);
        window.renderFrame();
        window.savePPM("frames/lighting/" + std::to_string((int) i) + ".ppm");

        window.clearPixels();
        cout << "finished frame : " << i << endl;
    }*/



    //draw(window);


    /// soft shadows
    /*mode = DrawMode::rayTrace;
    draw(index);
    window.renderFrame();
    for(int i = 0; i < 25 + 11; i++){
        window.savePPM("frames/shadows/" + std::to_string((int) i) + ".ppm");
        cout << "finished frame: " << i << endl;
    }*/

    //draw(index);
    //window.renderFrame();

    ///bumpMaps
    /*mode = DrawMode::rayTrace;
    camera.moveCamera(Direction::rotateX, glm::radians(21.0f));
    draw(index);
    window.renderFrame();
    cout << pauseTime + 64 << endl;
    for(int i = 0; i <= pauseTime + pauseTime + 64;i++){


        if(i >= pauseTime && i < pauseTime + 64){

            window.clearPixels();
            draw(index);
            window.renderFrame();
            camera.moveCamera(Direction::rotateX, glm::radians(-0.25f));
        }
        else{
            cout << i << endl;
        }
        window.savePPM("frames/bumpMaps/" + std::to_string((int) i) + ".ppm");
        cout << "finished frame: " << i << endl;
    }*/

    ///textureMaps
    /*mode = DrawMode::rayTrace;
    for(int i = 0; i < 84 + (int) (pauseTime/2); i++){
        if(i < 42){
            draw(index);
            window.renderFrame();
            camera.moveCamera(Direction::rotateX, glm::radians(0.25f));
        }
        window.savePPM("frames/texture/"+ std::to_string((int) i) + ".ppm");
        cout << "finished frame: " << i << endl;
    }*/
    //camera.moveCamera(Direction::rotateX, glm::radians(5.0f));

    /*camera.moveCamera(Direction::forwards, -2.0f);
    camera.moveCamera(Direction::rotateY, glm::radians(3.0 * 10));
    camera.moveCamera(Direction::up, 0.4f);
    camera.moveCamera(Direction::rotateX, glm::radians(6.0));
    camera.moveCamera(Direction::right, 0.2f);*/


    /*float forwardStepSize = -2.0f/120.f;
    for(int i = 0; i < 120;i++){
        camera.moveCamera(Direction::forwards, forwardStepSize);
        camera.moveCamera(Direction::rotateY, glm::radians(0.25));
        camera.moveCamera(Direction::up, 0.4f / 120.0f);
        camera.moveCamera(Direction::rotateX, glm::radians(6.0 / 120.f));
        camera.moveCamera(Direction::right, 0.2f/120.f);
    }

    //camera.moveCamera(Direction::up, 0.4f);
    //camera.moveCamera(Direction::right, 0.2f);
    draw(index);
	window.renderFrame();*/


    //window.savePPM("output.ppm");
    int frameIndex = 0;
    draw(index);
    window.renderFrame();
    for(int i = 0; i < pauseTime; i++){
        window.savePPM("frames/mirrors/" + std::to_string((int) frameIndex) + ".ppm");
        frameIndex++;
        cout << "finished frame: " << frameIndex << endl;
    }

    for(float i = 0; i < 16 * 3;i++){
        camera.moveCamera(Direction::forwards, -.049f);
        camera.moveCamera(Direction::rotateY, glm::radians(1.0f));

        draw(index);
        window.renderFrame();
        window.savePPM("frames/mirrors/" + std::to_string((int) frameIndex) + ".ppm");
        frameIndex++;
        cout << "finished frame: " << frameIndex << endl;
    }

    /*for(float i = 0; i < 5 * 3; i++){
        camera.moveCamera
    }*/

    for(float i = 0; i < 4/0.25f; i++){
        camera.moveCamera(Direction::rotateX, glm::radians(1.5f * .25f));
        camera.moveCamera(Direction::up, 0.1f * .25f);
        draw(index);
        window.renderFrame();
        window.savePPM("frames/mirrors/" + std::to_string((int) frameIndex) + ".ppm");
        frameIndex++;
        cout << "finished frame: " << frameIndex << endl;
    }

    for(int i = 0; i < pauseTime; i++){
        window.savePPM("frames/mirrors/" + std::to_string((int) frameIndex) + ".ppm");
        frameIndex++;
        cout << "finished frame: " << frameIndex << endl;
    }

    cout << "FINISHED\n";
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		//window.renderFrame();
	}


	return 0;

}
