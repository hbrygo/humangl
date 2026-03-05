#include "animation.hpp"
#include <cmath>


struct AnimAngles
{
    float rightArm = 0.0f;
    float leftArm = 0.0f;
    float rightLeg = 0.0f;
    float leftLeg = 0.0f;
    float rightKnee = 0.0f;
    float leftKnee = 0.0f;
    float rightElbow = 0.0f; // additional bend of the forearm around the elbow
    float leftElbow = 0.0f;
    glm::vec3 rightArmAxis = glm::vec3(1.0f, 0.0f, 0.0f);

    glm::vec3 leftArmAxis = glm::vec3(-1.0f, 0.0f, 0.0f);
    glm::vec3 bodyOffset = glm::vec3(0.0f, 0.0f, 0.0f);
    float torsoAngle = 0.0f; // forward tilt around torso base (X axis)
};


static glm::vec3 getPivotPoint(const body& myBody, int partType, bool proximal)
{
    for (const auto& part : myBody.getParts())
    {
        if (part.getPartType() != static_cast<BodyPartType>(partType))
            continue;
        float px = part.getX();
        float py = part.getY();
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


static AnimAngles anim_t_pose(float t)
{
    (void)t; // unused
    AnimAngles a;
    // Bras droit : rotation +90° autour de Z => bras part vers la droite
    a.rightArm     = glm::radians(90.0f);
    a.rightArmAxis = glm::vec3(0.0f, 0.0f, 1.0f);
    // Bras gauche : rotation -90° autour de Z => bras part vers la gauche (symétrique)
    a.leftArm      = glm::radians(-90.0f);
    a.leftArmAxis  = glm::vec3(0.0f, 0.0f, 1.0f);
    return a;
}

static AnimAngles anim_waving(float t)
{
    AnimAngles a;
    a.rightArm     = glm::radians(160.0f + 20.0f * std::sin(t * 3.0f));
    a.rightArmAxis = glm::vec3(0.0f, 0.0f, 1.0f);
    return a;
}


static float clampUnit(float x)
{
    if (x < 0.f)
        return 0.f;
    if (x > 1.f)
        return 1.f;
    return x;
}

// courbe cubique
static float easeInOut(float x)
{
    return x * x * (3.0f - 2.0f * x);
}

static float linearInterp(float a, float b, float s)
{
    return a + (b - a) * s;
}

static float phaseProgress(float p, float start, float end)
{
    float t = (p - start) / (end - start);
    return easeInOut(clampUnit(t));
}

static AnimAngles anim_jumping(float t)
{
    AnimAngles a;

    const float period = 2.2f;
    float p = std::fmod(t, period) / period;


    float kneeAngle  = 0.0f;
    float legAngle   = 0.0f;
    float armAngle   = 0.0f;
    float elbowAngle = 0.0f;
    float torsoAngle = 0.0f;
    float bodyY      = 0.0f;

    // Part 1 [0-18%]: Squat
    if (p < 0.18f) {
        float s = phaseProgress(p, 0.0f, 0.18f);
        kneeAngle = linearInterp(0.0f, glm::radians(75.0f), s);
        legAngle = linearInterp(0.0f, glm::radians(-38.0f), s);
        armAngle = linearInterp(0.0f, glm::radians(25.0f), s);
        elbowAngle = linearInterp(0.0f, -glm::radians(70.0f), s);
        torsoAngle = linearInterp(0.0f, glm::radians(18.0f), s);
        bodyY = linearInterp(0.0f, -0.5f, s);
    }
    // Part 2 [18-36%]: Jump
    else if (p < 0.36f) {
        float s = phaseProgress(p, 0.18f, 0.36f);
        kneeAngle = linearInterp(glm::radians(75.0f), 0.0f, s);
        legAngle = linearInterp(glm::radians(-38.0f), glm::radians(-22.0f), s);
        armAngle = linearInterp(glm::radians(25.0f), glm::radians(-165.0f), s);
        elbowAngle = linearInterp(-glm::radians(70.0f), 0.0f, s);
        torsoAngle = linearInterp(glm::radians(18.0f), 0.0f, s);
        bodyY = linearInterp(-0.5f, 3.6f, s);
    }
    // Part 3 [36-52%]: Apex
    else if (p < 0.52f) {
        float s = phaseProgress(p, 0.36f, 0.52f);
        kneeAngle = linearInterp(0.0f, glm::radians(65.0f), s);
        legAngle = linearInterp(glm::radians(-22.0f), glm::radians(-8.0f), s);
        armAngle = glm::radians(-165.0f);
        bodyY = linearInterp(3.6f, 3.1f, s);
    }
    // Part 4 [52-65%]: Descent
    else if (p < 0.65f) {
        float s = phaseProgress(p, 0.52f, 0.65f);
        kneeAngle = linearInterp(glm::radians(65.0f), 0.0f, s);
        legAngle = linearInterp(glm::radians(-8.0f), 0.0f, s);
        armAngle = linearInterp(glm::radians(-165.0f), glm::radians(-60.0f), s);
        elbowAngle = linearInterp(0.0f, -glm::radians(35.0f), s);
        bodyY = linearInterp(3.1f, 0.0f, s);
    }
    // Part 5 [65-100%]: Recover
    else {
        if (p < 0.82f) {
            float s = easeInOut(clampUnit((p - 0.65f) / 0.17f));
            armAngle = linearInterp(glm::radians(-60.0f), glm::radians(25.0f), s);
            elbowAngle = linearInterp(-glm::radians(35.0f), -glm::radians(70.0f), s);
        }
        else {
            float s = easeInOut(clampUnit((p - 0.82f) / 0.18f));
            armAngle = linearInterp(glm::radians(25.0f), 0.0f, s);
            elbowAngle = linearInterp(-glm::radians(70.0f), 0.0f, s);
        }
        bodyY = 0.0f;
    }

    a.bodyOffset = glm::vec3(0.0f, bodyY, 0.0f);
    a.leftLeg = legAngle;
    a.rightLeg = legAngle;
    a.leftKnee = kneeAngle;
    a.rightKnee = kneeAngle;
    a.leftArm = armAngle;
    a.rightArm = armAngle;
    a.leftElbow = elbowAngle;
    a.rightElbow = elbowAngle;
    a.torsoAngle = torsoAngle;
    a.leftArmAxis = glm::vec3(1.0f, 0.0f, 0.0f); //to patch
    a.rightArmAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    return a;
}


static AnimAngles anim_walking(float t)
{
    AnimAngles a;
    float swing = std::sin(t * 4.0f);
    a.leftArmAxis = glm::vec3(1.0f, 0.0f, 0.0f); // to patch
    a.leftLeg = glm::radians(35.0f * swing);
    a.rightLeg = -glm::radians(35.0f * swing);
    a.leftArm = -glm::radians(15.0f * swing);
    a.rightArm = glm::radians(15.0f * swing);
    a.leftKnee = glm::radians(30.0f) * std::max(0.0f, swing);
    a.rightKnee = glm::radians(30.0f) * std::max(0.0f, -swing);
    // Elbow bends more when arm swings back, less when forward.
    // Baseline ~80°, oscillation ±25° in phase with arm swing.
    a.rightElbow = -glm::radians(20.0f + 25.0f * -swing);
    a.leftElbow = -glm::radians(20.0f + 25.0f * swing);
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
        case T_POSE:
            return anim_t_pose(t);
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

// Rotate the forearm: first apply shoulder rotation, then elbow bend around the elbow pivot.
static void applyElbowRotation(glm::mat4& model, const glm::vec3& shoulder, float shoulderAngle, const glm::vec3& shoulderAxis, const glm::vec3& elbow, float elbowAngle, const glm::vec3& partPos)
{
    const glm::vec3 elbowAxis(1.0f, 0.0f, 0.0f);
    model = glm::translate(model, shoulder);
    model = glm::rotate(model, shoulderAngle, shoulderAxis);
    model = glm::translate(model, elbow - shoulder);
    model = glm::rotate(model, elbowAngle, elbowAxis);
    model = glm::translate(model, glm::vec3(-elbow.x, -elbow.y, -elbow.z));
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
    const glm::vec3 torsoBase = getPivotPoint(myBody, TORSO, false) + a.bodyOffset;

    auto withTorsoTilt = [&](glm::vec3 pt) -> glm::vec3 {
        if (a.torsoAngle == 0.0f)
            return pt;
        glm::vec3 rel = pt - torsoBase;
        float c = std::cos(a.torsoAngle), s = std::sin(a.torsoAngle);
        return torsoBase + glm::vec3(rel.x, rel.y * c - rel.z * s, rel.y * s + rel.z * c);
    };

    const glm::vec3 leftShoulder = withTorsoTilt(getPivotPoint(myBody, LEFT_UPPER_ARM, true) + a.bodyOffset);
    const glm::vec3 rightShoulder = withTorsoTilt(getPivotPoint(myBody, RIGHT_UPPER_ARM, true) + a.bodyOffset);
    const glm::vec3 leftElbow = withTorsoTilt(getPivotPoint(myBody, LEFT_UPPER_ARM, false) + a.bodyOffset);
    const glm::vec3 rightElbow = withTorsoTilt(getPivotPoint(myBody, RIGHT_UPPER_ARM, false) + a.bodyOffset);
    const glm::vec3 leftHip = getPivotPoint(myBody, LEFT_THIGH, true) + a.bodyOffset;
    const glm::vec3 rightHip = getPivotPoint(myBody, RIGHT_THIGH, true) + a.bodyOffset;
    const glm::vec3 leftKnee = getPivotPoint(myBody, LEFT_THIGH, false) + a.bodyOffset;
    const glm::vec3 rightKnee = getPivotPoint(myBody, RIGHT_THIGH, false) + a.bodyOffset;

    if (_state == NONE) {
        myBody.draw_head(ourShader);
        myBody.draw_body(ourShader);
        myBody.draw_arm(ourShader);
        myBody.draw_leg(ourShader);
        return;
    }

    ourShader.setBool("useOverrideColor", true);

    // ---- HEAD ----
    ourShader.setVec3("overrideColor", 1.0f, 187.0f/255.0f, 119.0f/255.0f);
    for (const auto& part : myBody.getParts()) {
        if (part.getPartType() != HEAD) continue;
        glm::vec3 pos(part.getX(), part.getY(), part.getZ());
        glm::mat4 model(1.0f);
        applyPivotRotation(model, torsoBase, a.torsoAngle, glm::vec3(1,0,0), pos + a.bodyOffset);
        model = glm::scale(model, part.getScale());
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // ---- TORSO ----
    ourShader.setVec3("overrideColor", 0.0f, 238.0f/255.0f, 221.0f/255.0f);
    for (const auto& part : myBody.getParts()) {
        if (part.getPartType() != TORSO) continue;
        glm::vec3 pos(part.getX(), part.getY(), part.getZ());
        glm::mat4 model(1.0f);
        applyPivotRotation(model, torsoBase, a.torsoAngle, glm::vec3(1,0,0), pos + a.bodyOffset);
        model = glm::scale(model, part.getScale());
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // ---- ARMS ----
    ourShader.setVec3("overrideColor", 1.0f, 187.0f/255.0f, 119.0f/255.0f);
    for (const auto& part : myBody.getParts()) {
        BodyPartType type = part.getPartType();
        if (type != LEFT_UPPER_ARM && type != LEFT_LOWER_ARM && type != RIGHT_UPPER_ARM && type != RIGHT_LOWER_ARM)
            continue;
        glm::vec3 pos = withTorsoTilt(glm::vec3(part.getX(), part.getY(), part.getZ()) + a.bodyOffset);
        glm::mat4 model(1.0f);
        bool isRightArm = (part.getX() > 0.0f);
        bool isLower = (type == LEFT_LOWER_ARM || type == RIGHT_LOWER_ARM);
        if (isRightArm) {
            if (isLower && a.rightElbow != 0.0f)
                applyElbowRotation(model, rightShoulder, a.rightArm, a.rightArmAxis,
                                   rightElbow, a.rightElbow, pos);
            else
                applyPivotRotation(model, rightShoulder, a.rightArm, a.rightArmAxis, pos);
        } else {
            if (isLower && a.leftElbow != 0.0f)
                applyElbowRotation(model, leftShoulder, a.leftArm, a.leftArmAxis,
                                   leftElbow, a.leftElbow, pos);
            else
                applyPivotRotation(model, leftShoulder, a.leftArm, a.leftArmAxis, pos);
        }
        model = glm::scale(model, part.getScale());
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // ---- LEGS ----
    ourShader.setVec3("overrideColor", 0.0f, 136.0f/255.0f, 204.0f/255.0f);
    for (const auto& part : myBody.getParts()) {
        BodyPartType type = part.getPartType();
        if (type != LEFT_THIGH && type != LEFT_LOWER_LEG && type != RIGHT_THIGH && type != RIGHT_LOWER_LEG)
            continue;
        glm::vec3 pos(part.getX(), part.getY(), part.getZ());
        glm::mat4 model(1.0f);
        bool isLeft = (pos.x < 0.0f);
        if (type == LEFT_THIGH || type == RIGHT_THIGH) {
            if (isLeft)
                applyPivotRotation(model, leftHip,  a.leftLeg,  glm::vec3(1,0,0), pos + a.bodyOffset);
            else
                applyPivotRotation(model, rightHip, a.rightLeg, glm::vec3(1,0,0), pos + a.bodyOffset);
        } else {
            if (isLeft)
                applyKneeRotation(model, leftHip,  a.leftLeg,  leftKnee,  a.leftKnee,  pos + a.bodyOffset);
            else
                applyKneeRotation(model, rightHip, a.rightLeg, rightKnee, a.rightKnee, pos + a.bodyOffset);
        }
        model = glm::scale(model, part.getScale());
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    /******************************************************** */
    /*              dessine les arretes en rouge              */
    /******************************************************** */
    ourShader.setVec3("overrideColor", 255.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonOffset(-1.0f, -1.0f);
    for (const auto& part : myBody.getParts())
    {
        BodyPartType type = part.getPartType();
        glm::mat4 model(1.0f);
        if (type == HEAD || type == TORSO) {
            glm::vec3 pos(part.getX(), part.getY(), part.getZ());
            applyPivotRotation(model, torsoBase, a.torsoAngle, glm::vec3(1,0,0), pos + a.bodyOffset);
        } else if (type == LEFT_UPPER_ARM || type == LEFT_LOWER_ARM ||
                   type == RIGHT_UPPER_ARM || type == RIGHT_LOWER_ARM) {
            glm::vec3 pos = withTorsoTilt(glm::vec3(part.getX(), part.getY(), part.getZ()) + a.bodyOffset);
            bool isRightArm = (part.getX() > 0.0f);
            bool isLower = (type == LEFT_LOWER_ARM || type == RIGHT_LOWER_ARM);
            if (isRightArm) {
                if (isLower && a.rightElbow != 0.0f)
                    applyElbowRotation(model, rightShoulder, a.rightArm, a.rightArmAxis,
                                       rightElbow, a.rightElbow, pos);
                else
                    applyPivotRotation(model, rightShoulder, a.rightArm, a.rightArmAxis, pos);
            } else {
                if (isLower && a.leftElbow != 0.0f)
                    applyElbowRotation(model, leftShoulder, a.leftArm, a.leftArmAxis,
                                       leftElbow, a.leftElbow, pos);
                else
                    applyPivotRotation(model, leftShoulder, a.leftArm, a.leftArmAxis, pos);
            }
        } else if (type == LEFT_THIGH || type == LEFT_LOWER_LEG ||
                   type == RIGHT_THIGH || type == RIGHT_LOWER_LEG) {
            glm::vec3 pos(part.getX(), part.getY(), part.getZ());
            bool isLeft = (pos.x < 0.0f);
            if (type == LEFT_THIGH || type == RIGHT_THIGH) {
                if (isLeft)
                    applyPivotRotation(model, leftHip,  a.leftLeg,  glm::vec3(1,0,0), pos + a.bodyOffset);
                else
                    applyPivotRotation(model, rightHip, a.rightLeg, glm::vec3(1,0,0), pos + a.bodyOffset);
            } else {
                if (isLeft)
                    applyKneeRotation(model, leftHip,  a.leftLeg,  leftKnee,  a.leftKnee,  pos + a.bodyOffset);
                else
                    applyKneeRotation(model, rightHip, a.rightLeg, rightKnee, a.rightKnee, pos + a.bodyOffset);
            }
        } else {
            continue;
        }
        model = glm::scale(model, part.getScale());
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    // restore state
    glDisable(GL_POLYGON_OFFSET_LINE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    /* ******************************************************* */

    ourShader.setBool("useOverrideColor", false);
}
