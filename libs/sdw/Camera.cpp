#include "Camera.h"

Camera::Camera() {
    position = glm::vec3(0,0,3);
    focalLength = 2;
}

void Camera::translateObject(Direction direction, float step) {
    switch (direction) {
        case Direction::forwards:
            position = position + glm::vec3(0,0,step);
            break;
        case Direction::right:
            position = position + glm::vec3(step,0,0);
            break;
        case Direction::up:
            position = position + glm::vec3(0, step,0);
            std::cout << position[1] << std::endl;
            break;
        
    }

    //std::cout << position[2] << std::endl;
}