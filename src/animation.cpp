#include "animation.hpp"
#include <cmath>


struct AnimAngles
{
    float rightArm  = 0.0f;
    float leftArm   = 0.0f;
    float rightLeg  = 0.0f;
    float leftLeg   = 0.0f;
    float rightKnee = 0.0f;
    float leftKnee  = 0.0f;
    glm::vec3 rightArmAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 bodyOffset   = glm::vec3(0.0f, 0.0f, 0.0f);
};


static glm::vec3 getPivotPoint(const body& myBody, int partType, bool proximal)
{
    for (const auto& part : myBody.getParts())
    {
        if (part.getPartType() != static_cast<BodyPartType>(partType)) continue;
        float px    = part.getX();
        float py    = part.getY();
        glm::vec3 s = part.getScale();
        float pivotY = proximal ? py + s.y * 0.5f : py - s.y * 0.5f;
        float pivotX;
        if (partType == LEFT_UPPER_ARM || partType == RIGHT_UPPER_ARM)
            pivotX = px - (px > 0.0f ? 1.0f : -1.0f) * s.x * 0.5f;
        else
            pivotX = px;
        return glm::vec3(pivotX, pivotY, 0.0f);
    }
    return glm::vec3(0.0f, 0.0f, 0.0f);
}


static AnimAngles anim_waving(float t)
{
    AnimAngles a;
    a.rightArm     = glm::radians(160.0f + 20.0f * std::sin(t * 3.0f));
    a.rightArmAxis = glm::vec3(0.0f, 0.0f, 1.0f);
    return a;
}


static AnimAngles anim_jumping(float t)
{
    // TODO: FIX IT THAT'S FUCKED UP
    AnimAngles a;

    float modulo = std::sin(t) / (float)M_PI;
    float rise = 0.0f;
    if (modulo > 0.0f)
        rise = modulo * 2.0f;
    a.bodyOffset = glm::vec3(0.0f, 3.0f * rise, 0.0f);
    a.leftArm      = glm::radians(-120.0f) * rise;
    a.rightArm     = glm::radians(-120.0f) * rise;
    a.rightArmAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    float kneeBend = glm::radians(55.0f) * (1.0f - rise);
    a.leftKnee  = kneeBend;
    a.rightKnee = kneeBend;
    a.leftLeg  = glm::radians(-20.0f) * rise;
    a.rightLeg = glm::radians(-20.0f) * rise;
    return a;
}


static AnimAngles anim_walking(float t)
{
    AnimAngles a;
    float swing = std::sin(t * 4.0f);
    a.leftLeg   =  glm::radians(35.0f * swing);
    a.rightLeg  = -glm::radians(35.0f * swing);
    a.leftArm   = -glm::radians(25.0f * swing);
    a.rightArm  =  glm::radians(25.0f * swing);
    a.leftKnee  = glm::radians(30.0f) * std::max(0.0f, -swing);
    a.rightKnee = glm::radians(30.0f) * std::max(0.0f,  swing);
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
        case JUMPING:
            return anim_jumping(t);
        default:
            return AnimAngles();
    }
}

