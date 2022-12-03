#include "Rasteriser.h"

Rasteriser::Rasteriser() {
    TextureMap map = TextureMap("C:\\Users\\izami\\Documents\\UoBYr3\\wireframes\\texture.ppm");
    pixels = map.pixels;
    textureHeight = map.height;
    textureWidth = map.width;
}

void Rasteriser::getLineVariables(float &numberOfSteps, float &xStep, float &yStep, float &depthStep, CanvasPoint start, CanvasPoint finish) {
    
    float xDiff = finish.x - start.x;
    float yDiff = finish.y - start.y;
    float depthDiff = finish.depth - start.depth;
    
    numberOfSteps = std::max(std::fabs(xDiff), std::fabs(yDiff));
    
    xStep = xDiff / numberOfSteps;
    yStep = yDiff / numberOfSteps;
    depthStep = depthDiff / numberOfSteps;
}

CanvasPoint Rasteriser::getScaledPoint(CanvasPoint a, CanvasPoint b, CanvasPoint c) {
    float scale = (float)(a.y - b.y) / (float)(a.y - c.y);
    float x = round((float)((c.x - a.x) * scale)) + a.x;
    float depth = (float)((c.depth - a.depth) * scale) + a.depth;
    b.x = x;
    b.depth = depth;
    return b;
}

bool Rasteriser::checkAndUpdateBuffer(float x, float y, float depth, std::array<std::array<float, WIDTH>, HEIGHT> &buffer) {
    float reciprocal = 1/depth;
    if((y < HEIGHT && y >= 0 && x < WIDTH && x >= 0) && (reciprocal > buffer[y][x])){
        buffer[y][x] = reciprocal;
        return true;
    }

    return false;
}

glm::vec3 Rasteriser::pointToVec(CanvasPoint& point) {
    return glm::vec3(point.x, point.y, point.depth);
}

float Rasteriser::calculateArea(CanvasTriangle& tri) {
    glm::vec3 v0 = pointToVec(tri.v0());
    glm::vec3 v1 = pointToVec(tri.v1());
    glm::vec3 v2 = pointToVec(tri.v2());
    //std::cout << tri << std::endl;
    glm::vec3 normal = glm::cross(v1 - v0, v2 - v0);
    //std::cout << normal[0] << std::endl;
    return glm::distance(normal, glm::vec3(0, 0, 0)) * 0.5f;
}

TexturePoint Rasteriser::barycentricToCartesian(CanvasTriangle& tri, float u, float v, float w) {
    float x = tri.v0().texturePoint.x * u + tri.v1().texturePoint.x * v + tri.v2().texturePoint.x * w;
    float y = tri.v0().texturePoint.y * u + tri.v1().texturePoint.y * v + tri.v2().texturePoint.y * w;

    return TexturePoint(x, y);
}

void Rasteriser::cartesianToBarycentric(CanvasTriangle& tri, CanvasPoint& point, float overallArea, float& u, float& v, float& w) {
    CanvasTriangle sector = CanvasTriangle(tri.v0(), tri.v1(), point);
    //std::cout << calculateArea(sector) << std::endl;
    u = calculateArea(sector) / overallArea;
    sector = CanvasTriangle(tri.v0(), tri.v2(), point);
    v = calculateArea(sector) / overallArea;
    w = 1.0f - u - v;
}




void Rasteriser::drawLine(DrawingWindow &window, CanvasPoint &start, CanvasPoint &finish, Colour &colour, std::array<std::array<float, WIDTH>, HEIGHT> &buffer) {
    float numberOfSteps, xStep, yStep, depthStep;
    getLineVariables(numberOfSteps, xStep, yStep, depthStep, start, finish);
    for(float i = 0.0f; i <= numberOfSteps; i++){
        float x = start.x + xStep * i;
        float y = start.y + yStep * i;
        float depth = start.depth + depthStep * i;
        float newDepth = 1 + std::exp(-(start.depth + depthStep * i));
        //float depth = start.depth + depthStep * i;
        if(depth < 0 && checkAndUpdateBuffer(floor(x), ceil(y), newDepth, buffer))
            window.setPixelColour(floor(x), ceil(y), colour.getArbg());
    }
}

