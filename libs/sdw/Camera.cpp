#include "Camera.h"

Camera::Camera() {
    position = glm::vec3(0,0,3);
    orientation = glm::mat3(glm::vec3(1,0,0),
                            glm::vec3(0,1,0),
                            glm::vec3(0,0,1));
    adjustedPos = glm::vec3(0,0,3);
    focalLength = 2;
}

void Camera::moveCamera(Direction direction, float value) {
    float sin, cos;
    glm::mat3 rotation;
    switch (direction) {
        case Direction::forwards:
            position = position + glm::vec3(0,0,value);
            break;
        case Direction::right:
            position = position + glm::vec3(value,0,0);
            break;
        case Direction::up:
            position = position + glm::vec3(0, value,0);
            break;
        case Direction::rotateX:
            sin = std::sin(value);
            cos = std::cos(value);
            rotation = glm::mat3(glm::vec3(1,0,0),
                                            glm::vec3(0, cos, -sin),
                                            glm::vec3(0, sin, cos));
            orientation = orientation * rotation;
            break;
        case Direction::rotateY:
            sin = std::sin(value);
            cos = std::cos(value);
            rotation = glm::mat3(glm::vec3(cos, 0, sin),
                                           glm::vec3(0,1,0),
                                           glm::vec3(-sin, 0, cos));
            orientation = orientation * rotation;
            break;
        default:
            break;
        //std::cout << "here\n";
        //adjustedPos = orientation * position;
    }

    //std::cout << position[2] << std::endl;
}