static void applyKneeRotation(glm::mat4& model, const glm::vec3& hip, float hipAngle, const glm::vec3& knee, float kneeAngle, const glm::vec3& partPos)
{
    const glm::vec3 axis(1.0f, 0.0f, 0.0f);
    model = glm::translate(model, hip);
    model = glm::rotate(model, hipAngle, axis);
    model = glm::translate(model, knee - hip);
    model = glm::rotate(model, kneeAngle, axis);
    model = glm::translate(model, glm::vec3(-knee.x, -knee.y, -knee.z));
    model = glm::translate(model, partPos);
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
    const glm::vec3 leftShoulder  = getPivotPoint(myBody, LEFT_UPPER_ARM,  true);
    const glm::vec3 rightShoulder = getPivotPoint(myBody, RIGHT_UPPER_ARM, true);
    const glm::vec3 leftHip       = getPivotPoint(myBody, LEFT_THIGH,      true);
    const glm::vec3 rightHip      = getPivotPoint(myBody, RIGHT_THIGH,     true);
    const glm::vec3 leftKnee      = getPivotPoint(myBody, LEFT_THIGH,      false);
    const glm::vec3 rightKnee     = getPivotPoint(myBody, RIGHT_THIGH,     false);

    if (_state == NONE) {
        myBody.draw_head(ourShader);
        myBody.draw_body(ourShader);
        myBody.draw_arm(ourShader);
        myBody.draw_leg(ourShader);
        return;
    }

    ourShader.setBool("useOverrideColor", true);
    ourShader.setVec3("overrideColor", 1.0f, 187.0f/255.0f, 119.0f/255.0f);
    for (const auto& part : myBody.getParts()) {
        if (part.getPartType() != HEAD) continue;
        glm::vec3 pos(part.getX(), part.getY(), part.getZ());
        glm::mat4 model(1.0f);
        model = glm::translate(model, pos + a.bodyOffset);
        model = glm::scale(model, part.getScale());
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    ourShader.setVec3("overrideColor", 0.0f, 238.0f/255.0f, 221.0f/255.0f);
    for (const auto& part : myBody.getParts()) {
        if (part.getPartType() != TORSO) continue;
        glm::vec3 pos(part.getX(), part.getY(), part.getZ());
        glm::mat4 model(1.0f);
        model = glm::translate(model, pos + a.bodyOffset);
        model = glm::scale(model, part.getScale());
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    ourShader.setVec3("overrideColor", 1.0f, 187.0f/255.0f, 119.0f/255.0f);
    for (const auto& part : myBody.getParts()) {
        BodyPartType type = part.getPartType();
        if (type != LEFT_UPPER_ARM && type != LEFT_LOWER_ARM && type != RIGHT_UPPER_ARM && type != RIGHT_LOWER_ARM)
            continue;
        glm::vec3 pos(part.getX(), part.getY(), part.getZ());
        glm::mat4 model(1.0f);
        if (pos.x > 0.0f)
            applyPivotRotation(model, rightShoulder, a.rightArm, a.rightArmAxis, pos + a.bodyOffset);
        else
            applyPivotRotation(model, leftShoulder,  a.leftArm,  glm::vec3(1,0,0), pos + a.bodyOffset);
        model = glm::scale(model, part.getScale());
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    ourShader.setVec3("overrideColor", 0.0f, 136.0f/255.0f, 204.0f/255.0f);
    for (const auto& part : myBody.getParts()) {
        BodyPartType type = part.getPartType();
        if (type != LEFT_THIGH && type != LEFT_LOWER_LEG && type != RIGHT_THIGH && type != RIGHT_LOWER_LEG)
            continue;
        glm::vec3 pos(part.getX(), part.getY(), part.getZ());
        glm::mat4 model(1.0f);
        bool isLeft = (pos.x < 0.0f);
        if (type == LEFT_THIGH || type == RIGHT_THIGH)
        {
            // thigh: simple rotation around the hip
            if (isLeft)
                applyPivotRotation(model, leftHip,  a.leftLeg,  glm::vec3(1,0,0), pos + a.bodyOffset);
            else
                applyPivotRotation(model, rightHip, a.rightLeg, glm::vec3(1,0,0), pos + a.bodyOffset);
        }
        else
        {
            if (isLeft)
                applyKneeRotation(model, leftHip,  a.leftLeg,  leftKnee,  a.leftKnee,  pos + a.bodyOffset);
            else
                applyKneeRotation(model, rightHip, a.rightLeg, rightKnee, a.rightKnee, pos + a.bodyOffset);
        }
        model = glm::scale(model, part.getScale());
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    ourShader.setBool("useOverrideColor", false);
}