void Rasteriser::drawTexturedLine(DrawingWindow& window, CanvasTriangle& tri, float area, CanvasPoint& start, CanvasPoint& finish) {
    float numberOfSteps, xStep, yStep, depthStep;
    getLineVariables(numberOfSteps, xStep, yStep, depthStep, start, finish);
    for (float i = 0.0f; i <= numberOfSteps; i++) {
        //std::cout << xStep << " " << yStep << std::endl;
        float x = start.x + xStep * i;
        float y = start.y + yStep * i;
        CanvasPoint p = CanvasPoint(x, y, 0);
        //Colour colour;
        /*
            1. get the barycentric coord
            2. find corresponding cartesian coord in texture map
            3. get its colour and write pixel to screen
        */

        float u, v, w;
        //std::cout << p << std::endl;
        cartesianToBarycentric(tri, p, area, u, v, w);
        //std::cout << area << " " << u << " " << v << " " << w << std::endl;
        TexturePoint texturePoint = barycentricToCartesian(tri, u, v, w);
        //std::cout << texturePoint << std::endl << pixels.size();
        int index = texturePoint.x * (textureWidth) + texturePoint.x;
        window.setPixelColour(floor(x), ceil(y), pixels[index]);
    }
}

void Rasteriser::drawStrokedTriangle(DrawingWindow &window, CanvasTriangle &tri, Colour &colour, std::array<std::array<float, WIDTH>, HEIGHT> &buffer) {
    drawLine(window, tri.v0(), tri.v1(), colour, buffer);
    drawLine(window, tri.v1(), tri.v2(), colour, buffer);
    drawLine(window, tri.v0(), tri.v2(), colour, buffer);
}

void Rasteriser::drawTexturedTriangle(DrawingWindow& window, CanvasTriangle& tri, std::array< std::array<float, WIDTH>, HEIGHT>& buffer) {
    CanvasPoint intersect = getScaledPoint(tri.v0(), tri.v1(), tri.v2());
    float area = calculateArea(tri);
    for (float y = tri.v0().y+1; y <= intersect.y; y++) {
        CanvasPoint start = getScaledPoint(tri.v0(), CanvasPoint(0, y), tri.v2());
        CanvasPoint end = getScaledPoint(tri.v0(), CanvasPoint(0, y), tri.v1());
        if (end.x == INFINITY) {
            intersect = tri.v0();
            break;
        }
        
        //std::cout << start << " " << end << std::endl;
        drawTexturedLine(window, tri, area, start, end);
        //drawLine(window, start, end, Colour(255, 255, 255), buffer);
    }

    //for (float y = intersect.y+1; y <= tri.v2().y; y++) {
    //    CanvasPoint start = getScaledPoint(tri.v0(), CanvasPoint(0, y), tri.v2());
    //    CanvasPoint end = getScaledPoint(tri.v1(), CanvasPoint(0, y), tri.v2());
    //    //drawLine(window, start, end, Colour(255, 255, 255), buffer);
    //    //drawTexturedLine(window, tri, area, start, end);
    //}
}

void Rasteriser::drawFilledTriangle(DrawingWindow &window, CanvasTriangle &tri, Colour &colour, std::array<std::array<float, WIDTH>, HEIGHT> &buffer) {
    CanvasPoint intersect = getScaledPoint(tri.v0(), tri.v1(), tri.v2());

    for (float y = tri.v0().y; y <= intersect.y; y++) {
        CanvasPoint start = getScaledPoint(tri.v0(), CanvasPoint(0, y), tri.v2());
        CanvasPoint end = getScaledPoint(tri.v0(), CanvasPoint(0, y), tri.v1());
        if (end.x == INFINITY) {
            intersect = tri.v0();
            break;
        }

        //std::cout << start << " " << end << std::endl;
        drawLine(window, start, end, colour, buffer);
    }

    for (float y = intersect.y; y <= tri.v2().y; y++) {
        CanvasPoint start = getScaledPoint(tri.v0(), CanvasPoint(0, y), tri.v2());
        CanvasPoint end = getScaledPoint(tri.v1(), CanvasPoint(0, y), tri.v2());
        drawLine(window, start, end, colour, buffer);
    }
}