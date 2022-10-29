#pragma once
#include "DrawingWindow.h"
#include "CanvasPoint.h"
#include "CanvasTriangle.h"
#include "Colour.h"
#include <cmath>
#include <array>
#define WIDTH 1000
#define HEIGHT 1000

class Rasteriser{
    std::array<std::array<float, WIDTH>,HEIGHT> buffer;
    void getLineVariables(float &numberOfSteps, float &xStep, float &yStep, float &depthStep, CanvasPoint start, CanvasPoint finish);
    CanvasPoint getScaledPoint(CanvasPoint a, CanvasPoint b, CanvasPoint c);
    bool checkAndUpdateBuffer(float x, float y, float depth);
public:
    Rasteriser(std::array<std::array<float, WIDTH>, HEIGHT>& buff);
    void drawLine(DrawingWindow& window, CanvasPoint& start, CanvasPoint& finish, Colour& colour);
    void drawStrokedTriangle(DrawingWindow& window, CanvasTriangle& tri, Colour& colour );
    void drawFilledTriangle(DrawingWindow& window, CanvasTriangle &tri, Colour &colour);
};