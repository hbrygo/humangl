#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "glm.hpp"

class Camera {
    public:
        enum Movement { FORWARD, BACKWARD, LEFT, RIGHT };

        // ctor with vectors (stub - you will implement behavior)
        Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
            float yaw = -90.0f,
            float pitch = 0.0f)
            : Position(position), Front(glm::vec3(0.0f, 0.0f, -1.0f)),
            Up(up), Right(glm::vec3(0.0f,0.0f,0.0f)), WorldUp(up),
            Yaw(yaw), Pitch(pitch), MovementSpeed(2.5f), MouseSensitivity(0.1f), Zoom(45.0f)
        {
            updateCameraVectors();
        }

        // returns the view matrix (stub)
        glm::mat4 GetViewMatrix() const {
            glm::mat4 view = glm::lookAt(Position, Position + Front, Up);

            return view;
        }

        // input processing (stubs)
        void ProcessKeyboard(Movement direction, float deltaTime) {
            // std::cout << "In ProcessKeyboard -> Camera position: (" << Position.x << ", " << Position.y << ", " << Position.z << ")" << std::endl;
            float velocity = MovementSpeed * deltaTime;
            if (direction == FORWARD)
                Position = glm::vec3(Position.x + Front.x * velocity,
                                     Position.y + Front.y * velocity,
                                     Position.z + Front.z * velocity);
            if (direction == BACKWARD)
                Position = glm::vec3(Position.x - Front.x * velocity,
                                     Position.y - Front.y * velocity,
                                     Position.z - Front.z * velocity);
            if (direction == LEFT)
                Position = glm::vec3(Position.x - Right.x * velocity,
                                     Position.y - Right.y * velocity,
                                     Position.z - Right.z * velocity);
            if (direction == RIGHT)
                Position = glm::vec3(Position.x + Right.x * velocity,
                                     Position.y + Right.y * velocity,
                                     Position.z + Right.z * velocity);
        }

        void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
            xoffset *= MouseSensitivity;
            yoffset *= MouseSensitivity;

            Yaw   += xoffset;
            Pitch += yoffset;

            // make sure that when pitch is out of bounds, screen doesn't get flipped
            if (constrainPitch)
            {
                if (Pitch > 89.0f)
                    Pitch = 89.0f;
                if (Pitch < -89.0f)
                    Pitch = -89.0f;
            }

            // std::cout << "Before -> Camera position: (" << Position.x << ", " << Position.y << ", " << Position.z << ")" << std::endl;
            // update Front, Right and Up Vectors using the updated Euler angles
            updateCameraVectors();
            // std::cout << "After -> Camera position: (" << Position.x << ", " << Position.y << ", " << Position.z << ")" << std::endl;
        }

        void ProcessMouseScroll(float yoffset) {
            Zoom -= yoffset;
            if (Zoom < 1.0f)
                Zoom = 1.0f;
            if (Zoom > 45.0f)
                Zoom = 45.0f; 
        }

        // camera attributes
        glm::vec3 Position;
        glm::vec3 Front;
        glm::vec3 Up;
        glm::vec3 Right;
        glm::vec3 WorldUp;

        // euler angles
        float Yaw;
        float Pitch;

        // camera options
        float MovementSpeed;
        float MouseSensitivity;
        float Zoom;

    private:
        void updateCameraVectors() {
            // calculate the new Front vector
            float yawRad = Yaw * (PI / 180.0f);
            float pitchRad = Pitch * (PI / 180.0f);
            glm::vec3 front;
            front.x = std::cos(yawRad) * std::cos(pitchRad);
            front.y = std::sin(pitchRad);
            front.z = std::sin(yawRad) * std::cos(pitchRad);
            // normalize front
            float len = std::sqrt(front.x*front.x + front.y*front.y + front.z*front.z);
            if (len > 0.0f) { front.x /= len; front.y /= len; front.z /= len; }
            Front = front;
            // also re-calculate the Right and Up vector
            // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
            glm::vec3 right;
            right.x = Front.y * WorldUp.z - Front.z * WorldUp.y;
            right.y = Front.z * WorldUp.x - Front.x * WorldUp.z;
            right.z = Front.x * WorldUp.y - Front.y * WorldUp.x;
            len = std::sqrt(right.x*right.x + right.y*right.y + right.z*right.z);
            if (len > 0.0f) { right.x /= len; right.y /= len; right.z /= len; }
            Right = right;

            glm::vec3 up;
            up.x = Right.y * Front.z - Right.z * Front.y;
            up.y = Right.z * Front.x - Right.x * Front.z;
            up.z = Right.x * Front.y - Right.y * Front.x;
            len = std::sqrt(up.x*up.x + up.y*up.y + up.z*up.z);
            if (len > 0.0f) { up.x /= len; up.y /= len; up.z /= len; }
            Up = up;
        }
};

#endif
