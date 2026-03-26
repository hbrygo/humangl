#include "animation.hpp"
#include <cmath>


struct AnimAngles
{
    float leftArm = 0.0f;
    float rightArm = 0.0f;
    float leftLeg = 0.0f;
    float rightLeg = 0.0f;
    float leftKnee = 0.0f;
    float rightKnee = 0.0f;
    float leftElbow = 0.0f;
    float rightElbow = 0.0f;
    float leftArmSide = 0.0f;
    float rightArmSide = 0.0f;
    float rightElbowSide = 0.0f;
    glm::vec3 leftArmAxis = glm::vec3(1.0f, 0.0f, 0.0f);

    glm::vec3 rightArmAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 leftElbowAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 rightElbowAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 leftLegAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 rightLegAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 bodyOffset = glm::vec3(0.0f, 0.0f, 0.0f);
    float bodyPitch = 0.0f;
    float bodyYaw = 0.0f;
    float headPitch = 0.0f;
    float torsoAngle = 0.0f;
    float shoulderDrop = 0.0f;
};


static float clampUnit(float x)
{
    const float minUnit = 0.0f;
    const float maxUnit = 1.0f;
    if (x < minUnit)
        return minUnit;
    if (x > maxUnit)
        return maxUnit;
    return x;
}


static float easeInOut(float x)
{
    const float easeMul = 3.0f;
    const float easeSub = 2.0f;
    return x * x * (easeMul - easeSub * x);
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


static glm::vec3 getPivotPoint(const body& myBody, int partType, bool proximal)
{
    const float halfScale = 0.5f;
    const float sideRight = 1.0f;
    const float sideLeft = -1.0f;
    const float pivotZ = 0.0f;
    for (const auto& part : myBody.getParts())
    {
        if (part.getPartType() != static_cast<BodyPartType>(partType))
            continue;
        float px = part.getX();
        float py = part.getY();
        glm::vec3 s = part.getScale();
        float pivotY = proximal ? py + s.y * halfScale : py - s.y * halfScale;
        float pivotX;
        if (partType == RIGHT_UPPER_ARM || partType == LEFT_UPPER_ARM)
            pivotX = px - (px > 0.0f ? sideRight : sideLeft) * s.x * halfScale;
        else
            pivotX = px;
        return glm::vec3(pivotX, pivotY, pivotZ);
    }
    return glm::vec3(0.0f, 0.0f, pivotZ);
}


static AnimAngles anim_eagle_flight(float t)
{
    AnimAngles a;
    const float riseTime = 0.4f;
    const float flapFrequency = 50.0f;
    const float wingFlapAmplitude = 25.0f;
    const float shoulderDropScale = -0.5f;
    const float liftSpeed = 30.0f;

    float angle;
    if (t < riseTime) {
        float s = t / riseTime;
        angle = glm::radians(90.0f) * s;
    } else {
        float flap = std::sin((t - riseTime) * flapFrequency);
        angle = glm::radians(90.0f + wingFlapAmplitude * flap);
    }

    a.leftArm     = angle;
    a.leftArmAxis = glm::vec3(0.0f, 0.0f, 1.0f);
    a.rightArm      = -angle;
    a.rightArmAxis  = glm::vec3(0.0f, 0.0f, 1.0f);
    a.shoulderDrop = (std::min(angle, glm::radians(90.0f)) / glm::radians(90.0f)) * shoulderDropScale;

    if (t >= riseTime) {
        float lift = (t - riseTime) * liftSpeed;
        a.bodyOffset = glm::vec3(0.0f, lift, 0.0f);
    }

    return a;
}


static AnimAngles anim_t_pose(float t)
{
    AnimAngles a;
    const float period = 4.0f;
    const float raiseEnd = 0.1f;
    const float holdEnd = 0.7f;
    const float lowerEnd = 0.80f;
    const float shoulderDropScale = -0.5f;
    float p = std::fmod(t, period) / period;

    float angle;
    if (p < raiseEnd) {
        float s = p / raiseEnd;
        angle = glm::radians(90.0f) * s;
    } else if (p < holdEnd) {
        angle = glm::radians(90.0f);
    } else if (p < lowerEnd) {
        float s = (p - holdEnd) / (lowerEnd - holdEnd);
        angle = glm::radians(90.0f) * (1.0f - s);
    } else {
        angle = 0.0f;
    }

    a.leftArm     = angle;
    a.leftArmAxis = glm::vec3(0.0f, 0.0f, 1.0f);
    a.rightArm      = -angle;
    a.rightArmAxis  = glm::vec3(0.0f, 0.0f, 1.0f);

    a.shoulderDrop = (angle / glm::radians(90.0f)) * shoulderDropScale;
    return a;
}


static AnimAngles anim_waving(float t)
{
    AnimAngles a;
    const float introDuration = 0.45f;
    const float sweepDelay = 0.12f;
    const float sweepFrequency = 6.5f;
    const float baseArm = glm::radians(-145.0f);
    const float baseElbow = glm::radians(-50.0f);
    const float elbowBlendBias = 0.5f;
    const float elbowBlendScale = 0.5f;

    a.leftArmAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    a.leftElbowAxis = glm::vec3(0.0f, 0.0f, 1.0f);

    if (t < introDuration) {
        float s = easeInOut(clampUnit(t / introDuration));
        a.leftArm = linearInterp(0.0f, baseArm, s);
        a.leftElbow = linearInterp(0.0f, baseElbow, s);
        return a;
    }

    float waveT = t - introDuration;
    const float speedElbow = 7.5f;
    float elbowWave = std::sin(waveT * speedElbow);

    if (waveT < sweepDelay) {
        a.leftArmAxis = glm::vec3(1.0f, 0.0f, 0.0f);
        a.leftArm = baseArm;
        a.leftArmSide = 0.0f;
    } else {
        float sweepT = waveT - sweepDelay;
        float zSweep = std::sin(sweepT * sweepFrequency);
        a.leftArmAxis = glm::vec3(1.0f, 0.0f, 0.0f);
        a.leftArm = baseArm;
        a.leftArmSide = -glm::radians(7.0f) * zSweep;
    }

    a.leftElbow = baseElbow + glm::radians(50.0f) * (elbowBlendBias + elbowBlendScale * elbowWave);
    return a;
}


static AnimAngles anim_jumping(float t)
{
    AnimAngles a;
    const float period = 2.2f;
    const float phase1End = 0.18f;
    const float phase2End = 0.36f;
    const float phase3End = 0.52f;
    const float phase4End = 0.65f;
    const float phase5Mid = 0.82f;
    const float phase5aDuration = 0.17f;
    const float phase5bDuration = 0.18f;

    const float bodySquatY = -0.5f;
    const float bodyJumpPeakY = 3.6f;
    const float bodyApexY = 3.1f;
    float p = std::fmod(t, period) / period;
    float kneeAngle  = 0.0f;
    float legAngle   = 0.0f;
    float armAngle   = 0.0f;
    float elbowAngle = 0.0f;
    float torsoAngle = 0.0f;
    float bodyY      = 0.0f;

    // Part 1 [0-18%]: Squat
    if (p < phase1End) {
        float s = phaseProgress(p, 0.0f, phase1End);
        kneeAngle = linearInterp(0.0f, glm::radians(75.0f), s);
        legAngle = linearInterp(0.0f, glm::radians(-38.0f), s);
        armAngle = linearInterp(0.0f, glm::radians(25.0f), s);
        elbowAngle = linearInterp(0.0f, -glm::radians(70.0f), s);
        torsoAngle = linearInterp(0.0f, glm::radians(18.0f), s);
        bodyY = linearInterp(0.0f, bodySquatY, s);
    }
    // Part 2 [18-36%]: Jump
    else if (p < phase2End) {
        float s = phaseProgress(p, phase1End, phase2End);
        kneeAngle = linearInterp(glm::radians(75.0f), 0.0f, s);
        legAngle = linearInterp(glm::radians(-38.0f), glm::radians(-22.0f), s);
        armAngle = linearInterp(glm::radians(25.0f), glm::radians(-165.0f), s);
        elbowAngle = linearInterp(-glm::radians(70.0f), 0.0f, s);
        torsoAngle = linearInterp(glm::radians(18.0f), 0.0f, s);
        bodyY = linearInterp(bodySquatY, bodyJumpPeakY, s);
    }
    // Part 3 [36-52%]: Apex
    else if (p < phase3End) {
        float s = phaseProgress(p, phase2End, phase3End);
        kneeAngle = linearInterp(0.0f, glm::radians(65.0f), s);
        legAngle = linearInterp(glm::radians(-22.0f), glm::radians(-8.0f), s);
        armAngle = glm::radians(-165.0f);
        bodyY = linearInterp(bodyJumpPeakY, bodyApexY, s);
    }
    // Part 4 [52-65%]: Descent
    else if (p < phase4End) {
        float s = phaseProgress(p, phase3End, phase4End);
        kneeAngle = linearInterp(glm::radians(65.0f), 0.0f, s);
        legAngle = linearInterp(glm::radians(-8.0f), 0.0f, s);
        armAngle = linearInterp(glm::radians(-165.0f), glm::radians(-60.0f), s);
        elbowAngle = linearInterp(0.0f, -glm::radians(35.0f), s);
        bodyY = linearInterp(bodyApexY, 0.0f, s);
    }
    // Part 5 [65-100%]: Recover
    else {
        if (p < phase5Mid) {
            float s = easeInOut(clampUnit((p - phase4End) / phase5aDuration));
            armAngle = linearInterp(glm::radians(-60.0f), glm::radians(25.0f), s);
            elbowAngle = linearInterp(-glm::radians(35.0f), -glm::radians(70.0f), s);
        }
        else {
            float s = easeInOut(clampUnit((p - phase5Mid) / phase5bDuration));
            armAngle = linearInterp(glm::radians(25.0f), 0.0f, s);
            elbowAngle = linearInterp(-glm::radians(70.0f), 0.0f, s);
        }
        bodyY = 0.0f;
    }

    a.bodyOffset = glm::vec3(0.0f, bodyY, 0.0f);
    a.rightLeg = legAngle;
    a.leftLeg = legAngle;
    a.rightKnee = kneeAngle;
    a.leftKnee = kneeAngle;
    a.rightArm = armAngle;
    a.leftArm = armAngle;
    a.rightElbow = elbowAngle;
    a.leftElbow = elbowAngle;
    a.torsoAngle = torsoAngle;
    return a;
}


static AnimAngles anim_walking(float t)
{
    AnimAngles a;
    const float walkSwingFrequency = 4.0f;
    float swing = std::sin(t * walkSwingFrequency);

    a.rightLeg = glm::radians(35.0f * swing);
    a.leftLeg = -glm::radians(35.0f * swing);
    a.rightArm = -glm::radians(15.0f * swing);
    a.leftArm = glm::radians(15.0f * swing);
    a.rightKnee = glm::radians(30.0f) * std::max(0.0f, swing);
    a.leftKnee = glm::radians(30.0f) * std::max(0.0f, -swing);
    a.leftElbow = -glm::radians(20.0f + 25.0f * -swing);
    a.rightElbow = -glm::radians(20.0f + 25.0f * swing);
    return a;
}


static AnimAngles anim_naruto_run(float t)
{
    AnimAngles a;
    const float armSwingFrequency = 5.0f;
    const float legSwingFrequency = 12.0f;
    float armSwing = std::sin(t * armSwingFrequency);
    float legSwing = std::sin(t * legSwingFrequency);

    a.torsoAngle = glm::radians(45.0f);
    a.leftArm = glm::radians(55.0f + 8.0f * armSwing);
    a.rightArm = glm::radians(55.0f - 8.0f * armSwing);
    a.rightLeg = glm::radians(45.0f * legSwing);
    a.leftLeg = -glm::radians(45.0f * legSwing);
    a.rightKnee = glm::radians(70.0f) * std::max(0.0f, legSwing);
    a.leftKnee = glm::radians(70.0f) * std::max(0.0f, -legSwing);

    return a;
}


static AnimAngles anim_gangnam_style(float t)
{
    AnimAngles a;
    const float period = 0.8f;
    const float armPulseMultiplier = 2.0f;
    const float legAxisX = 0.85f;
    const float legAxisZ = 0.55f;
    const float elbowAxisX = 0.55f;
    const float elbowAxisZ = 0.84f;
    const float bodyOffsetX = 0.10f;
    const float bodyOffsetYBase = 0.22f;
    const float bodyOffsetYScale = 0.18f;
    float p = std::fmod(t, period) / period;
    const float twoPi = 6.28318530718f;
    float beat = std::sin(p * twoPi);
    float armPulse = std::sin(p * twoPi * armPulseMultiplier);
    float rightLift = std::max(0.0f, beat);
    float leftLift = std::max(0.0f, -beat);
    const float archHip = 42.0f;
    const float raiseHip = 26.0f;
    const float archKnee = 42.0f;
    const float raiseKnee = 30.0f;

    a.rightLegAxis = glm::normalize(glm::vec3(legAxisX, 0.0f, legAxisZ));
    a.leftLegAxis = glm::normalize(glm::vec3(legAxisX, 0.0f, -legAxisZ));
    a.rightLeg = glm::radians(-archHip - raiseHip * rightLift + 8.0f * leftLift);
    a.leftLeg = glm::radians(-archHip - raiseHip * leftLift + 8.0f * rightLift);
    a.rightKnee = glm::radians(archKnee + raiseKnee * rightLift + 8.0f * leftLift);
    a.leftKnee = glm::radians(archKnee + raiseKnee * leftLift + 8.0f * rightLift);
    a.rightArmAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    a.leftArmAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    a.rightArm = glm::radians(-48.0f + 2.0f * armPulse);
    a.leftArm = glm::radians(-43.0f + 2.0f * armPulse);
    a.rightElbowAxis = glm::normalize(glm::vec3(elbowAxisX, 0.0f, -elbowAxisZ));
    a.leftElbowAxis = glm::normalize(glm::vec3(elbowAxisX, 0.0f, elbowAxisZ));
    a.rightElbow = -glm::radians(68.0f);
    a.leftElbow = -glm::radians(62.0f);
    a.bodyOffset = glm::vec3(bodyOffsetX * beat, bodyOffsetYBase + bodyOffsetYScale * std::fabs(beat), 0.0f);
    a.torsoAngle = glm::radians(6.0f * beat);

    return a;
}


static AnimAngles anim_mj_penching(float t)
{
    AnimAngles a;
    const float introDuration = 0.9f;
    const float s = easeInOut(clampUnit(t / introDuration));

    a.bodyPitch = glm::radians(45.0f) * s;
    a.rightArmAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    a.rightArm = -glm::radians(100.0f) * s;
    a.rightArmSide = glm::radians(35.0f) * s;
    a.rightElbowAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    a.rightElbow = -glm::radians(115.0f) * s;
    a.rightElbowSide = glm::radians(35.0f) * s;

    return a;
}

static AnimAngles anim_jumpstyle(float t)
{
    AnimAngles a;
    const float cycleDuration = 0.8f;
    const float frontBending = 20.0f;
    const float legCenter = -10.0f;
    const float legAmplitude = 35.0f;
    const float holdPhaseEnd = 0.10f;
    const float firstSwingEnd = 0.50f;
    const float secondHoldEnd = 0.60f;
    const float swingPhaseSpan = 0.40f;
    const float headSwingOscAmplitude = 20.0f;
    const float twoPi = 6.28318530718f;

    float p = std::fmod(t, cycleDuration) / cycleDuration;
    float swing;
    float headSwingOsc = 0.0f;
    if (p < holdPhaseEnd) {
        swing = 1.0f;
    } else if (p < firstSwingEnd) {
        float s = (p - holdPhaseEnd) / swingPhaseSpan;
        swing = 1.0f - 2.0f * s;
        headSwingOsc = std::sin(s * twoPi) * headSwingOscAmplitude;
    } else if (p < secondHoldEnd) {
        swing = -1.0f;
    } else {
        float s = (p - secondHoldEnd) / swingPhaseSpan;
        swing = -1.0f + 2.0f * s;
        headSwingOsc = std::sin(s * twoPi) * headSwingOscAmplitude;
    }

    const float torsoSwingMax = 10.0f;
    float torsoHoldWeight = std::fabs(swing);

    a.bodyPitch = glm::radians(frontBending);
    a.bodyYaw = glm::radians(55.0f * swing);
    a.headPitch = glm::radians(frontBending + headSwingOsc);
    a.torsoAngle = glm::radians(torsoSwingMax * swing * torsoHoldWeight);
    a.rightArmAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    a.leftArmAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    a.rightArm = glm::radians(-frontBending + 90.0f * swing);
    a.leftArm = glm::radians(-frontBending - 90.0f * swing);
    a.rightElbow = -glm::radians(30.0f);
    a.leftElbow = -glm::radians(30.0f);
    a.rightLegAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    a.leftLegAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    a.rightLeg = glm::radians(legCenter - legAmplitude * swing);
    a.leftLeg = glm::radians(legCenter + legAmplitude * swing);
    a.rightKnee = glm::radians(90.0f) * std::max(0.0f, -swing);
    a.leftKnee = glm::radians(90.0f) * std::max(0.0f, swing);

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
        case NARUTO_RUN:
            return anim_naruto_run(t);
        case EAGLE_FLIGHT:
            return anim_eagle_flight(t);
        case GANGNAM_STYLE:
            return anim_gangnam_style(t);
        case MJ_PENCHING:
            return anim_mj_penching(t);
        case HARDBASS_ROBLOX:
            return anim_jumpstyle(t);
        default:
            return AnimAngles();
    }
}


