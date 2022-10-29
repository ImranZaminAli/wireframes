#include "Rasteriser.h"


Rasteriser::Rasteriser(std::array<std::array<float, WIDTH>, HEIGHT>& buff) {
    buffer = buff;
}

void Rasteriser::getLineVariables(float &numberOfSteps, float &xStep, float &yStep, float &depthStep, CanvasPoint start, CanvasPoint finish) {
    float xDiff = finish.x - start.x;
    float yDiff = finish.y - start.y;
    float depthDiff = finish.depth - start.depth;
    numberOfSteps = std::max(std::max(std::abs(xDiff), std::abs(yDiff)), std::abs(depthDiff));

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

bool Rasteriser::checkAndUpdateBuffer(float x, float y, float depth) {
    float reciprocal = 1/depth;
    if(buffer[y][x] == 0 || reciprocal > buffer[y][x]){
        buffer[y][x] = reciprocal;
        return true;
    }

    return false;
}

void Rasteriser::drawLine(DrawingWindow &window, CanvasPoint &start, CanvasPoint &finish, Colour &colour) {
    float numberOfSteps, xStep, yStep, depthStep;
    getLineVariables(numberOfSteps, xStep, yStep, depthStep, start, finish);
    for(float i = 0.0f; i < numberOfSteps; i++){
        float x = start.x + xStep * i;
        float y = start.y + yStep * i;
        float depth = start.depth + depthStep * i;
        if(checkAndUpdateBuffer(ceil(x), floor(y), depth))
            window.setPixelColour(ceil(x), floor(y), colour.getArbg());
    }
}

void Rasteriser::drawStrokedTriangle(DrawingWindow &window, CanvasTriangle &tri, Colour &colour) {
    drawLine(window, tri.v0(), tri.v1(), colour);
    drawLine(window, tri.v1(), tri.v2(), colour);
    drawLine(window, tri.v0(), tri.v2(), colour);
}

void Rasteriser::drawFilledTriangle(DrawingWindow &window, CanvasTriangle &tri, Colour &colour) {
    CanvasPoint intersect = getScaledPoint(tri.v0(), tri.v1(), tri.v2());

    for (float y = tri.v0().y; y <= intersect.y; y++) {
        CanvasPoint start = getScaledPoint(tri.v0(), CanvasPoint(0, y), tri.v2());
        CanvasPoint end = getScaledPoint(tri.v0(), CanvasPoint(0, y), tri.v1());
        if (end.x == INFINITY) {
            intersect = tri.v0();
            break;
        }
        drawLine(window, start, end, colour);
    }

    for (float y = intersect.y; y <= tri.v2().y; y++) {
        CanvasPoint start = getScaledPoint(tri.v0(), CanvasPoint(0, y), tri.v2());
        CanvasPoint end = getScaledPoint(tri.v1(), CanvasPoint(0, y), tri.v2());
        drawLine(window, start, end, colour);
    }
}