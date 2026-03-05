#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "glm.hpp"

#define RADIUS 20.0f
#define PITCH 0.0f
#define YAW 90.0f
#define SPEED 60.0f
#define ZOOM 45.0f
#define WORLD_UP glm::vec3(0.0f, 1.0f, 0.0f)


class Camera
{
    public:
        glm::vec3 position;
        glm::vec3 target;
        glm::vec3 front;
        glm::vec3 up;
        glm::vec3 right;
        glm::vec3 worldup;

        float radius;
        float yaw;
        float pitch;

        float speed;
        float zoom;

        enum Movement { ORBIT_LEFT, ORBIT_RIGHT, ORBIT_UP, ORBIT_DOWN };

        Camera(glm::vec3 target): target(target), worldup(WORLD_UP), radius(RADIUS), yaw(YAW), pitch(PITCH), speed(SPEED), zoom(ZOOM)
        {
            updateCameraVectors();
        }

        glm::mat4 GetViewMatrix() const
        {
            return glm::lookAt(position, target, up);
        }

        void ProcessKeyboard(Movement direction, float deltaTime)
        {
            float angle = speed * deltaTime;
            if (direction == ORBIT_LEFT)
                yaw -= angle;
            if (direction == ORBIT_RIGHT)
                yaw += angle;
            if (direction == ORBIT_UP)
                pitch += angle;
            if (direction == ORBIT_DOWN)
                pitch -= angle;
            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;

            updateCameraVectors();
        }

        void ProcessMouseScroll(float yoffset)
        {
            radius -= yoffset;
            if (radius <  1.0f)
                radius =  1.0f;
            if (radius > 50.0f)
                radius = 50.0f;
            updateCameraVectors();
        }


    private:
        void updateCameraVectors()
        {
            float yawRad   = yaw   * (PI / 180.0f);
            float pitchRad = pitch * (PI / 180.0f);

            glm::vec3 offset;
            offset.x = radius * std::cos(pitchRad) * std::cos(yawRad);
            offset.y = radius * std::sin(pitchRad);
            offset.z = radius * std::cos(pitchRad) * std::sin(yawRad);

            position = glm::vec3(target.x + offset.x,
                                 target.y + offset.y,
                                 target.z + offset.z);

            float len = std::sqrt(offset.x*offset.x + offset.y*offset.y + offset.z*offset.z);
            if (len > 0.0f)
                front = glm::vec3(-offset.x / len, -offset.y / len, -offset.z / len);

            glm::vec3 right;
            right.x = front.y * worldup.z - front.z * worldup.y;
            right.y = front.z * worldup.x - front.x * worldup.z;
            right.z = front.x * worldup.y - front.y * worldup.x;
            len = std::sqrt(right.x*right.x + right.y*right.y + right.z*right.z);
            if (len > 0.0f) { right.x /= len; right.y /= len; right.z /= len; }
            this->right = right;

            glm::vec3 up;
            up.x = right.y * front.z - right.z * front.y;
            up.y = right.z * front.x - right.x * front.z;
            up.z = right.x * front.y - right.y * front.x;
            len = std::sqrt(up.x*up.x + up.y*up.y + up.z*up.z);
            if (len > 0.0f) { up.x /= len; up.y /= len; up.z /= len; }
            this->up = up;
        }
};

#endif
