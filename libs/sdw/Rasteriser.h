#pragma once

#include "DrawingWindow.h"
#include "CanvasPoint.h"
#include "CanvasTriangle.h"
#include "Colour.h"
#include "TexturePoint.h"
#include "TextureMap.h"
#include <cmath>
#include <array>
#include <glm/glm.hpp>

#define WIDTH 640
#define HEIGHT 480
// -4.76395
class Rasteriser{
    std::vector<uint32_t> pixels;
    float textureHeight;
    float textureWidth;
    void getLineVariables(float &numberOfSteps, float &xStep, float &yStep, float &depthStep, CanvasPoint start, CanvasPoint finish);
    CanvasPoint getScaledPoint(CanvasPoint a, CanvasPoint b, CanvasPoint c);
    bool checkAndUpdateBuffer(float x, float y, float depth, std::array<std::array<float, WIDTH>, HEIGHT> &buffer);
    glm::vec3 pointToVec(CanvasPoint& point);
    TexturePoint barycentricToCartesian(CanvasTriangle& tri, float u, float v, float w);
    void cartesianToBarycentric(CanvasTriangle& tri, CanvasPoint& point, float overallArea,  float& u, float& v, float& w);
    float calculateArea(CanvasTriangle& tri);
public:
    Rasteriser();
    void drawLine(DrawingWindow& window, CanvasPoint& start, CanvasPoint& finish, Colour& colour, std::array<std::array<float, WIDTH>, HEIGHT> &buffer);
    void drawTexturedLine(DrawingWindow& window, CanvasTriangle& tri, float area, CanvasPoint& start, CanvasPoint& finish);
    void drawStrokedTriangle(DrawingWindow& window, CanvasTriangle& tri, Colour& colour , std::array<std::array<float, WIDTH>, HEIGHT>& buffer);
    void drawFilledTriangle(DrawingWindow& window, CanvasTriangle &tri, Colour &colour, std::array<std::array<float, WIDTH>, HEIGHT>& buffer);
    void drawTexturedTriangle(DrawingWindow& window, CanvasTriangle& tri, std::array<std::array<float, WIDTH>, HEIGHT>& buffer);
};