static void applyGlobalBodyRotation(glm::mat4& model, const glm::vec3& torsoBase, float bodyPitch, float bodyYaw)
{
    if (bodyPitch == 0.0f && bodyYaw == 0.0f)
        return;

    model = glm::translate(model, torsoBase);
    model = glm::rotate(model, bodyYaw, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, bodyPitch, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-torsoBase.x, -torsoBase.y, -torsoBase.z));
}


static void applyKneeRotation(glm::mat4& model, const glm::vec3& hip, float hipAngle, const glm::vec3& hipAxis, const glm::vec3& knee, float kneeAngle, const glm::vec3& partPos)
{
    const glm::vec3 axis(1.0f, 0.0f, 0.0f);
    model = glm::translate(model, hip);
    model = glm::rotate(model, hipAngle, hipAxis);
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


static void applyTorsoArmRotation(glm::mat4& model, const glm::vec3& torsoBase, float torsoAngle, const glm::vec3& shoulder, float armAngle, const glm::vec3& armAxis, const glm::vec3& partPos, float sideAngle = 0.0f)
{
    model = glm::translate(model, torsoBase);
    model = glm::rotate(model, torsoAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-torsoBase.x, -torsoBase.y, -torsoBase.z));

    model = glm::translate(model, shoulder);
    model = glm::rotate(model, armAngle, armAxis);
    if (sideAngle != 0.0f)
        model = glm::rotate(model, sideAngle, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-shoulder.x, -shoulder.y, -shoulder.z));

    model = glm::translate(model, partPos);
}


