#include "animation.hpp"
#include <cmath>


struct AnimAngles
{
    float rightArm = 0.0f;
    float leftArm  = 0.0f;
    float rightLeg = 0.0f;
    float leftLeg  = 0.0f;
    glm::vec3 rightArmAxis = glm::vec3(1.0f, 0.0f, 0.0f);
};

static const glm::vec3 RIGHT_SHOULDER(1.5f, -0.5f, 0.0f);
static const glm::vec3 LEFT_SHOULDER(-1.5f, -0.5f, 0.0f);
static const glm::vec3 LEFT_HIP(-0.5f, -2.5f, 0.0f);
static const glm::vec3 RIGHT_HIP(0.5f, -2.5f, 0.0f);


static AnimAngles anim_waving(float t)
{
    AnimAngles a;
    a.rightArm     = glm::radians(160.0f + 20.0f * std::sin(t * 3.0f));
    a.rightArmAxis = glm::vec3(0.0f, 0.0f, 1.0f);
    return a;
}


static AnimAngles anim_walking(float t)
{
    AnimAngles a;
    float swing = std::sin(t * 4.0f);
    a.leftLeg  =  glm::radians(35.0f * swing);
    a.rightLeg = -glm::radians(35.0f * swing);
    a.leftArm  = -glm::radians(25.0f * swing);
    a.rightArm =  glm::radians(25.0f * swing);
    return a;
}


static AnimAngles getAnimAngles(int state, float t)
{
    switch (state)
    {
        case WAVING:
            return anim_waving(t);
        case WALKING:
            return anim_walking(t);
        default:
            return AnimAngles();
    }
}


static void applyPivotRotation(glm::mat4& model, const glm::vec3& pivot, float angle, const glm::vec3& axis, const glm::vec3& partPos)
{
    glm::vec3 neg(-pivot.x, -pivot.y, -pivot.z);
    model = glm::translate(model, pivot);
    model = glm::rotate(model, angle, axis);
    model = glm::translate(model, neg);
    model = glm::translate(model, partPos);
}


Animator::Animator() : _state(NONE), _time(0.0f) {}


void Animator::setState(Animations state)
{
    _state = state;
    _time  = 0.0f;
}


void Animator::update(float deltaTime)
{
    if (_state != NONE)
        _time += deltaTime;
}


void Animator::draw(Shader& ourShader, body& myBody)
{
    const AnimAngles a = getAnimAngles(_state, _time);

    myBody.draw_head(ourShader);
    myBody.draw_body(ourShader);
    if (_state == NONE) {
        //TODO : include draw_head et draw_body dedans si animation requierant leur mouvement
        myBody.draw_arm(ourShader);
        myBody.draw_leg(ourShader);
        return;
    }

    ourShader.setBool("useOverrideColor", true);
    ourShader.setVec3("overrideColor", 1.0f, 187.0f/255.0f, 119.0f/255.0f);
    for (const auto& part : myBody.getParts()) {
        BodyPartType type = part.getPartType();
        if (type != UPPER_ARM && type != LOWER_ARM)
            continue;
        glm::vec3 pos(part.getX(), part.getY(), part.getZ());
        glm::mat4 model(1.0f);
        if (pos.x > 0.0f)
            applyPivotRotation(model, RIGHT_SHOULDER, a.rightArm, a.rightArmAxis, pos);
        else
            applyPivotRotation(model, LEFT_SHOULDER,  a.leftArm,  glm::vec3(1,0,0), pos);
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    ourShader.setVec3("overrideColor", 0.0f, 136.0f/255.0f, 204.0f/255.0f);
    for (const auto& part : myBody.getParts()) {
        BodyPartType type = part.getPartType();
        if (type != THIGH && type != LOWER_PART)
            continue;
        glm::vec3 pos(part.getX(), part.getY(), part.getZ());
        glm::mat4 model(1.0f);
        if (pos.x < 0.0f)
            applyPivotRotation(model, LEFT_HIP,  a.leftLeg,  glm::vec3(1,0,0), pos);
        else
            applyPivotRotation(model, RIGHT_HIP, a.rightLeg, glm::vec3(1,0,0), pos);
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    ourShader.setBool("useOverrideColor", false);
}
