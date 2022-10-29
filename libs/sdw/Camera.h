#pragma once
#include "CanvasPoint.h"
#include <glm/glm.hpp>
class Camera{
public:
    glm::vec3 position;
    float focalLength;
    Camera();
};