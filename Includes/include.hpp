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
#include <map>

#define SIZE 1
// Allow using glm::vec3 as key in ordered containers (std::map, std::set) by
// providing a strict-weak-ordering comparator specialization for std::less.
// This performs a lexicographic compare on (x, y, z).
namespace std {
    template<>
    struct less<glm::vec3> {
        bool operator()(glm::vec3 const& a, glm::vec3 const& b) const noexcept {
            if (a.x < b.x) return true;
            if (b.x < a.x) return false;
            if (a.y < b.y) return true;
            if (b.y < a.y) return false;
            return a.z < b.z;
        }
    };
}

enum BodyPartType {
    HEAD,
    TORSO,
    LEFT_UPPER_ARM,
    LEFT_LOWER_ARM,
    RIGHT_UPPER_ARM,
    RIGHT_LOWER_ARM,
    LEFT_THIGH, // leg
    RIGHT_THIGH, // leg
    LEFT_LOWER_LEG, // leg
    RIGHT_LOWER_LEG, // leg
    WALL
};

class bodyPart {
    private:
        // this is the coordinates of each body part
        float x;
        float y;
        float z;
        glm::vec3 initialPosition;
        BodyPartType partType;
        glm::vec3 scaleVec;
        // dans la map: 0 = plus proche du tronc -> 1 = plus loin
        // int none/fixe/mobile -> 0 = n'existe pas, 1 = fixe, 2 = mobile
        std::map<glm::vec3, int> attachmentPoints;
        float orientation;

        // angle de depart = 0

    public:
        bodyPart(float x, float y, float z, BodyPartType partType, std::map<glm::vec3, int> attachmentPoints) : x(x), y(y), z(z), initialPosition(x, y, z), partType(partType), attachmentPoints(attachmentPoints), orientation(0.0f) { (void)orientation; }

        float getX() const { return x; }
        float getY() const { return y; }
        float getZ() const { return z; }
        glm::vec3 getInitialPosition() const { return initialPosition; }
        BodyPartType getPartType() const { return partType; }

        glm::vec3 getPosition() { return glm::vec3(x, y, z); }

        // expose attachment points so other systems (e.g., renderer) can access them
        const std::map<glm::vec3, int>& getAttachmentPoints() const { return attachmentPoints; }

        int operator==(const bodyPart& other) const {
            return (partType == other.partType);
        }

        glm::vec3 getScale() const { return scaleVec; }

        // resize now updates the scale vector
        void setSize(const glm::vec3 &newSize) { scaleVec = newSize; }
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

        std::map<glm::vec3, int> getAttachmentPoints(int type) const {
            switch (type) {
                case HEAD:
                    return parts[0].getAttachmentPoints();
                case TORSO:
                    return parts[1].getAttachmentPoints();
                case LEFT_UPPER_ARM:
                    return parts[2].getAttachmentPoints();
                case LEFT_LOWER_ARM:
                    return parts[3].getAttachmentPoints();
                case LEFT_THIGH:
                    return parts[4].getAttachmentPoints();
                case LEFT_LOWER_LEG:
                    return parts[5].getAttachmentPoints();
                case RIGHT_UPPER_ARM:
                    return parts[6].getAttachmentPoints();
                case RIGHT_LOWER_ARM:
                    return parts[7].getAttachmentPoints();
                case RIGHT_THIGH:
                    return parts[8].getAttachmentPoints();
                case RIGHT_LOWER_LEG:
                    return parts[9].getAttachmentPoints();
                default:
                    return {};
            }
        }