static void applyTorsoElbowRotation(glm::mat4& model, const glm::vec3& torsoBase, float torsoAngle, const glm::vec3& shoulder, float shoulderAngle, const glm::vec3& shoulderAxis, const glm::vec3& elbow, float elbowAngle, const glm::vec3& elbowAxis, const glm::vec3& partPos, float shoulderSideAngle = 0.0f, float elbowSideAngle = 0.0f)
{
    model = glm::translate(model, torsoBase);
    model = glm::rotate(model, torsoAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-torsoBase.x, -torsoBase.y, -torsoBase.z));

    model = glm::translate(model, shoulder);
    model = glm::rotate(model, shoulderAngle, shoulderAxis);
    if (shoulderSideAngle != 0.0f)
        model = glm::rotate(model, shoulderSideAngle, glm::vec3(0.0f, 0.0f, 1.0f));

    model = glm::translate(model, elbow - shoulder);
    model = glm::rotate(model, elbowAngle, elbowAxis);
    if (elbowSideAngle != 0.0f)
        model = glm::rotate(model, elbowSideAngle, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-elbow.x, -elbow.y, -elbow.z));

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
    const glm::vec3 xAxis(1.0f, 0.0f, 0.0f);
    const int cubeVertexCount = 36;
    const float skinR = 1.0f;
    const float skinG = 187.0f / 255.0f;
    const float skinB = 119.0f / 255.0f;
    const float torsoR = 0.0f;
    const float torsoG = 238.0f / 255.0f;
    const float torsoB = 221.0f / 255.0f;
    const float legR = 0.0f;
    const float legG = 136.0f / 255.0f;
    const float legB = 204.0f / 255.0f;
    const float capR = 0.0f;
    const float capG = 0.0f;
    const float capB = 0.0f;
    const float visorR = 1.0f;
    const float visorG = 0.0f;
    const float visorB = 0.0f;
    const float wireR = 255.0f;
    const float wireG = 0.0f;
    const float wireB = 0.0f;
    const float wireLineWidth = 2.0f;
    const float polygonOffsetFactor = -1.0f;
    const float polygonOffsetUnits = -1.0f;

    const AnimAngles a = getAnimAngles(_state, _time);
    const glm::vec3 torsoBase = getPivotPoint(myBody, TORSO, false) + a.bodyOffset;
    const glm::vec3 headBase = getPivotPoint(myBody, HEAD, false) + a.bodyOffset;

    const glm::vec3 shoulderOff = glm::vec3(0.0f, a.shoulderDrop, 0.0f);
    const glm::vec3 rightShoulder = getPivotPoint(myBody, RIGHT_UPPER_ARM, true) + a.bodyOffset + shoulderOff;
    const glm::vec3 leftShoulder = getPivotPoint(myBody, LEFT_UPPER_ARM, true) + a.bodyOffset + shoulderOff;
    const glm::vec3 rightElbow = getPivotPoint(myBody, RIGHT_UPPER_ARM, false) + a.bodyOffset + shoulderOff;
    const glm::vec3 leftElbow = getPivotPoint(myBody, LEFT_UPPER_ARM, false) + a.bodyOffset + shoulderOff;
    const glm::vec3 rightHip = getPivotPoint(myBody, RIGHT_THIGH, true) + a.bodyOffset;
    const glm::vec3 leftHip = getPivotPoint(myBody, LEFT_THIGH, true) + a.bodyOffset;
    const glm::vec3 rightKnee = getPivotPoint(myBody, RIGHT_THIGH, false) + a.bodyOffset;
    const glm::vec3 leftKnee = getPivotPoint(myBody, LEFT_THIGH, false) + a.bodyOffset;

    if (_state == NONE) {
        myBody.draw_head(ourShader);
        myBody.draw_body(ourShader);
        myBody.draw_arm(ourShader);
        myBody.draw_leg(ourShader);
        myBody.draw_cap(ourShader);
        return;
    }
    
    ourShader.setBool("useOverrideColor", true);

    // ---- CAP / VISIERE ----
    for (const auto& part : myBody.getParts()) {
        BodyPartType type = part.getPartType();
        if (type != CAP && type != VISIERE) continue;
        glm::vec3 pos(part.getX(), part.getY(), part.getZ());
        glm::vec3 partPos = pos + a.bodyOffset;
        glm::mat4 model(1.0f);
        applyGlobalBodyRotation(model, torsoBase, a.bodyPitch, a.bodyYaw);
        model = glm::translate(model, torsoBase);
        model = glm::rotate(model, a.torsoAngle, xAxis);
        model = glm::translate(model, glm::vec3(-torsoBase.x, -torsoBase.y, -torsoBase.z));
        model = glm::translate(model, headBase);
        model = glm::rotate(model, a.headPitch, xAxis);
        model = glm::translate(model, glm::vec3(-headBase.x, -headBase.y, -headBase.z));
        model = glm::translate(model, partPos);
        model = glm::scale(model, part.getScale());
        if (type == CAP)
            ourShader.setVec3("overrideColor", capR, capG, capB);
        else
            ourShader.setVec3("overrideColor", visorR, visorG, visorB);
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
    }

    // ---- HEAD ----
    ourShader.setVec3("overrideColor", skinR, skinG, skinB);
    for (const auto& part : myBody.getParts()) {
        if (part.getPartType() != HEAD) continue;
        glm::vec3 pos(part.getX(), part.getY(), part.getZ());
        glm::vec3 partPos = pos + a.bodyOffset;
        glm::mat4 model(1.0f);
        applyGlobalBodyRotation(model, torsoBase, a.bodyPitch, a.bodyYaw);
        model = glm::translate(model, torsoBase);
        model = glm::rotate(model, a.torsoAngle, xAxis);
        model = glm::translate(model, glm::vec3(-torsoBase.x, -torsoBase.y, -torsoBase.z));
        model = glm::translate(model, headBase);
        model = glm::rotate(model, a.headPitch, xAxis);
        model = glm::translate(model, glm::vec3(-headBase.x, -headBase.y, -headBase.z));
        model = glm::translate(model, partPos);
        model = glm::scale(model, part.getScale());
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
    }

    // ---- TORSO ----
    ourShader.setVec3("overrideColor", torsoR, torsoG, torsoB);
    for (const auto& part : myBody.getParts()) {
        if (part.getPartType() != TORSO) continue;
        glm::vec3 pos(part.getX(), part.getY(), part.getZ());
        glm::mat4 model(1.0f);
        applyGlobalBodyRotation(model, torsoBase, a.bodyPitch, a.bodyYaw);
        applyPivotRotation(model, torsoBase, a.torsoAngle, xAxis, pos + a.bodyOffset);
        model = glm::scale(model, part.getScale());
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
    }

    // ---- ARMS ----
    ourShader.setVec3("overrideColor", skinR, skinG, skinB);
    for (const auto& part : myBody.getParts()) {
        BodyPartType type = part.getPartType();
        if (type != RIGHT_UPPER_ARM && type != RIGHT_LOWER_ARM && type != LEFT_UPPER_ARM && type != LEFT_LOWER_ARM)
            continue;
        glm::vec3 pos(part.getX(), part.getY(), part.getZ());
        pos = pos + a.bodyOffset + shoulderOff;
        glm::mat4 model(1.0f);
        applyGlobalBodyRotation(model, torsoBase, a.bodyPitch, a.bodyYaw);
        bool isLeftArm = (part.getX() > 0.0f);
        bool isLower = (type == RIGHT_LOWER_ARM || type == LEFT_LOWER_ARM);
        if (isLeftArm) {
            if (isLower && a.leftElbow != 0.0f)
                applyTorsoElbowRotation(model, torsoBase, a.torsoAngle, leftShoulder, a.leftArm, a.leftArmAxis,
                                        leftElbow, a.leftElbow, a.leftElbowAxis, pos,
                                        a.leftArmSide, 0.0f);
            else
                applyTorsoArmRotation(model, torsoBase, a.torsoAngle, leftShoulder, a.leftArm, a.leftArmAxis, pos,
                                      a.leftArmSide);
        } else {
            if (isLower && a.rightElbow != 0.0f)
                applyTorsoElbowRotation(model, torsoBase, a.torsoAngle, rightShoulder, a.rightArm, a.rightArmAxis,
                                        rightElbow, a.rightElbow, a.rightElbowAxis, pos,
                                        a.rightArmSide, a.rightElbowSide);
            else
                applyTorsoArmRotation(model, torsoBase, a.torsoAngle, rightShoulder, a.rightArm, a.rightArmAxis, pos,
                                      a.rightArmSide);
        }
        model = glm::scale(model, part.getScale());
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
    }

    // ---- LEGS ----
    ourShader.setVec3("overrideColor", legR, legG, legB);
    for (const auto& part : myBody.getParts()) {
        BodyPartType type = part.getPartType();
        if (type != RIGHT_THIGH && type != RIGHT_LOWER_LEG && type != LEFT_THIGH && type != LEFT_LOWER_LEG)
            continue;
        glm::vec3 pos(part.getX(), part.getY(), part.getZ());
        glm::mat4 model(1.0f);
        applyGlobalBodyRotation(model, torsoBase, a.bodyPitch, a.bodyYaw);
        model = glm::translate(model, torsoBase);
        model = glm::rotate(model, a.torsoAngle, xAxis);
        model = glm::translate(model, glm::vec3(-torsoBase.x, -torsoBase.y, -torsoBase.z));
        bool isRight = (pos.x < 0.0f);
        if (type == RIGHT_THIGH || type == LEFT_THIGH) {
            if (isRight)
                applyPivotRotation(model, rightHip,  a.rightLeg,  a.rightLegAxis, pos + a.bodyOffset);
            else
                applyPivotRotation(model, leftHip, a.leftLeg, a.leftLegAxis, pos + a.bodyOffset);
        } else {
            if (isRight)
                applyKneeRotation(model, rightHip,  a.rightLeg, a.rightLegAxis, rightKnee,  a.rightKnee,  pos + a.bodyOffset);
            else
                applyKneeRotation(model, leftHip, a.leftLeg, a.leftLegAxis, leftKnee, a.leftKnee, pos + a.bodyOffset);
        }
        model = glm::scale(model, part.getScale());
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
    }


    ourShader.setVec3("overrideColor", wireR, wireG, wireB);
    glLineWidth(wireLineWidth);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonOffset(polygonOffsetFactor, polygonOffsetUnits);
    for (const auto& part : myBody.getParts())
    {
        BodyPartType type = part.getPartType();
        glm::mat4 model(1.0f);
        applyGlobalBodyRotation(model, torsoBase, a.bodyPitch, a.bodyYaw);
        if (type == HEAD || type == TORSO) {
            glm::vec3 pos(part.getX(), part.getY(), part.getZ());
            glm::vec3 partPos = pos + a.bodyOffset;
            model = glm::translate(model, torsoBase);
            model = glm::rotate(model, a.torsoAngle, xAxis);
            model = glm::translate(model, glm::vec3(-torsoBase.x, -torsoBase.y, -torsoBase.z));
            if (type == HEAD) {
                model = glm::translate(model, headBase);
                model = glm::rotate(model, a.headPitch, xAxis);
                model = glm::translate(model, glm::vec3(-headBase.x, -headBase.y, -headBase.z));
            }
            model = glm::translate(model, partPos);
        } else if (type == RIGHT_UPPER_ARM || type == RIGHT_LOWER_ARM ||
                   type == LEFT_UPPER_ARM || type == LEFT_LOWER_ARM) {
            glm::vec3 pos(part.getX(), part.getY(), part.getZ());
            pos = pos + a.bodyOffset + shoulderOff;
            bool isLeftArm = (part.getX() > 0.0f);
            bool isLower = (type == RIGHT_LOWER_ARM || type == LEFT_LOWER_ARM);
            if (isLeftArm) {
                if (isLower && a.leftElbow != 0.0f)
                    applyTorsoElbowRotation(model, torsoBase, a.torsoAngle, leftShoulder, a.leftArm, a.leftArmAxis,
                                            leftElbow, a.leftElbow, a.leftElbowAxis, pos,
                                            a.leftArmSide, 0.0f);
                else
                    applyTorsoArmRotation(model, torsoBase, a.torsoAngle, leftShoulder, a.leftArm, a.leftArmAxis, pos,
                                          a.leftArmSide);
            } else {
                if (isLower && a.rightElbow != 0.0f)
                    applyTorsoElbowRotation(model, torsoBase, a.torsoAngle, rightShoulder, a.rightArm, a.rightArmAxis,
                                            rightElbow, a.rightElbow, a.rightElbowAxis, pos,
                                            a.rightArmSide, a.rightElbowSide);
                else
                    applyTorsoArmRotation(model, torsoBase, a.torsoAngle, rightShoulder, a.rightArm, a.rightArmAxis, pos,
                                          a.rightArmSide);
            }
        } else if (type == RIGHT_THIGH || type == RIGHT_LOWER_LEG ||
                   type == LEFT_THIGH || type == LEFT_LOWER_LEG) {
            glm::vec3 pos(part.getX(), part.getY(), part.getZ());
            model = glm::translate(model, torsoBase);
            model = glm::rotate(model, a.torsoAngle, xAxis);
            model = glm::translate(model, glm::vec3(-torsoBase.x, -torsoBase.y, -torsoBase.z));
            bool isRight = (pos.x < 0.0f);
            if (type == RIGHT_THIGH || type == LEFT_THIGH) {
                if (isRight)
                    applyPivotRotation(model, rightHip,  a.rightLeg,  a.rightLegAxis, pos + a.bodyOffset);
                else
                    applyPivotRotation(model, leftHip, a.leftLeg, a.leftLegAxis, pos + a.bodyOffset);
            } else {
                if (isRight)
                    applyKneeRotation(model, rightHip,  a.rightLeg, a.rightLegAxis, rightKnee,  a.rightKnee,  pos + a.bodyOffset);
                else
                    applyKneeRotation(model, leftHip, a.leftLeg, a.leftLegAxis, leftKnee, a.leftKnee, pos + a.bodyOffset);
            }
        } else {
            continue;
        }
        model = glm::scale(model, part.getScale());
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
    }

    glDisable(GL_POLYGON_OFFSET_LINE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    ourShader.setBool("useOverrideColor", false);
}
