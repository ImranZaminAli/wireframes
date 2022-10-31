#pragma once
#include "CanvasPoint.h"
#include "Direction.cpp"
#include <cmath>
#include <glm/glm.hpp>
class Camera{
public:
    glm::vec3 position;
    glm::mat3 orientation;
    glm::vec3 adjustedPos;
    float focalLength;

    void moveCamera(Direction direction, float value);
    Camera();
};