        void draw_head(Shader& ourShader) {
            // set override color for head (skin tone)
            ourShader.setBool("useOverrideColor", true);
            ourShader.setVec3("overrideColor", 1.0f, 187.0f/255.0f, 119.0f/255.0f);
            for (const auto& part : parts) {
                if (part.getPartType() == BodyPartType::HEAD) {
                    float x = part.getX();
                    float y = part.getY();
                    float z = part.getZ();
                    glm::vec3 position = {x, y, z};
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, position);
                    model = glm::scale(model, part.getScale());

                    ourShader.setMat4("model", model);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                    {
                        GLenum _err = glGetError();
                        if (_err != GL_NO_ERROR)
                        std::cout << "GL error after draw: " << _err << std::endl;
                    }
                }
            }
            // disable override so other draws use vertex colors unless they enable their own override
            ourShader.setBool("useOverrideColor", false);
        }

        void draw_body(Shader& ourShader) {
            // set override color for torso
            ourShader.setBool("useOverrideColor", true);
            ourShader.setVec3("overrideColor", 0.0f, 238.0f/255.0f, 221.0f/255.0f);
            for (const auto& part : parts) {
                if (part.getPartType() == BodyPartType::TORSO) {
                    float x = part.getX();
                    float y = part.getY();
                    float z = part.getZ();
                    glm::vec3 position = {x, y, z};
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, position);
                    model = glm::scale(model, part.getScale());

                    ourShader.setMat4("model", model);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                    {
                        GLenum _err = glGetError();
                        if (_err != GL_NO_ERROR)
                        std::cout << "GL error after draw: " << _err << std::endl;
                    }
                }
            }
            ourShader.setBool("useOverrideColor", false);
        }
        
        void draw_wall(Shader& ourShader) {
            // set override color for walls (light gray)
            ourShader.setBool("useOverrideColor", true);
            ourShader.setVec3("overrideColor", 0.9f, 0.9f, 0.9f);
            for (const auto& part : parts) {
                if (part.getPartType() == BodyPartType::WALL) {
                    float x = part.getX();
                    float y = part.getY();
                    float z = part.getZ();
                    glm::vec3 position = {x, y, z};
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, position);
                    model = glm::scale(model, part.getScale());

                    ourShader.setMat4("model", model);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                    {
                        GLenum _err = glGetError();
                        if (_err != GL_NO_ERROR)
                        std::cout << "GL error after draw: " << _err << std::endl;
                    }
                }
            }
            ourShader.setBool("useOverrideColor", false);
        }
        
        void draw_arm(Shader& ourShader) {
            // set override color for arms (same as head)
            ourShader.setBool("useOverrideColor", true);
            ourShader.setVec3("overrideColor", 1.0f, 187.0f/255.0f, 119.0f/255.0f);
            BodyPartType bodyPart;
            for (const auto& part : parts) {
                bodyPart = part.getPartType();
                if (bodyPart == BodyPartType::LEFT_UPPER_ARM || bodyPart == BodyPartType::LEFT_LOWER_ARM  || bodyPart == BodyPartType::RIGHT_UPPER_ARM || bodyPart == BodyPartType::RIGHT_LOWER_ARM) {
                    float x = part.getX();
                    float y = part.getY();
                    float z = part.getZ();
                    glm::vec3 position = {x, y, z};
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, position);
                    model = glm::scale(model, part.getScale());

                    ourShader.setMat4("model", model);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                    {
                        GLenum _err = glGetError();
                        if (_err != GL_NO_ERROR)
                        std::cout << "GL error after draw: " << _err << std::endl;
                    }
                }
            }
            ourShader.setBool("useOverrideColor", false);
        }
        
        void draw_leg(Shader& ourShader) {
            // set override color for legs
            ourShader.setBool("useOverrideColor", true);
            ourShader.setVec3("overrideColor", 0.0f, 136.0f/255.0f, 204.0f/255.0f);
            BodyPartType bodyPart;
            for (const auto& part : parts) {
                bodyPart = part.getPartType();
                if (bodyPart == BodyPartType::LEFT_THIGH || bodyPart == BodyPartType::LEFT_LOWER_LEG || bodyPart == BodyPartType::RIGHT_THIGH || bodyPart == BodyPartType::RIGHT_LOWER_LEG) {
                    float x = part.getX();
                    float y = part.getY();
                    float z = part.getZ();
                    glm::vec3 position = {x, y, z};
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, position);
                    model = glm::scale(model, part.getScale());

                    ourShader.setMat4("model", model);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                    {
                        GLenum _err = glGetError();
                        if (_err != GL_NO_ERROR)
                            std::cout << "GL error after draw: " << _err << std::endl;
                    }
                }
            }
            ourShader.setBool("useOverrideColor", false);
        }
};

#endif