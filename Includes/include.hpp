#ifndef INCLUDE_HPP
#define INCLUDE_HPP

#define GLFW_INCLUDE_NONE
#include "glad.h"
#include <GLFW/glfw3.h>
#include "khrplatform.h"
#include <iostream>
#include <vector>
#include "shader.h"
// Minimal replacement for external GLM usage in this project
#include "glm.hpp"
#include "FileSystem.hpp"
#include "stb_image.h"
#include "Camera.hpp"


enum BodyPartType {
    HEAD,
    TORSO,
    ARM,
    LEG
};

class bodyPart {
    private:
        // this is the coordinates of each body part
        float x;
        float y;
        float z;
        glm::vec3 initialPosition;
        BodyPartType partType;

    public:
        bodyPart(float x, float y, float z, BodyPartType partType) : x(x), y(y), z(z), initialPosition(x, y, z), partType(partType) {}

        float getX() const { return x; }
        float getY() const { return y; }
        float getZ() const { return z; }
        glm::vec3 getInitialPosition() const { return initialPosition; }
        BodyPartType getPartType() const { return partType; }

        glm::vec3 getPosition() { return glm::vec3(x, y, z); }

        int operator==(const bodyPart& other) const {
            return (partType == other.partType);
        }
};

class body {
    private:
        std::vector<bodyPart> parts;
    public:
        void addPart(const bodyPart& part) {
            parts.push_back(part);
        }

        const std::vector<bodyPart>& getParts() const {
            return parts;
        }

        void draw_head(Shader& ourShader) {
            for (const auto& part : parts) {
                if (part.getPartType() == BodyPartType::HEAD) {
                    float x = part.getX();
                    float y = part.getY();
                    float z = part.getZ();
                    glm::vec3 position = {x, y, z};
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, position);

                    ourShader.setMat4("model", model);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                    {
                        GLenum _err = glGetError();
                        if (_err != GL_NO_ERROR)
                        std::cout << "GL error after draw: " << _err << std::endl;
                    }
                }
            }
        }

        void draw_body(Shader& ourShader) {
            for (const auto& part : parts) {
                if (part.getPartType() == BodyPartType::TORSO) {
                    float x = part.getX();
                    float y = part.getY();
                    float z = part.getZ();
                    glm::vec3 position = {x, y, z};
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, position);
                    
                    ourShader.setMat4("model", model);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                    {
                        GLenum _err = glGetError();
                        if (_err != GL_NO_ERROR)
                        std::cout << "GL error after draw: " << _err << std::endl;
                    }
                }
            }
        }
        
        void draw_arm(Shader& ourShader) {
            for (const auto& part : parts) {
                if (part.getPartType() == BodyPartType::ARM) {
                    float x = part.getX();
                    float y = part.getY();
                    float z = part.getZ();
                    glm::vec3 position = {x, y, z};
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, position);
                    
                    ourShader.setMat4("model", model);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                    {
                        GLenum _err = glGetError();
                        if (_err != GL_NO_ERROR)
                        std::cout << "GL error after draw: " << _err << std::endl;
                    }
                }
            }
        }
        
        void draw_leg(Shader& ourShader) {
            for (const auto& part : parts) {
                if (part.getPartType() == BodyPartType::LEG) {
                    float x = part.getX();
                    float y = part.getY();
                    float z = part.getZ();
                    glm::vec3 position = {x, y, z};
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, position);
                    
                    ourShader.setMat4("model", model);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                    {
                        GLenum _err = glGetError();
                        if (_err != GL_NO_ERROR)
                            std::cout << "GL error after draw: " << _err << std::endl;
                    }
                }
            }
        }
};

#endif