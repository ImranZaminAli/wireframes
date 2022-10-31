#pragma once
#include "CanvasPoint.h"
#include "Direction.cpp"
#include <glm/glm.hpp>
class Camera{
public:
    glm::vec3 position;
    float focalLength;

    void translateObject(Direction direction, float step);
    Camera();
};